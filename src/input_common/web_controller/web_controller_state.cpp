#include "input_common/web_controller/web_controller_state.h"
#include "input_common/web_controller/controller_html.h"
#include "video_core/renderer_base.h"

#include <QBuffer>
#include <QImage>
#include <chrono>
#include <json.hpp>
#include <httplib.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#endif

#include <unordered_map>

namespace InputCommon::WebController {

// Maps the string names sent from the browser to NativeButton indices.
static const std::unordered_map<std::string, int>& ButtonNameMap() {
    namespace B = Settings::NativeButton;
    static const std::unordered_map<std::string, int> map{
        {"a", B::A},           {"b", B::B},         {"x", B::X},
        {"y", B::Y},           {"up", B::Up},        {"down", B::Down},
        {"left", B::Left},     {"right", B::Right},  {"l", B::L},
        {"r", B::R},           {"start", B::Start},  {"select", B::Select},
        {"debug", B::Debug},   {"gpio14", B::Gpio14},
    };
    return map;
}

static constexpr char MANIFEST_JSON[] = R"({
  "name": "Lime3DS Controller",
  "short_name": "3DS Ctrl",
  "display": "fullscreen",
  "orientation": "landscape",
  "background_color": "#111111",
  "theme_color": "#111111",
  "icons": []
})";

WebControllerState::WebControllerState(u16 port)
    : state_(std::make_shared<WebControllerButtonState>()),
      server_(std::make_unique<httplib::Server>()) {

    server_->set_default_headers({{"Access-Control-Allow-Origin", "*"}});

    // ── Static assets ──────────────────────────────────────────────────────
    server_->Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(CONTROLLER_HTML, "text/html; charset=utf-8");
    });

    server_->Get("/manifest.json", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(MANIFEST_JSON, "application/json");
    });

    // ── Single-JPEG bottom-screen snapshot (works on iOS Safari) ──────────
    // The browser JS polls this at ~20fps using a canvas + Image() loop.
    // Recording the poll time tells FrameRequestLoop a client is active.
    server_->Get("/screen", [this](const httplib::Request&, httplib::Response& res) {
        // Signal FrameRequestLoop that a client is actively polling.
        last_poll_ns_.store(
            std::chrono::steady_clock::now().time_since_epoch().count());

        if (!streaming_active_.load()) {
            res.status = 503;
            res.set_content("Stream not started — launch a game first", "text/plain");
            return;
        }
        auto frame = latest_frame_;
        if (!frame) {
            res.status = 503;
            return;
        }
        std::vector<uint8_t> data;
        {
            std::lock_guard lock(frame->mutex);
            data = frame->jpeg;
        }
        if (data.empty()) {
            res.status = 503;
            res.set_content("No frame yet", "text/plain");
            return;
        }
        res.set_header("Cache-Control", "no-cache, no-store");
        res.set_content(reinterpret_cast<const char*>(data.data()), data.size(), "image/jpeg");
    });

    // ── Input endpoint (buttons, analog, touch) ────────────────────────────
    auto state_weak = std::weak_ptr<WebControllerButtonState>(state_);
    server_->Post("/input", [state_weak](const httplib::Request& req, httplib::Response& res) {
        auto state = state_weak.lock();
        if (!state) {
            res.status = 503;
            return;
        }
        try {
            auto j = nlohmann::json::parse(req.body);

            std::lock_guard lock(state->mutex);
            if (j.contains("button")) {
                const std::string name = j["button"].get<std::string>();
                const bool pressed = j.value("pressed", false);
                const auto& map = ButtonNameMap();
                auto it = map.find(name);
                if (it != map.end() && it->second < static_cast<int>(state->buttons.size())) {
                    state->buttons[it->second] = pressed;
                }
            } else if (j.contains("analog")) {
                state->circle_pad_x =
                    std::clamp(j["analog"]["x"].get<float>(), -1.0f, 1.0f);
                state->circle_pad_y =
                    std::clamp(j["analog"]["y"].get<float>(), -1.0f, 1.0f);
            } else if (j.contains("touch")) {
                state->touch_x = std::clamp(j["touch"]["x"].get<float>(), 0.0f, 1.0f);
                state->touch_y = std::clamp(j["touch"]["y"].get<float>(), 0.0f, 1.0f);
                state->touch_pressed = j["touch"]["pressed"].get<bool>();
            }
            res.set_content("{\"ok\":true}", "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content("{\"error\":\"bad request\"}", "application/json");
        }
    });

    server_thread_ = std::thread([this, port]() { ServerThread(port); });
    server_->wait_until_ready();
    bound_port_.store(port);
}

WebControllerState::~WebControllerState() {
    StopStreaming();
    if (server_ && server_->is_running()) {
        server_->stop();
    }
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
}

void WebControllerState::ServerThread(u16 port) {
    server_->listen("0.0.0.0", port);
}

bool WebControllerState::IsRunning() const {
    return server_ && server_->is_running();
}

// ── Screen streaming ───────────────────────────────────────────────────────────

void WebControllerState::SetRenderer(VideoCore::RendererBase* renderer,
                                      const Layout::FramebufferLayout& layout) {
    StopStreaming(); // stop any existing stream/thread first
    renderer_ = renderer;
    stream_layout_ = layout;
    capture_buffer_.resize(static_cast<size_t>(layout.width) * layout.height * 4);
    latest_frame_ = std::make_shared<LatestFrame>();
    last_poll_ns_.store(0);
    streaming_active_.store(true);
    frame_request_thread_ = std::thread([this] { FrameRequestLoop(); });
}

void WebControllerState::StopStreaming() {
    streaming_active_.store(false);
    // Join the frame thread before nulling renderer_ so the thread can't race
    // a RequestNextFrame call against a dangling renderer pointer.
    if (frame_request_thread_.joinable()) {
        frame_request_thread_.join();
    }
    // Any in-flight callback will see streaming_active_=false and skip the JPEG encode,
    // then clear capture_in_progress_. Reset it here so the next SetRenderer() starts clean.
    capture_in_progress_.store(false);
    renderer_ = nullptr;
    if (latest_frame_) {
        latest_frame_->cv.notify_all();
        latest_frame_.reset();
    }
}

void WebControllerState::FrameRequestLoop() {
    // Fires RequestNextFrame at ~20fps, but only while a client is actively polling.
    // If no /screen request has arrived in the last 3 seconds, we stop arming
    // screenshots so the emu thread has zero capture overhead during idle periods.
    static constexpr long long kThreeSecNs = 3'000'000'000LL;
    static constexpr auto kInterval = std::chrono::milliseconds(50);

    while (streaming_active_.load()) {
        std::this_thread::sleep_for(kInterval);
        if (!streaming_active_.load()) break;

        const auto now_ns = std::chrono::steady_clock::now().time_since_epoch().count();
        const auto last_poll = last_poll_ns_.load();
        if (last_poll > 0 && now_ns - last_poll < kThreeSecNs) {
            RequestNextFrame();
        }
    }
}

void WebControllerState::RequestNextFrame() {
    if (!streaming_active_.load() || !renderer_) return;
    // Use capture_in_progress_ instead of IsScreenshotPending().
    // IsScreenshotPending() returns false as soon as the emu thread's RenderScreenshot()
    // exchanges the flag to false — but the callback hasn't been called yet at that point.
    // Arming a new screenshot there overwrites screenshot_complete_callback while the emu
    // thread is still about to read+call it → data race → SIGSEGV.
    // capture_in_progress_ stays true until the callback body finishes, closing the window.
    if (capture_in_progress_.load()) return;
    auto frame = latest_frame_;
    if (!frame) return;

    capture_in_progress_.store(true);
    renderer_->RequestScreenshot(
        capture_buffer_.data(),
        [this, frame](bool invert_y) {
            // The bool is invert_y (OpenGL=true, Vulkan=false), NOT a success flag.
            if (frame && streaming_active_.load()) {
                QImage img(capture_buffer_.data(), static_cast<int>(stream_layout_.width),
                           static_cast<int>(stream_layout_.height), QImage::Format_RGB32);
                if (invert_y) {
                    img = img.mirrored(false, true);
                }
                QByteArray arr;
                QBuffer buf(&arr);
                buf.open(QIODevice::WriteOnly);
                img.save(&buf, "JPEG", 65);
                buf.close();

                {
                    std::lock_guard lock(frame->mutex);
                    frame->jpeg.assign(arr.begin(), arr.end());
                    frame->fresh = true;
                }
                frame->cv.notify_all();
            }
            // Release the guard — FrameRequestLoop may now arm the next screenshot.
            capture_in_progress_.store(false);
        },
        stream_layout_);
}

// ── Network helpers ────────────────────────────────────────────────────────────

std::string WebControllerState::GetLocalIPAddress() const {
#ifdef _WIN32
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        return "127.0.0.1";
    }
    addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET;
    if (getaddrinfo(hostname, nullptr, &hints, &res) != 0) {
        return "127.0.0.1";
    }
    char ip[INET_ADDRSTRLEN];
    auto* addr = reinterpret_cast<sockaddr_in*>(res->ai_addr);
    inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
    freeaddrinfo(res);
    return ip;
#else
    ifaddrs* addrs = nullptr;
    if (getifaddrs(&addrs) != 0) {
        return "127.0.0.1";
    }
    std::string result = "127.0.0.1";
    for (ifaddrs* ifa = addrs; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) {
            continue;
        }
        const auto* addr = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
        if (addr->sin_addr.s_addr == htonl(INADDR_LOOPBACK)) {
            continue;
        }
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr->sin_addr, buf, sizeof(buf));
        result = buf;
        break;
    }
    freeifaddrs(addrs);
    return result;
#endif
}

} // namespace InputCommon::WebController

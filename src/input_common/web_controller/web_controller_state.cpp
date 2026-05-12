#include "input_common/web_controller/web_controller_state.h"
#include "input_common/web_controller/controller_html.h"

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
        {"a", B::A},           {"b", B::B},       {"x", B::X},
        {"y", B::Y},           {"up", B::Up},     {"down", B::Down},
        {"left", B::Left},     {"right", B::Right}, {"l", B::L},
        {"r", B::R},           {"start", B::Start}, {"select", B::Select},
        {"debug", B::Debug},   {"gpio14", B::Gpio14},
    };
    return map;
}

WebControllerState::WebControllerState(u16 port)
    : state_(std::make_shared<WebControllerButtonState>()),
      server_(std::make_unique<httplib::Server>()) {

    server_->set_default_headers({{"Access-Control-Allow-Origin", "*"}});

    // Serve the controller UI
    server_->Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(CONTROLLER_HTML, "text/html; charset=utf-8");
    });

    // Receive button / analog events from the browser
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
                state->circle_pad_x = std::clamp(j["analog"]["x"].get<float>(), -1.0f, 1.0f);
                state->circle_pad_y = std::clamp(j["analog"]["y"].get<float>(), -1.0f, 1.0f);
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

#pragma once

#include <array>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "common/settings.h"
#include "core/frontend/framebuffer_layout.h"

// Forward-declared to avoid including the heavy httplib header here.
namespace httplib {
class Server;
}

namespace VideoCore {
class RendererBase;
}

namespace InputCommon::WebController {

constexpr u16 DEFAULT_PORT = 8080;

struct WebControllerButtonState {
    mutable std::mutex mutex;
    std::array<bool, Settings::NativeButton::NumButtons> buttons{};
    float circle_pad_x{0.0f};
    float circle_pad_y{0.0f};
    // Touch screen (bottom screen) — normalized 0.0–1.0
    float touch_x{0.5f};
    float touch_y{0.5f};
    bool touch_pressed{false};
};

class WebControllerState {
public:
    explicit WebControllerState(u16 port);
    ~WebControllerState();

    WebControllerState(const WebControllerState&) = delete;
    WebControllerState& operator=(const WebControllerState&) = delete;

    std::shared_ptr<WebControllerButtonState> GetSharedState() const {
        return state_;
    }

    std::string GetLocalIPAddress() const;
    u16 GetPort() const {
        return bound_port_.load();
    }
    bool IsRunning() const;

    // Screen streaming — call after the renderer is ready (BootGame).
    void SetRenderer(VideoCore::RendererBase* renderer,
                     const Layout::FramebufferLayout& layout);

    // Stop the capture chain before ShutdownGame destroys the renderer.
    void StopStreaming();

private:
    void ServerThread(u16 port);
    void RequestNextFrame();
    void EncodeThread();
    void FrameRequestLoop();

    std::shared_ptr<WebControllerButtonState> state_;
    std::unique_ptr<httplib::Server> server_;
    std::thread server_thread_;
    std::atomic<u16> bound_port_{0};

    // Streaming state
    struct LatestFrame {
        std::mutex mutex;
        std::vector<uint8_t> jpeg;
    };
    std::shared_ptr<LatestFrame> latest_frame_;
    std::atomic<bool> streaming_active_{false};
    VideoCore::RendererBase* renderer_{nullptr};
    Layout::FramebufferLayout stream_layout_{};

    // capture_buffer_: written by emu thread callback (raw BGRA pixels from GPU readback)
    std::vector<uint8_t> capture_buffer_;

    // encode_buffer_: copied from capture_buffer_ inside the callback (fast memcpy),
    // then consumed by EncodeThread to produce JPEG — keeps JPEG work off the emu thread.
    std::mutex encode_mutex_;
    std::condition_variable encode_cv_;
    std::vector<uint8_t> encode_buffer_;
    bool encode_invert_y_{false};
    bool encode_pending_{false};
    bool encode_stop_{false};
    std::thread encode_thread_;

    // Frame request thread: fires RequestNextFrame at ~10fps only when a client is polling.
    // last_poll_ns_ is updated by the /screen handler; if it's stale (>3s), we stop arming.
    std::thread frame_request_thread_;
    std::atomic<long long> last_poll_ns_{0};

    // Guards against the data race between FrameRequestLoop arming a new screenshot and
    // the emu thread still executing a previous callback. Stays true from RequestScreenshot()
    // until the callback body finishes (after signalling the encode thread).
    std::atomic<bool> capture_in_progress_{false};
};

} // namespace InputCommon::WebController

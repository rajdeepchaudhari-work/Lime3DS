#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "common/settings.h"

// Forward-declared to avoid including the heavy httplib header here.
namespace httplib {
class Server;
}

namespace InputCommon::WebController {

constexpr u16 DEFAULT_PORT = 8080;

struct WebControllerButtonState {
    mutable std::mutex mutex;
    std::array<bool, Settings::NativeButton::NumButtons> buttons{};
    float circle_pad_x{0.0f};
    float circle_pad_y{0.0f};
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

private:
    void ServerThread(u16 port);

    std::shared_ptr<WebControllerButtonState> state_;
    std::unique_ptr<httplib::Server> server_;
    std::thread server_thread_;
    std::atomic<u16> bound_port_{0};
};

} // namespace InputCommon::WebController

#pragma once

#include <memory>
#include "core/frontend/input.h"
#include "input_common/web_controller/web_controller_state.h"

namespace Common {
class ParamPackage;
}

namespace InputCommon::WebController {

class WebButtonFactory final : public Input::Factory<Input::ButtonDevice> {
public:
    explicit WebButtonFactory(std::shared_ptr<WebControllerButtonState> state);
    std::unique_ptr<Input::ButtonDevice> Create(const Common::ParamPackage& params) override;

private:
    std::shared_ptr<WebControllerButtonState> state_;
};

class WebAnalogFactory final : public Input::Factory<Input::AnalogDevice> {
public:
    explicit WebAnalogFactory(std::shared_ptr<WebControllerButtonState> state);
    std::unique_ptr<Input::AnalogDevice> Create(const Common::ParamPackage& params) override;

private:
    std::shared_ptr<WebControllerButtonState> state_;
};

class WebTouchFactory final : public Input::Factory<Input::TouchDevice> {
public:
    explicit WebTouchFactory(std::shared_ptr<WebControllerButtonState> state);
    std::unique_ptr<Input::TouchDevice> Create(const Common::ParamPackage& params) override;

private:
    std::shared_ptr<WebControllerButtonState> state_;
};

// Start the HTTP server and register all input factories (buttons, analog, touch).
std::unique_ptr<WebControllerState> Init(u16 port);

// Unregister all factories. Call before the WebControllerState is destroyed.
void Shutdown();

} // namespace InputCommon::WebController

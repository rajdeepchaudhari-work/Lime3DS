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

// Constructs a WebControllerState (starts the server) and registers both factories.
// Returns the state object; caller must keep it alive while the server should run.
std::unique_ptr<WebControllerState> Init(u16 port);

} // namespace InputCommon::WebController

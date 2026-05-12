#include "input_common/web_controller/web_controller.h"

#include <mutex>
#include <tuple>
#include "common/param_package.h"
#include "core/frontend/input.h"

namespace InputCommon::WebController {

// ── Button device ──────────────────────────────────────────────────────────────

class WebButtonDevice final : public Input::ButtonDevice {
public:
    WebButtonDevice(std::shared_ptr<WebControllerButtonState> state, int button_index)
        : state_(std::move(state)), button_index_(button_index) {}

    bool GetStatus() const override {
        std::lock_guard lock(state_->mutex);
        if (button_index_ < 0 || button_index_ >= static_cast<int>(state_->buttons.size())) {
            return false;
        }
        return state_->buttons[button_index_];
    }

private:
    std::shared_ptr<WebControllerButtonState> state_;
    int button_index_;
};

// ── Analog device ──────────────────────────────────────────────────────────────

class WebAnalogDevice final : public Input::AnalogDevice {
public:
    explicit WebAnalogDevice(std::shared_ptr<WebControllerButtonState> state)
        : state_(std::move(state)) {}

    std::tuple<float, float> GetStatus() const override {
        std::lock_guard lock(state_->mutex);
        return {state_->circle_pad_x, state_->circle_pad_y};
    }

private:
    std::shared_ptr<WebControllerButtonState> state_;
};

// ── Touch device ───────────────────────────────────────────────────────────────

class WebTouchDevice final : public Input::TouchDevice {
public:
    explicit WebTouchDevice(std::shared_ptr<WebControllerButtonState> state)
        : state_(std::move(state)) {}

    std::tuple<float, float, bool> GetStatus() const override {
        std::lock_guard lock(state_->mutex);
        return {state_->touch_x, state_->touch_y, state_->touch_pressed};
    }

private:
    std::shared_ptr<WebControllerButtonState> state_;
};

// ── Factories ──────────────────────────────────────────────────────────────────

WebButtonFactory::WebButtonFactory(std::shared_ptr<WebControllerButtonState> state)
    : state_(std::move(state)) {}

std::unique_ptr<Input::ButtonDevice> WebButtonFactory::Create(const Common::ParamPackage& params) {
    const int index = params.Get("button_index", 0);
    return std::make_unique<WebButtonDevice>(state_, index);
}

WebAnalogFactory::WebAnalogFactory(std::shared_ptr<WebControllerButtonState> state)
    : state_(std::move(state)) {}

std::unique_ptr<Input::AnalogDevice> WebAnalogFactory::Create(const Common::ParamPackage&) {
    return std::make_unique<WebAnalogDevice>(state_);
}

WebTouchFactory::WebTouchFactory(std::shared_ptr<WebControllerButtonState> state)
    : state_(std::move(state)) {}

std::unique_ptr<Input::TouchDevice> WebTouchFactory::Create(const Common::ParamPackage&) {
    return std::make_unique<WebTouchDevice>(state_);
}

// ── Init / Shutdown ────────────────────────────────────────────────────────────

std::unique_ptr<WebControllerState> Init(u16 port) {
    auto state_obj = std::make_unique<WebControllerState>(port);
    auto shared = state_obj->GetSharedState();

    Input::RegisterFactory<Input::ButtonDevice>("web_controller",
                                                std::make_shared<WebButtonFactory>(shared));
    Input::RegisterFactory<Input::AnalogDevice>("web_controller",
                                                std::make_shared<WebAnalogFactory>(shared));
    Input::RegisterFactory<Input::TouchDevice>("web_touch",
                                               std::make_shared<WebTouchFactory>(shared));
    return state_obj;
}

void Shutdown() {
    Input::UnregisterFactory<Input::ButtonDevice>("web_controller");
    Input::UnregisterFactory<Input::AnalogDevice>("web_controller");
    Input::UnregisterFactory<Input::TouchDevice>("web_touch");
}

} // namespace InputCommon::WebController

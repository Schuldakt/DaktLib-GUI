// Input implementation stubs
#include "dakt/gui/input/Input.hpp"
#include <algorithm>

namespace dakt::gui {

InputSystem::InputSystem() = default;
InputSystem::~InputSystem() = default;

void InputSystem::processMouse(const MouseInput& input) {
    mouseDelta_ = input.position - mousePos_;
    mousePos_ = input.position;
    mouseWheel_ = input.wheelDelta;

    // Track button state changes
    for (int i = 0; i < 5; i++) {
        mouseButtonsPrev_[i] = mouseButtonsCurrent_[i];
        mouseButtonsCurrent_[i] = input.buttons[i];
    }
}

void InputSystem::processKeyboard(const KeyboardInput& input) {
    // Copy previous state
    std::copy(std::begin(keysCurrent_), std::end(keysCurrent_), std::begin(keysPrev_));

    // Update current state
    for (int i = 0; i < 256; i++) {
        keysCurrent_[i] = input.keyPressed[i];
    }

    textInput_ = input.textInput;
    shift_ = input.shift;
    ctrl_ = input.ctrl;
    alt_ = input.alt;
}

bool InputSystem::isMouseButtonDown(MouseButton btn) const {
    int idx = static_cast<int>(btn);
    if (idx >= 0 && idx < 5)
        return mouseButtonsCurrent_[idx];
    return false;
}

bool InputSystem::isMouseButtonPressed(MouseButton btn) const {
    int idx = static_cast<int>(btn);
    if (idx >= 0 && idx < 5) {
        return mouseButtonsCurrent_[idx] && !mouseButtonsPrev_[idx];
    }
    return false;
}

bool InputSystem::isMouseButtonReleased(MouseButton btn) const {
    int idx = static_cast<int>(btn);
    if (idx >= 0 && idx < 5) {
        return !mouseButtonsCurrent_[idx] && mouseButtonsPrev_[idx];
    }
    return false;
}

bool InputSystem::isKeyDown(Key key) const {
    int idx = static_cast<int>(key);
    if (idx >= 0 && idx < 256)
        return keysCurrent_[idx];
    return false;
}

bool InputSystem::isKeyPressed(Key key) const {
    int idx = static_cast<int>(key);
    if (idx >= 0 && idx < 256) {
        return keysCurrent_[idx] && !keysPrev_[idx];
    }
    return false;
}

bool InputSystem::isKeyReleased(Key key) const {
    int idx = static_cast<int>(key);
    if (idx >= 0 && idx < 256) {
        return !keysCurrent_[idx] && keysPrev_[idx];
    }
    return false;
}

void InputSystem::resetFrameState() {
    mouseDelta_ = Vec2(0, 0);
    mouseWheel_ = 0.0f;
    textInput_.clear();
}

} // namespace dakt::gui

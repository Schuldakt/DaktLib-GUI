#include "dakt/gui/core/Frame.hpp"

namespace dakt::gui {

Frame::Frame() = default;
Frame::~Frame() = default;

void Frame::reset() {
    // Reset frame state
}

void Frame::updateInput(const MouseInput& mouse, const KeyboardInput& keyboard) {
    mouseInput_ = mouse;
    keyboardInput_ = keyboard;
}

} // namespace dakt::gui

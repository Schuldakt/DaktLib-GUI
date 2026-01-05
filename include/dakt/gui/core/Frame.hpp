#ifndef DAKT_GUI_FRAME_HPP
#define DAKT_GUI_FRAME_HPP

#include "../input/Input.hpp"
#include "Types.hpp"


namespace dakt::gui {

class Frame {
  public:
    Frame();
    ~Frame();

    void reset();
    void updateInput(const MouseInput& mouse, const KeyboardInput& keyboard);

    const MouseInput& getMouseInput() const { return mouseInput_; }
    const KeyboardInput& getKeyboardInput() const { return keyboardInput_; }

  private:
    MouseInput mouseInput_;
    KeyboardInput keyboardInput_;
};

} // namespace dakt::gui

#endif // DAKT_GUI_FRAME_HPP

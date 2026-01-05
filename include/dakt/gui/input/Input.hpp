#ifndef DAKT_GUI_INPUT_HPP
#define DAKT_GUI_INPUT_HPP

#include "../core/Types.hpp"
#include <cstdint>

namespace dakt::gui {

class InputSystem {
  public:
    InputSystem();
    ~InputSystem();

    // Input processing
    void processMouse(const MouseInput& input);
    void processKeyboard(const KeyboardInput& input);

    // State queries
    bool isMouseButtonDown(MouseButton btn) const;
    bool isMouseButtonPressed(MouseButton btn) const;
    bool isMouseButtonReleased(MouseButton btn) const;

    bool isKeyDown(Key key) const;
    bool isKeyPressed(Key key) const;
    bool isKeyReleased(Key key) const;

    Vec2 getMousePosition() const { return mousePos_; }
    Vec2 getMouseDelta() const { return mouseDelta_; }
    float getMouseWheel() const { return mouseWheel_; }

    const std::string& getTextInput() const { return textInput_; }

    // Focus management
    uint64_t getFocusedWidget() const { return focusedWidget_; }
    void setFocusedWidget(uint64_t id) { focusedWidget_ = id; }

    uint64_t getHoveredWidget() const { return hoveredWidget_; }
    void setHoveredWidget(uint64_t id) { hoveredWidget_ = id; }

    // Reset per-frame state
    void resetFrameState();

  private:
    Vec2 mousePos_;
    Vec2 mouseDelta_;
    float mouseWheel_ = 0.0f;

    bool mouseButtonsCurrent_[5] = {};
    bool mouseButtonsPrev_[5] = {};

    bool keysCurrent_[256] = {};
    bool keysPrev_[256] = {};

    std::string textInput_;

    uint64_t focusedWidget_ = 0;
    uint64_t hoveredWidget_ = 0;

    bool shift_ = false;
    bool ctrl_ = false;
    bool alt_ = false;
};

} // namespace dakt::gui

#endif

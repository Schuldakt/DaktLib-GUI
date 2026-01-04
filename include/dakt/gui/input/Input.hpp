#pragma once

#include "../core/Types.hpp"

namespace dakt::gui {

struct InputState {
  Vec2 mousePos{};
  bool mouseButtons[5]{};
  bool keys[256]{};
};

class InputSystem {
public:
  InputSystem() = default;
  const InputState &state() const { return state_; }
  void setState(const InputState &state) { state_ = state; }

private:
  InputState state_{};
};

} // namespace dakt::gui

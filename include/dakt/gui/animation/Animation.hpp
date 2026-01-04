#pragma once

#include "../core/Types.hpp"

namespace dakt::gui {

struct Easing {
  enum class Type { Linear, QuadInOut, CubicInOut };

  Type type{Type::Linear};
};

class Animator {
public:
  Animator() = default;
  void setEasing(Easing easing) { easing_ = easing; }
  Easing easing() const { return easing_; }

private:
  Easing easing_{};
};

} // namespace dakt::gui

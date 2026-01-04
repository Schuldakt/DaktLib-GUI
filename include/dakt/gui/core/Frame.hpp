#pragma once

#include "Types.hpp"

namespace dakt::gui {

struct FrameTiming {
  float deltaTime{0.0F};
};

class Frame {
public:
  Frame() = default;
  explicit Frame(FrameTiming timing) : timing_(timing) {}
  const FrameTiming &timing() const { return timing_; }
  void setTiming(const FrameTiming &timing) { timing_ = timing; }

private:
  FrameTiming timing_{};
};

} // namespace dakt::gui

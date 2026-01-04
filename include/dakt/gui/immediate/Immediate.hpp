#pragma once

#include "../core/Types.hpp"

namespace dakt::gui {

class Context;

class Immediate {
public:
  Immediate() = default;
  explicit Immediate(Context *ctx) : ctx_(ctx) {}

  void setContext(Context *ctx) { ctx_ = ctx; }
  Context *context() const { return ctx_; }

private:
  Context *ctx_{nullptr};
};

} // namespace dakt::gui

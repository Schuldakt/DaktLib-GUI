#pragma once

#include "../core/Types.hpp"

namespace dakt::gui {

class Context;

class Widget {
public:
  Widget() = default;
  virtual ~Widget() = default;

  virtual void build(Context &) {}
};

} // namespace dakt::gui

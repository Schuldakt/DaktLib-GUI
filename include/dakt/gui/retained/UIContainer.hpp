#pragma once

#include "../core/Types.hpp"

namespace dakt::gui {

class Context;
class Widget;

class UIContainer {
public:
  UIContainer() = default;
  virtual ~UIContainer() = default;

  virtual void build(Context &) {}
};

} // namespace dakt::gui

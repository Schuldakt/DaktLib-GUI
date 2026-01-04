#pragma once

#include "../core/Types.hpp"

namespace dakt::gui {

struct Theme {
  Color background{};
  Color foreground{};
  float spacing{4.0F};
  float radius{4.0F};
};

class Style {
public:
  Style() = default;
  explicit Style(Theme theme) : theme_(theme) {}
  const Theme &theme() const { return theme_; }
  void setTheme(const Theme &theme) { theme_ = theme; }

private:
  Theme theme_{};
};

} // namespace dakt::gui

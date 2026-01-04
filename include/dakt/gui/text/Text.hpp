#pragma once

#include "../core/Types.hpp"
#include <string>

namespace dakt::gui {

class FontAtlas {
public:
  FontAtlas() = default;
};

class Font {
public:
  Font() = default;
};

class TextShaper {
public:
  TextShaper() = default;
  void setFallbackEnabled(bool enabled) { fallbackEnabled_ = enabled; }
  bool fallbackEnabled() const { return fallbackEnabled_; }

private:
  bool fallbackEnabled_{true};
};

} // namespace dakt::gui

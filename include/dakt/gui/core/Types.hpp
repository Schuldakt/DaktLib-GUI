#pragma once

#if defined(_WIN32)
#if defined(DAKT_GUI_BUILD_SHARED)
#define DAKT_GUI_API __declspec(dllexport)
#elif defined(DAKT_GUI_USE_SHARED)
#define DAKT_GUI_API __declspec(dllimport)
#else
#define DAKT_GUI_API
#endif
#else
#define DAKT_GUI_API
#endif

namespace dakt::gui {

struct Vec2 {
  float x{0.0F};
  float y{0.0F};
};

struct Rect {
  float x{0.0F};
  float y{0.0F};
  float width{0.0F};
  float height{0.0F};
};

struct Color {
  float r{1.0F};
  float g{1.0F};
  float b{1.0F};
  float a{1.0F};
};

} // namespace dakt::gui

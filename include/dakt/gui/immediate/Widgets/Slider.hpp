#ifndef DAKTLIB_GUI_IMMEDIATECONTEXT_HPP
#define DAKTLIB_GUI_IMMEDIATECONTEXT_HPP

#include "../../core/Types.hpp"

namespace dakt::gui {

// ============================================================================
// Sliders & Drags
// ============================================================================

bool sliderInt(const char* label, int* value, int min, int max, const char* format = "%d");
bool sliderFloat(const char* label, float* value, float min, float max, const char* format = "%.3f");
bool sliderVec2(const char* label, Vec2* value, float min, float max, const char* format = "%.3f");

bool dragInt(const char* label, int* value, float speed = 1.0f, int min = 0, int max = 0);
bool dragFloat(const char* label, float* value, float speed = 1.0f, float min = 0.0f, float max = 0.0f, const char* format = "%.3f");

} // namespace dakt::gui

#endif
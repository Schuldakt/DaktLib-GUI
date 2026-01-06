#pragma once

#include "dakt/gui/core/Types.hpp"

namespace dakt::gui {

    DAKTLIB_GUI_API bool sliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d");
    DAKTLIB_GUI_API bool sliderFloat(const char* label, float* v, float v_min, float v_max, const char* formate = "%.3f");
    DAKTLIB_GUI_API bool sliderVec2(const char* label, Vec2* v, float v_min, float v_max, const char* format = "%.3f");

    DAKTLIB_GUI_API bool dragInt(const char* label, int* value, float speed = 1.0f, int min = 0, int max = 0);
    DAKTLIB_GUI_API bool dragFloat(const char* label, float* value, float speed = 1.0f, float min = 0.0f, float max = 0.0f, const char* format = "%.3f");

} // namespace dakt::gui
#pragma once

#include "dakt/gui/core/Types.hpp"

namespace dakt::gui {

    DAKTLIB_GUI_API bool button(const char* label, Vec2 size = Vec2(0, 0));
    DAKTLIB_GUI_API bool smallButton(const char* label);
    DAKTLIB_GUI_API bool invisibleButton(const char* id, Vec2 size);

    DAKTLIB_GUI_API bool colorButton(const char* id, Color color, Vec2 size = Vec2(0, 0));

} // namespace dakt::gui
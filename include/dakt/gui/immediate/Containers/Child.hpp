#pragma once

#include "dakt/gui/core/Types.hpp"

namespace dakt::gui {

    DAKTLIB_GUI_API bool beginChild(const char* id, Vec2 size = Vec2(0, 0), bool border = false);
    DAKTLIB_GUI_API void endChild();

} // namespace dakt::gui
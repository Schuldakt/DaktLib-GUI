#pragma once

#include "dakt/gui/core/Types.hpp"

namespace dakt::gui {

    DAKTLIB_GUI_API void beginTooltip();
    DAKTLIB_GUI_API void endTooltip();
    DAKTLIB_GUI_API void setTooltip(const char* fmt, ...);

} // namespace dakt::gui
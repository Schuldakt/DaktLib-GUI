#pragma once

#include "dakt/gui/core/Types.hpp"

namespace dakt::gui {

    DAKTLIB_GUI_API void text(const char* fmt, ...);
    DAKTLIB_GUI_API void textColored(Color color, const char* fmt, ...);
    DAKTLIB_GUI_API void textDisabled(const char* fmt, ...);
    DAKTLIB_GUI_API void textWrapped(const char* fmt, ...);

    // Label + value layout helper
    DAKTLIB_GUI_API void labelText(const char* label, const char* fmt, ...);

} // namespace dakt::gui
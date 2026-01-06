#pragma once

#include "dakt/gui/core/Types.hpp"

namespace dakt::gui {

    DAKTLIB_GUI_API bool beginMainMenuBar();
    DAKTLIB_GUI_API void endMainMenuBar();
    DAKTLIB_GUI_API bool beginMenuBar();
    DAKTLIB_GUI_API void endMenuBar();
    DAKTLIB_GUI_API bool beginMenu(const char* label, bool enabled = true);
    DAKTLIB_GUI_API void endMenu();
    DAKTLIB_GUI_API bool menuItem(const char* label, const char* shortcut = nullptr, bool selected = false, bool enabled = true);
    DAKTLIB_GUI_API bool menuItem(const char* label, const char* shortcut, bool* selected, bool enabled = true);

} // namespace dakt::gui
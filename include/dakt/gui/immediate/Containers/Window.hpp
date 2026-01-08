#pragma once

#include "dakt/gui/core/Types.hpp"

namespace dakt::gui {

    DAKTLIB_GUI_API bool beginWindow(const char* title, bool* open = nullptr, WindowFlags flags = WindowFlags::None);
    DAKTLIB_GUI_API void endWindow();

    // Next-window settings
    DAKTLIB_GUI_API void setNextWindowPos(Vec2 pos);
    DAKTLIB_GUI_API void setNextWindowSize(Vec2 size);
    DAKTLIB_GUI_API void setNextWindowCollapsed(bool collapsed);

    // Window metrics
    DAKTLIB_GUI_API Vec2 getWindowPos();
    DAKTLIB_GUI_API Vec2 getWindowSize();
    DAKTLIB_GUI_API Vec2 getContentRegionAvail();

    // Cursor
    DAKTLIB_GUI_API Vec2 getCursorPos();
    DAKTLIB_GUI_API void setCursorPos(Vec2 pos);

} // namespace dakt::gui
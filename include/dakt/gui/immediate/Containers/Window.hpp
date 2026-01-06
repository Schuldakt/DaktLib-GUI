#pragma once

#include <cstdint>
#include "dakt/gui/core/Types.hpp"

namespace dakt::gui {

    enum class WindowFlags : std::uint32_t {
        None        = 0,
        NoTitleBar  = 1 << 0,
        NoResize    = 1 << 1,
        NoMove      = 1 << 2,
        NoScrollBar = 1 << 3,
        NoCollapse  = 1 << 4
    };

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
    DAKTLIB_GUI_API Vec2 GetCursorPos();
    DAKTLIB_GUI_API void setCursorPos(Vec2 pos);

} // namespace dakt::gui
#pragma once

#include "dakt/gui/core/Types.hpp"

namespace dakt::gui {

    class Context;
    class DrawList;

    // ID stack / hashing
    DAKTLIB_GUI_API void pushID(const char* strId);
    DAKTLIB_GUI_API void pushID(ID id);
    DAKTLIB_GUI_API void popID();

    DAKTLIB_GUI_API ID getID(const char* strId);
    DAKTLIB_GUI_API ID getID(const void* ptr);

    // Mouse Utilities
    DAKTLIB_GUI_API Vec2 getMousePos();
    DAKTLIB_GUI_API bool isMouseDown(MouseButton button);
    DAKTLIB_GUI_API bool isMouseClicked(MouseButton button);
    DAKTLIB_GUI_API bool isMouseReleased(MouseButton button);

    // Item query utilities (read from ImmediateState last-item fields)
    DAKTLIB_GUI_API bool isItemHovered();
    DAKTLIB_GUI_API bool isItemActive();
    DAKTLIB_GUI_API bool isItemClicked();

    DAKTLIB_GUI_API Vec2 getItemRectMin();
    DAKTLIB_GUI_API Vec2 getItemRectMax();
    DAKTLIB_GUI_API Vec2 getItemRectSize();

    // Drawlist Access
    DAKTLIB_GUI_API DrawList* getWindowDrawList();

} // namespace dakt::gui
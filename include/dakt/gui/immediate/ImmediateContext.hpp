#pragma once

#include "dakt/gui/core/Types.hpp"
#include <cstdint>

namespace dakt::gui {
    class Context;
    class DrawList;
    class Widget; // retained base widget (forward)

    // Frame lifecycle
    DAKTLIB_GUI_API void beginFrame(Context& ctx, float deltaTime);
    DAKTLIB_GUI_API void endFrame(Context& ctx);

    DAKTLIB_GUI_API std::uint64_t getFrameIndex();

    // ID Stack
    DAKTLIB_GUI_API void pushID(const char* strId);
    DAKTLIB_GUI_API void pushID(ID id);
    DAKTLIB_GUI_API void popID();

    DAKTLIB_GUI_API ID getID(const char* strId);
    DAKTLIB_GUI_API ID getID(ID id);

    // Item queries (results from last submitted widget)
    DAKTLIB_GUI_API bool isItemHovered();
    DAKTLIB_GUI_API bool isItemActive();
    DAKTLIB_GUI_API bool isItemFocused();
    DAKTLIB_GUI_API bool isItemClicked(MouseButton button = MouseButton::Left);
    DAKTLIB_GUI_API bool isItemEdited();
    DAKTLIB_GUI_API bool isItemActivated();
    DAKTLIB_GUI_API bool isItemDeactivated();
    DAKTLIB_GUI_API Vec2 getItemRectMin();
    DAKTLIB_GUI_API Vec2 getItemRectMax();
    DAKTLIB_GUI_API Vec2 getItemRectSize();
    DAKTLIB_GUI_API void setItemDefaultFocus();
    DAKTLIB_GUI_API void setKeyboardFocusHere(int offset = 0);

    // Mouse utilities
    DAKTLIB_GUI_API bool isMouseHoveringRect(Vec2 min, Vec2 max);
    DAKTLIB_GUI_API bool isMouseClicked(MouseButton button = MouseButton::Left);
    DAKTLIB_GUI_API bool isMouseDoubleClicked(MouseButton button = MouseButton::Left);
    DAKTLIB_GUI_API bool isMouseDown(MouseButton button = MouseButton::Left);
    DAKTLIB_GUI_API bool isMouseReleased(MouseButton button = MouseButton::Left);
    DAKTLIB_GUI_API Vec2 getMousePos();
    DAKTLIB_GUI_API Vec2 getMouseDragDelta(MouseButton button = MouseButton::Left);

    // Draw
    DAKTLIB_GUI_API DrawList* getWindowDrawList();
    DAKTLIB_GUI_API DrawList* getForegroundDrawList();
    DAKTLIB_GUI_API DrawList* getBackgroundDrawList();

    // Retained embedding hook
    DAKTLIB_GUI_API void embedRetained(const char* id, Widget* widget, Vec2 size = Vec2(0, 0));

} // namespace dakt::gui
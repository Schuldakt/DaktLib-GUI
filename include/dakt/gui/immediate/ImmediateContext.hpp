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

    // Draw
    DAKTLIB_GUI_API DrawList* getWindowDrawList();

    // Retained embedding hook
    DAKTLIB_GUI_API void embedRetained(const char* id, Widget* widget, Vec2 size = Vec2(0, 0));

} // namespace dakt::gui
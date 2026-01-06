#pragma once

#include "dakt/gui/core/Types.hpp"

#include <cstdint>

namespace dakt::gui {

    enum class SelectableFlags : uint32_t {
        None                = 0,
        DontClosePopups     = 1 << 0,
        SpanAllColumns      = 1 << 1,
        AllowDoubleClick    = 1 << 2,
        Disabled            = 1 << 3 
    };

    inline SelectableFlags operator|(SelectableFlags a, SelectableFlags b) {
        return static_cast<SelectableFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    DAKTLIB_GUI_API bool selectable(const char* label, bool selected = false, Vec2 size = Vec2(0, 0));
    DAKTLIB_GUI_API bool selectable(const char* label, bool* selected, Vec2 size = Vec2(0, 0));

} // namespace dakt::gui
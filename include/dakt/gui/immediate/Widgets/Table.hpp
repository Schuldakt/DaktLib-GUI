#pragma once

#include "dakt/gui/core/Types.hpp"

#include <cstdint>

namespace dakt::gui {

    inline TableFlags operator|(TableFlags a, TableFlags b) {
        return static_cast<TableFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    DAKTLIB_GUI_API bool beginTable(const char* strId, int columns, TableFlags flags = TableFlags::None, Vec2 outerSize = Vec2(0, 0));
    DAKTLIB_GUI_API void endTable();
    DAKTLIB_GUI_API void tableNextRow();
    DAKTLIB_GUI_API bool tableNextColumn();
    DAKTLIB_GUI_API bool tableSetColumnIndex(int columnIndex);
    DAKTLIB_GUI_API void tableSetupColumn(const char* label, float initWidth = 0.0f);
    DAKTLIB_GUI_API void tableHeadersRow();

} // namespace dakt::gui
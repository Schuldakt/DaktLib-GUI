#ifndef DAKTLIB_GUI_IMMEDIATECONTEXT_HPP
#define DAKTLIB_GUI_IMMEDIATECONTEXT_HPP

#include "../../core/Types.hpp"

namespace dakt::gui {

// ============================================================================
// Tables
// ============================================================================

enum class TableFlags : uint32_t {
    None = 0,
    Resizable = 1 << 0,
    Reorderable = 1 << 1,
    Hideable = 1 << 2,
    Sortable = 1 << 3,
    RowBg = 1 << 4,
    Borders = 1 << 5,
    ScrollX = 1 << 6,
    ScrollY = 1 << 7,
};

inline TableFlags operator|(TableFlags a, TableFlags b) { return static_cast<TableFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }

bool beginTable(const char* strId, int columns, TableFlags flags = TableFlags::None, Vec2 outerSize = Vec2(0, 0));
void endTable();
void tableNextRow();
bool tableNextColumn();
bool tableSetColumnIndex(int columnIndex);
void tableSetupColumn(const char* label, float initWidth = 0.0f);
void tableHeadersRow();
enum class TableFlags : uint32_t {
    None = 0,
    Resizable = 1 << 0,
    Reorderable = 1 << 1,
    Hideable = 1 << 2,
    Sortable = 1 << 3,
    RowBg = 1 << 4,
    Borders = 1 << 5,
    ScrollX = 1 << 6,
    ScrollY = 1 << 7,
};

inline TableFlags operator|(TableFlags a, TableFlags b) { return static_cast<TableFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }

bool beginTable(const char* strId, int columns, TableFlags flags = TableFlags::None, Vec2 outerSize = Vec2(0, 0));
void endTable();
void tableNextRow();
bool tableNextColumn();
bool tableSetColumnIndex(int columnIndex);
void tableSetupColumn(const char* label, float initWidth = 0.0f);
void tableHeadersRow();

} // namespace dakt::gui

#endif
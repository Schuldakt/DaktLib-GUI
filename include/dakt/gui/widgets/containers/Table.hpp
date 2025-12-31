// ============================================================================
// DaktLib GUI Module - Table Container
// ============================================================================

#pragma once

#include <dakt/core/Types.hpp>
#include <dakt/gui/core/Types.hpp>

namespace dakt::gui
{

using namespace dakt::core;

// ============================================================================
// Table Flags
// ============================================================================

enum class TableFlags : u32
{
    None = 0,
    Resizable = 1 << 0,
    Reorderable = 1 << 1,
    Hideable = 1 << 2,
    Sortable = 1 << 3,
    NoSavedSettings = 1 << 4,
    ContextMenuInBody = 1 << 5,
    RowBg = 1 << 6,
    BordersInnerH = 1 << 7,
    BordersOuterH = 1 << 8,
    BordersInnerV = 1 << 9,
    BordersOuterV = 1 << 10,
    BordersH = BordersInnerH | BordersOuterH,
    BordersV = BordersInnerV | BordersOuterV,
    BordersInner = BordersInnerV | BordersInnerH,
    BordersOuter = BordersOuterV | BordersOuterH,
    Borders = BordersInner | BordersOuter,
    NoBordersInBody = 1 << 11,
    NoBordersInBodyUntilResize = 1 << 12,
    SizingFixedFit = 1 << 13,
    SizingFixedSame = 1 << 14,
    SizingStretchProp = 1 << 15,
    SizingStretchSame = 1 << 16,
    NoHostExtendX = 1 << 17,
    NoHostExtendY = 1 << 18,
    NoKeepColumnsVisible = 1 << 19,
    PreciseWidths = 1 << 20,
    NoClip = 1 << 21,
    PadOuterX = 1 << 22,
    NoPadOuterX = 1 << 23,
    NoPadInnerX = 1 << 24,
    ScrollX = 1 << 25,
    ScrollY = 1 << 26,
    SortMulti = 1 << 27,
    SortTristate = 1 << 28,
};

inline TableFlags operator|(TableFlags a, TableFlags b)
{
    return static_cast<TableFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

enum class TableColumnFlags : u32
{
    None = 0,
    Disabled = 1 << 0,
    DefaultHide = 1 << 1,
    DefaultSort = 1 << 2,
    WidthStretch = 1 << 3,
    WidthFixed = 1 << 4,
    NoResize = 1 << 5,
    NoReorder = 1 << 6,
    NoHide = 1 << 7,
    NoClip = 1 << 8,
    NoSort = 1 << 9,
    NoSortAscending = 1 << 10,
    NoSortDescending = 1 << 11,
    NoHeaderLabel = 1 << 12,
    NoHeaderWidth = 1 << 13,
    PreferSortAscending = 1 << 14,
    PreferSortDescending = 1 << 15,
    IndentEnable = 1 << 16,
    IndentDisable = 1 << 17,
    IsEnabled = 1 << 24,
    IsVisible = 1 << 25,
    IsSorted = 1 << 26,
    IsHovered = 1 << 27,
};

inline TableColumnFlags operator|(TableColumnFlags a, TableColumnFlags b)
{
    return static_cast<TableColumnFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

enum class TableRowFlags : u32
{
    None = 0,
    Headers = 1 << 0,
};

inline TableRowFlags operator|(TableRowFlags a, TableRowFlags b)
{
    return static_cast<TableRowFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

// ============================================================================
// Table Functions
// ============================================================================

// Table functions
bool beginTable(StringView strId, i32 columns, TableFlags flags = TableFlags::None, Vec2 outerSize = {0, 0},
                f32 innerWidth = 0.0f);
void endTable();

// Table setup (call after beginTable, before first row)
void tableSetupColumn(StringView label, TableColumnFlags flags = TableColumnFlags::None, f32 initWidthOrWeight = 0.0f,
                      u32 userId = 0);
void tableSetupScrollFreeze(i32 cols, i32 rows);
void tableHeadersRow();

// Table content
void tableNextRow(TableRowFlags flags = TableRowFlags::None, f32 minRowHeight = 0.0f);
bool tableNextColumn();
bool tableSetColumnIndex(i32 columnIndex);

// Table info
i32 tableGetColumnCount();
i32 tableGetColumnIndex();
i32 tableGetRowIndex();
StringView tableGetColumnName(i32 columnIndex = -1);
TableColumnFlags tableGetColumnFlags(i32 columnIndex = -1);
void tableSetColumnEnabled(i32 columnIndex, bool enabled);
void tableSetBgColor(i32 target, Color color, i32 columnIndex = -1);

}  // namespace dakt::gui

// ============================================================================
// DaktLib GUI Module - Grid Container
// ============================================================================

#pragma once

#include <dakt/core/Types.hpp>
#include <dakt/gui/core/Types.hpp>

namespace dakt::gui
{

using namespace dakt::core;

// ============================================================================
// Grid Layout
// ============================================================================

struct GridConfig
{
    i32 columns = 1;
    f32 columnWidth = -1.0f;  // -1 = auto
    f32 rowHeight = -1.0f;    // -1 = auto
    Vec2 cellPadding = {4.0f, 4.0f};
    bool drawBorders = false;
    bool stretchColumns = true;
};

// Begin a grid layout
bool beginGrid(StringView id, const GridConfig& config);
void endGrid();

// Move to next cell in grid
void nextCell();

// Set column width
void setColumnWidth(i32 column, f32 width);

}  // namespace dakt::gui

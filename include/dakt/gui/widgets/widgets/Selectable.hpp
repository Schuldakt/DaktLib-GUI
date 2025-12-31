// ============================================================================
// DaktLib GUI Module - Selectable Widget
// ============================================================================

#pragma once

#include <dakt/gui/framework/Context.hpp>
#include <dakt/gui/framework/Theme.hpp>

namespace dakt::gui
{

// ============================================================================
// Selectable
// ============================================================================

enum class SelectableFlags : u32
{
    None = 0,
    DontClosePopups = 1 << 0,
    SpanAllColumns = 1 << 1,
    AllowDoubleClick = 1 << 2,
    Disabled = 1 << 3,
    AllowItemOverlap = 1 << 4
};

inline SelectableFlags operator|(SelectableFlags a, SelectableFlags b)
{
    return static_cast<SelectableFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

bool selectable(StringView label, bool selected = false, SelectableFlags flags = SelectableFlags::None,
                Vec2 size = {0, 0});
bool selectable(StringView label, bool* selected, SelectableFlags flags = SelectableFlags::None, Vec2 size = {0, 0});

}  // namespace dakt::gui

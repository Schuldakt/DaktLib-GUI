#pragma once

#include "dakt/gui/core/Types.hpp"

namespace dakt::gui {
struct Vec2;
struct Rect;

using Id = std::uint32_t;

// Updates and returns the per-item state for an item with the given bounding rect.
// This is intentionally generic: all widgets should call this rather than duplicating logic.
//
// Implementation belongs in a .cpp (or inline if you insist), but header declares it.
//
// Typical logic:
// - hovered = mouse in rect
// - pressed = hovered && mousePressed
// - active while mouseDown after pressed
// - clicked on release while hovered and active
struct ItemState;

ItemState updateItemState(ID id, const Rect& bb);
} // namespace dakt::gui
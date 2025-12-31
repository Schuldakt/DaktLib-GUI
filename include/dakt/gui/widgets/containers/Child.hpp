// ============================================================================
// DaktLib GUI Module - Child Windows
// ============================================================================

#pragma once

#include <dakt/gui/widgets/containers/Window.hpp>

namespace dakt::gui
{

// ============================================================================
// Child Windows
// ============================================================================

// Begin a child region (scrollable subarea)
bool beginChild(StringView id, Vec2 size = {0, 0}, bool border = false, WindowFlags flags = WindowFlags::None);
void endChild();

}  // namespace dakt::gui

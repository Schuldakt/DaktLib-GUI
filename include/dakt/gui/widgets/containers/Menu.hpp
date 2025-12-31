// ============================================================================
// DaktLib GUI Module - Menu Container
// ============================================================================

#pragma once

#include <dakt/core/Types.hpp>
#include <dakt/gui/core/Types.hpp>

namespace dakt::gui
{

using namespace dakt::core;

// ============================================================================
// Menu Functions
// ============================================================================

// Main menu bar (at top of screen/window)
bool beginMainMenuBar();
void endMainMenuBar();

// Menu bar (in a window)
bool beginMenuBar();
void endMenuBar();

// Menu dropdown
bool beginMenu(StringView label, bool enabled = true);
void endMenu();

// Menu items
bool menuItem(StringView label, StringView shortcut = "", bool selected = false, bool enabled = true);
bool menuItem(StringView label, StringView shortcut, bool* selected, bool enabled = true);

}  // namespace dakt::gui

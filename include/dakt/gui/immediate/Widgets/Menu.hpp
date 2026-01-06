#ifndef DAKTLIB_GUI_IMMEDIATECONTEXT_HPP
#define DAKTLIB_GUI_IMMEDIATECONTEXT_HPP

namespace dakt::gui {

// ============================================================================
// Menus
// ============================================================================

bool beginMainMenuBar();
void endMainMenuBar();
bool beginMenuBar();
void endMenuBar();
bool beginMenu(const char* label, bool enabled = true);
void endMenu();
bool menuItem(const char* label, const char* shortcut = nullptr, bool selected = false, bool enabled = true);
bool menuItem(const char* label, const char* shortcut, bool* selected, bool enabled = true);

} // namespace dakt::gui

#endif
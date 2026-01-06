#ifndef DAKTLIB_GUI_IMMEDIATECONTEXT_HPP
#define DAKTLIB_GUI_IMMEDIATECONTEXT_HPP

namespace dakt::gui {

// ============================================================================
// Radio Button
// ============================================================================

bool radioButton(const char* label, bool active);
bool radioButton(const char* label, int* value, int buttonValue);

} // namespace dakt::gui

#endif
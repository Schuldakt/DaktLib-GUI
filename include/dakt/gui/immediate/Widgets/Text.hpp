#ifndef DAKTLIB_GUI_IMMEDIATECONTEXT_HPP
#define DAKTLIB_GUI_IMMEDIATECONTEXT_HPP

#include "../../core/Types.hpp"

namespace dakt::gui {

// ============================================================================
// Text
// ============================================================================

void text(const char* fmt, ...);
void textColored(Color color, const char* fmt, ...);
void textDisabled(const char* fmt, ...);
void textWrapped(const char* fmt, ...);
void labelText(const char* label, const char* fmt, ...);

} // namespace dakt::gui

#endif
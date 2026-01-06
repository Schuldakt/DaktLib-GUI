#ifndef DAKTLIB_GUI_IMMEDIATECONTEXT_HPP
#define DAKTLIB_GUI_IMMEDIATECONTEXT_HPP

#include "../../core/Types.hpp"

namespace dakt::gui {

// ============================================================================
// Button
// ============================================================================

bool button(const char* label, Vec2 size = Vec2(0, 0));
bool smallButton(const char* label);
bool invisibleButton(const char* strId, Vec2 size);
bool colorButton(const char* descId, Color color, Vec2 size = Vec2(0, 0));

} // namespace dakt::gui

#endif
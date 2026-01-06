#ifndef DAKTLIB_GUI_IMMEDIATECONTEXT_HPP
#define DAKTLIB_GUI_IMMEDIATECONTEXT_HPP

#include "../../core/Types.hpp"

namespace dakt::gui {

// ============================================================================
// Progress
// ============================================================================

void progressBar(float fraction, Vec2 size = Vec2(-1, 0), const char* overlay = nullptr);

} // namespace dakt::gui

#endif
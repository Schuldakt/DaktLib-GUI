// ============================================================================
// DaktLib GUI Module - Progress Bar Widget
// ============================================================================

#pragma once

#include <dakt/gui/framework/Context.hpp>
#include <dakt/gui/framework/Theme.hpp>

namespace dakt::gui
{

// ============================================================================
// Progress Bar
// ============================================================================

void progressBar(f32 fraction, Vec2 size = {-1, 0}, StringView overlay = "");

}  // namespace dakt::gui

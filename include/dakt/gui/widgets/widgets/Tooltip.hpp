// ============================================================================
// DaktLib GUI Module - Tooltip Widget
// ============================================================================

#pragma once

#include <dakt/gui/framework/Context.hpp>
#include <dakt/gui/framework/Theme.hpp>

namespace dakt::gui
{

// ============================================================================
// Tooltip
// ============================================================================

void setTooltip(StringView text);
void beginTooltip();
void endTooltip();

}  // namespace dakt::gui

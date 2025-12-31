// ============================================================================
// DaktLib GUI Module - Text Widgets
// ============================================================================

#pragma once

#include <dakt/gui/framework/Context.hpp>
#include <dakt/gui/framework/Theme.hpp>

namespace dakt::gui
{

// ============================================================================
// Text Widgets
// ============================================================================

void text(StringView str);
void textColored(Color color, StringView str);
void textDisabled(StringView str);
void textWrapped(StringView str);
void bulletText(StringView str);
void labelText(StringView label, StringView str);

}  // namespace dakt::gui

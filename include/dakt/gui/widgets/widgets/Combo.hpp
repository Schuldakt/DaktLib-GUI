// ============================================================================
// DaktLib GUI Module - Combo & List Box Widgets
// ============================================================================

#pragma once

#include <dakt/gui/framework/Context.hpp>
#include <dakt/gui/framework/Theme.hpp>

#include <span>

namespace dakt::gui
{

// ============================================================================
// Combo & List Boxes
// ============================================================================

bool beginCombo(StringView label, StringView previewValue);
void endCombo();
bool combo(StringView label, i32* currentItem, std::span<const StringView> items);
bool combo(StringView label, i32* currentItem, StringView itemsSeparatedByZeros);

bool beginListBox(StringView label, Vec2 size = {0, 0});
void endListBox();
bool listBox(StringView label, i32* currentItem, std::span<const StringView> items, i32 heightInItems = -1);

}  // namespace dakt::gui

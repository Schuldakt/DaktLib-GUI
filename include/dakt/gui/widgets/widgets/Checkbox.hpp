// ============================================================================
// DaktLib GUI Module - Checkbox & Radio Widgets
// ============================================================================

#pragma once

#include <dakt/gui/framework/Context.hpp>
#include <dakt/gui/framework/Theme.hpp>

namespace dakt::gui
{

// ============================================================================
// Checkbox & Radio
// ============================================================================

bool checkbox(StringView label, bool* value);
bool checkboxFlags(StringView label, u32* flags, u32 flagsValue);
bool radioButton(StringView label, bool active);
bool radioButton(StringView label, i32* value, i32 buttonValue);

}  // namespace dakt::gui

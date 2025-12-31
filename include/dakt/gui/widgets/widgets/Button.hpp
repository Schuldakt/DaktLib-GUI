// ============================================================================
// DaktLib GUI Module - Button Widgets
// ============================================================================

#pragma once

#include <dakt/gui/framework/Context.hpp>
#include <dakt/gui/framework/Theme.hpp>

namespace dakt::gui
{

// ============================================================================
// Button Widgets
// ============================================================================

bool button(StringView label, Vec2 size = {0, 0});
bool smallButton(StringView label);
bool invisibleButton(StringView strId, Vec2 size);
bool arrowButton(StringView strId, i32 dir);  // 0=left, 1=right, 2=up, 3=down
bool imageButton(uintptr_t textureId, Vec2 size, Vec2 uv0 = {0, 0}, Vec2 uv1 = {1, 1},
                 Color bgColor = Color::transparent(), Color tintColor = Color::white());

}  // namespace dakt::gui

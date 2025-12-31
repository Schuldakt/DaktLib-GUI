// ============================================================================
// DaktLib GUI Module - Image Widget
// ============================================================================

#pragma once

#include <dakt/gui/framework/Context.hpp>
#include <dakt/gui/framework/Theme.hpp>

namespace dakt::gui
{

// ============================================================================
// Image
// ============================================================================

void image(uintptr_t textureId, Vec2 size, Vec2 uv0 = {0, 0}, Vec2 uv1 = {1, 1}, Color tintColor = Color::white(),
           Color borderColor = Color::transparent());

}  // namespace dakt::gui

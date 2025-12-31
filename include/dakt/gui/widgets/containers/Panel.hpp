// ============================================================================
// DaktLib GUI Module - Panel Container
// ============================================================================

#pragma once

#include <dakt/core/Types.hpp>
#include <dakt/gui/core/Types.hpp>

namespace dakt::gui
{

using namespace dakt::core;

// ============================================================================
// Panel Style
// ============================================================================

struct PanelStyle
{
    Color background = Color::fromFloat(0.15f, 0.15f, 0.15f, 1.0f);
    Color border = Color::fromFloat(0.4f, 0.4f, 0.4f, 1.0f);
    f32 borderWidth = 1.0f;
    f32 rounding = 4.0f;
    Vec2 padding = {8.0f, 8.0f};
    bool drawBorder = true;
    bool drawBackground = true;
};

// ============================================================================
// Panel Functions
// ============================================================================

// Begin a simple panel (styled container without window functionality)
bool beginPanel(StringView id, Vec2 size = {0, 0}, const PanelStyle& style = {});
void endPanel();

}  // namespace dakt::gui

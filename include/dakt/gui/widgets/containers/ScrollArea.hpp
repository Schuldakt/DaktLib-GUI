// ============================================================================
// DaktLib GUI Module - Scroll Area Container
// ============================================================================

#pragma once

#include <dakt/core/Types.hpp>
#include <dakt/gui/core/Types.hpp>

namespace dakt::gui
{

using namespace dakt::core;

// ============================================================================
// Scroll Area State
// ============================================================================

struct ScrollState
{
    Vec2 scroll = {0.0f, 0.0f};
    Vec2 scrollMax = {0.0f, 0.0f};
    Vec2 contentSize = {0.0f, 0.0f};
    bool scrollingX = false;
    bool scrollingY = false;
    bool showScrollbarX = false;
    bool showScrollbarY = false;
};

// ============================================================================
// Scroll Area Functions
// ============================================================================

// Begin a scroll area with automatic scrollbars
bool beginScrollArea(StringView id, Vec2 size, bool horizontalScroll = false);
void endScrollArea();

// Get/Set scroll position
Vec2 getScrollPos();
void setScrollPos(Vec2 pos);
void setScrollHereX(f32 centerRatio = 0.5f);
void setScrollHereY(f32 centerRatio = 0.5f);
void setScrollFromPosX(f32 localX, f32 centerRatio = 0.5f);
void setScrollFromPosY(f32 localY, f32 centerRatio = 0.5f);
f32 getScrollX();
f32 getScrollY();
f32 getScrollMaxX();
f32 getScrollMaxY();

}  // namespace dakt::gui

// ============================================================================
// DaktLib GUI Module - Slider & Drag Widgets
// ============================================================================

#pragma once

#include <dakt/gui/framework/Context.hpp>
#include <dakt/gui/framework/Theme.hpp>

namespace dakt::gui
{

// ============================================================================
// Sliders & Drags
// ============================================================================

bool sliderFloat(StringView label, f32* value, f32 min, f32 max, StringView format = "%.3f");
bool sliderFloat2(StringView label, f32 value[2], f32 min, f32 max, StringView format = "%.3f");
bool sliderFloat3(StringView label, f32 value[3], f32 min, f32 max, StringView format = "%.3f");
bool sliderFloat4(StringView label, f32 value[4], f32 min, f32 max, StringView format = "%.3f");
bool sliderInt(StringView label, i32* value, i32 min, i32 max, StringView format = "%d");
bool sliderInt2(StringView label, i32 value[2], i32 min, i32 max, StringView format = "%d");
bool sliderInt3(StringView label, i32 value[3], i32 min, i32 max, StringView format = "%d");
bool sliderInt4(StringView label, i32 value[4], i32 min, i32 max, StringView format = "%d");
bool sliderAngle(StringView label, f32* radians, f32 minDegrees = -360.0f, f32 maxDegrees = 360.0f);
bool vSliderFloat(StringView label, Vec2 size, f32* value, f32 min, f32 max, StringView format = "%.3f");
bool vSliderInt(StringView label, Vec2 size, i32* value, i32 min, i32 max, StringView format = "%d");

bool dragFloat(StringView label, f32* value, f32 speed = 1.0f, f32 min = 0.0f, f32 max = 0.0f,
               StringView format = "%.3f");
bool dragFloat2(StringView label, f32 value[2], f32 speed = 1.0f, f32 min = 0.0f, f32 max = 0.0f,
                StringView format = "%.3f");
bool dragFloat3(StringView label, f32 value[3], f32 speed = 1.0f, f32 min = 0.0f, f32 max = 0.0f,
                StringView format = "%.3f");
bool dragFloat4(StringView label, f32 value[4], f32 speed = 1.0f, f32 min = 0.0f, f32 max = 0.0f,
                StringView format = "%.3f");
bool dragInt(StringView label, i32* value, f32 speed = 1.0f, i32 min = 0, i32 max = 0, StringView format = "%d");
bool dragInt2(StringView label, i32 value[2], f32 speed = 1.0f, i32 min = 0, i32 max = 0, StringView format = "%d");
bool dragInt3(StringView label, i32 value[3], f32 speed = 1.0f, i32 min = 0, i32 max = 0, StringView format = "%d");
bool dragInt4(StringView label, i32 value[4], f32 speed = 1.0f, i32 min = 0, i32 max = 0, StringView format = "%d");

}  // namespace dakt::gui

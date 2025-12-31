// ============================================================================
// DaktLib GUI Module - Color Editor Widgets
// ============================================================================

#pragma once

#include <dakt/gui/framework/Context.hpp>
#include <dakt/gui/framework/Theme.hpp>

namespace dakt::gui
{

// ============================================================================
// Color Editors
// ============================================================================

enum class ColorEditFlags : u32
{
    None = 0,
    NoAlpha = 1 << 1,
    NoPicker = 1 << 2,
    NoOptions = 1 << 3,
    NoSmallPreview = 1 << 4,
    NoInputs = 1 << 5,
    NoTooltip = 1 << 6,
    NoLabel = 1 << 7,
    NoSidePreview = 1 << 8,
    NoDragDrop = 1 << 9,
    NoBorder = 1 << 10,
    AlphaBar = 1 << 16,
    AlphaPreview = 1 << 17,
    AlphaPreviewHalf = 1 << 18,
    HDR = 1 << 19,
    DisplayRGB = 1 << 20,
    DisplayHSV = 1 << 21,
    DisplayHex = 1 << 22,
    Uint8 = 1 << 23,
    Float = 1 << 24,
    PickerHueBar = 1 << 25,
    PickerHueWheel = 1 << 26,
    InputRGB = 1 << 27,
    InputHSV = 1 << 28
};

inline ColorEditFlags operator|(ColorEditFlags a, ColorEditFlags b)
{
    return static_cast<ColorEditFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

bool colorEdit3(StringView label, f32 col[3], ColorEditFlags flags = ColorEditFlags::None);
bool colorEdit4(StringView label, f32 col[4], ColorEditFlags flags = ColorEditFlags::None);
bool colorPicker3(StringView label, f32 col[3], ColorEditFlags flags = ColorEditFlags::None);
bool colorPicker4(StringView label, f32 col[4], ColorEditFlags flags = ColorEditFlags::None);
bool colorButton(StringView descId, Color col, ColorEditFlags flags = ColorEditFlags::None, Vec2 size = {0, 0});

}  // namespace dakt::gui

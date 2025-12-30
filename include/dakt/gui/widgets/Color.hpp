// ============================================================================
// DaktLib GUI Module - Color Widgets
// ============================================================================
// Color editing widgets: colorEdit3/4, colorPicker3/4, colorButton
// Provides HSV/RGB color selection with optional alpha channel.
// ============================================================================

#pragma once

#include <dakt/gui/core/Types.hpp>

namespace dakt::gui
{

// ============================================================================
// Color Edit Flags
// ============================================================================

enum class ColorEditFlags : u32
{
    None = 0,

    // Display options
    NoAlpha = 1 << 1,         // Ignore alpha component
    NoPicker = 1 << 2,        // Disable picker popup when clicking on color square
    NoOptions = 1 << 3,       // Disable right-click context menu
    NoSmallPreview = 1 << 4,  // Disable color square preview next to input
    NoInputs = 1 << 5,        // Disable inputs (RGB/HSV sliders or hex)
    NoTooltip = 1 << 6,       // Disable tooltip when hovering preview
    NoLabel = 1 << 7,         // Disable label (after the inputs)
    NoSidePreview = 1 << 8,   // Disable side preview in picker
    NoDragDrop = 1 << 9,      // Disable drag & drop target
    NoBorder = 1 << 10,       // Disable border around preview

    // Alpha handling
    AlphaBar = 1 << 16,          // Show vertical alpha bar in picker
    AlphaPreview = 1 << 17,      // Display preview as checkered for transparency
    AlphaPreviewHalf = 1 << 18,  // Display half checkered preview

    // Input format
    HDR = 1 << 19,         // Allow HDR values (values > 1.0)
    DisplayRGB = 1 << 20,  // Override display format to RGB
    DisplayHSV = 1 << 21,  // Override display format to HSV
    DisplayHex = 1 << 22,  // Override display format to Hex

    // Data format
    Uint8 = 1 << 23,  // Display values formatted as 0..255
    Float = 1 << 24,  // Display values formatted as 0.0..1.0

    // Picker style
    PickerHueBar = 1 << 25,    // Use bar for hue selection
    PickerHueWheel = 1 << 26,  // Use wheel for hue selection

    // Input type
    InputRGB = 1 << 27,  // Input in RGB format
    InputHSV = 1 << 28,  // Input in HSV format

    // Convenience combinations
    DefaultOptions = Uint8 | DisplayRGB | InputRGB | PickerHueBar
};

inline ColorEditFlags operator|(ColorEditFlags a, ColorEditFlags b)
{
    return static_cast<ColorEditFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

inline ColorEditFlags operator&(ColorEditFlags a, ColorEditFlags b)
{
    return static_cast<ColorEditFlags>(static_cast<u32>(a) & static_cast<u32>(b));
}

inline bool hasFlag(ColorEditFlags flags, ColorEditFlags flag)
{
    return (static_cast<u32>(flags) & static_cast<u32>(flag)) != 0;
}

// ============================================================================
// Color Space Conversion Utilities
// ============================================================================

/// Convert RGB to HSV
/// @param r Red component (0.0 - 1.0)
/// @param g Green component (0.0 - 1.0)
/// @param b Blue component (0.0 - 1.0)
/// @param h Output hue (0.0 - 1.0)
/// @param s Output saturation (0.0 - 1.0)
/// @param v Output value (0.0 - 1.0)
void colorConvertRGBtoHSV(f32 r, f32 g, f32 b, f32& h, f32& s, f32& v);

/// Convert HSV to RGB
/// @param h Hue (0.0 - 1.0)
/// @param s Saturation (0.0 - 1.0)
/// @param v Value (0.0 - 1.0)
/// @param r Output red component (0.0 - 1.0)
/// @param g Output green component (0.0 - 1.0)
/// @param b Output blue component (0.0 - 1.0)
void colorConvertHSVtoRGB(f32 h, f32 s, f32 v, f32& r, f32& g, f32& b);

/// Convert RGB float array to packed u32 color (ABGR format)
/// @param in RGB or RGBA float array
/// @return Packed color value
u32 colorConvertFloat4ToU32(const f32 in[4]);

/// Convert packed u32 color to RGB float array
/// @param in Packed color value (ABGR format)
/// @param out Output float array [R, G, B, A]
void colorConvertU32ToFloat4(u32 in, f32 out[4]);

/// Convert HSL to RGB
void colorConvertHSLtoRGB(f32 h, f32 s, f32 l, f32& r, f32& g, f32& b);

/// Convert RGB to HSL
void colorConvertRGBtoHSL(f32 r, f32 g, f32 b, f32& h, f32& s, f32& l);

// ============================================================================
// Color Editor Widgets
// ============================================================================

/// Edit a color with RGB components
/// @param label Widget label
/// @param col RGB color array [0.0 - 1.0]
/// @param flags ColorEditFlags options
/// @return true if the color was modified
bool colorEdit3(StringView label, f32 col[3], ColorEditFlags flags = ColorEditFlags::None);

/// Edit a color with RGBA components
/// @param label Widget label
/// @param col RGBA color array [0.0 - 1.0]
/// @param flags ColorEditFlags options
/// @return true if the color was modified
bool colorEdit4(StringView label, f32 col[4], ColorEditFlags flags = ColorEditFlags::None);

/// Full color picker with RGB components
/// @param label Widget label
/// @param col RGB color array [0.0 - 1.0]
/// @param flags ColorEditFlags options
/// @return true if the color was modified
bool colorPicker3(StringView label, f32 col[3], ColorEditFlags flags = ColorEditFlags::None);

/// Full color picker with RGBA components
/// @param label Widget label
/// @param col RGBA color array [0.0 - 1.0]
/// @param flags ColorEditFlags options
/// @return true if the color was modified
bool colorPicker4(StringView label, f32 col[4], ColorEditFlags flags = ColorEditFlags::None);

/// Display a color button that can open a color picker
/// @param descId Unique string ID for the button
/// @param col Color to display
/// @param flags ColorEditFlags options
/// @param size Button size (0,0 for auto)
/// @return true if the button was clicked
bool colorButton(StringView descId, Color col, ColorEditFlags flags = ColorEditFlags::None, Vec2 size = {0, 0});

/// Set the color picker reference color (shown as comparison)
/// @param col Reference color (RGBA)
void setColorEditReference(const f32 col[4]);

// ============================================================================
// Internal Color Picker Structures (for advanced customization)
// ============================================================================

struct ColorPickerState
{
    f32 hue = 0.0f;
    f32 saturation = 1.0f;
    f32 value = 1.0f;
    f32 alpha = 1.0f;

    bool draggingSV = false;     // Dragging saturation/value square
    bool draggingHue = false;    // Dragging hue bar/wheel
    bool draggingAlpha = false;  // Dragging alpha bar

    f32 referenceCol[4] = {0};  // Reference color for comparison
    bool hasReference = false;
};

/// Advanced color picker with full control
/// @param strId Unique string ID
/// @param col RGBA color array [0.0 - 1.0]
/// @param flags ColorEditFlags options
/// @param refCol Optional reference color for comparison
/// @return true if the color was modified
bool colorPickerAdvanced(StringView strId, f32 col[4], ColorEditFlags flags = ColorEditFlags::None,
                         const f32* refCol = nullptr);

// ============================================================================
// Gradient & Palette Utilities
// ============================================================================

/// Display a horizontal gradient bar
/// @param col1 Start color
/// @param col2 End color
/// @param size Bar size
void colorGradientBar(Color col1, Color col2, Vec2 size);

/// Display a color palette for quick selection
/// @param label Widget label
/// @param currentCol Current color (modified on selection)
/// @param palette Array of palette colors
/// @param paletteSize Number of colors in palette
/// @param colsPerRow Colors per row (0 = auto)
/// @return true if a color was selected
bool colorPalette(StringView label, f32 currentCol[4], const Color* palette, usize paletteSize, i32 colsPerRow = 0);

/// Display a saved colors strip for user favorites
/// @param label Widget label
/// @param savedColors Array to store saved colors
/// @param maxColors Maximum number of saved colors
/// @param numSaved Current number of saved colors
/// @param currentCol Current color (added on click)
/// @return true if a color was selected or saved
bool colorSavedStrip(StringView label, f32 savedColors[][4], usize maxColors, usize& numSaved, f32 currentCol[4]);

}  // namespace dakt::gui
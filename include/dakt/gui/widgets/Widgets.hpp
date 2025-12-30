// ============================================================================
// DaktLib GUI Module - Widgets
// ============================================================================
// Basic immediate-mode widgets: Button, Label, Checkbox, etc.
// ============================================================================

#pragma once

#include <dakt/gui/framework/Context.hpp>
#include <dakt/gui/framework/Theme.hpp>

#include <functional>

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

// ============================================================================
// Button Widgets
// ============================================================================

bool button(StringView label, Vec2 size = {0, 0});
bool smallButton(StringView label);
bool invisibleButton(StringView strId, Vec2 size);
bool arrowButton(StringView strId, i32 dir);  // 0=left, 1=right, 2=up, 3=down
bool imageButton(uintptr_t textureId, Vec2 size, Vec2 uv0 = {0, 0}, Vec2 uv1 = {1, 1},
                 Color bgColor = Color::transparent(), Color tintColor = Color::white());

// ============================================================================
// Checkbox & Radio
// ============================================================================

bool checkbox(StringView label, bool* value);
bool checkboxFlags(StringView label, u32* flags, u32 flagsValue);
bool radioButton(StringView label, bool active);
bool radioButton(StringView label, i32* value, i32 buttonValue);

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

// ============================================================================
// Input Fields
// ============================================================================

enum class InputTextFlags : u32
{
    None = 0,
    CharsDecimal = 1 << 0,
    CharsHexadecimal = 1 << 1,
    CharsUppercase = 1 << 2,
    CharsNoBlank = 1 << 3,
    AutoSelectAll = 1 << 4,
    EnterReturnsTrue = 1 << 5,
    CallbackCompletion = 1 << 6,
    CallbackHistory = 1 << 7,
    CallbackAlways = 1 << 8,
    CallbackCharFilter = 1 << 9,
    AllowTabInput = 1 << 10,
    CtrlEnterForNewLine = 1 << 11,
    NoHorizontalScroll = 1 << 12,
    AlwaysOverwrite = 1 << 13,
    ReadOnly = 1 << 14,
    Password = 1 << 15,
    NoUndoRedo = 1 << 16,
    Multiline = 1 << 17
};

inline InputTextFlags operator|(InputTextFlags a, InputTextFlags b)
{
    return static_cast<InputTextFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

inline bool hasFlag(InputTextFlags flags, InputTextFlags flag)
{
    return (static_cast<u32>(flags) & static_cast<u32>(flag)) != 0;
}

bool inputText(StringView label, char* buf, usize bufSize, InputTextFlags flags = InputTextFlags::None);
bool inputText(StringView label, String& str, InputTextFlags flags = InputTextFlags::None);
bool inputTextMultiline(StringView label, char* buf, usize bufSize, Vec2 size = {0, 0},
                        InputTextFlags flags = InputTextFlags::None);
bool inputTextMultiline(StringView label, String& str, Vec2 size = {0, 0}, InputTextFlags flags = InputTextFlags::None);
bool inputFloat(StringView label, f32* value, f32 step = 0.0f, f32 stepFast = 0.0f, StringView format = "%.3f");
bool inputFloat2(StringView label, f32 value[2], StringView format = "%.3f");
bool inputFloat3(StringView label, f32 value[3], StringView format = "%.3f");
bool inputFloat4(StringView label, f32 value[4], StringView format = "%.3f");
bool inputInt(StringView label, i32* value, i32 step = 1, i32 stepFast = 100);
bool inputInt2(StringView label, i32 value[2]);
bool inputInt3(StringView label, i32 value[3]);
bool inputInt4(StringView label, i32 value[4]);

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

// ============================================================================
// Selectable
// ============================================================================

enum class SelectableFlags : u32
{
    None = 0,
    DontClosePopups = 1 << 0,
    SpanAllColumns = 1 << 1,
    AllowDoubleClick = 1 << 2,
    Disabled = 1 << 3,
    AllowItemOverlap = 1 << 4
};

inline SelectableFlags operator|(SelectableFlags a, SelectableFlags b)
{
    return static_cast<SelectableFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

bool selectable(StringView label, bool selected = false, SelectableFlags flags = SelectableFlags::None,
                Vec2 size = {0, 0});
bool selectable(StringView label, bool* selected, SelectableFlags flags = SelectableFlags::None, Vec2 size = {0, 0});

// ============================================================================
// Progress Bar
// ============================================================================

void progressBar(f32 fraction, Vec2 size = {-1, 0}, StringView overlay = "");

// ============================================================================
// Tooltip
// ============================================================================

void setTooltip(StringView text);
void beginTooltip();
void endTooltip();

// ============================================================================
// Image
// ============================================================================

void image(uintptr_t textureId, Vec2 size, Vec2 uv0 = {0, 0}, Vec2 uv1 = {1, 1}, Color tintColor = Color::white(),
           Color borderColor = Color::transparent());

}  // namespace dakt::gui

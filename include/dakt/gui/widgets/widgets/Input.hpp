// ============================================================================
// DaktLib GUI Module - Input Field Widgets
// ============================================================================

#pragma once

#include <dakt/gui/framework/Context.hpp>
#include <dakt/gui/framework/Theme.hpp>

namespace dakt::gui
{

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

}  // namespace dakt::gui

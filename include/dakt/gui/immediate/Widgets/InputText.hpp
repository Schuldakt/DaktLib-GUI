#ifndef DAKTLIB_GUI_IMMEDIATECONTEXT_HPP
#define DAKTLIB_GUI_IMMEDIATECONTEXT_HPP

#include "../../core/Types.hpp"

namespace dakt::gui {

// ============================================================================
// Input Text
// ============================================================================

enum class InputTextFlags : uint32_t {
    None = 0,
    CharsDecimal = 1 << 0,
    CharsHexadecimal = 1 << 1,
    CharsUppercase = 1 << 2,
    CharsNoBlank = 1 << 3,
    AutoSelectAll = 1 << 4,
    EnterReturnsTrue = 1 << 5,
    Password = 1 << 6,
    ReadOnly = 1 << 7,
    Multiline = 1 << 8,
};

inline InputTextFlags operator|(InputTextFlags a, InputTextFlags b) { return static_cast<InputTextFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }

bool inputText(const char* label, char* buf, size_t bufSize, InputTextFlags flags = InputTextFlags::None);
bool inputTextMultiline(const char* label, char* buf, size_t bufSize, Vec2 size = Vec2(0, 0), InputTextFlags flags = InputTextFlags::None);
bool inputInt(const char* label, int* value, int step = 1, int stepFast = 100);
bool inputFloat(const char* label, float* value, float step = 0.0f, float stepFast = 0.0f, const char* format = "%.3f");

} // namespace dakt::gui

#endif
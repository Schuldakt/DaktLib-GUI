#pragma once

#include <cstddef>
#include <cstdint>

#include "dakt/gui/core/Types.hpp"

namespace dakt::gui {

    enum class InputTextFlags : std::uint32_t { 
        None            = 0, 
        ReadOnly        = 1 << 0, 
        Password        = 1 << 1, 
        Multiline       = 1 << 2, 
        AllowTabInput   = 1 << 3 };

    DAKTLIB_GUI_API bool inputText(const char* label, char* buf, std::size_t buf_size, InputTextFlags flags = InputTextFlags::None);
    
    DAKTLIB_GUI_API bool inputTextMultiline(
        const char* label,
        char* buf,
        std::size_t buf_size,
        Vec2 size = Vec2(0, 0),
        InputTextFlags flags = InputTextFlags::None
    );

    DAKTLIB_GUI_API bool inputInt(const char* label, int* v);
    DAKTLIB_GUI_API bool inputFloat(const char* label, float* v);

} // namespace dakt::gui

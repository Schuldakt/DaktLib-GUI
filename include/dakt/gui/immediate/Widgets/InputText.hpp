#pragma once

#include <cstddef>

#include "dakt/gui/core/Types.hpp"

namespace dakt::gui {

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

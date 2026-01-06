#pragma once

#include "dakt/gui/core/Types.hpp"

namespace dakt::gui {

    DAKTLIB_GUI_API bool colorEdit3(const char* label, Color* color);
    DAKTLIB_GUI_API bool colorEdit4(const char* label, Color* color);
    DAKTLIB_GUI_API bool colorPicker3(const char* label, Color* color);
    DAKTLIB_GUI_API bool colorPicker4(const char* label, Color* color);

} // namespace dakt::gui
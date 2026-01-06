#pragma once

#include "dakt/gui/core/Types.hpp"
namespace dakt::gui {

    DAKTLIB_GUI_API bool checkbox(const char* label, bool* v);

    DAKTLIB_GUI_API bool radioButton(const char* label, bool active);
    DAKTLIB_GUI_API bool radioButton(const char* label, int* v, int v_button);

} // namespace dakt::gui
#pragma once

#include "dakt/gui/core/Types.hpp"

namespace dakt::gui {

    DAKTLIB_GUI_API bool listBox(const char* label, int* currentItem, const char* const items[], int itemCount, int heightInItems = -1);

} // namespace dakt::gui
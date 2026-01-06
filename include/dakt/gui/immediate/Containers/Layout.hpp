#pragma once

#include "dakt/gui/core/Types.hpp"
namespace dakt::gui {

    DAKTLIB_GUI_API void sameLine(float offsetX = 0.0f, float spacing = -1.0f);
    DAKTLIB_GUI_API void newLine();
    DAKTLIB_GUI_API void spacing();
    DAKTLIB_GUI_API void separator();

    DAKTLIB_GUI_API void indent(float indentW = 0.0f);
    DAKTLIB_GUI_API void unindent(float indentW = 0.0f);

} // namespace dakt::gui
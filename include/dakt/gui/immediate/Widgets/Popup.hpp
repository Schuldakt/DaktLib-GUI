#pragma once

#include "dakt/gui/core/Types.hpp"

namespace dakt::gui {

    DAKTLIB_GUI_API void openPopup(const char* strId);
    DAKTLIB_GUI_API bool beginPopup(const char* strId);
    DAKTLIB_GUI_API bool beginPopupModal(const char* name, bool* open = nullptr);
    DAKTLIB_GUI_API void endPopup();
    DAKTLIB_GUI_API void closeCurrentPopup();

    DAKTLIB_GUI_API bool beginPopupContextItem(const char* strId = nullptr);
    DAKTLIB_GUI_API bool beginPopupContextWindow(const char* strId = nullptr);

} // namespace dakt::gui
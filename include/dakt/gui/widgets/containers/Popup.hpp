// ============================================================================
// DaktLib GUI Module - Popup Container
// ============================================================================

#pragma once

#include <dakt/gui/widgets/containers/Window.hpp>

namespace dakt::gui
{

// ============================================================================
// Popup Functions
// ============================================================================

// Popups (modal/non-modal)
void openPopup(StringView strId);
bool beginPopup(StringView strId, WindowFlags flags = WindowFlags::None);
bool beginPopupModal(StringView name, bool* open = nullptr, WindowFlags flags = WindowFlags::None);
bool beginPopupContextItem(StringView strId = "", i32 mouseButton = 1);
bool beginPopupContextWindow(StringView strId = "", i32 mouseButton = 1);
bool beginPopupContextVoid(StringView strId = "", i32 mouseButton = 1);
void endPopup();
void closeCurrentPopup();
bool isPopupOpen(StringView strId);

}  // namespace dakt::gui

#include "dakt/gui/immediate/Widgets/Popup.hpp"

#include "dakt/gui/core/Context.hpp"

#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"

#include <cstring>

namespace dakt::gui {

    void openPopup(const char* strId) {
        ImmediateState& s = getState();
        PopUpState& pop = s.popupState;
        pop.pendingPopup = strId;
    }

    bool beginPopup(const char* strId) {
        Context* ctx = getCurrentContext();
        if (!ctx || !strId) return false;

        ImmediateState& s = getState();
        PopUpState& pop = s.popupState;

        if (pop.pendingPopup && strcmp(pop.pendingPopup, strId) == 0) {
            pop.pendingPopup = nullptr;
            pop.popupStack.push_back(strId);

            Vec2 mousePos = getMousePos();
            pop.popupPositions.push_back(mousePos);
        }

        for (const auto& popup : pop.popupStack) {
            if (strcmp(popup, strId) == 0) {
                return true;
            }
        }

        return false;
    }

    bool beginPopupModal(const char* name, bool* open) {
        Context* ctx = getCurrentContext();
        if (!ctx || !name) return false;

        if (open && !*open)
            return false;

        ImmediateState& s = getState();
        PopUpState& pop = s.popupState;
        
        pop.popupStack.push_back(name);
        pop.popupPositions.push_back(Vec2(100, 100));

        return true;
    }

    void endPopup() {
        ImmediateState& s = getState();
        PopUpState& pop = s.popupState;
        if (!pop.popupStack.empty()) {
            pop.popupStack.pop_back();
            if (!pop.popupPositions.empty()) {
                pop.popupPositions.pop_back();
            }
        }
    }

    void closeCurrentPopup() {
        ImmediateState& s = getState();
        PopUpState& pop = s.popupState;
        pop.closeRequested = true;
    }

    bool beginPopupContextItem(const char* strId) {
        Context* ctx = getCurrentContext();
        if (!ctx) return false;

        ImmediateState& s = getState();
        
        if (s.lastItemHovered && isMouseClicked(MouseButton::Right)) {
            openPopup(strId ? strId : "context_item");
        }

        return beginPopup(strId ? strId : "context_item");
    }

    bool beginPopupContextWindow(const char* strId) {
        Context* ctx = getCurrentContext();
        if (!ctx) return false;

        if (isMouseClicked(MouseButton::Right)) {
            openPopup(strId ? strId : "context_window");
        }

        return beginPopup(strId ? strId : "context_window");
    }
} // namespace dakt::gui
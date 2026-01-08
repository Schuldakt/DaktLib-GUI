#include "dakt/gui/immediate/Widgets/Popup.hpp"
#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/WidgetBase.hpp"

#include <cstring>

namespace dakt::gui {
    
    void openPopup(const char* strId) {
        PopUpState& pop = getState().popupState;
        pop.pendingPopup = strId;
    }

    bool beginPopup(const char* strId) {
        auto w = widgetSetup();
        if (!w) return false;

        // Check if this popup should be opened
        PopUpState& pop = w.state->popupState;

        if (pop.pendingPopup && strcmp(pop.pendingPopup, strId) == 0) {
            pop.pendingPopup = nullptr;
            pop.popupStack.push_back(strId);

            Vec2 mousePos = w.state->input ? w.state->input->getMousePosition() : Vec2(100, 100);
            pop.popupPositions.push_back(mousePos);
        }

        // Check if popup is open
        for (const auto& popup : pop.popupStack) {
            if (strcmp(popup, strId) == 0) {
                // Draw popup background
                Vec2 pos = pop.popupPositions.back();
                Rect popupRect(pos.x, pos.y, 220, 200);

                // Shadow
                w.dl->drawRectFilledRounded(Rect(pos.x + 3, pos.y + 3, 220, 200), Color(0, 0, 0, 0), 4.0f);
                // Background
                w.dl->drawRectFilledRounded(popupRect, w.colors->surface, 4.0f);
                // Border
                w.dl->drawRectRounded(popupRect, w.colors->border, 4.0f);

                // Update cursor for popup content
                setNextWindowPos(pos);
                setNextWindowSize(Vec2(220, 200));

                return true;
            }
        }

        return false;
    }

    bool beginPopupModal(const char* name, bool* open) {
        auto w = widgetSetup();
        if (!w) return false;

        if (open && !*open)
            return false;

        // Draw modal overlay
        Rect viewport(0, 0, 1280.0f, 720.0f);
        w.dl->drawRectFilled(viewport, Color(0, 0, 0, 128));

        // Draw modal window
        Vec2 modalSize(400, 300);
        Vec2 modalPos((viewport.width - modalSize.x) / 2, (viewport.height - modalSize.y) / 2);

        Rect modalRect(modalPos.x, modalPos.y, modalSize.x, modalSize.y);
        w.dl->drawRectFilledRounded(modalRect, w.colors->surface, 6.0f);

        // Title bar
        Rect titleRect(modalPos.x, modalPos.y, modalSize.x, 32);
        w.dl->drawRectFilledRounded(titleRect, w.colors->surfaceVariant, BorderRadius(6, 6, 0, 0));
        w.dl->drawText(Vec2(modalPos.x + 12, modalPos.y + 8), name, w.colors->textPrimary);

        // Close button
        if (open) {
            Rect closeRect(modalPos.x + modalSize.x - 28, modalPos.y + 4, 24, 24);
            bool closeHovered = w.state->input && closeRect.contains(w.state->input->getMousePosition());
            if (closeHovered) {
                w.dl->drawRectFilledRounded(closeRect, w.colors->surfaceVariant, 4.0f);
            }
            w.dl->drawText(Vec2(closeRect.x + 6, closeRect.y + 4), "x", w.colors->textPrimary);

            if (closeHovered && w.state->input && w.state->input->isMouseButtonPressed(MouseButton::Left)) {
                *open = false;
                return false;
            }
        }

        PopUpState pop = getState().popupState;
        pop.popupStack.push_back(name);
        pop.popupPositions.push_back(modalPos + Vec2(0, 32));

        setNextWindowPos(modalPos + Vec2(12, 40));
        setNextWindowSize(Vec2(modalSize.x - 24, modalSize.y - 52));

        return true;
    }

    void endPopup() {
        PopUpState pop = getState().popupState;
        if (!pop.popupStack.empty()) {
            pop.popupStack.pop_back();
            pop.popupPositions.pop_back();
        }
    }

    void closeCurrentPopup() {
        PopUpState pop = getState().popupState;
        pop.closeRequested = true;
    }

    bool beginPopupContextItem(const char* strId) {
        auto w = widgetSetup();
        if (!w || !w.state->input)
            return false;

        // Check for right-click on last item
        if (w.state->lastItemHovered && w.state->input->isMouseButtonPressed(MouseButton::Right)) {
            openPopup(strId ? strId : "context_item");
        }

        return beginPopup(strId ? strId : "context_item");
    }

    bool beginPopupContextWindow(const char* strId) {
        auto w = widgetSetup();
        if (!w || !w.state->input)
            return false;

        // Check for right-click in window
        if (w.state->input->isMouseButtonPressed(MouseButton::Right)) {
            openPopup(strId ? strId : "context_window");
        }

        return beginPopup(strId ? strId : "context_window");
    }
} // namespace dakt::gui
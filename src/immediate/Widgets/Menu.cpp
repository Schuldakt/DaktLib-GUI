#include "dakt/gui/immediate/Widgets/Menu.hpp"
#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/WidgetBase.hpp"

#include <cstring>

namespace dakt::gui {

    bool beginMainMenuBar() {
        auto w = widgetSetup();
        if (!w) return false;

        MenuState& menu = w.state->menuState;

        float screenWidth = 1280.0f; // Default screen width
        Rect barRect(0, 0, screenWidth, menu.menuBarHeight);

        w.dl->drawRectFilled(barRect, w.colors->surface);
        w.dl->drawLine(Vec2(0, menu.menuBarHeight), Vec2(screenWidth, menu.menuBarHeight), w.colors->border);

        menu.mainMenuBarOpen = true;
        menu.menuPosition = Vec2(8, 0);
        menu.menuItemRects.clear();

        return true;
    }

    void endMainMenuBar() {
        MenuState& menu = getState().menuState;
        menu.mainMenuBarOpen = false;
    }

    bool beginMenuBar() {
        auto w = widgetSetup();
        if (!w || !w.state->currentWindow)
            return false;

        MenuState& menu = w.state->menuState;
        menu.menuBarOpen = true;
        menu.menuPosition = getWindowPos();
        return true;
    }

    void endMenuBar() {
        MenuState& menu = getState().menuState;
        menu.menuBarOpen = false;
    }

    bool beginMenu(const char* label, bool enabled) {
        auto w = widgetSetup();
        if (!w) return false;
        if (!enabled) return false;

        MenuState& menu = w.state->menuState;

        float labelWidth = static_cast<float>(strlen(label)) * 8.0f + 16.0f;
        Rect menuRect(menu.menuPosition.x, menu.menuPosition.y, labelWidth, menu.menuBarHeight);

        bool hovered = w.state->input && menuRect.contains(w.state->input->getMousePosition());
        bool clicked = hovered && w.state->input && w.state->input->isMouseButtonPressed(MouseButton::Left);

        // Draw menu title
        if (hovered) {
            w.dl->drawRectFilled(menuRect, w.colors->surfaceVariant);
        }

        w.dl->drawText(Vec2(menuRect.x + 8, menuRect.y + (menu.menuBarHeight - 14) / 2), label, w.colors->textPrimary);

        menu.menuPosition.x += labelWidth;

        if (clicked) {
            menu.menuStack.push_back(label);
            return true;
        }

        // Check if this menu is already open
        if (!menu.menuStack.empty() && strcmp(menu.menuStack.back(), label) == 0) {
            return true;
        }

        return false;
    }

    void endMenu() {
        MenuState& menu = getState().menuState;
        if (!menu.menuStack.empty()) {
            menu.menuStack.pop_back();
        }
    }

    bool menuItem(const char* label, const char* shortcut, bool selected, bool enabled) {
        auto w = widgetSetup();
        if (!w) return false;

        Vec2 pos = getCursorPos();
        Vec2 windowPos = getWindowPos();

        float itemHeight = 24.0f;
        float itemWidth = 200.0f;
        Rect itemRect(windowPos.x + pos.x, windowPos.y + pos.y, itemWidth, itemHeight);

        bool hovered = enabled && w.state->input && itemRect.contains(w.state->input->getMousePosition());
        bool clicked = hovered && w.state->input && w.state->input->isMouseButtonPressed(MouseButton::Left);

        // Draw background
        if (hovered) {
            w.dl->drawRectFilled(itemRect, w.colors->surfaceVariant);
        }

        // Draw checkmark if selected
        if (selected) {
            w.dl->drawText(Vec2(itemRect.x + 4, itemRect.y + 4), "√", w.colors->textPrimary);
        }

        // Draw label
        Color textColor = enabled ? w.colors->textPrimary : w.colors->textDisabled;
        w.dl->drawText(Vec2(itemRect.x + 24, itemRect.y + 4), label, textColor);

        // Draw shortcut
        if (shortcut) {
            float shortcutWidth = static_cast<float>(strlen(shortcut)) * 7.0f;
            w.dl->drawText(Vec2(itemRect.x + itemWidth - shortcutWidth - 8, itemRect.y + 4), shortcut, w.colors->textSecondary);
        }

        setCursorPos(Vec2(pos.x, pos.y + itemHeight));

        return clicked;
    }

    bool menuItem(const char* label, const char* shortcut, bool* selected, bool enabled) {
        bool clicked = menuItem(label, shortcut, selected ? *selected : false, enabled);
        if (clicked && selected) {
            *selected = !(*selected);
        }
        return clicked;
    }
} // namespace dakt::gui

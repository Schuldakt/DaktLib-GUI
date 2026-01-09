#include "dakt/gui/immediate/Widgets/Menu.hpp"

#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"

#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"

#include <cstring>

namespace dakt::gui {

    static bool pointInRect(const Vec2& p, const Rect& r) {
        return p.x >= r.x && p.y >= r.y && p.x <= (r.x + r.width) && p.y < (r.y + r.height);
    }

    bool beginMainMenuBar() {
        Context* ctx = getCurrentContext();
        if (!ctx) return false;

        ImmediateState& s = getState();
        MenuState& menu = s.menuState;

        menu.mainMenuBarOpen = true;
        menu.menuPosition = Vec2(8, 0);
        menu.menuItemRects.clear();

        return true;
    }

    void endMainMenuBar() {
        ImmediateState& s = getState();
        MenuState& menu = s.menuState;
        menu.mainMenuBarOpen = false;
    }

    bool beginMenuBar() {
        Context* ctx = getCurrentContext();
        if (!ctx) return false;

        ImmediateState& s = getState();
        if (!s.currentWindow) return false;

        MenuState& menu = s.menuState;
        menu.menuBarOpen = true;
        menu.menuPosition = Vec2(0, 0);  // Simplified
        return true;
    }

    void endMenuBar() {
        ImmediateState& s = getState();
        MenuState& menu = s.menuState;
        menu.menuBarOpen = false;
    }

    bool beginMenu(const char* label, bool enabled) {
        Context* ctx = getCurrentContext();
        if (!ctx || !label || !enabled) return false;

        ImmediateState& s = getState();
        MenuState& menu = s.menuState;

        // Simplified menu - just track if open
        if (!menu.menuStack.empty() && strcmp(menu.menuStack.back(), label) == 0) {
            return true;
        }

        return false;
    }

    void endMenu() {
        ImmediateState& s = getState();
        MenuState& menu = s.menuState;
        if (!menu.menuStack.empty()) {
            menu.menuStack.pop_back();
        }
    }

    bool menuItem(const char* label, const char* shortcut, bool selected, bool enabled) {
        Context* ctx = getCurrentContext();
        if (!ctx || !label) return false;

        ImmediateState& s = getState();
        if (!s.currentWindow) return false;
        if (s.currentWindow->skipItems) return false;

        WindowState* win = s.currentWindow;

        ID id = getID(label);

        const float itemHeight = 24.0f;
        const float itemWidth = 200.0f;

        Vec2 pos = win->cursorPos;
        Rect bb(pos.x, pos.y, itemWidth, itemHeight);

        win->cursorPos.x = win->cursorStartPos.x;
        win->cursorPos.y += itemHeight;

        Vec2 mousePos = getMousePos();
        bool hovered = enabled && pointInRect(mousePos, bb);

        if (hovered) s.hotId = id;

        bool active = (s.activeId == id);
        bool clicked = false;

        if (hovered && isMouseClicked(MouseButton::Left)) {
            s.activeId = id;
            active = true;
        }

        if (isMouseReleased(MouseButton::Left) && s.activeId == id) {
            if (hovered) {
                clicked = true;
            }
            s.activeId = 0;
            active = false;
        }

        s.lastItemId = id;
        s.lastItemRect = bb;
        s.lastItemHovered = hovered;
        s.lastItemActive = active;
        s.lastItemClicked = clicked;

        DrawList* dl = getWindowDrawList();
        if (dl) {
            if (hovered) {
                dl->drawRectFilled(bb, Color::fromFloats(0.25f, 0.25f, 0.25f, 1.0f));
            }

            if (selected) {
                dl->drawText(Vec2(pos.x + 4, pos.y + 4), "√", Color::fromFloats(1.0f, 1.0f, 1.0f, 1.0f));
            }

            Color textColor = enabled ? Color::fromFloats(1.0f, 1.0f, 1.0f, 1.0f) : Color::fromFloats(0.5f, 0.5f, 0.5f, 1.0f);
            dl->drawText(Vec2(pos.x + 24, pos.y + 4), label, textColor);

            if (shortcut) {
                float shortcutWidth = static_cast<float>(strlen(shortcut)) * 7.0f;
                dl->drawText(Vec2(pos.x + itemWidth - shortcutWidth - 8, pos.y + 4), shortcut, Color::fromFloats(0.7f, 0.7f, 0.7f, 1.0f));
            }
        }

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

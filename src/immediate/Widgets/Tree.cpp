#include "dakt/gui/immediate/Widgets/Tree.hpp"

#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/immediate/Widgets/Text.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"

#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"

#include <cstring>

namespace dakt::gui {

    static bool pointInRect(const Vec2& p, const Rect& r) {
        return p.x >= r.x && p.y >= r.y && p.x <= (r.x + r.width) && p.y < (r.y + r.height);
    }

    bool treeNode(const char* label, TreeNodeFlags flags) {
        Context* ctx = getCurrentContext();
        if (!ctx || !label) return false;

        ImmediateState& s = getState();
        if (!s.currentWindow) return false;
        if (s.currentWindow->skipItems) return false;

        WindowState* win = s.currentWindow;

        ID id = getID(label);

        const float nodeHeight = 20.0f;
        const float nodeWidth = 200.0f;
        const float spacingY = 4.0f;

        Vec2 pos = win->cursorPos;
        Rect bb(pos.x, pos.y, nodeWidth, nodeHeight);

        win->cursorPos.x = win->cursorStartPos.x;
        win->cursorPos.y += nodeHeight + spacingY;

        Vec2 mousePos = getMousePos();
        bool hovered = pointInRect(mousePos, bb);

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
            dl->drawText(pos, ">", Color::fromFloats(1.0f, 1.0f, 1.0f, 1.0f));
            dl->drawText(Vec2(pos.x + 16, pos.y), label, Color::fromFloats(1.0f, 1.0f, 1.0f, 1.0f));
        }

        return clicked;
    }

    bool treeNodeEx(const char* strId, TreeNodeFlags flags, const char* fmt, ...) {
        return treeNode(strId, flags);
    }

    void treePop() {
        // Reduce indentation (not implemented in new system yet)
    }

    bool collapsingHeader(const char* label, TreeNodeFlags flags) {
        return treeNode(label, flags);
    }

    bool selectable(const char* label, bool selected, Vec2 size) {
        Context* ctx = getCurrentContext();
        if (!ctx || !label) return false;

        ImmediateState& s = getState();
        if (!s.currentWindow) return false;
        if (s.currentWindow->skipItems) return false;

        WindowState* win = s.currentWindow;

        ID id = getID(label);

        const float spacingY = 0.0f;
        const float defaultHeight = 20.0f;
        const float defaultWidth = 200.0f;

        Vec2 itemSize = size;
        if (itemSize.x <= 0) itemSize.x = defaultWidth;
        if (itemSize.y <= 0) itemSize.y = defaultHeight;

        Vec2 pos = win->cursorPos;
        Rect bb(pos.x, pos.y, itemSize.x, itemSize.y);

        win->cursorPos.x = win->cursorStartPos.x;
        win->cursorPos.y += itemSize.y + spacingY;

        Vec2 mousePos = getMousePos();
        bool hovered = pointInRect(mousePos, bb);

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
            if (selected || hovered) {
                Color bgColor = selected ? Color::fromFloats(0.30f, 0.50f, 0.80f, 1.0f) : Color::fromFloats(0.25f, 0.25f, 0.25f, 1.0f);
                dl->drawRectFilled(bb, bgColor);
            }

            Color textColor = selected ? Color::white() : Color::fromFloats(1.0f, 1.0f, 1.0f, 1.0f);
            dl->drawText(Vec2(pos.x + 4, pos.y + 2), label, textColor);
        }

        return clicked;
    }

    bool selectable(const char* label, bool* selected, Vec2 size) {
        if (selectable(label, *selected, size)) {
            *selected = !(*selected);
            return true;
        }
        return false;
    }

    bool listBox(const char* label, int* currentItem, const char* const items[], int itemCount, int heightInItems) {
        if (!currentItem || !items) return false;

        text("%s", label);
        bool changed = false;
        for (int i = 0; i < itemCount; i++) {
            if (selectable(items[i], i == *currentItem, Vec2(0, 0))) {
                *currentItem = i;
                changed = true;
            }
        }
        return changed;
    }

} // namespace dakt::gui
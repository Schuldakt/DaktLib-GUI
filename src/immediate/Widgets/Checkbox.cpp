#include "dakt/gui/immediate/Widgets/Checkbox.hpp"

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

    static Vec2 calcTextSize(const char* text) {
        float w = 0.0f;
        float h = 16.0f;
        if (!text) return Vec2(0, h);
        for (const char* c = text; *c; ++c) {
            w += 8.0f;
        }
        return Vec2(w, h);
    }

    bool checkbox(const char* label, bool* value) {
        Context* ctx = getCurrentContext();
        if (!ctx || !label || !value) return false;

        ImmediateState& s = getState();
        if (!s.currentWindow) return false;
        if (s.currentWindow->skipItems) return false;

        WindowState* win = s.currentWindow;

        ID id = getID(label);

        const float boxSize = 18.0f;
        const float spacingX = 6.0f;
        const float spacingY = 4.0f;

        Vec2 textSize = calcTextSize(label);
        Vec2 totalSize(boxSize + spacingX + textSize.x, boxSize);

        Vec2 pos = win->cursorPos;
        Rect bb(pos.x, pos.y, totalSize.x, totalSize.y);
        Rect checkRect(pos.x, pos.y, boxSize, boxSize);

        win->cursorPos.x = win->cursorStartPos.x;
        win->cursorPos.y += totalSize.y + spacingY;

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
                *value = !(*value);
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
            Color bgColor = *value ? Color::fromFloats(0.30f, 0.50f, 0.80f, 1.0f) : Color::fromFloats(0.20f, 0.20f, 0.20f, 1.0f);
            if (hovered) bgColor = *value ? Color::fromFloats(0.35f, 0.55f, 0.85f, 1.0f) : Color::fromFloats(0.25f, 0.25f, 0.25f, 1.0f);

            dl->drawRectFilledRounded(checkRect, bgColor, 2.0f);
            dl->drawRectRounded(checkRect, Color::fromFloats(0.30f, 0.30f, 0.30f, 1.0f), 2.0f);

            if (*value) {
                Vec2 p1(pos.x + 4, pos.y + 9);
                Vec2 p2(pos.x + 7, pos.y + 13);
                Vec2 p3(pos.x + 14, pos.y + 5);
                dl->drawLine(p1, p2, Color::white(), 2.0f);
                dl->drawLine(p2, p3, Color::white(), 2.0f);
            }

            Vec2 textPos(pos.x + boxSize + spacingX, pos.y + 1);
            dl->drawText(textPos, label, Color::fromFloats(1.0f, 1.0f, 1.0f, 1.0f));
        }

        return clicked;
    }

    bool radioButton(const char* label, bool active) {
        Context* ctx = getCurrentContext();
        if (!ctx || !label) return false;

        ImmediateState& s = getState();
        if (!s.currentWindow) return false;
        if (s.currentWindow->skipItems) return false;

        WindowState* win = s.currentWindow;

        ID id = getID(label);

        const float radius = 9.0f;
        const float spacingX = 6.0f;
        const float spacingY = 4.0f;

        Vec2 textSize = calcTextSize(label);
        Vec2 totalSize(radius * 2 + spacingX + textSize.x, radius * 2);

        Vec2 pos = win->cursorPos;
        Rect bb(pos.x, pos.y, totalSize.x, totalSize.y);
        Vec2 center(pos.x + radius, pos.y + radius);

        win->cursorPos.x = win->cursorStartPos.x;
        win->cursorPos.y += totalSize.y + spacingY;

        Vec2 mousePos = getMousePos();
        bool hovered = pointInRect(mousePos, bb);

        if (hovered) s.hotId = id;

        bool isActive = (s.activeId == id);
        bool clicked = false;

        if (hovered && isMouseClicked(MouseButton::Left)) {
            s.activeId = id;
            isActive = true;
        }

        if (isMouseReleased(MouseButton::Left) && s.activeId == id) {
            if (hovered) {
                clicked = true;
            }
            s.activeId = 0;
            isActive = false;
        }

        s.lastItemId = id;
        s.lastItemRect = bb;
        s.lastItemHovered = hovered;
        s.lastItemActive = isActive;
        s.lastItemClicked = clicked;

        DrawList* dl = getWindowDrawList();
        if (dl) {
            Color bgColor = active ? Color::fromFloats(0.30f, 0.50f, 0.80f, 1.0f) : Color::fromFloats(0.20f, 0.20f, 0.20f, 1.0f);
            if (hovered) bgColor = active ? Color::fromFloats(0.35f, 0.55f, 0.85f, 1.0f) : Color::fromFloats(0.25f, 0.25f, 0.25f, 1.0f);

            dl->drawCircleFilled(center, radius, bgColor);
            dl->drawCircle(center, radius, Color::fromFloats(0.30f, 0.30f, 0.30f, 1.0f));

            if (active) {
                dl->drawCircleFilled(center, radius * 0.4f, Color::white());
            }

            Vec2 textPos(pos.x + radius * 2 + spacingX, pos.y + 1);
            dl->drawText(textPos, label, Color::fromFloats(1.0f, 1.0f, 1.0f, 1.0f));
        }

        return clicked;
    }

    bool radioButton(const char* label, int* value, int buttonValue) {
        bool active = (*value == buttonValue);
        if (radioButton(label, active)) {
            *value = buttonValue;
            return true;
        }
        return false;
    }
} // namespace dakt::gui
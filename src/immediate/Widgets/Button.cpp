#include "dakt/gui/immediate/Widgets/Button.hpp"

#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"

#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"

#include <cstring>  // for strlen

namespace dakt::gui {

    static bool pointInRect(const Vec2& p, const Rect& r) {
        return p.x >= r.x && p.y >= r.y && p.x <= (r.x + r.width) && p.y < (r.y + r.height);
    }

    // Temporary text size approximation.
    // If you already have a text-size function elsewhere, us it instead.
    static Vec2 calcTextSize(const char* text) {
        float w = 0.0f;
        float h = 16.0f;
        if (!text) return Vec2(0, h);

        for (const char* c = text; *c; ++c) {
            w += 8.0f;
        }
        return Vec2(w, h);
    }

    bool button(const char* label, Vec2 size) {
        Context* ctx = getCurrentContext();
        if (!ctx || !label) return false;

        ImmediateState& s = getState();
        if (!s.currentWindow) return false;
        if (s.currentWindow->skipItems) return false;

        WindowState* win = s.currentWindow;

        // ID
        ID id = getID(label);

        // Layout
        const float paddingX = 10.0f;
        const float paddingY = 6.0f;
        const float spacingY = 8.0f;

        Vec2 textSize = calcTextSize(label);

        Vec2 finalSize = size;
        if (finalSize.x <= 0.0f) finalSize.x = textSize.x + paddingX * 2.0f;
        if (finalSize.y <= 0.0f) finalSize.y = textSize.y + paddingY * 2.0f;

        Vec2 pos = win->cursorPos;
        Rect bb(pos.x, pos.y, finalSize.x, finalSize.y);

        // Advance cursor (default vertical stacking)
        win->cursorPos.x = win->cursorStartPos.x;
        win->cursorPos.y += finalSize.y + spacingY;

        // Interaction
        Vec2 mousePos = getMousePos();
        bool hovered = pointInRect(mousePos, bb);

        if (hovered) {
            s.hotId = id;
        }

        // Determine active field name.
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

        // Update last-item info (required for item queries + sameLine())
        s.lastItemId = id;
        s.lastItemRect = bb;
        s.lastItemHovered = hovered;
        s.lastItemActive = active;
        s.lastItemClicked = clicked;

        // Draw
        DrawList* dl = getWindowDrawList();
        if (dl) {
            Color bg = Color::fromFloats(0.20f, 0.20f, 0.20f, 1.0f);
            if (hovered) bg = Color::fromFloats(0.25f, 0.25f, 0.25f, 1.0f);
            if (active) bg = Color::fromFloats(0.18f, 0.18f, 0.18f, 1.0f);

            Color border = Color::fromFloats(0.30f, 0.30f, 0.30f, 1.0f);
            Color text = Color::fromFloats(1.0f, 1.0f, 1.0f, 1.0f);

            dl->drawRectFilledRounded(bb, bg, 4.0f);
            dl->drawRectFilledRounded(bb, border, 4.0f);

            Vec2 textPos(bb.x + paddingX, bb.y + paddingY);
            dl->drawText(textPos, label, text);
        }

        return clicked;
    }

    bool smallButton(const char* label) {
        return button(label, Vec2(0, 0));
    }

    bool invisibleButton(const char* idStr, Vec2 size) {
    if (!idStr) return false;

    ImmediateState& s = getState();
    if (!s.currentWindow) return false;
    if (s.currentWindow->skipItems) return false;

    WindowState* win = s.currentWindow;

    ID id = getID(idStr);

    const float spacingY = 8.0f;

    Vec2 finalSize = size;
    if (finalSize.x <= 0.0f) finalSize.x = 10.0f;
    if (finalSize.y <= 0.0f) finalSize.y = 10.0f;

    Vec2 pos = win->cursorPos;
    Rect bb(pos.x, pos.y, finalSize.x, finalSize.y);

    win->cursorPos.x = win->cursorStartPos.x;
    win->cursorPos.y += finalSize.y + spacingY;

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
        if (hovered) clicked = true;
        s.activeId = 0;
        active = false;
    }

    s.lastItemId = id;
    s.lastItemRect = bb;
    s.lastItemHovered = hovered;
    s.lastItemActive  = active;
    s.lastItemClicked = clicked;

    return clicked;
}

bool colorButton(const char* idStr, Color color, Vec2 size) {
    if (!idStr) return false;

    ImmediateState& s = getState();
    if (!s.currentWindow) return false;
    if (s.currentWindow->skipItems) return false;

    WindowState* win = s.currentWindow;

    ID id = getID(idStr);

    const float spacingY = 8.0f;

    Vec2 finalSize = size;
    if (finalSize.x <= 0.0f) finalSize.x = 20.0f;
    if (finalSize.y <= 0.0f) finalSize.y = 20.0f;

    Vec2 pos = win->cursorPos;
    Rect bb(pos.x, pos.y, finalSize.x, finalSize.y);

    win->cursorPos.x = win->cursorStartPos.x;
    win->cursorPos.y += finalSize.y + spacingY;

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
        if (hovered) clicked = true;
        s.activeId = 0;
        active = false;
    }

    s.lastItemId = id;
    s.lastItemRect = bb;
    s.lastItemHovered = hovered;
    s.lastItemActive  = active;
    s.lastItemClicked = clicked;

    DrawList* dl = getWindowDrawList();
    if (dl) {
        dl->drawRectFilledRounded(bb, color, 3.0f);
        dl->drawRectRounded(bb, Color::fromFloats(0.30f, 0.30f, 0.30f, 1.0f), 3.0f);
    }

    return clicked;
    }

} // namespace dakt::gui
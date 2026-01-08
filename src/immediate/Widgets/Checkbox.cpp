#include "dakt/gui/immediate/Widgets/Checkbox.hpp"
#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/WidgetBase.hpp"

#include <cstring>

namespace dakt::gui {

    bool checkbox(const char* label, bool* value) {
        auto w = widgetSetup();
        if (!w) return false;

        ID id = getID(label);

        Vec2 pos = getCursorPos();
        Vec2 windowPos = getWindowPos();
        Vec2 checkPos = windowPos + pos;

        float boxSize = 18.0f;

        Rect checkRect(checkPos.x, checkPos.y, boxSize, boxSize);

        bool hovered = isMouseHoveringRect(checkPos, checkPos + Vec2(boxSize, boxSize));
        bool clicked = hovered && isMouseReleased(MouseButton::Left);

        if (clicked) {
            *value = !(*value);
        }

        Color bgColor = *value ? w.colors->primary : w.colors->surface;
        if (hovered)
            bgColor = *value ? w.colors->primaryHover : w.colors->surfaceVariant;

        w.dl->drawRectFilledRounded(checkRect, bgColor, 2.0f);
        w.dl->drawRectRounded(checkRect, w.colors->border, 2.0f);

        if (*value) {
            Vec2 p1 = checkPos + Vec2(4, 9);
            Vec2 p2 = checkPos + Vec2(7, 13);
            Vec2 p3 = checkPos + Vec2(14, 5);
            w.dl->drawLine(p1, p2, Color::white(), 2.0f);
            w.dl->drawLine(p2, p3, Color::white(), 2.0f);
        }

        float labelX = checkPos.x + boxSize + 6.0f;
        w.dl->drawText(Vec2(labelX, checkPos.y + 1), label, w.colors->textPrimary);

        w.state->lastItemRect = checkRect;
        w.state->lastItemEdited = clicked;
        updateItemState(id, hovered, hovered && isMouseDown(MouseButton::Left));

        setCursorPos(Vec2(pos.x, pos.y + boxSize + 4.0f));

        return clicked;

    }

    bool radioButton(const char* label, bool active) {
        auto w = widgetSetup();
        if (!w) return false;

        ID id = getID(label);

        Vec2 pos = getCursorPos();
        Vec2 windowPos = getWindowPos();
        Vec2 radioPos = windowPos + pos;

        float radius = 9.0f;

        Vec2 center = radioPos + Vec2(radius, radius);

        bool hovered = isMouseHoveringRect(radioPos, radioPos + Vec2(radius * 2, radius * 2));
        bool clicked = hovered && isMouseReleased(MouseButton::Left);

        Color bgColor = active ? w.colors->primary : w.colors->surface;
        if (hovered)
            bgColor = active ? w.colors->primaryHover : w.colors->surfaceVariant;

        w.dl->drawCircleFilled(center, radius, bgColor);
        w.dl->drawCircle(center, radius, w.colors->border);

        if (active) {
            w.dl->drawCircleFilled(center, radius * 0.4f, Color::white());
        }

        float labelX = radioPos.x + radius * 2 + 6.0f;

        w.dl->drawText(Vec2(labelX, radioPos.y + 1), label, w.colors->textPrimary);

        w.state->lastItemRect = Rect(radioPos.x, radioPos.y, radius * 2 + 6.0f + static_cast<float>(strlen(label)) * 8.0f, radius * 2);
        w.state->lastItemEdited = clicked;
        updateItemState(id, hovered, hovered && isMouseDown(MouseButton::Left));

        setCursorPos(Vec2(pos.x, pos.y + radius * 2 + 4.0f));

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
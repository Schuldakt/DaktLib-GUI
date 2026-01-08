#include "dakt/gui/immediate/Widgets/Button.hpp"
#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/WidgetBase.hpp"

#include <cstring>  // for strlen

namespace dakt::gui {

    bool button(const char* label, Vec2 size) {
        auto w = widgetSetup();
        if (!w) return false;

        const WidgetStyle& style = w.ctx->getTheme().getButtonStyle();

        ID id = getID(label);

        Vec2 cursorPos = getCursorPos();
        Vec2 windowPos = getWindowPos();
        Vec2 buttonPos = windowPos + cursorPos;

        float labelWidth = static_cast<float>(strlen(label)) * 8.0f;

        Vec2 buttonSize = size;
        if (buttonSize.x <= 0)
            buttonSize.x = labelWidth + 16.0f;
        if (buttonSize.y <= 0)
            buttonSize.y = 24.0f;

        Rect buttonRect(buttonPos.x, buttonPos.y, buttonSize.x, buttonSize.y);

        bool hovered = isMouseHoveringRect(buttonPos, buttonPos + buttonSize);
        bool held = hovered && isMouseDown(MouseButton::Left);
        bool clicked = hovered && isMouseReleased(MouseButton::Left);

        Color bgColor = style.backgroundColor;
        if (held) {
            bgColor = style.backgroundColorActive;
        } else if (hovered) {
            bgColor = style.backgroundColorHover;
        }

        w.dl->drawRectFilledRounded(buttonRect, bgColor, 4.0f);
        w.dl->drawRectRounded(buttonRect, w.colors->border, 4.0f);

        Vec2 textPos = Vec2(buttonPos.x + (buttonSize.x - labelWidth) * 0.5f, buttonPos.y + (buttonSize.y - 16.0f) * 0.5f);
        w.dl->drawText(textPos, label, w.colors->textPrimary);

        w.state->lastItemRect = buttonRect;
        updateItemState(id, hovered, held);

        setCursorPos(Vec2(cursorPos.x, cursorPos.y + buttonSize.y + 4.0f));

        return clicked;
    }

    bool smallButton(const char* label) {
        return button(label, Vec2(0, 20));
    }

    bool invisibleButton(const char* strId, Vec2 size) {
        auto w = widgetSetup();
        if (!w) return false;

        ID id = getID(strId);

        Vec2 cursorPos = getCursorPos();
        Vec2 windowPos = getWindowPos();
        Vec2 buttonPos = windowPos + cursorPos;

        bool hovered = isMouseHoveringRect(buttonPos, buttonPos + size);
        bool held = hovered && isMouseDown(MouseButton::Left);
        bool clicked = hovered && isMouseReleased(MouseButton::Left);

        w.state->lastItemRect = Rect(buttonPos.x, buttonPos.y, size.x, size.y);
        updateItemState(id, hovered, held);

        setCursorPos(Vec2(cursorPos.x, cursorPos.y + size.y + 4.0f));

        return clicked;
    }

    bool colorButton(const char* descId, Color color, Vec2 size) {
        auto w = widgetSetup();
        if (!w) return false;

        ID id = getID(descId);

        Vec2 pos = getCursorPos();
        Vec2 windowPos = getWindowPos();
        Vec2 buttonPos = windowPos + pos;

        Vec2 buttonSize = size;
        if (buttonSize.x <= 0)
            buttonSize.x = 24.0f;
        if (buttonSize.y <= 0)
            buttonSize.y = 24.0f;

        Rect buttonRect(buttonPos.x, buttonPos.y, buttonSize.x, buttonSize.y);

        bool hovered = isMouseHoveringRect(buttonPos, buttonPos + buttonSize);
        bool clicked = hovered && isMouseReleased(MouseButton::Left);

        w.dl->drawRectFilledRounded(buttonRect, color, 2.0f);
        w.dl->drawRectRounded(buttonRect, w.colors->border, 2.0f);

        w.state->lastItemRect = buttonRect;
        updateItemState(id, hovered, hovered && isMouseDown(MouseButton::Left));

        setCursorPos(Vec2(pos.x, pos.y + buttonSize.y + 4.0f));

        return clicked;
    }
} // namespace dakt::gui
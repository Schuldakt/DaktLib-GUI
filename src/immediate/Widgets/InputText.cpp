#include "dakt/gui/immediate/Widgets/InputText.hpp"

#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/immediate/Widgets/Text.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"

#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"

#include <cstring>
#include <cstdio>

namespace dakt::gui {

    static bool pointInRect(const Vec2& p, const Rect& r) {
        return p.x >= r.x && p.y >= r.y && p.x <= (r.x + r.width) && p.y < (r.y + r.height);
    }

    bool inputText(const char* label, char* buf, size_t bufSize, InputTextFlags flags) {
        Context* ctx = getCurrentContext();
        if (!ctx || !label || !buf) return false;

        ImmediateState& s = getState();
        if (!s.currentWindow) return false;
        if (s.currentWindow->skipItems) return false;

        WindowState* win = s.currentWindow;

        ID id = getID(label);

        const float labelWidth = static_cast<float>(strlen(label)) * 8.0f + 8.0f;
        const float inputWidth = 200.0f;
        const float inputHeight = 22.0f;
        const float spacingY = 4.0f;

        Vec2 pos = win->cursorPos;
        Vec2 boxPos(pos.x + labelWidth, pos.y);
        Rect boxRect(boxPos.x, boxPos.y, inputWidth, inputHeight);
        Rect bb(pos.x, pos.y, labelWidth + inputWidth, inputHeight);

        win->cursorPos.x = win->cursorStartPos.x;
        win->cursorPos.y += inputHeight + spacingY;

        Vec2 mousePos = getMousePos();
        bool hovered = pointInRect(mousePos, boxRect);

        if (hovered) s.hotId = id;

        bool active = (s.activeId == id);

        if (hovered && isMouseClicked(MouseButton::Left)) {
            s.activeId = id;
            active = true;
        }

        s.lastItemId = id;
        s.lastItemRect = bb;
        s.lastItemHovered = hovered;
        s.lastItemActive = active;

        DrawList* dl = getWindowDrawList();
        if (dl) {
            dl->drawText(pos, label, Color::fromFloats(1.0f, 1.0f, 1.0f, 1.0f));

            Color bgColor = hovered ? Color::fromFloats(0.25f, 0.25f, 0.25f, 1.0f) : Color::fromFloats(0.20f, 0.20f, 0.20f, 1.0f);
            dl->drawRectFilledRounded(boxRect, bgColor, 2.0f);
            dl->drawRectRounded(boxRect, Color::fromFloats(0.30f, 0.30f, 0.30f, 1.0f), 2.0f);

            Vec2 textPos(boxPos.x + 4, boxPos.y + 3);
            dl->drawText(textPos, buf, Color::fromFloats(1.0f, 1.0f, 1.0f, 1.0f));
        }

        return false;
    }

    bool inputTextMultiline(const char* label, char* buf, size_t bufSize, Vec2 size, InputTextFlags flags) {
        return inputText(label, buf, bufSize, flags);
    }

    bool inputInt(const char* label, int* value, int step, int stepFast) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%d", *value);
        text("%s: %s", label, buf);
        return false;
    }

    bool inputFloat(const char* label, float* value, float step, float stepFast, const char* format) {
        char buf[64];
        snprintf(buf, sizeof(buf), format, *value);
        text("%s: %s", label, buf);
        return false;
    }

} // namespace dakt::gui
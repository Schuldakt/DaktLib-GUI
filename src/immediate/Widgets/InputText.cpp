#include "dakt/gui/immediate/Widgets/InputText.hpp"
#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/Widgets/Text.hpp"
#include "dakt/gui/immediate/internal/WidgetBase.hpp"

#include <cstring>

namespace dakt::gui {

    bool inputText(const char* label, char* buf, size_t bufSize, InputTextFlags flags) {
        auto w = widgetSetup();
        if (!w) return false;

        ID id = getID(label);

        Vec2 pos = getCursorPos();
        Vec2 windowPos = getWindowPos();
        Vec2 inputPos = windowPos + pos;

        float labelWidth = static_cast<float>(strlen(label)) * 8.0f + 8.0f;
        float inputWidth = 200.0f;
        float inputHeight = 22.0f;

        Vec2 boxPos = Vec2(inputPos.x + labelWidth, inputPos.y);
        Rect boxRect(boxPos.x, boxPos.y, inputWidth, inputHeight);

        bool hovered = isMouseHoveringRect(boxPos, boxPos + Vec2(inputWidth, inputHeight));

        w.dl->drawText(inputPos, label, w.colors->textPrimary);

        Color bgColor = hovered ? w.colors->surfaceVariant : w.colors->surface;
        w.dl->drawRectFilledRounded(boxRect, bgColor, 2.0f);
        w.dl->drawRectRounded(boxRect, w.colors->border, 2.0f);

        w.dl->drawText(Vec2(boxPos.x + 4, boxPos.y + 3), buf, w.colors->textPrimary);

        w.state->lastItemId = id;
        w.state->lastItemHovered = hovered;

        setCursorPos(Vec2(pos.x, pos.y + inputHeight + 4.0f));

        return false;
    }

    bool inputTextMultiline(const char* label, char* buf, size_t bufSize, Vec2 size, InputTextFlags flags) {
        return inputText(label, buf, bufSize, flags);
    }

    bool inputInt(const char* label, int* value, int step, int stepFast) {
        text("%s: %d", label, *value);
        return false;
    }

    bool inputFloat(const char* label, float* value, float step, float stepFast, const char* format) {
        char buf[64];
        snprintf(buf, sizeof(buf), format, *value);
        text("%s: %s", label, buf);
        return false;
    }

} // namespace dakt::gui
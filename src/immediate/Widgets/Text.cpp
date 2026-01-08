#include "dakt/gui/immediate/Widgets/Text.hpp"
#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/WidgetBase.hpp"

#include <cstring>

namespace dakt::gui {

    void text(const char* fmt, ...) {
        auto w = widgetSetup();
        if (!w) return;

        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        Vec2 pos = getCursorPos();
        Vec2 windowPos = getWindowPos();
        Vec2 textPos = windowPos + pos;

        w.dl->drawText(textPos, buf, w.colors->textPrimary);

        float textHeight = 16.0f;
        float textWidth = static_cast<float>(strlen(buf)) * 8.0f;
        w.state->lastItemRect = Rect(textPos.x, textPos.y, textWidth, textHeight);

        setCursorPos(Vec2(pos.x, pos.y + textHeight + 4.0f));
    }

    void textColored(Color color, const char* fmt, ...) {
        auto w = widgetSetup();
        if (!w) return;

        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        Vec2 pos = getCursorPos();
        Vec2 windowPos = getWindowPos();
        Vec2 textPos = windowPos + pos;

        w.dl->drawText(textPos, buf, color);

        float textHeight = 16.0f;
        w.state->lastItemRect = Rect(textPos.x, textPos.y, static_cast<float>(strlen(buf)) * 8.0f, textHeight);
        setCursorPos(Vec2(pos.x, pos.y + textHeight + 4.0f));
    }

    void textDisabled(const char* fmt, ...) {
        auto w = widgetSetup();
        if (!w) return;

        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        Vec2 pos = getCursorPos();
        Vec2 windowPos = getWindowPos();
        Vec2 textPos = windowPos + pos;

        w.dl->drawText(textPos, buf, w.colors->textDisabled);

        float textHeight = 16.0f;
        setCursorPos(Vec2(pos.x, pos.y + textHeight + 4.0f));
    }

    void textWrapped(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        char buf[1024];
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        text("%s", buf);
    }

    void labelText(const char* label, const char* fmt, ...) {
        auto w = widgetSetup();
        if (!w) return;

        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        Vec2 pos = getCursorPos();
        Vec2 windowPos = getWindowPos();
        Vec2 textPos = windowPos + pos;

        w.dl->drawText(textPos, label, w.colors->textSecondary);

        float labelWidth = static_cast<float>(strlen(label)) * 8.0f + 8.0f;

        w.dl->drawText(Vec2(textPos.x + labelWidth, textPos.y), buf, w.colors->textPrimary);

        float textHeight = 16.0f;

        setCursorPos(Vec2(pos.x, pos.y + textHeight + 4.0f));
    }
} // namespace dakt::gui

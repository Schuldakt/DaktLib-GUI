#include "dakt/gui/immediate/Widgets/Text.hpp"

#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"

#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"

#include <cstring>
#include <cstdarg>

namespace dakt::gui {

    void text(const char* fmt, ...) {
        Context* ctx = getCurrentContext();
        if (!ctx || !fmt) return;

        ImmediateState& s = getState();
        if (!s.currentWindow) return;
        if (s.currentWindow->skipItems) return;

        WindowState* win = s.currentWindow;

        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        const float textHeight = 16.0f;
        const float textWidth = static_cast<float>(strlen(buf)) * 8.0f;
        const float spacingY = 4.0f;

        Vec2 pos = win->cursorPos;
        Rect bb(pos.x, pos.y, textWidth, textHeight);

        win->cursorPos.x = win->cursorStartPos.x;
        win->cursorPos.y += textHeight + spacingY;

        s.lastItemRect = bb;

        DrawList* dl = getWindowDrawList();
        if (dl) {
            dl->drawText(pos, buf, Color::fromFloats(1.0f, 1.0f, 1.0f, 1.0f));
        }
    }

    void textColored(Color color, const char* fmt, ...) {
        Context* ctx = getCurrentContext();
        if (!ctx || !fmt) return;

        ImmediateState& s = getState();
        if (!s.currentWindow) return;
        if (s.currentWindow->skipItems) return;

        WindowState* win = s.currentWindow;

        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        const float textHeight = 16.0f;
        const float textWidth = static_cast<float>(strlen(buf)) * 8.0f;
        const float spacingY = 4.0f;

        Vec2 pos = win->cursorPos;
        Rect bb(pos.x, pos.y, textWidth, textHeight);

        win->cursorPos.x = win->cursorStartPos.x;
        win->cursorPos.y += textHeight + spacingY;

        s.lastItemRect = bb;

        DrawList* dl = getWindowDrawList();
        if (dl) {
            dl->drawText(pos, buf, color);
        }
    }

    void textDisabled(const char* fmt, ...) {
        Context* ctx = getCurrentContext();
        if (!ctx || !fmt) return;

        ImmediateState& s = getState();
        if (!s.currentWindow) return;
        if (s.currentWindow->skipItems) return;

        WindowState* win = s.currentWindow;

        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        const float textHeight = 16.0f;
        const float spacingY = 4.0f;

        Vec2 pos = win->cursorPos;

        win->cursorPos.x = win->cursorStartPos.x;
        win->cursorPos.y += textHeight + spacingY;

        DrawList* dl = getWindowDrawList();
        if (dl) {
            dl->drawText(pos, buf, Color::fromFloats(0.5f, 0.5f, 0.5f, 1.0f));
        }
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
        Context* ctx = getCurrentContext();
        if (!ctx || !label || !fmt) return;

        ImmediateState& s = getState();
        if (!s.currentWindow) return;
        if (s.currentWindow->skipItems) return;

        WindowState* win = s.currentWindow;

        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        const float textHeight = 16.0f;
        const float spacingY = 4.0f;
        const float labelWidth = static_cast<float>(strlen(label)) * 8.0f + 8.0f;

        Vec2 pos = win->cursorPos;

        win->cursorPos.x = win->cursorStartPos.x;
        win->cursorPos.y += textHeight + spacingY;

        DrawList* dl = getWindowDrawList();
        if (dl) {
            dl->drawText(pos, label, Color::fromFloats(0.7f, 0.7f, 0.7f, 1.0f));
            dl->drawText(Vec2(pos.x + labelWidth, pos.y), buf, Color::fromFloats(1.0f, 1.0f, 1.0f, 1.0f));
        }
    }
} // namespace dakt::gui

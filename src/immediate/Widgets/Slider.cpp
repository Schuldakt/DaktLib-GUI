#include "dakt/gui/immediate/Widgets/Slider.hpp"

#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/immediate/Widgets/Text.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"

#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"

#include <algorithm>
#include <cstring>
#include <cstdio>

namespace dakt::gui {

    static bool pointInRect(const Vec2& p, const Rect& r) {
        return p.x >= r.x && p.y >= r.y && p.x <= (r.x + r.width) && p.y < (r.y + r.height);
    }

    bool sliderFloat(const char* label, float* value, float min, float max, const char* format) {
        Context* ctx = getCurrentContext();
        if (!ctx || !label || !value) return false;

        ImmediateState& s = getState();
        if (!s.currentWindow) return false;
        if (s.currentWindow->skipItems) return false;

        WindowState* win = s.currentWindow;

        ID id = getID(label);

        const float labelWidth = static_cast<float>(strlen(label)) * 8.0f + 8.0f;
        const float sliderWidth = 150.0f;
        const float sliderHeight = 18.0f;
        const float spacingY = 4.0f;

        Vec2 pos = win->cursorPos;
        Vec2 trackPos(pos.x + labelWidth, pos.y);
        Rect trackRect(trackPos.x, trackPos.y, sliderWidth, sliderHeight);
        Rect bb(pos.x, pos.y, labelWidth + sliderWidth, sliderHeight);

        win->cursorPos.x = win->cursorStartPos.x;
        win->cursorPos.y += sliderHeight + spacingY;

        Vec2 mousePos = getMousePos();
        bool hovered = pointInRect(mousePos, trackRect);

        if (hovered) s.hotId = id;

        bool active = (s.activeId == id);
        bool changed = false;

        if (hovered && isMouseClicked(MouseButton::Left)) {
            s.activeId = id;
            active = true;
        }

        if (active && isMouseDown(MouseButton::Left)) {
            float t = (mousePos.x - trackPos.x) / sliderWidth;
            t = std::clamp(t, 0.0f, 1.0f);
            float newValue = min + t * (max - min);
            if (newValue != *value) {
                *value = newValue;
                changed = true;
            }
        }

        if (isMouseReleased(MouseButton::Left) && s.activeId == id) {
            s.activeId = 0;
            active = false;
        }

        s.lastItemId = id;
        s.lastItemRect = bb;
        s.lastItemHovered = hovered;
        s.lastItemActive = active;
        s.lastItemClicked = changed;

        DrawList* dl = getWindowDrawList();
        if (dl) {
            dl->drawText(pos, label, Color::fromFloats(1.0f, 1.0f, 1.0f, 1.0f));

            Color trackColor = Color::fromFloats(0.20f, 0.20f, 0.20f, 1.0f);
            dl->drawRectFilledRounded(trackRect, trackColor, 2.0f);

            float t = (*value - min) / (max - min);
            float fillWidth = sliderWidth * t;
            Rect fillRect(trackPos.x, trackPos.y, fillWidth, sliderHeight);
            Color fillColor = Color::fromFloats(0.30f, 0.50f, 0.80f, 1.0f);
            dl->drawRectFilledRounded(fillRect, fillColor, 2.0f);

            dl->drawRectRounded(trackRect, Color::fromFloats(0.30f, 0.30f, 0.30f, 1.0f), 2.0f);

            char valueBuf[64];
            snprintf(valueBuf, sizeof(valueBuf), format, *value);
            float valueWidth = static_cast<float>(strlen(valueBuf)) * 8.0f;
            Vec2 valuePos(trackPos.x + (sliderWidth - valueWidth) * 0.5f, trackPos.y + 1);
            dl->drawText(valuePos, valueBuf, Color::fromFloats(1.0f, 1.0f, 1.0f, 1.0f));
        }

        return changed;
    }

    bool sliderInt(const char* label, int* value, int min, int max, const char* format) {
        float fValue = static_cast<float>(*value);
        bool changed = sliderFloat(label, &fValue, static_cast<float>(min), static_cast<float>(max), format);
        if (changed) {
            *value = static_cast<int>(fValue);
        }
        return changed;
    }

    bool sliderVec2(const char* label, Vec2* value, float min, float max, const char* format) {
        pushID(label);
        text("%s", label);
        bool changed = false;
        changed |= sliderFloat("X", &value->x, min, max, format);
        changed |= sliderFloat("Y", &value->y, min, max, format);
        popID();
        return changed;
    }

    bool dragInt(const char* label, int* value, float speed, int min, int max) {
        return sliderInt(
            label,
            value,
            min == 0 && max == 0 ? -1000 : min,
            max == 0 ? 1000 : max
        );
    }

    bool dragFloat(const char* label, float* value, float speed, float min, float max, const char* format) {
        return sliderFloat(
            label,
            value,
            min == 0.0f && max == 0.0f ? -1000.0f : min,
            max == 0.0f ? 1000.0f : max,
            format
        );
    }
} // namespace dakt::gui
#include "dakt/gui/immediate/Widgets/Slider.hpp"
#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/Widgets/Text.hpp"
#include "dakt/gui/immediate/internal/WidgetBase.hpp"

#include <algorithm>
#include <cstring>

namespace dakt::gui {

    bool sliderFloat(const char* label, float* value, float min, float max, const char* format) {
        auto w = widgetSetup();
        if (!w) return false;

        ID id = getID(label);

        Vec2 pos = getCursorPos();
        Vec2 windowPos = getWindowPos();
        Vec2 sliderPos = windowPos + pos;

        float labelWidth = static_cast<float>(strlen(label)) * 8.0f + 8.0f;
        float sliderWidth = 150.0f;
        float sliderHeight = 18.0f;

        Vec2 trackPos = Vec2(sliderPos.x + labelWidth, sliderPos.y);
        Rect trackRect(trackPos.x, trackPos.y, sliderWidth, sliderHeight);

        bool hovered = isMouseHoveringRect(trackPos, trackPos + Vec2(sliderWidth, sliderHeight));
        bool dragging = hovered && isMouseDown(MouseButton::Left);
        bool changed = false;

        if (dragging) {
            float mouseX = getMousePos().x;
            float t = (mouseX - trackPos.x) / sliderWidth;
            t = std::clamp(t, 0.0f, 1.0f);
            float newValue = min + t * (max - min);
            if (newValue != *value) {
                *value = newValue;
                changed = true;
            }
        }

        w.dl->drawText(sliderPos, label, w.colors->textPrimary);

        w.dl->drawRectFilledRounded(trackRect, w.colors->surface, 2.0f);

        float t = (*value - min) / (max - min);
        float fillWidth = sliderWidth * t;
        Rect fillRect(trackPos.x, trackPos.y, fillWidth, sliderHeight);
        w.dl->drawRectFilledRounded(fillRect, w.colors->primary, 2.0f);

        w.dl->drawRectRounded(trackRect, w.colors->border, 2.0f);

        char valueBuf[64];
        snprintf(valueBuf, sizeof(valueBuf), format, *value);
        float valueWidth = static_cast<float>(strlen(valueBuf)) * 8.0f;
        Vec2 valuePos = trackPos + Vec2((sliderWidth - valueWidth) * 0.5f, 1);
        w.dl->drawText(valuePos, valueBuf, w.colors->textPrimary);

        w.state->lastItemRect = trackRect;
        w.state->lastItemEdited = changed;
        updateItemState(id, hovered, dragging);

        setCursorPos(Vec2(pos.x, pos.y + sliderHeight + 4.0f));

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
            min == 0 && max == 0 ? -1000.0f : min,
            max == 0 ? 1000.0f : max,
            format
        );
    }
} // namespace dakt::gui
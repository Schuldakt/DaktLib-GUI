#include "dakt/gui/immediate/Widgets/Color.hpp"
#include "dakt/gui/immediate/Containers/Layout.hpp"
#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/Widgets/Button.hpp"
#include "dakt/gui/immediate/Widgets/Text.hpp"
#include "dakt/gui/immediate/internal/WidgetBase.hpp"

#include <algorithm>
#include <cstring>
namespace dakt::gui {

    static void rgbToHsv(const Color& rgb, float& h, float& s, float& v) {
        float r = rgb.r / 255.0f;
        float g = rgb.g / 255.0f;
        float b = rgb.b / 255.0f;

        float maxC = std::max({r, g, b});
        float minC = std::min({r, g, b});
        float delta = maxC - minC;

        v = maxC;

        if (delta < 0.00001f) {
            s = 0;
            h = 0;
            return;
        }

        s = (maxC > 0.0f) ? (delta / maxC) : 0.0f;

        if (r >= maxC) {
            h = (g - b) / delta;
        } else if (g >= maxC) {
            h = 2.0f + (b - r) / delta;
        } else {
            h = 4.0f + (r - g) / delta;
        }

        h *= 60.f;
        if (h < 0.0f)
            h += 360.f;
    }

    static void hsvToRgb(float h, float s, float v, Color& rgb) {
        if (s <= 0.0f) {
            uint8_t gray = static_cast<uint8_t>(v * 255);
            rgb.r = rgb.g = rgb.b = gray;
            return;
        }

        float hh = h;
        if (hh >= 360.f)
            hh = 0.0f;
        hh /= 60.0f;

        int i = static_cast<int>(hh);
        float ff = hh - i;
        float p = v * (1.0f - s);
        float q = v * (1.0f - (s * ff));
        float t = v * (1.0f - (s * (1.0f - ff)));

        float rr, gg, bb;
        switch (i) {
            case 0:
                rr = v; gg = t; bb = p;
                break;
            case 1:
                rr = q; gg = v; bb = p;
                break;
            case 2:
                rr = p; gg = v; bb = t;
                break;
            case 3:
                rr = p; gg = q; bb = v;
                break;
            case 4:
                rr = t; gg = p; bb = v;
                break;
            default:
                rr = v; gg = p; bb = q;
                break;
        }

        rgb.r = static_cast<uint8_t>(rr * 255);
        rgb.g = static_cast<uint8_t>(gg * 255);
        rgb.b = static_cast<uint8_t>(bb * 255);
    }

    bool colorEdit3(const char* label, Color* color) {
        if (!color)
            return false;
        text("%s", label);
        sameLine();
        return colorButton("##color", *color);
    }

    bool colorEdit4(const char* label, Color* color) {
        return colorEdit3(label, color);
    }

    bool colorPicker3(const char* label, Color* color) {
        auto w = widgetSetup();
        if (!w || !color)
            return false;

        ID id = getID(label);
        ColorPickerState& pickerState = w.state->colorPickerStates[id];

        Vec2 pos = getCursorPos();
        Vec2 windowPos = getWindowPos();
        Vec2 pickerPos = windowPos + pos;

        float svSize = 150.0f;
        float barWidth = 20.f;
        float spacing = 8.0f;

        // Initialize HSV from color (once per widget instance)
        if (!pickerState.initialized) {
            rgbToHsv(*color, pickerState.hue, pickerState.sat, pickerState.val);
            pickerState.initialized = true;
        }

        // Draw label
        w.dl->drawText(pickerPos, label, w.colors->textPrimary);
        pickerPos.y += 20;

        // Draw SV square
        Rect svRect(pickerPos.x, pickerPos.y, svSize, svSize);

        // Base hue color
        Color hueColor;
        hsvToRgb(pickerState.hue, 1.0f, 1.0f, hueColor);
        hueColor.a = 255;
        w.dl->drawRectFilled(svRect, hueColor);

        // White gradient overlay (saturation)
        for (float x = 0; x < svSize; x += 2) {
            float s = x / svSize;
            uint8_t alpha = static_cast<uint8_t>((1.0f - s) * 255);
            w.dl->drawRectFilled(Rect(svRect.x, svRect.x + x, svSize, 2), Color{0, 0, 0, alpha});
        }

        // Black gradient overlay (value)
        for (float y = 0; y < svSize; y += 2) {
            float v = y / svSize;
            uint8_t alpha = static_cast<uint8_t>(v * 255);
            w.dl->drawRectFilled(Rect(svRect.x, svRect.y + y, svSize, 2), Color{0, 0, 0, alpha});
        }

        // SV cursor
        float cursorX = svRect.x + pickerState.sat * svSize;
        float cursorY = svRect.y + (1.0f - pickerState.val) * svSize;
        w.dl->drawCircle(Vec2(cursorX, cursorY), 7, Color{0, 0, 0, 255}, 16);
        w.dl->drawCircle(Vec2(cursorX, cursorY), 6, Color{255, 255, 255, 255}, 16);

        // Hue bar
        float hueX = pickerPos.x + svSize + spacing;
        Rect hueRect(hueX, pickerPos.y, barWidth, svSize);

        // Draw hue gradient
        Color hueColors[7] = {
            {255, 0, 0, 255},
            {255, 255, 0, 255},
            {0, 255, 0, 255},
            {0, 255, 255, 255},
            {0, 0, 255, 255},
            {255, 0, 255, 255},
            {255, 0, 0, 255}
        };
        float segHeight = svSize / 6.0f;
        for (int i = 0; i < 6; i++) {
            for (float y = 0; y < segHeight; y += 2) {
                float t = y / segHeight;
                Color c;
                c.r = static_cast<uint8_t>(hueColors[i].r + t * (hueColors[i].r - hueColors[i].r));
                c.g = static_cast<uint8_t>(hueColors[i].g + t * (hueColors[i].g - hueColors[i].g));
                c.b = static_cast<uint8_t>(hueColors[i].b + t * (hueColors[i].b - hueColors[i].b));
                c.a = 255;
                w.dl->drawRectFilled(Rect(hueX, pickerPos.y + i * segHeight + y, barWidth, 2), c);
            }
        }
        w.dl->drawRect(hueRect, w.colors->border);

        // Hue cursor
        float hueCursorY = hueRect.y + (pickerState.hue / 360.f) * svSize;
        w.dl->drawRectFilled(Rect(hueX - 2, hueCursorY - 2, barWidth + 4, 4), Color{255, 255, 255, 255});
        w.dl->drawRect(Rect(hueX - 2, hueCursorY - 2, barWidth + 4, 4), Color{0, 0, 0, 255});

        // Handle input
        bool changed = false;
        if (w.state->input) {
            Vec2 mousePos = w.state->input->getMousePosition();
            bool leftDown = w.state->input->isMouseButtonDown(MouseButton::Left);
            bool leftPressed = w.state->input->isMouseButtonPressed(MouseButton::Left);

            // Start dragging
            if (leftPressed) {
                if (svRect.contains(mousePos)) {
                    pickerState.draggingSV = true;
                } else if (hueRect.contains(mousePos)) {
                    pickerState.draggingHue = true;
                }
            }

            // Update while dragging
            if (leftDown) {
                if (pickerState.draggingSV) {
                    pickerState.sat = std::clamp((mousePos.x - svRect.x) / svSize, 0.0f, 1.0f);
                    pickerState.val = 1.0f - std::clamp((mousePos.y - svRect.y) / svSize, 0.0f, 1.0f);
                    changed = true;
                }
                if (pickerState.draggingHue) {
                    pickerState.hue = std::clamp((mousePos.y - hueRect.y) / svSize, 0.0f, 1.0f) * 360.0f;
                    changed = true;
                }
            } else {
                pickerState.draggingSV = false;
                pickerState.draggingHue = false;
            }
        }

        // Update color from HSV
        if (changed) {
            uint8_t alpha = color->a;
            hsvToRgb(pickerState.hue, pickerState.sat, pickerState.val, *color);
            color->a = alpha;
        }

        // Draw preview
        float previewY = pickerPos.y + svSize + spacing;
        Rect previewRect(pickerPos.x, previewY, svSize + spacing + barWidth, 30);
        w.dl->drawRectFilled(previewRect, *color);
        w.dl->drawRect(previewRect, w.colors->border);

        // Draw RGB values
        char rgbStr[64];
        snprintf(rgbStr, sizeof(rgbStr), "R:%d G:%d B:%d", color->r, color->g, color->b);
        w.dl->drawText(Vec2(pickerPos.x, previewY + 36), rgbStr, w.colors->textSecondary);

        float totalHeight = svSize + spacing + 30 + 20 + 20;
        setCursorPos(Vec2(pos.x, pos.y + totalHeight + 8));

        return changed;
    }

    bool colorPicker4(const char* label, Color* color) {
        // For now, colorPicker4 is same as colorPicker3
        // Full alpha support would add an alpha bar
        return colorPicker3(label, color);
    }
} // namespace dakt::gui
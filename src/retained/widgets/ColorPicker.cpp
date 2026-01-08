#include "dakt/gui/retained/widgets/ColorPicker.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace dakt::gui {

ColorPicker::ColorPicker() : Widget() {
    setPreferredSize(Vec2(220, 280));
    updateHSVFromColor();
}

ColorPicker::ColorPicker(const Color& initialColor) : Widget(), color_(initialColor) {
    setPreferredSize(Vec2(220, 280));
    updateHSVFromColor();
}

void ColorPicker::setColor(const Color& color) {
    if (color_.r != color.r || color_.g != color.g || color_.b != color.b || color_.a != color.a) {
        color_ = color;
        updateHSVFromColor();
        markDirty();
    }
}

void ColorPicker::setHSV(float h, float s, float v) {
    hue_ = std::max(0.0f, std::min(360.0f, h));
    saturation_ = std::max(0.0f, std::min(1.0f, s));
    value_ = std::max(0.0f, std::min(1.0f, v));
    updateColorFromHSV();
    markDirty();
}

void ColorPicker::rgbToHsv(const Color& rgb, float& h, float& s, float& v) {
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

    h *= 60.0f;
    if (h < 0.0f)
        h += 360.0f;
}

void ColorPicker::hsvToRgb(float h, float s, float v, Color& rgb) {
    if (s <= 0.0f) {
        uint8_t gray = static_cast<uint8_t>(v * 255);
        rgb.r = rgb.g = rgb.b = gray;
        return;
    }

    float hh = h;
    if (hh >= 360.0f)
        hh = 0.0f;
    hh /= 60.0f;

    int i = static_cast<int>(hh);
    float ff = hh - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - (s * ff));
    float t = v * (1.0f - (s * (1.0f - ff)));

    float r, g, b;
    switch (i) {
    case 0:
        r = v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = v;
        b = p;
        break;
    case 2:
        r = p;
        g = v;
        b = t;
        break;
    case 3:
        r = p;
        g = q;
        b = v;
        break;
    case 4:
        r = t;
        g = p;
        b = v;
        break;
    default:
        r = v;
        g = p;
        b = q;
        break;
    }

    rgb.r = static_cast<uint8_t>(r * 255);
    rgb.g = static_cast<uint8_t>(g * 255);
    rgb.b = static_cast<uint8_t>(b * 255);
}

void ColorPicker::updateColorFromHSV() {
    uint8_t alpha = color_.a;
    hsvToRgb(hue_, saturation_, value_, color_);
    color_.a = alpha;
}

void ColorPicker::updateHSVFromColor() { rgbToHsv(color_, hue_, saturation_, value_); }

bool ColorPicker::isInSVSquare(const Vec2& pos) const { return pos.x >= svSquareRect_.x && pos.x <= svSquareRect_.x + svSquareRect_.width && pos.y >= svSquareRect_.y && pos.y <= svSquareRect_.y + svSquareRect_.height; }

bool ColorPicker::isInHueBar(const Vec2& pos) const { return pos.x >= hueBarRect_.x && pos.x <= hueBarRect_.x + hueBarRect_.width && pos.y >= hueBarRect_.y && pos.y <= hueBarRect_.y + hueBarRect_.height; }

bool ColorPicker::isInAlphaBar(const Vec2& pos) const { return pos.x >= alphaBarRect_.x && pos.x <= alphaBarRect_.x + alphaBarRect_.width && pos.y >= alphaBarRect_.y && pos.y <= alphaBarRect_.y + alphaBarRect_.height; }

Vec2 ColorPicker::measureContent() {
    float width = svSquareSize_ + barSpacing_ + barWidth_;
    if (showAlpha_)
        width += barSpacing_ + barWidth_;

    float height = svSquareSize_;
    if (showPreview_)
        height += barSpacing_ + previewHeight_;
    if (showInputFields_)
        height += barSpacing_ + 24.0f;
    if (showHexInput_)
        height += barSpacing_ + 24.0f;

    return Vec2(width + padding_.left + padding_.right, height + padding_.top + padding_.bottom);
}

bool ColorPicker::handleInput(const WidgetEvent& event) {
    if (isDisabled())
        return false;

    switch (event.type) {
    case WidgetEventType::Press:
    case WidgetEventType::DragStart:
        if (isInSVSquare(event.mousePos)) {
            draggingSV_ = true;
            addFlag(RetainedWidgetFlags::Active);
        } else if (isInHueBar(event.mousePos)) {
            draggingHue_ = true;
            addFlag(RetainedWidgetFlags::Active);
        } else if (showAlpha_ && isInAlphaBar(event.mousePos)) {
            draggingAlpha_ = true;
            addFlag(RetainedWidgetFlags::Active);
        }
        [[fallthrough]];

    case WidgetEventType::DragMove: {
        bool changed = false;

        if (draggingSV_) {
            float s = (event.mousePos.x - svSquareRect_.x) / svSquareRect_.width;
            float v = 1.0f - (event.mousePos.y - svSquareRect_.y) / svSquareRect_.height;
            saturation_ = std::max(0.0f, std::min(1.0f, s));
            value_ = std::max(0.0f, std::min(1.0f, v));
            updateColorFromHSV();
            changed = true;
        }

        if (draggingHue_) {
            float h = (event.mousePos.y - hueBarRect_.y) / hueBarRect_.height * 360.0f;
            hue_ = std::max(0.0f, std::min(360.0f, h));
            updateColorFromHSV();
            changed = true;
        }

        if (draggingAlpha_) {
            float a = 1.0f - (event.mousePos.y - alphaBarRect_.y) / alphaBarRect_.height;
            color_.a = static_cast<uint8_t>(std::max(0.0f, std::min(1.0f, a)) * 255);
            changed = true;
        }

        if (changed) {
            markDirty();
            if (onColorChanged_) {
                WidgetEvent e = event;
                e.source = this;
                e.type = WidgetEventType::ValueChanged;
                onColorChanged_(e);
            }
        }
        return changed;
    }

    case WidgetEventType::Release:
    case WidgetEventType::DragEnd:
        draggingSV_ = false;
        draggingHue_ = false;
        draggingAlpha_ = false;
        removeFlag(RetainedWidgetFlags::Active);
        return true;

    default:
        break;
    }

    return Widget::handleInput(event);
}

void ColorPicker::drawSVSquare(DrawList& drawList, const Rect& rect) {
    // Draw saturation-value gradient
    // First fill with the hue color
    Color hueColor;
    hsvToRgb(hue_, 1.0f, 1.0f, hueColor);
    hueColor.a = 255;
    drawList.drawRectFilled(rect, hueColor);

    // Draw white gradient (left to right for saturation)
    for (float x = 0; x < rect.width; x += 2) {
        float s = x / rect.width;
        uint8_t alpha = static_cast<uint8_t>((1.0f - s) * 255);
        Color white{255, 255, 255, alpha};
        drawList.drawRectFilled(Rect(rect.x + x, rect.y, 2, rect.height), white);
    }

    // Draw black gradient (top to bottom for value)
    for (float y = 0; y < rect.height; y += 2) {
        float v = y / rect.height;
        uint8_t alpha = static_cast<uint8_t>(v * 255);
        Color black{0, 0, 0, alpha};
        drawList.drawRectFilled(Rect(rect.x, rect.y + y, rect.width, 2), black);
    }

    // Draw cursor
    float cursorX = rect.x + saturation_ * rect.width;
    float cursorY = rect.y + (1.0f - value_) * rect.height;
    float cursorRadius = 6.0f;

    drawList.drawCircle(Vec2(cursorX, cursorY), cursorRadius + 1, Color{0, 0, 0, 255}, 16);
    drawList.drawCircle(Vec2(cursorX, cursorY), cursorRadius, Color{255, 255, 255, 255}, 16);
}

void ColorPicker::drawHueBar(DrawList& drawList, const Rect& rect) {
    // Draw hue gradient
    float segmentHeight = rect.height / 6.0f;
    Color hueColors[7] = {
        {255, 0, 0, 255},   // Red
        {255, 255, 0, 255}, // Yellow
        {0, 255, 0, 255},   // Green
        {0, 255, 255, 255}, // Cyan
        {0, 0, 255, 255},   // Blue
        {255, 0, 255, 255}, // Magenta
        {255, 0, 0, 255}    // Red (wrap)
    };

    for (int i = 0; i < 6; ++i) {
        float y = rect.y + i * segmentHeight;
        // Draw gradient segment
        for (float dy = 0; dy < segmentHeight; dy += 2) {
            float t = dy / segmentHeight;
            Color c;
            c.r = static_cast<uint8_t>(hueColors[i].r + t * (hueColors[i + 1].r - hueColors[i].r));
            c.g = static_cast<uint8_t>(hueColors[i].g + t * (hueColors[i + 1].g - hueColors[i].g));
            c.b = static_cast<uint8_t>(hueColors[i].b + t * (hueColors[i + 1].b - hueColors[i].b));
            c.a = 255;
            drawList.drawRectFilled(Rect(rect.x, y + dy, rect.width, 2), c);
        }
    }

    // Draw border
    drawList.drawRectRounded(rect, Color{80, 80, 84, 255}, 2.0f);

    // Draw hue cursor
    float cursorY = rect.y + (hue_ / 360.0f) * rect.height;
    drawList.drawRectFilled(Rect(rect.x - 2, cursorY - 2, rect.width + 4, 4), Color{255, 255, 255, 255});
    drawList.drawRect(Rect(rect.x - 2, cursorY - 2, rect.width + 4, 4), Color{0, 0, 0, 255});
}

void ColorPicker::drawAlphaBar(DrawList& drawList, const Rect& rect) {
    // Draw checkerboard pattern for transparency
    float checkSize = 6.0f;
    for (float y = rect.y; y < rect.y + rect.height; y += checkSize) {
        for (float x = rect.x; x < rect.x + rect.width; x += checkSize) {
            int ix = static_cast<int>((x - rect.x) / checkSize);
            int iy = static_cast<int>((y - rect.y) / checkSize);
            Color checkColor = ((ix + iy) % 2 == 0) ? Color{180, 180, 180, 255} : Color{120, 120, 120, 255};
            float w = std::min(checkSize, rect.x + rect.width - x);
            float h = std::min(checkSize, rect.y + rect.height - y);
            drawList.drawRectFilled(Rect(x, y, w, h), checkColor);
        }
    }

    // Draw alpha gradient
    Color solidColor = color_;
    solidColor.a = 255;

    for (float y = 0; y < rect.height; y += 2) {
        float a = 1.0f - y / rect.height;
        Color c = solidColor;
        c.a = static_cast<uint8_t>(a * 255);
        drawList.drawRectFilled(Rect(rect.x, rect.y + y, rect.width, 2), c);
    }

    // Draw border
    drawList.drawRectRounded(rect, Color{80, 80, 84, 255}, 2.0f);

    // Draw alpha cursor
    float cursorY = rect.y + (1.0f - color_.a / 255.0f) * rect.height;
    drawList.drawRectFilled(Rect(rect.x - 2, cursorY - 2, rect.width + 4, 4), Color{255, 255, 255, 255});
    drawList.drawRect(Rect(rect.x - 2, cursorY - 2, rect.width + 4, 4), Color{0, 0, 0, 255});
}

void ColorPicker::drawPreview(DrawList& drawList, const Rect& rect) {
    // Draw checkerboard for transparency
    float checkSize = 8.0f;
    for (float y = rect.y; y < rect.y + rect.height; y += checkSize) {
        for (float x = rect.x; x < rect.x + rect.width; x += checkSize) {
            int ix = static_cast<int>((x - rect.x) / checkSize);
            int iy = static_cast<int>((y - rect.y) / checkSize);
            Color checkColor = ((ix + iy) % 2 == 0) ? Color{180, 180, 180, 255} : Color{120, 120, 120, 255};
            float w = std::min(checkSize, rect.x + rect.width - x);
            float h = std::min(checkSize, rect.y + rect.height - y);
            drawList.drawRectFilled(Rect(x, y, w, h), checkColor);
        }
    }

    // Draw the color
    drawList.drawRectFilled(rect, color_);

    // Draw border
    drawList.drawRectRounded(rect, Color{80, 80, 84, 255}, 2.0f);
}

void ColorPicker::drawColorWheel(DrawList& drawList, const Rect& rect) {
    // Color wheel implementation for Mode::Wheel
    float cx = rect.x + rect.width / 2;
    float cy = rect.y + rect.height / 2;
    float radius = std::min(rect.width, rect.height) / 2 - 4;

    // Draw wheel segments
    int segments = 60;
    for (int i = 0; i < segments; ++i) {
        float angle1 = (2 * M_PI * i) / segments;
        float angle2 = (2 * M_PI * (i + 1)) / segments;
        float hue = (360.0f * i) / segments;

        Color c;
        hsvToRgb(hue, 1.0f, 1.0f, c);
        c.a = 255;

        Vec2 p1(cx, cy);
        Vec2 p2(cx + radius * std::cos(angle1), cy + radius * std::sin(angle1));
        Vec2 p3(cx + radius * std::cos(angle2), cy + radius * std::sin(angle2));

        drawList.drawTriangleFilled(p1, p2, p3, c);
    }
}

void ColorPicker::drawContent(DrawList& drawList) {
    float x = bounds_.x + padding_.left;
    float y = bounds_.y + padding_.top;

    // Update cached rects
    svSquareRect_ = Rect(x, y, svSquareSize_, svSquareSize_);
    hueBarRect_ = Rect(x + svSquareSize_ + barSpacing_, y, barWidth_, svSquareSize_);

    if (showAlpha_) {
        alphaBarRect_ = Rect(hueBarRect_.x + barWidth_ + barSpacing_, y, barWidth_, svSquareSize_);
    }

    // Draw main picker based on mode
    if (mode_ == Mode::Wheel) {
        drawColorWheel(drawList, svSquareRect_);
    } else {
        drawSVSquare(drawList, svSquareRect_);
    }

    // Draw hue bar
    drawHueBar(drawList, hueBarRect_);

    // Draw alpha bar
    if (showAlpha_) {
        drawAlphaBar(drawList, alphaBarRect_);
    }

    y += svSquareSize_ + barSpacing_;

    // Draw preview
    if (showPreview_) {
        float previewWidth = svSquareSize_ + barSpacing_ + barWidth_;
        if (showAlpha_)
            previewWidth += barSpacing_ + barWidth_;
        Rect previewRect(x, y, previewWidth, previewHeight_);
        drawPreview(drawList, previewRect);
        y += previewHeight_ + barSpacing_;
    }

    // Draw hex input display
    if (showHexInput_) {
        char hexStr[16];
        if (showAlpha_) {
            snprintf(hexStr, sizeof(hexStr), "#%02X%02X%02X%02X", color_.r, color_.g, color_.b, color_.a);
        } else {
            snprintf(hexStr, sizeof(hexStr), "#%02X%02X%02X", color_.r, color_.g, color_.b);
        }

        Rect hexRect(x, y, 100, 22);
        drawList.drawRectFilled(hexRect, Color{35, 35, 38, 255});
        drawList.drawRectRounded(hexRect, Color{60, 60, 64, 255}, 2.0f);
        drawList.drawText(Vec2(hexRect.x + 6, hexRect.y + 4), hexStr, Color{200, 200, 200, 255});
        y += 24 + barSpacing_;
    }

    // Draw RGB/HSV input fields
    if (showInputFields_) {
        char rStr[8], gStr[8], bStr[8];
        snprintf(rStr, sizeof(rStr), "R:%d", color_.r);
        snprintf(gStr, sizeof(gStr), "G:%d", color_.g);
        snprintf(bStr, sizeof(bStr), "B:%d", color_.b);

        float fieldWidth = 55;
        float fieldSpacing = 6;

        drawList.drawText(Vec2(x, y + 4), rStr, Color{255, 150, 150, 255});
        drawList.drawText(Vec2(x + fieldWidth + fieldSpacing, y + 4), gStr, Color{150, 255, 150, 255});
        drawList.drawText(Vec2(x + 2 * (fieldWidth + fieldSpacing), y + 4), bStr, Color{150, 150, 255, 255});
    }
}

} // namespace dakt::gui

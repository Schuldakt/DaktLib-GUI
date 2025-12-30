// ============================================================================
// DaktLib GUI Module - Color Widgets Implementation
// ============================================================================

#include <dakt/gui/core/DrawList.hpp>
#include <dakt/gui/framework/Context.hpp>
#include <dakt/gui/framework/Theme.hpp>
#include <dakt/gui/widgets/Color.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace dakt::gui
{

// ============================================================================
// Color Space Conversion
// ============================================================================

void colorConvertRGBtoHSV(f32 r, f32 g, f32 b, f32& h, f32& s, f32& v)
{
    f32 maxVal = std::max({r, g, b});
    f32 minVal = std::min({r, g, b});
    f32 delta = maxVal - minVal;

    v = maxVal;

    if (maxVal > 0.0f)
    {
        s = delta / maxVal;
    }
    else
    {
        s = 0.0f;
        h = 0.0f;
        return;
    }

    if (delta < 0.00001f)
    {
        h = 0.0f;
        return;
    }

    if (r >= maxVal)
    {
        h = (g - b) / delta;
    }
    else if (g >= maxVal)
    {
        h = 2.0f + (b - r) / delta;
    }
    else
    {
        h = 4.0f + (r - g) / delta;
    }

    h /= 6.0f;

    if (h < 0.0f)
    {
        h += 1.0f;
    }
}

void colorConvertHSVtoRGB(f32 h, f32 s, f32 v, f32& r, f32& g, f32& b)
{
    if (s <= 0.0f)
    {
        r = g = b = v;
        return;
    }

    f32 hh = h;
    if (hh >= 1.0f)
        hh = 0.0f;
    hh *= 6.0f;

    i32 i = static_cast<i32>(hh);
    f32 ff = hh - i;
    f32 p = v * (1.0f - s);
    f32 q = v * (1.0f - (s * ff));
    f32 t = v * (1.0f - (s * (1.0f - ff)));

    switch (i)
    {
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
}

u32 colorConvertFloat4ToU32(const f32 in[4])
{
    u32 out;
    out = static_cast<u32>(std::clamp(in[0], 0.0f, 1.0f) * 255.0f + 0.5f);
    out |= static_cast<u32>(std::clamp(in[1], 0.0f, 1.0f) * 255.0f + 0.5f) << 8;
    out |= static_cast<u32>(std::clamp(in[2], 0.0f, 1.0f) * 255.0f + 0.5f) << 16;
    out |= static_cast<u32>(std::clamp(in[3], 0.0f, 1.0f) * 255.0f + 0.5f) << 24;
    return out;
}

void colorConvertU32ToFloat4(u32 in, f32 out[4])
{
    out[0] = static_cast<f32>((in >> 0) & 0xFF) / 255.0f;
    out[1] = static_cast<f32>((in >> 8) & 0xFF) / 255.0f;
    out[2] = static_cast<f32>((in >> 16) & 0xFF) / 255.0f;
    out[3] = static_cast<f32>((in >> 24) & 0xFF) / 255.0f;
}

void colorConvertHSLtoRGB(f32 h, f32 s, f32 l, f32& r, f32& g, f32& b)
{
    if (s <= 0.0f)
    {
        r = g = b = l;
        return;
    }

    auto hueToRgb = [](f32 p, f32 q, f32 t) -> f32
    {
        if (t < 0.0f)
            t += 1.0f;
        if (t > 1.0f)
            t -= 1.0f;
        if (t < 1.0f / 6.0f)
            return p + (q - p) * 6.0f * t;
        if (t < 1.0f / 2.0f)
            return q;
        if (t < 2.0f / 3.0f)
            return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
        return p;
    };

    f32 q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
    f32 p = 2.0f * l - q;

    r = hueToRgb(p, q, h + 1.0f / 3.0f);
    g = hueToRgb(p, q, h);
    b = hueToRgb(p, q, h - 1.0f / 3.0f);
}

void colorConvertRGBtoHSL(f32 r, f32 g, f32 b, f32& h, f32& s, f32& l)
{
    f32 maxVal = std::max({r, g, b});
    f32 minVal = std::min({r, g, b});
    f32 delta = maxVal - minVal;

    l = (maxVal + minVal) * 0.5f;

    if (delta < 0.00001f)
    {
        h = s = 0.0f;
        return;
    }

    s = l > 0.5f ? delta / (2.0f - maxVal - minVal) : delta / (maxVal + minVal);

    if (r >= maxVal)
    {
        h = (g - b) / delta + (g < b ? 6.0f : 0.0f);
    }
    else if (g >= maxVal)
    {
        h = (b - r) / delta + 2.0f;
    }
    else
    {
        h = (r - g) / delta + 4.0f;
    }

    h /= 6.0f;
}

// ============================================================================
// Internal Helpers
// ============================================================================

namespace
{

// Global state for color picker
ColorPickerState g_colorPickerState;

Context* ctx()
{
    return getContext();
}

DrawList& drawList()
{
    return ctx()->getDrawList();
}

Theme* theme()
{
    return ctx()->getTheme();
}

// Draw a checkerboard pattern for transparency preview
void drawCheckerboard(Rect rect, f32 gridSize = 8.0f)
{
    Color dark(102, 102, 102, 255);
    Color light(153, 153, 153, 255);

    f32 x = rect.min.x;
    i32 col = 0;
    while (x < rect.max.x)
    {
        f32 y = rect.min.y;
        i32 row = 0;
        f32 w = std::min(gridSize, rect.max.x - x);

        while (y < rect.max.y)
        {
            f32 h = std::min(gridSize, rect.max.y - y);
            Color c = ((row + col) % 2 == 0) ? dark : light;
            drawList().addRectFilled(Rect{{x, y}, {x + w, y + h}}, c);
            y += gridSize;
            ++row;
        }
        x += gridSize;
        ++col;
    }
}

// Draw the saturation/value picker square
void drawSVPicker(Rect rect, f32 hue, f32& saturation, f32& value, bool& dragging)
{
    auto* c = ctx();

    // Draw white to color gradient (horizontal)
    f32 r, g, b;
    colorConvertHSVtoRGB(hue, 1.0f, 1.0f, r, g, b);
    Color hueColor(static_cast<u8>(r * 255), static_cast<u8>(g * 255), static_cast<u8>(b * 255), 255);

    // White to hue color (horizontal gradient)
    drawList().addRectFilledMultiColor(rect, Color::white(), hueColor, hueColor, Color::white());

    // Black overlay (vertical gradient)
    drawList().addRectFilledMultiColor(rect, Color::transparent(), Color::transparent(), Color::black(),
                                       Color::black());

    // Draw border
    drawList().addRect(rect, theme()->border);

    // Handle interaction
    Vec2 mousePos = c->getMousePos();
    bool hovered = rect.contains(mousePos);

    if (hovered && c->isMouseClicked(MouseButton::Left))
    {
        dragging = true;
    }

    if (dragging)
    {
        if (c->isMouseDown(MouseButton::Left))
        {
            // Clamp mouse position to rect
            f32 mx = std::clamp(mousePos.x, rect.min.x, rect.max.x);
            f32 my = std::clamp(mousePos.y, rect.min.y, rect.max.y);

            saturation = (mx - rect.min.x) / rect.width();
            value = 1.0f - (my - rect.min.y) / rect.height();
        }
        else
        {
            dragging = false;
        }
    }

    // Draw cursor
    f32 cursorX = rect.min.x + saturation * rect.width();
    f32 cursorY = rect.min.y + (1.0f - value) * rect.height();
    Vec2 cursorPos{cursorX, cursorY};

    drawList().addCircle(cursorPos, 6.0f, Color::white(), 12, 2.0f);
    drawList().addCircle(cursorPos, 5.0f, Color::black(), 12, 1.0f);
}

// Draw the hue bar (vertical)
void drawHueBar(Rect rect, f32& hue, bool& dragging)
{
    auto* c = ctx();

    // Draw rainbow gradient
    const i32 numSegments = 6;
    f32 segmentHeight = rect.height() / numSegments;

    for (i32 i = 0; i < numSegments; ++i)
    {
        f32 h1 = static_cast<f32>(i) / numSegments;
        f32 h2 = static_cast<f32>(i + 1) / numSegments;

        f32 r1, g1, b1, r2, g2, b2;
        colorConvertHSVtoRGB(h1, 1.0f, 1.0f, r1, g1, b1);
        colorConvertHSVtoRGB(h2, 1.0f, 1.0f, r2, g2, b2);

        Color c1(static_cast<u8>(r1 * 255), static_cast<u8>(g1 * 255), static_cast<u8>(b1 * 255), 255);
        Color c2(static_cast<u8>(r2 * 255), static_cast<u8>(g2 * 255), static_cast<u8>(b2 * 255), 255);

        Rect segRect{{rect.min.x, rect.min.y + i * segmentHeight}, {rect.max.x, rect.min.y + (i + 1) * segmentHeight}};

        drawList().addRectFilledMultiColor(segRect, c1, c1, c2, c2);
    }

    // Draw border
    drawList().addRect(rect, theme()->border);

    // Handle interaction
    Vec2 mousePos = c->getMousePos();
    bool hovered = rect.contains(mousePos);

    if (hovered && c->isMouseClicked(MouseButton::Left))
    {
        dragging = true;
    }

    if (dragging)
    {
        if (c->isMouseDown(MouseButton::Left))
        {
            f32 my = std::clamp(mousePos.y, rect.min.y, rect.max.y);
            hue = (my - rect.min.y) / rect.height();
        }
        else
        {
            dragging = false;
        }
    }

    // Draw cursor
    f32 cursorY = rect.min.y + hue * rect.height();
    Rect cursorRect{{rect.min.x - 2, cursorY - 2}, {rect.max.x + 2, cursorY + 2}};
    drawList().addRectFilled(cursorRect, Color::white());
    drawList().addRect(cursorRect, Color::black());
}

// Draw the alpha bar (vertical)
void drawAlphaBar(Rect rect, const f32 col[3], f32& alpha, bool& dragging)
{
    auto* c = ctx();

    // Draw checkerboard background
    drawCheckerboard(rect, 4.0f);

    // Draw alpha gradient
    Color c1(static_cast<u8>(col[0] * 255), static_cast<u8>(col[1] * 255), static_cast<u8>(col[2] * 255), 0);
    Color c2(static_cast<u8>(col[0] * 255), static_cast<u8>(col[1] * 255), static_cast<u8>(col[2] * 255), 255);

    drawList().addRectFilledMultiColor(rect, c1, c1, c2, c2);

    // Draw border
    drawList().addRect(rect, theme()->border);

    // Handle interaction
    Vec2 mousePos = c->getMousePos();
    bool hovered = rect.contains(mousePos);

    if (hovered && c->isMouseClicked(MouseButton::Left))
    {
        dragging = true;
    }

    if (dragging)
    {
        if (c->isMouseDown(MouseButton::Left))
        {
            f32 my = std::clamp(mousePos.y, rect.min.y, rect.max.y);
            alpha = (my - rect.min.y) / rect.height();
        }
        else
        {
            dragging = false;
        }
    }

    // Draw cursor
    f32 cursorY = rect.min.y + alpha * rect.height();
    Rect cursorRect{{rect.min.x - 2, cursorY - 2}, {rect.max.x + 2, cursorY + 2}};
    drawList().addRectFilled(cursorRect, Color::white());
    drawList().addRect(cursorRect, Color::black());
}

// Draw color preview square
void drawColorPreview(Rect rect, const f32 col[4], bool showAlpha)
{
    if (showAlpha && col[3] < 1.0f)
    {
        // Draw checkerboard for transparency
        drawCheckerboard(rect, 4.0f);
    }

    // Draw color
    Color c(static_cast<u8>(col[0] * 255), static_cast<u8>(col[1] * 255), static_cast<u8>(col[2] * 255),
            static_cast<u8>(col[3] * 255));

    drawList().addRectFilled(rect, c);
    drawList().addRect(rect, theme()->border);
}

// Draw RGB/HSV input fields
bool drawColorInputs(f32 col[4], bool hasAlpha, ColorEditFlags flags)
{
    auto* c = ctx();
    auto* t = theme();
    bool changed = false;

    bool displayHSV = hasFlag(flags, ColorEditFlags::DisplayHSV);
    bool displayHex = hasFlag(flags, ColorEditFlags::DisplayHex);
    bool uint8Mode = hasFlag(flags, ColorEditFlags::Uint8);

    f32 inputWidth = 50.0f;

    if (displayHex)
    {
        // Hex input
        char hexBuf[16];
        if (hasAlpha)
        {
            std::snprintf(hexBuf, sizeof(hexBuf), "%02X%02X%02X%02X", static_cast<u8>(col[0] * 255),
                          static_cast<u8>(col[1] * 255), static_cast<u8>(col[2] * 255), static_cast<u8>(col[3] * 255));
        }
        else
        {
            std::snprintf(hexBuf, sizeof(hexBuf), "%02X%02X%02X", static_cast<u8>(col[0] * 255),
                          static_cast<u8>(col[1] * 255), static_cast<u8>(col[2] * 255));
        }

        // Draw hex text (simplified - full implementation would have input field)
        Vec2 pos = c->getCursorScreenPos();
        drawList().addText(pos, hexBuf, t->text);
    }
    else if (displayHSV)
    {
        f32 h, s, v;
        colorConvertRGBtoHSV(col[0], col[1], col[2], h, s, v);

        // Draw H, S, V labels and values
        Vec2 pos = c->getCursorScreenPos();
        char buf[32];

        if (uint8Mode)
        {
            std::snprintf(buf, sizeof(buf), "H:%d S:%d V:%d", static_cast<i32>(h * 360), static_cast<i32>(s * 100),
                          static_cast<i32>(v * 100));
        }
        else
        {
            std::snprintf(buf, sizeof(buf), "H:%.2f S:%.2f V:%.2f", h, s, v);
        }
        drawList().addText(pos, buf, t->text);
    }
    else
    {
        // RGB display
        Vec2 pos = c->getCursorScreenPos();
        char buf[64];

        if (uint8Mode)
        {
            if (hasAlpha)
            {
                std::snprintf(buf, sizeof(buf), "R:%d G:%d B:%d A:%d", static_cast<i32>(col[0] * 255),
                              static_cast<i32>(col[1] * 255), static_cast<i32>(col[2] * 255),
                              static_cast<i32>(col[3] * 255));
            }
            else
            {
                std::snprintf(buf, sizeof(buf), "R:%d G:%d B:%d", static_cast<i32>(col[0] * 255),
                              static_cast<i32>(col[1] * 255), static_cast<i32>(col[2] * 255));
            }
        }
        else
        {
            if (hasAlpha)
            {
                std::snprintf(buf, sizeof(buf), "R:%.2f G:%.2f B:%.2f A:%.2f", col[0], col[1], col[2], col[3]);
            }
            else
            {
                std::snprintf(buf, sizeof(buf), "R:%.2f G:%.2f B:%.2f", col[0], col[1], col[2]);
            }
        }
        drawList().addText(pos, buf, t->text);
    }

    return changed;
}

}  // anonymous namespace

// ============================================================================
// Color Edit Widgets
// ============================================================================

bool colorEdit3(StringView label, f32 col[3], ColorEditFlags flags)
{
    f32 col4[4] = {col[0], col[1], col[2], 1.0f};
    bool changed = colorEdit4(label, col4, flags | ColorEditFlags::NoAlpha);
    if (changed)
    {
        col[0] = col4[0];
        col[1] = col4[1];
        col[2] = col4[2];
    }
    return changed;
}

bool colorEdit4(StringView label, f32 col[4], ColorEditFlags flags)
{
    auto* c = ctx();
    if (!c)
        return false;

    auto* t = theme();
    bool changed = false;
    bool hasAlpha = !hasFlag(flags, ColorEditFlags::NoAlpha);

    Vec2 pos = c->getCursorScreenPos();
    f32 previewSize = 20.0f;
    f32 spacing = 4.0f;

    // Draw color preview button
    Rect previewRect{pos, pos + Vec2{previewSize, previewSize}};

    if (!hasFlag(flags, ColorEditFlags::NoSmallPreview))
    {
        drawColorPreview(previewRect, col, hasAlpha);

        // Check for click to open picker
        Vec2 mousePos = c->getMousePos();
        if (previewRect.contains(mousePos) && c->isMouseClicked(MouseButton::Left))
        {
            if (!hasFlag(flags, ColorEditFlags::NoPicker))
            {
                // TODO: Open popup picker
            }
        }

        pos.x += previewSize + spacing;
    }

    // Draw input fields
    if (!hasFlag(flags, ColorEditFlags::NoInputs))
    {
        c->setCursorScreenPos(pos);
        changed = drawColorInputs(col, hasAlpha, flags);
    }

    // Draw label
    if (!hasFlag(flags, ColorEditFlags::NoLabel))
    {
        c->sameLine();
        c->text(label);
    }

    // Advance cursor
    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(Vec2{200, previewSize});
    }

    return changed;
}

bool colorPicker3(StringView label, f32 col[3], ColorEditFlags flags)
{
    f32 col4[4] = {col[0], col[1], col[2], 1.0f};
    bool changed = colorPicker4(label, col4, flags | ColorEditFlags::NoAlpha);
    if (changed)
    {
        col[0] = col4[0];
        col[1] = col4[1];
        col[2] = col4[2];
    }
    return changed;
}

bool colorPicker4(StringView label, f32 col[4], ColorEditFlags flags)
{
    return colorPickerAdvanced(label, col, flags, nullptr);
}

bool colorPickerAdvanced(StringView strId, f32 col[4], ColorEditFlags flags, const f32* refCol)
{
    auto* c = ctx();
    if (!c)
        return false;

    auto* t = theme();
    bool changed = false;
    bool hasAlpha = !hasFlag(flags, ColorEditFlags::NoAlpha);

    // Dimensions
    const f32 pickerSize = 180.0f;
    const f32 barWidth = 20.0f;
    const f32 spacing = 8.0f;
    const f32 previewHeight = 40.0f;

    Vec2 pos = c->getCursorScreenPos();

    // Convert current color to HSV
    f32 h, s, v;
    colorConvertRGBtoHSV(col[0], col[1], col[2], h, s, v);

    // Store in state if not already dragging
    if (!g_colorPickerState.draggingSV && !g_colorPickerState.draggingHue && !g_colorPickerState.draggingAlpha)
    {
        g_colorPickerState.hue = h;
        g_colorPickerState.saturation = s;
        g_colorPickerState.value = v;
        g_colorPickerState.alpha = col[3];
    }

    // Draw SV picker
    Rect svRect{pos, pos + Vec2{pickerSize, pickerSize}};
    drawSVPicker(svRect, g_colorPickerState.hue, g_colorPickerState.saturation, g_colorPickerState.value,
                 g_colorPickerState.draggingSV);

    // Draw hue bar
    Rect hueRect{{pos.x + pickerSize + spacing, pos.y}, {pos.x + pickerSize + spacing + barWidth, pos.y + pickerSize}};
    drawHueBar(hueRect, g_colorPickerState.hue, g_colorPickerState.draggingHue);

    // Draw alpha bar if needed
    if (hasAlpha && hasFlag(flags, ColorEditFlags::AlphaBar))
    {
        Rect alphaRect{{hueRect.max.x + spacing, pos.y}, {hueRect.max.x + spacing + barWidth, pos.y + pickerSize}};

        f32 rgbPreview[3];
        colorConvertHSVtoRGB(g_colorPickerState.hue, g_colorPickerState.saturation, g_colorPickerState.value,
                             rgbPreview[0], rgbPreview[1], rgbPreview[2]);
        drawAlphaBar(alphaRect, rgbPreview, g_colorPickerState.alpha, g_colorPickerState.draggingAlpha);
    }

    // Check if any dragging occurred
    if (g_colorPickerState.draggingSV || g_colorPickerState.draggingHue || g_colorPickerState.draggingAlpha)
    {
        colorConvertHSVtoRGB(g_colorPickerState.hue, g_colorPickerState.saturation, g_colorPickerState.value, col[0],
                             col[1], col[2]);
        col[3] = g_colorPickerState.alpha;
        changed = true;
    }

    // Draw preview area
    if (!hasFlag(flags, ColorEditFlags::NoSidePreview))
    {
        f32 previewX = pos.x;
        f32 previewY = pos.y + pickerSize + spacing;
        f32 previewWidth = pickerSize;

        if (refCol)
        {
            // Split preview: current vs reference
            Rect currentRect{{previewX, previewY}, {previewX + previewWidth / 2, previewY + previewHeight}};
            Rect refRect{{previewX + previewWidth / 2, previewY}, {previewX + previewWidth, previewY + previewHeight}};

            drawColorPreview(currentRect, col, hasAlpha);
            drawColorPreview(refRect, refCol, hasAlpha);
        }
        else
        {
            Rect previewRect{{previewX, previewY}, {previewX + previewWidth, previewY + previewHeight}};
            drawColorPreview(previewRect, col, hasAlpha);
        }
    }

    // Draw inputs
    if (!hasFlag(flags, ColorEditFlags::NoInputs))
    {
        Vec2 inputPos{pos.x, pos.y + pickerSize + spacing + previewHeight + spacing};
        c->setCursorScreenPos(inputPos);
        drawColorInputs(col, hasAlpha, flags);
    }

    // Advance cursor
    f32 totalWidth = pickerSize + spacing + barWidth;
    if (hasAlpha && hasFlag(flags, ColorEditFlags::AlphaBar))
    {
        totalWidth += spacing + barWidth;
    }
    f32 totalHeight = pickerSize + spacing + previewHeight;
    if (!hasFlag(flags, ColorEditFlags::NoInputs))
    {
        totalHeight += spacing + 20.0f;
    }

    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(Vec2{totalWidth, totalHeight});
    }

    return changed;
}

bool colorButton(StringView descId, Color col, ColorEditFlags flags, Vec2 size)
{
    auto* c = ctx();
    if (!c)
        return false;

    auto* t = theme();
    Vec2 buttonSize = (size.x > 0 && size.y > 0) ? size : Vec2{20, 20};

    Vec2 pos = c->getCursorScreenPos();
    Rect rect{pos, pos + buttonSize};

    // Draw button
    bool hasAlpha = !hasFlag(flags, ColorEditFlags::NoAlpha);
    f32 colF[4] = {col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, col.a / 255.0f};
    drawColorPreview(rect, colF, hasAlpha);

    // Check for click
    Vec2 mousePos = c->getMousePos();
    bool hovered = rect.contains(mousePos);
    bool clicked = hovered && c->isMouseClicked(MouseButton::Left);

    // Draw hover effect
    if (hovered)
    {
        drawList().addRect(rect, t->borderShadow, 0, 2.0f);
    }

    // Advance cursor
    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(buttonSize);
    }

    return clicked;
}

void setColorEditReference(const f32 col[4])
{
    g_colorPickerState.referenceCol[0] = col[0];
    g_colorPickerState.referenceCol[1] = col[1];
    g_colorPickerState.referenceCol[2] = col[2];
    g_colorPickerState.referenceCol[3] = col[3];
    g_colorPickerState.hasReference = true;
}

// ============================================================================
// Gradient & Palette Utilities
// ============================================================================

void colorGradientBar(Color col1, Color col2, Vec2 size)
{
    auto* c = ctx();
    if (!c)
        return;

    Vec2 pos = c->getCursorScreenPos();
    Rect rect{pos, pos + size};

    drawList().addRectFilledMultiColor(rect, col1, col2, col2, col1);
    drawList().addRect(rect, theme()->border);

    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(size);
    }
}

bool colorPalette(StringView label, f32 currentCol[4], const Color* palette, usize paletteSize, i32 colsPerRow)
{
    auto* c = ctx();
    if (!c || !palette || paletteSize == 0)
        return false;

    bool changed = false;
    f32 buttonSize = 20.0f;
    f32 spacing = 2.0f;

    if (colsPerRow <= 0)
    {
        colsPerRow = static_cast<i32>(std::sqrt(static_cast<f32>(paletteSize)));
    }

    Vec2 startPos = c->getCursorScreenPos();
    Vec2 pos = startPos;

    for (usize i = 0; i < paletteSize; ++i)
    {
        if (i > 0 && (i % colsPerRow) == 0)
        {
            pos.x = startPos.x;
            pos.y += buttonSize + spacing;
        }

        Rect rect{pos, pos + Vec2{buttonSize, buttonSize}};
        f32 palCol[4] = {palette[i].r / 255.0f, palette[i].g / 255.0f, palette[i].b / 255.0f, palette[i].a / 255.0f};

        drawColorPreview(rect, palCol, true);

        Vec2 mousePos = c->getMousePos();
        if (rect.contains(mousePos) && c->isMouseClicked(MouseButton::Left))
        {
            currentCol[0] = palCol[0];
            currentCol[1] = palCol[1];
            currentCol[2] = palCol[2];
            currentCol[3] = palCol[3];
            changed = true;
        }

        pos.x += buttonSize + spacing;
    }

    i32 numRows = (static_cast<i32>(paletteSize) + colsPerRow - 1) / colsPerRow;
    f32 totalHeight = numRows * (buttonSize + spacing);

    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(Vec2{colsPerRow * (buttonSize + spacing), totalHeight});
    }

    return changed;
}

bool colorSavedStrip(StringView label, f32 savedColors[][4], usize maxColors, usize& numSaved, f32 currentCol[4])
{
    auto* c = ctx();
    if (!c)
        return false;

    bool changed = false;
    f32 buttonSize = 16.0f;
    f32 spacing = 2.0f;

    Vec2 startPos = c->getCursorScreenPos();
    Vec2 pos = startPos;

    // Draw saved colors
    for (usize i = 0; i < numSaved; ++i)
    {
        Rect rect{pos, pos + Vec2{buttonSize, buttonSize}};
        drawColorPreview(rect, savedColors[i], true);

        Vec2 mousePos = c->getMousePos();
        if (rect.contains(mousePos))
        {
            if (c->isMouseClicked(MouseButton::Left))
            {
                // Select this color
                currentCol[0] = savedColors[i][0];
                currentCol[1] = savedColors[i][1];
                currentCol[2] = savedColors[i][2];
                currentCol[3] = savedColors[i][3];
                changed = true;
            }
            else if (c->isMouseClicked(MouseButton::Right))
            {
                // Remove this color
                for (usize j = i; j < numSaved - 1; ++j)
                {
                    savedColors[j][0] = savedColors[j + 1][0];
                    savedColors[j][1] = savedColors[j + 1][1];
                    savedColors[j][2] = savedColors[j + 1][2];
                    savedColors[j][3] = savedColors[j + 1][3];
                }
                --numSaved;
            }
        }

        pos.x += buttonSize + spacing;
    }

    // Draw "add" button
    if (numSaved < maxColors)
    {
        Rect addRect{pos, pos + Vec2{buttonSize, buttonSize}};
        drawList().addRectFilled(addRect, theme()->frameBg);
        drawList().addRect(addRect, theme()->border);

        // Draw "+"
        Vec2 center = addRect.center();
        drawList().addLine({center.x - 4, center.y}, {center.x + 4, center.y}, theme()->text, 1.0f);
        drawList().addLine({center.x, center.y - 4}, {center.x, center.y + 4}, theme()->text, 1.0f);

        Vec2 mousePos = c->getMousePos();
        if (addRect.contains(mousePos) && c->isMouseClicked(MouseButton::Left))
        {
            // Save current color
            savedColors[numSaved][0] = currentCol[0];
            savedColors[numSaved][1] = currentCol[1];
            savedColors[numSaved][2] = currentCol[2];
            savedColors[numSaved][3] = currentCol[3];
            ++numSaved;
        }
    }

    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(Vec2{(maxColors + 1) * (buttonSize + spacing), buttonSize});
    }

    return changed;
}

}  // namespace dakt::gui
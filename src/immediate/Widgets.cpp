#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"
#include "dakt/gui/immediate/ImmediateState.hpp"
#include "dakt/gui/subsystems/input/Input.hpp"
#include "dakt/gui/subsystems/style/Style.hpp"
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <stack>

namespace dakt::gui {

// Forward declarations for internal types (must match Immediate.cpp)
struct WindowState {
    ID id = 0;
    const char* name = nullptr;
    Vec2 pos;
    Vec2 size;
    Vec2 contentSize;
    Vec2 cursorPos;
    Vec2 cursorStartPos;
    WindowFlags flags = WindowFlags::None;
    bool collapsed = false;
    bool skipItems = false;
};

struct ImmediateState {
    Context* ctx = nullptr;
    float deltaTime = 0.0f;

    std::vector<ID> idStack;
    ID currentId = 0;

    std::stack<WindowState> windowStack;
    WindowState* currentWindow = nullptr;

    bool nextWindowPosSet = false;
    Vec2 nextWindowPos;
    bool nextWindowSizeSet = false;
    Vec2 nextWindowSize;

    ID lastItemId = 0;
    Rect lastItemRect;
    bool lastItemHovered = false;
    bool lastItemActive = false;
    bool lastItemEdited = false;
    bool lastItemActivated = false;
    bool lastItemDeactivated = false;

    ID hotId = 0;
    ID activeId = 0;
    ID prevActiveId = 0;

    ID focusRequestId = 0;
    int focusRequestOffset = 0;
    ID keyboardFocusId = 0;
    ID defaultFocusId = 0;

    float lastClickTime[5] = {};
    Vec2 lastClickPos[5] = {};
    ID lastClickedId[5] = {};
    static constexpr float DOUBLE_CLICK_TIME = 0.3f;
    static constexpr float DOUBLE_CLICK_DIST = 6.0f;

    InputSystem* input = nullptr;
    float totalTime = 0.0f;
};

// External state defined in Immediate.cpp
extern ImmediateState g_state;

// Helper to update item state tracking for interactive widgets
static void updateItemState(ID id, bool hovered, bool active) {
    // Check for activation (becoming active this frame)
    bool wasActive = (g_state.prevActiveId == id);
    g_state.lastItemActivated = active && !wasActive;

    // Check for deactivation (was active, now not)
    g_state.lastItemDeactivated = wasActive && !active;

    // Update active tracking
    if (active) {
        g_state.activeId = id;
    } else if (g_state.activeId == id) {
        g_state.activeId = 0;
    }

    g_state.lastItemId = id;
    g_state.lastItemHovered = hovered;
    g_state.lastItemActive = active;
}

// ============================================================================
// Text Widgets
// ============================================================================

void text(const char* fmt, ...) {
    if (!g_state.ctx)
        return;

    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

    Vec2 pos = getCursorPos();
    Vec2 windowPos = getWindowPos();
    Vec2 textPos = windowPos + pos;

    dl.drawText(textPos, buf, colors.textPrimary);

    float textHeight = 16.0f;
    float textWidth = static_cast<float>(strlen(buf)) * 8.0f;
    g_state.lastItemRect = Rect(textPos.x, textPos.y, textWidth, textHeight);

    setCursorPos(Vec2(pos.x, pos.y + textHeight + 4.0f));
}

void textColored(Color color, const char* fmt, ...) {
    if (!g_state.ctx)
        return;

    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    DrawList& dl = g_state.ctx->getDrawList();

    Vec2 pos = getCursorPos();
    Vec2 windowPos = getWindowPos();
    Vec2 textPos = windowPos + pos;

    dl.drawText(textPos, buf, color);

    float textHeight = 16.0f;
    g_state.lastItemRect = Rect(textPos.x, textPos.y, static_cast<float>(strlen(buf)) * 8.0f, textHeight);
    setCursorPos(Vec2(pos.x, pos.y + textHeight + 4.0f));
}

void textDisabled(const char* fmt, ...) {
    if (!g_state.ctx)
        return;

    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

    Vec2 pos = getCursorPos();
    Vec2 windowPos = getWindowPos();
    Vec2 textPos = windowPos + pos;

    dl.drawText(textPos, buf, colors.textDisabled);

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
    if (!g_state.ctx)
        return;

    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

    Vec2 pos = getCursorPos();
    Vec2 windowPos = getWindowPos();
    Vec2 textPos = windowPos + pos;

    dl.drawText(textPos, label, colors.textSecondary);

    float labelWidth = static_cast<float>(strlen(label)) * 8.0f + 8.0f;
    dl.drawText(Vec2(textPos.x + labelWidth, textPos.y), buf, colors.textPrimary);

    float textHeight = 16.0f;
    setCursorPos(Vec2(pos.x, pos.y + textHeight + 4.0f));
}

// ============================================================================
// Button Widgets
// ============================================================================

bool button(const char* label, Vec2 size) {
    if (!g_state.ctx)
        return false;

    ID id = getID(label);
    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();
    const WidgetStyle& style = g_state.ctx->getTheme().getButtonStyle();

    Vec2 pos = getCursorPos();
    Vec2 windowPos = getWindowPos();
    Vec2 buttonPos = windowPos + pos;

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

    dl.drawRectFilledRounded(buttonRect, bgColor, 4.0f);
    dl.drawRectRounded(buttonRect, colors.border, 4.0f);

    Vec2 textPos = Vec2(buttonPos.x + (buttonSize.x - labelWidth) * 0.5f, buttonPos.y + (buttonSize.y - 16.0f) * 0.5f);
    dl.drawText(textPos, label, colors.textPrimary);

    g_state.lastItemRect = buttonRect;
    updateItemState(id, hovered, held);

    setCursorPos(Vec2(pos.x, pos.y + buttonSize.y + 4.0f));

    return clicked;
}

bool smallButton(const char* label) { return button(label, Vec2(0, 20)); }

bool invisibleButton(const char* strId, Vec2 size) {
    ID id = getID(strId);

    Vec2 pos = getCursorPos();
    Vec2 windowPos = getWindowPos();
    Vec2 buttonPos = windowPos + pos;

    bool hovered = isMouseHoveringRect(buttonPos, buttonPos + size);
    bool held = hovered && isMouseDown(MouseButton::Left);
    bool clicked = hovered && isMouseReleased(MouseButton::Left);

    g_state.lastItemRect = Rect(buttonPos.x, buttonPos.y, size.x, size.y);
    updateItemState(id, hovered, held);

    setCursorPos(Vec2(pos.x, pos.y + size.y + 4.0f));

    return clicked;
}

bool colorButton(const char* descId, Color color, Vec2 size) {
    if (!g_state.ctx)
        return false;

    ID id = getID(descId);
    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

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

    dl.drawRectFilledRounded(buttonRect, color, 2.0f);
    dl.drawRectRounded(buttonRect, colors.border, 2.0f);

    g_state.lastItemRect = buttonRect;
    updateItemState(id, hovered, hovered && isMouseDown(MouseButton::Left));

    setCursorPos(Vec2(pos.x, pos.y + buttonSize.y + 4.0f));

    return clicked;
}

// ============================================================================
// Checkbox & Radio
// ============================================================================

bool checkbox(const char* label, bool* value) {
    if (!g_state.ctx || !value)
        return false;

    ID id = getID(label);
    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

    Vec2 pos = getCursorPos();
    Vec2 windowPos = getWindowPos();
    Vec2 checkPos = windowPos + pos;

    float boxSize = 18.0f;
    Rect checkRect(checkPos.x, checkPos.y, boxSize, boxSize);

    bool hovered = isMouseHoveringRect(checkPos, checkPos + Vec2(boxSize, boxSize));
    bool clicked = hovered && isMouseReleased(MouseButton::Left);

    if (clicked) {
        *value = !(*value);
    }

    Color bgColor = *value ? colors.primary : colors.surface;
    if (hovered)
        bgColor = *value ? colors.primaryHover : colors.surfaceVariant;

    dl.drawRectFilledRounded(checkRect, bgColor, 2.0f);
    dl.drawRectRounded(checkRect, colors.border, 2.0f);

    if (*value) {
        Vec2 p1 = checkPos + Vec2(4, 9);
        Vec2 p2 = checkPos + Vec2(7, 13);
        Vec2 p3 = checkPos + Vec2(14, 5);
        dl.drawLine(p1, p2, Color::white(), 2.0f);
        dl.drawLine(p2, p3, Color::white(), 2.0f);
    }

    float labelX = checkPos.x + boxSize + 6.0f;
    dl.drawText(Vec2(labelX, checkPos.y + 1), label, colors.textPrimary);

    g_state.lastItemRect = checkRect;
    g_state.lastItemEdited = clicked; // Checkbox was edited if clicked
    updateItemState(id, hovered, hovered && isMouseDown(MouseButton::Left));

    setCursorPos(Vec2(pos.x, pos.y + boxSize + 4.0f));

    return clicked;
}

bool radioButton(const char* label, bool active) {
    if (!g_state.ctx)
        return false;

    ID id = getID(label);
    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

    Vec2 pos = getCursorPos();
    Vec2 windowPos = getWindowPos();
    Vec2 radioPos = windowPos + pos;

    float radius = 9.0f;
    Vec2 center = radioPos + Vec2(radius, radius);

    bool hovered = isMouseHoveringRect(radioPos, radioPos + Vec2(radius * 2, radius * 2));
    bool clicked = hovered && isMouseReleased(MouseButton::Left);

    Color bgColor = active ? colors.primary : colors.surface;
    if (hovered)
        bgColor = active ? colors.primaryHover : colors.surfaceVariant;

    dl.drawCircleFilled(center, radius, bgColor);
    dl.drawCircle(center, radius, colors.border);

    if (active) {
        dl.drawCircleFilled(center, radius * 0.4f, Color::white());
    }

    float labelX = radioPos.x + radius * 2 + 6.0f;
    dl.drawText(Vec2(labelX, radioPos.y + 1), label, colors.textPrimary);

    g_state.lastItemRect = Rect(radioPos.x, radioPos.y, radius * 2 + 6.0f + static_cast<float>(strlen(label)) * 8.0f, radius * 2);
    g_state.lastItemEdited = clicked;
    updateItemState(id, hovered, hovered && isMouseDown(MouseButton::Left));

    setCursorPos(Vec2(pos.x, pos.y + radius * 2 + 4.0f));

    return clicked;
}

bool radioButton(const char* label, int* value, int buttonValue) {
    bool active = (*value == buttonValue);
    if (radioButton(label, active)) {
        *value = buttonValue;
        return true;
    }
    return false;
}

// ============================================================================
// Progress Bar
// ============================================================================

void progressBar(float fraction, Vec2 size, const char* overlay) {
    if (!g_state.ctx)
        return;

    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

    Vec2 pos = getCursorPos();
    Vec2 windowPos = getWindowPos();
    Vec2 barPos = windowPos + pos;

    Vec2 barSize = size;
    if (barSize.x < 0)
        barSize.x = getContentRegionAvail().x;
    if (barSize.y <= 0)
        barSize.y = 16.0f;

    fraction = std::clamp(fraction, 0.0f, 1.0f);

    Rect bgRect(barPos.x, barPos.y, barSize.x, barSize.y);
    dl.drawRectFilledRounded(bgRect, colors.surface, 2.0f);

    if (fraction > 0) {
        Rect fillRect(barPos.x, barPos.y, barSize.x * fraction, barSize.y);
        dl.drawRectFilledRounded(fillRect, colors.primary, 2.0f);
    }

    dl.drawRectRounded(bgRect, colors.border, 2.0f);

    if (overlay) {
        float textWidth = static_cast<float>(strlen(overlay)) * 8.0f;
        Vec2 textPos = barPos + Vec2((barSize.x - textWidth) * 0.5f, (barSize.y - 14) * 0.5f);
        dl.drawText(textPos, overlay, colors.textPrimary);
    }

    setCursorPos(Vec2(pos.x, pos.y + barSize.y + 4.0f));
}

// ============================================================================
// Sliders
// ============================================================================

bool sliderFloat(const char* label, float* value, float min, float max, const char* format) {
    if (!g_state.ctx || !value)
        return false;

    ID id = getID(label);
    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

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

    dl.drawText(sliderPos, label, colors.textPrimary);

    dl.drawRectFilledRounded(trackRect, colors.surface, 2.0f);

    float t = (*value - min) / (max - min);
    float fillWidth = sliderWidth * t;
    Rect fillRect(trackPos.x, trackPos.y, fillWidth, sliderHeight);
    dl.drawRectFilledRounded(fillRect, colors.primary, 2.0f);

    dl.drawRectRounded(trackRect, colors.border, 2.0f);

    char valueBuf[64];
    snprintf(valueBuf, sizeof(valueBuf), format, *value);
    float valueWidth = static_cast<float>(strlen(valueBuf)) * 8.0f;
    Vec2 valuePos = trackPos + Vec2((sliderWidth - valueWidth) * 0.5f, 1);
    dl.drawText(valuePos, valueBuf, colors.textPrimary);

    g_state.lastItemRect = trackRect;
    g_state.lastItemEdited = changed; // Slider was edited if value changed
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
    popID();
    return changed;
}

// ============================================================================
// Drag Controls
// ============================================================================

bool dragInt(const char* label, int* value, float speed, int min, int max) { return sliderInt(label, value, min == 0 && max == 0 ? -1000 : min, max == 0 ? 1000 : max); }

bool dragFloat(const char* label, float* value, float speed, float min, float max, const char* format) { return sliderFloat(label, value, min == 0 && max == 0 ? -1000.0f : min, max == 0 ? 1000.0f : max, format); }

// ============================================================================
// Input Text
// ============================================================================

bool inputText(const char* label, char* buf, size_t bufSize, InputTextFlags flags) {
    if (!g_state.ctx)
        return false;

    ID id = getID(label);
    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

    Vec2 pos = getCursorPos();
    Vec2 windowPos = getWindowPos();
    Vec2 inputPos = windowPos + pos;

    float labelWidth = static_cast<float>(strlen(label)) * 8.0f + 8.0f;
    float inputWidth = 200.0f;
    float inputHeight = 22.0f;

    Vec2 boxPos = Vec2(inputPos.x + labelWidth, inputPos.y);
    Rect boxRect(boxPos.x, boxPos.y, inputWidth, inputHeight);

    bool hovered = isMouseHoveringRect(boxPos, boxPos + Vec2(inputWidth, inputHeight));

    dl.drawText(inputPos, label, colors.textPrimary);

    Color bgColor = hovered ? colors.surfaceVariant : colors.surface;
    dl.drawRectFilledRounded(boxRect, bgColor, 2.0f);
    dl.drawRectRounded(boxRect, colors.border, 2.0f);

    dl.drawText(Vec2(boxPos.x + 4, boxPos.y + 3), buf, colors.textPrimary);

    g_state.lastItemId = id;
    g_state.lastItemHovered = hovered;

    setCursorPos(Vec2(pos.x, pos.y + inputHeight + 4.0f));

    return false;
}

bool inputTextMultiline(const char* label, char* buf, size_t bufSize, Vec2 size, InputTextFlags flags) { return inputText(label, buf, bufSize, flags); }

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

// ============================================================================
// Color Editors
// ============================================================================

// Color conversion helpers for HSV
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

    h *= 60.0f;
    if (h < 0.0f)
        h += 360.0f;
}

static void hsvToRgb(float h, float s, float v, Color& rgb) {
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

    float rr, gg, bb;
    switch (i) {
    case 0:
        rr = v;
        gg = t;
        bb = p;
        break;
    case 1:
        rr = q;
        gg = v;
        bb = p;
        break;
    case 2:
        rr = p;
        gg = v;
        bb = t;
        break;
    case 3:
        rr = p;
        gg = q;
        bb = v;
        break;
    case 4:
        rr = t;
        gg = p;
        bb = v;
        break;
    default:
        rr = v;
        gg = p;
        bb = q;
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

bool colorEdit4(const char* label, Color* color) { return colorEdit3(label, color); }

// State for color picker dragging
static struct {
    bool draggingSV = false;
    bool draggingHue = false;
    bool draggingAlpha = false;
    float hue = 0;
    float sat = 1;
    float val = 1;
} g_colorPickerState;

bool colorPicker3(const char* label, Color* color) {
    if (!g_state.ctx || !color)
        return false;

    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

    Vec2 pos = getCursorPos();
    Vec2 windowPos = getWindowPos();
    Vec2 pickerPos = windowPos + pos;

    float svSize = 150.0f;
    float barWidth = 20.0f;
    float spacing = 8.0f;

    // Initialize HSV from color
    static bool initialized = false;
    if (!initialized) {
        rgbToHsv(*color, g_colorPickerState.hue, g_colorPickerState.sat, g_colorPickerState.val);
        initialized = true;
    }

    // Draw label
    dl.drawText(pickerPos, label, colors.textPrimary);
    pickerPos.y += 20;

    // Draw SV square
    Rect svRect(pickerPos.x, pickerPos.y, svSize, svSize);

    // Base hue color
    Color hueColor;
    hsvToRgb(g_colorPickerState.hue, 1.0f, 1.0f, hueColor);
    hueColor.a = 255;
    dl.drawRectFilled(svRect, hueColor);

    // White gradient overlay (saturation)
    for (float x = 0; x < svSize; x += 2) {
        float s = x / svSize;
        uint8_t alpha = static_cast<uint8_t>((1.0f - s) * 255);
        dl.drawRectFilled(Rect(svRect.x + x, svRect.y, 2, svSize), Color{255, 255, 255, alpha});
    }

    // Black gradient overlay (value)
    for (float y = 0; y < svSize; y += 2) {
        float v = y / svSize;
        uint8_t alpha = static_cast<uint8_t>(v * 255);
        dl.drawRectFilled(Rect(svRect.x, svRect.y + y, svSize, 2), Color{0, 0, 0, alpha});
    }

    // SV cursor
    float cursorX = svRect.x + g_colorPickerState.sat * svSize;
    float cursorY = svRect.y + (1.0f - g_colorPickerState.val) * svSize;
    dl.drawCircle(Vec2(cursorX, cursorY), 7, Color{0, 0, 0, 255}, 16);
    dl.drawCircle(Vec2(cursorX, cursorY), 6, Color{255, 255, 255, 255}, 16);

    // Hue bar
    float hueX = pickerPos.x + svSize + spacing;
    Rect hueRect(hueX, pickerPos.y, barWidth, svSize);

    // Draw hue gradient
    Color hueColors[7] = {{255, 0, 0, 255}, {255, 255, 0, 255}, {0, 255, 0, 255}, {0, 255, 255, 255}, {0, 0, 255, 255}, {255, 0, 255, 255}, {255, 0, 0, 255}};
    float segHeight = svSize / 6.0f;
    for (int i = 0; i < 6; i++) {
        for (float y = 0; y < segHeight; y += 2) {
            float t = y / segHeight;
            Color c;
            c.r = static_cast<uint8_t>(hueColors[i].r + t * (hueColors[i + 1].r - hueColors[i].r));
            c.g = static_cast<uint8_t>(hueColors[i].g + t * (hueColors[i + 1].g - hueColors[i].g));
            c.b = static_cast<uint8_t>(hueColors[i].b + t * (hueColors[i + 1].b - hueColors[i].b));
            c.a = 255;
            dl.drawRectFilled(Rect(hueX, pickerPos.y + i * segHeight + y, barWidth, 2), c);
        }
    }
    dl.drawRect(hueRect, colors.border);

    // Hue cursor
    float hueCursorY = hueRect.y + (g_colorPickerState.hue / 360.0f) * svSize;
    dl.drawRectFilled(Rect(hueX - 2, hueCursorY - 2, barWidth + 4, 4), Color{255, 255, 255, 255});
    dl.drawRect(Rect(hueX - 2, hueCursorY - 2, barWidth + 4, 4), Color{0, 0, 0, 255});

    // Handle input
    bool changed = false;
    if (g_state.input) {
        Vec2 mousePos = g_state.input->getMousePosition();
        bool leftDown = g_state.input->isMouseButtonDown(MouseButton::Left);
        bool leftPressed = g_state.input->isMouseButtonPressed(MouseButton::Left);

        // Start dragging
        if (leftPressed) {
            if (svRect.contains(mousePos)) {
                g_colorPickerState.draggingSV = true;
            } else if (hueRect.contains(mousePos)) {
                g_colorPickerState.draggingHue = true;
            }
        }

        // Update while dragging
        if (leftDown) {
            if (g_colorPickerState.draggingSV) {
                g_colorPickerState.sat = std::clamp((mousePos.x - svRect.x) / svSize, 0.0f, 1.0f);
                g_colorPickerState.val = 1.0f - std::clamp((mousePos.y - svRect.y) / svSize, 0.0f, 1.0f);
                changed = true;
            }
            if (g_colorPickerState.draggingHue) {
                g_colorPickerState.hue = std::clamp((mousePos.y - hueRect.y) / svSize, 0.0f, 1.0f) * 360.0f;
                changed = true;
            }
        } else {
            g_colorPickerState.draggingSV = false;
            g_colorPickerState.draggingHue = false;
        }
    }

    // Update color from HSV
    if (changed) {
        uint8_t alpha = color->a;
        hsvToRgb(g_colorPickerState.hue, g_colorPickerState.sat, g_colorPickerState.val, *color);
        color->a = alpha;
    }

    // Draw preview
    float previewY = pickerPos.y + svSize + spacing;
    Rect previewRect(pickerPos.x, previewY, svSize + spacing + barWidth, 30);
    dl.drawRectFilled(previewRect, *color);
    dl.drawRect(previewRect, colors.border);

    // Draw RGB values
    char rgbStr[64];
    snprintf(rgbStr, sizeof(rgbStr), "R:%d G:%d B:%d", color->r, color->g, color->b);
    dl.drawText(Vec2(pickerPos.x, previewY + 36), rgbStr, colors.textSecondary);

    float totalHeight = svSize + spacing + 30 + 20 + 20;
    setCursorPos(Vec2(pos.x, pos.y + totalHeight + 8));

    return changed;
}

bool colorPicker4(const char* label, Color* color) {
    // For now, colorPicker4 is same as colorPicker3
    // Full alpha support would add an alpha bar
    return colorPicker3(label, color);
}

// ============================================================================
// Trees
// ============================================================================

bool treeNode(const char* label, TreeNodeFlags flags) {
    if (!g_state.ctx)
        return false;

    ID id = getID(label);
    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

    Vec2 pos = getCursorPos();
    Vec2 windowPos = getWindowPos();
    Vec2 nodePos = windowPos + pos;

    bool hovered = isMouseHoveringRect(nodePos, nodePos + Vec2(200, 20));
    bool clicked = hovered && isMouseReleased(MouseButton::Left);

    dl.drawText(nodePos, ">", colors.textPrimary);
    dl.drawText(Vec2(nodePos.x + 16, nodePos.y), label, colors.textPrimary);

    setCursorPos(Vec2(pos.x, pos.y + 20 + 4.0f));

    return clicked;
}

bool treeNodeEx(const char* strId, TreeNodeFlags flags, const char* fmt, ...) { return treeNode(strId, flags); }

void treePop() { unindent(); }

bool collapsingHeader(const char* label, TreeNodeFlags flags) { return treeNode(label, flags); }

bool selectable(const char* label, bool selected, Vec2 size) {
    if (!g_state.ctx)
        return false;

    ID id = getID(label);
    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

    Vec2 pos = getCursorPos();
    Vec2 windowPos = getWindowPos();
    Vec2 itemPos = windowPos + pos;

    Vec2 itemSize = size;
    if (itemSize.x <= 0)
        itemSize.x = getContentRegionAvail().x;
    if (itemSize.y <= 0)
        itemSize.y = 20.0f;

    Rect itemRect(itemPos.x, itemPos.y, itemSize.x, itemSize.y);

    bool hovered = isMouseHoveringRect(itemPos, itemPos + itemSize);
    bool clicked = hovered && isMouseReleased(MouseButton::Left);

    if (selected || hovered) {
        Color bgColor = selected ? colors.primary : colors.surfaceVariant;
        dl.drawRectFilled(itemRect, bgColor);
    }

    dl.drawText(Vec2(itemPos.x + 4, itemPos.y + 2), label, selected ? Color::white() : colors.textPrimary);

    g_state.lastItemId = id;
    g_state.lastItemHovered = hovered;

    setCursorPos(Vec2(pos.x, pos.y + itemSize.y));

    return clicked;
}

bool selectable(const char* label, bool* selected, Vec2 size) {
    if (selectable(label, *selected, size)) {
        *selected = !(*selected);
        return true;
    }
    return false;
}

bool listBox(const char* label, int* currentItem, const char* const items[], int itemCount, int heightInItems) {
    if (!currentItem || !items)
        return false;

    text("%s", label);
    bool changed = false;
    for (int i = 0; i < itemCount; i++) {
        if (selectable(items[i], i == *currentItem)) {
            *currentItem = i;
            changed = true;
        }
    }
    return changed;
}

// ============================================================================
// Menus
// ============================================================================

// Menu state tracking
static struct {
    bool mainMenuBarOpen = false;
    bool menuBarOpen = false;
    std::vector<const char*> menuStack;
    Vec2 menuPosition;
    float menuBarHeight = 28.0f;
    int hoveredMenuItem = -1;
    std::vector<Rect> menuItemRects;
} g_menuState;

bool beginMainMenuBar() {
    if (!g_state.ctx)
        return false;

    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

    float screenWidth = 1280.0f; // Default screen width
    Rect barRect(0, 0, screenWidth, g_menuState.menuBarHeight);

    dl.drawRectFilled(barRect, colors.surface);
    dl.drawLine(Vec2(0, g_menuState.menuBarHeight), Vec2(screenWidth, g_menuState.menuBarHeight), colors.border);

    g_menuState.mainMenuBarOpen = true;
    g_menuState.menuPosition = Vec2(8, 0);
    g_menuState.menuItemRects.clear();

    return true;
}

void endMainMenuBar() { g_menuState.mainMenuBarOpen = false; }

bool beginMenuBar() {
    if (!g_state.ctx || !g_state.currentWindow)
        return false;

    g_menuState.menuBarOpen = true;
    g_menuState.menuPosition = getWindowPos();
    return true;
}

void endMenuBar() { g_menuState.menuBarOpen = false; }

bool beginMenu(const char* label, bool enabled) {
    if (!g_state.ctx)
        return false;
    if (!enabled)
        return false;

    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

    float labelWidth = static_cast<float>(strlen(label)) * 8.0f + 16.0f;
    Rect menuRect(g_menuState.menuPosition.x, g_menuState.menuPosition.y, labelWidth, g_menuState.menuBarHeight);

    bool hovered = g_state.input && menuRect.contains(g_state.input->getMousePosition());
    bool clicked = hovered && g_state.input && g_state.input->isMouseButtonPressed(MouseButton::Left);

    // Draw menu title
    if (hovered) {
        dl.drawRectFilled(menuRect, colors.surfaceVariant);
    }

    dl.drawText(Vec2(menuRect.x + 8, menuRect.y + (g_menuState.menuBarHeight - 14) / 2), label, colors.textPrimary);

    g_menuState.menuPosition.x += labelWidth;

    if (clicked) {
        g_menuState.menuStack.push_back(label);
        return true;
    }

    // Check if this menu is already open
    if (!g_menuState.menuStack.empty() && strcmp(g_menuState.menuStack.back(), label) == 0) {
        return true;
    }

    return false;
}

void endMenu() {
    if (!g_menuState.menuStack.empty()) {
        g_menuState.menuStack.pop_back();
    }
}

bool menuItem(const char* label, const char* shortcut, bool selected, bool enabled) {
    if (!g_state.ctx)
        return false;

    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

    Vec2 pos = getCursorPos();
    Vec2 windowPos = getWindowPos();

    float itemHeight = 24.0f;
    float itemWidth = 200.0f;
    Rect itemRect(windowPos.x + pos.x, windowPos.y + pos.y, itemWidth, itemHeight);

    bool hovered = enabled && g_state.input && itemRect.contains(g_state.input->getMousePosition());
    bool clicked = hovered && g_state.input && g_state.input->isMouseButtonPressed(MouseButton::Left);

    // Draw background
    if (hovered) {
        dl.drawRectFilled(itemRect, colors.surfaceVariant);
    }

    // Draw checkmark if selected
    if (selected) {
        dl.drawText(Vec2(itemRect.x + 4, itemRect.y + 4), "✓", colors.textPrimary);
    }

    // Draw label
    Color textColor = enabled ? colors.textPrimary : colors.textDisabled;
    dl.drawText(Vec2(itemRect.x + 24, itemRect.y + 4), label, textColor);

    // Draw shortcut
    if (shortcut) {
        float shortcutWidth = static_cast<float>(strlen(shortcut)) * 7.0f;
        dl.drawText(Vec2(itemRect.x + itemWidth - shortcutWidth - 8, itemRect.y + 4), shortcut, colors.textSecondary);
    }

    setCursorPos(Vec2(pos.x, pos.y + itemHeight));

    return clicked;
}

bool menuItem(const char* label, const char* shortcut, bool* selected, bool enabled) {
    bool clicked = menuItem(label, shortcut, selected ? *selected : false, enabled);
    if (clicked && selected) {
        *selected = !(*selected);
    }
    return clicked;
}

// ============================================================================
// Popups
// ============================================================================

static struct {
    std::vector<const char*> popupStack;
    std::vector<Vec2> popupPositions;
    const char* pendingPopup = nullptr;
    bool closeRequested = false;
} g_popupState;

void openPopup(const char* strId) { g_popupState.pendingPopup = strId; }

bool beginPopup(const char* strId) {
    if (!g_state.ctx)
        return false;

    // Check if this popup should be opened
    if (g_popupState.pendingPopup && strcmp(g_popupState.pendingPopup, strId) == 0) {
        g_popupState.pendingPopup = nullptr;
        g_popupState.popupStack.push_back(strId);

        Vec2 mousePos = g_state.input ? g_state.input->getMousePosition() : Vec2(100, 100);
        g_popupState.popupPositions.push_back(mousePos);
    }

    // Check if popup is open
    for (const auto& popup : g_popupState.popupStack) {
        if (strcmp(popup, strId) == 0) {
            // Draw popup background
            DrawList& dl = g_state.ctx->getDrawList();
            const ColorScheme& colors = g_state.ctx->getTheme().colors();

            Vec2 pos = g_popupState.popupPositions.back();
            Rect popupRect(pos.x, pos.y, 220, 200);

            // Shadow
            dl.drawRectFilledRounded(Rect(pos.x + 3, pos.y + 3, 220, 200), Color(0, 0, 0, 60), 4.0f);
            // Background
            dl.drawRectFilledRounded(popupRect, colors.surface, 4.0f);
            // Border
            dl.drawRectRounded(popupRect, colors.border, 4.0f);

            // Update cursor for popup content
            setNextWindowPos(pos);
            setNextWindowSize(Vec2(220, 200));

            return true;
        }
    }

    return false;
}

bool beginPopupModal(const char* name, bool* open) {
    if (!g_state.ctx)
        return false;

    if (open && !*open)
        return false;

    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

    // Draw modal overlay
    Rect viewport(0, 0, 1280.0f, 720.0f); // Default viewport
    dl.drawRectFilled(viewport, Color(0, 0, 0, 128));

    // Draw modal window
    Vec2 modalSize(400, 300);
    Vec2 modalPos((viewport.width - modalSize.x) / 2, (viewport.height - modalSize.y) / 2);

    Rect modalRect(modalPos.x, modalPos.y, modalSize.x, modalSize.y);
    dl.drawRectFilledRounded(modalRect, colors.surface, 6.0f);

    // Title bar
    Rect titleRect(modalPos.x, modalPos.y, modalSize.x, 32);
    dl.drawRectFilledRounded(titleRect, colors.surfaceVariant, BorderRadius(6, 6, 0, 0));
    dl.drawText(Vec2(modalPos.x + 12, modalPos.y + 8), name, colors.textPrimary);

    // Close button
    if (open) {
        Rect closeRect(modalPos.x + modalSize.x - 28, modalPos.y + 4, 24, 24);
        bool closeHovered = g_state.input && closeRect.contains(g_state.input->getMousePosition());
        if (closeHovered) {
            dl.drawRectFilledRounded(closeRect, colors.surfaceVariant, 4.0f);
        }
        dl.drawText(Vec2(closeRect.x + 6, closeRect.y + 4), "×", colors.textPrimary);

        if (closeHovered && g_state.input && g_state.input->isMouseButtonPressed(MouseButton::Left)) {
            *open = false;
            return false;
        }
    }

    g_popupState.popupStack.push_back(name);
    g_popupState.popupPositions.push_back(modalPos + Vec2(0, 32));

    setNextWindowPos(modalPos + Vec2(12, 40));
    setNextWindowSize(Vec2(modalSize.x - 24, modalSize.y - 52));

    return true;
}

void endPopup() {
    if (!g_popupState.popupStack.empty()) {
        g_popupState.popupStack.pop_back();
        g_popupState.popupPositions.pop_back();
    }
}

void closeCurrentPopup() { g_popupState.closeRequested = true; }

bool beginPopupContextItem(const char* strId) {
    if (!g_state.ctx || !g_state.input)
        return false;

    // Check for right-click on last item
    if (g_state.lastItemHovered && g_state.input->isMouseButtonPressed(MouseButton::Right)) {
        openPopup(strId ? strId : "context_item");
    }

    return beginPopup(strId ? strId : "context_item");
}

bool beginPopupContextWindow(const char* strId) {
    if (!g_state.ctx || !g_state.input)
        return false;

    // Check for right-click in window
    if (g_state.input->isMouseButtonPressed(MouseButton::Right)) {
        openPopup(strId ? strId : "context_window");
    }

    return beginPopup(strId ? strId : "context_window");
}

// ============================================================================
// Tooltips
// ============================================================================

static struct {
    bool tooltipActive = false;
    std::string tooltipText;
    Vec2 tooltipPos;
} g_tooltipState;

void beginTooltip() {
    g_tooltipState.tooltipActive = true;
    if (g_state.input) {
        g_tooltipState.tooltipPos = g_state.input->getMousePosition() + Vec2(16, 16);
    }
}

void endTooltip() {
    if (!g_state.ctx || !g_tooltipState.tooltipActive)
        return;

    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

    float textWidth = static_cast<float>(g_tooltipState.tooltipText.length()) * 7.0f + 16;
    float textHeight = 24.0f;

    Vec2 pos = g_tooltipState.tooltipPos;
    Rect tooltipRect(pos.x, pos.y, textWidth, textHeight);

    // Shadow
    dl.drawRectFilledRounded(Rect(pos.x + 2, pos.y + 2, textWidth, textHeight), Color(0, 0, 0, 60), 4.0f);
    // Background
    dl.drawRectFilledRounded(tooltipRect, Color(60, 60, 64, 240), 4.0f);
    // Border
    dl.drawRectRounded(tooltipRect, Color(80, 80, 84, 255), 4.0f);
    // Text
    dl.drawText(Vec2(pos.x + 8, pos.y + 5), g_tooltipState.tooltipText.c_str(), colors.textPrimary);

    g_tooltipState.tooltipActive = false;
    g_tooltipState.tooltipText.clear();
}

void setTooltip(const char* fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    beginTooltip();
    g_tooltipState.tooltipText = buf;
    endTooltip();
}

// ============================================================================
// Tables
// ============================================================================

static struct {
    bool tableActive = false;
    const char* tableId = nullptr;
    int columns = 0;
    int currentColumn = -1;
    int currentRow = -1;
    TableFlags flags = TableFlags::None;
    Vec2 tablePos;
    Vec2 tableSize;
    std::vector<float> columnWidths;
    std::vector<const char*> columnLabels;
    float rowHeight = 24.0f;
    float headerHeight = 28.0f;
} g_tableState;

bool beginTable(const char* strId, int columns, TableFlags flags, Vec2 outerSize) {
    if (!g_state.ctx || g_tableState.tableActive)
        return false;

    g_tableState.tableActive = true;
    g_tableState.tableId = strId;
    g_tableState.columns = columns;
    g_tableState.flags = flags;
    g_tableState.currentColumn = -1;
    g_tableState.currentRow = -1;

    g_tableState.tablePos = getWindowPos() + getCursorPos();
    g_tableState.tableSize = outerSize;
    if (outerSize.x <= 0)
        g_tableState.tableSize.x = getContentRegionAvail().x;
    if (outerSize.y <= 0)
        g_tableState.tableSize.y = 200.0f;

    // Initialize column widths
    g_tableState.columnWidths.clear();
    g_tableState.columnLabels.clear();
    float defaultWidth = g_tableState.tableSize.x / columns;
    for (int i = 0; i < columns; i++) {
        g_tableState.columnWidths.push_back(defaultWidth);
        g_tableState.columnLabels.push_back("");
    }

    // Draw table background
    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

    Rect tableRect(g_tableState.tablePos.x, g_tableState.tablePos.y, g_tableState.tableSize.x, g_tableState.tableSize.y);
    dl.drawRectFilled(tableRect, colors.surface);

    if (static_cast<uint32_t>(flags) & static_cast<uint32_t>(TableFlags::Borders)) {
        dl.drawRect(tableRect, colors.border);
    }

    return true;
}

void endTable() {
    if (!g_tableState.tableActive)
        return;

    // Move cursor past the table
    Vec2 pos = getCursorPos();
    float tableHeight = g_tableState.headerHeight + (g_tableState.currentRow + 1) * g_tableState.rowHeight;
    setCursorPos(Vec2(pos.x, pos.y + tableHeight + 4));

    g_tableState.tableActive = false;
    g_tableState.columnWidths.clear();
    g_tableState.columnLabels.clear();
}

void tableNextRow() {
    g_tableState.currentRow++;
    g_tableState.currentColumn = -1;
}

bool tableNextColumn() {
    if (!g_tableState.tableActive)
        return false;

    g_tableState.currentColumn++;
    return g_tableState.currentColumn < g_tableState.columns;
}

bool tableSetColumnIndex(int columnIndex) {
    if (!g_tableState.tableActive || columnIndex < 0 || columnIndex >= g_tableState.columns) {
        return false;
    }
    g_tableState.currentColumn = columnIndex;
    return true;
}

void tableSetupColumn(const char* label, float initWidth) {
    if (!g_tableState.tableActive)
        return;

    static int setupColumn = 0;
    if (setupColumn < g_tableState.columns) {
        g_tableState.columnLabels[setupColumn] = label;
        if (initWidth > 0) {
            g_tableState.columnWidths[setupColumn] = initWidth;
        }
        setupColumn++;
    } else {
        setupColumn = 0;
    }
}

void tableHeadersRow() {
    if (!g_state.ctx || !g_tableState.tableActive)
        return;

    DrawList& dl = g_state.ctx->getDrawList();
    const ColorScheme& colors = g_state.ctx->getTheme().colors();

    float x = g_tableState.tablePos.x;
    float y = g_tableState.tablePos.y;

    // Draw header background
    Rect headerRect(x, y, g_tableState.tableSize.x, g_tableState.headerHeight);
    dl.drawRectFilled(headerRect, Color(55, 55, 60, 255));

    // Draw column headers
    for (int i = 0; i < g_tableState.columns; i++) {
        float colWidth = g_tableState.columnWidths[i];
        Rect colRect(x, y, colWidth, g_tableState.headerHeight);

        if (g_tableState.columnLabels[i] && strlen(g_tableState.columnLabels[i]) > 0) {
            dl.drawText(Vec2(x + 8, y + 6), g_tableState.columnLabels[i], colors.textPrimary);
        }

        // Column separator
        if (static_cast<uint32_t>(g_tableState.flags) & static_cast<uint32_t>(TableFlags::Borders)) {
            dl.drawLine(Vec2(x + colWidth, y), Vec2(x + colWidth, y + g_tableState.headerHeight), colors.border);
        }

        x += colWidth;
    }

    // Header bottom border
    dl.drawLine(Vec2(g_tableState.tablePos.x, y + g_tableState.headerHeight), Vec2(g_tableState.tablePos.x + g_tableState.tableSize.x, y + g_tableState.headerHeight), colors.border);

    g_tableState.currentRow = -1;
}

} // namespace dakt::gui

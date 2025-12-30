// ============================================================================
// DaktLib GUI Module - Widgets Implementation
// ============================================================================

#include <dakt/gui/Widgets.hpp>

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace dakt::gui
{

// ============================================================================
// Helper Functions
// ============================================================================

namespace
{

Context* ctx()
{
    return getContext();
}

Theme* theme()
{
    auto* c = ctx();
    return c ? c->getTheme() : nullptr;
}

DrawList& drawList()
{
    return ctx()->getDrawList();
}

// Calculate widget size with default
Vec2 calcWidgetSize(Vec2 requested, Vec2 defaultSize)
{
    return Vec2{requested.x > 0.0f ? requested.x : defaultSize.x, requested.y > 0.0f ? requested.y : defaultSize.y};
}

// Check if mouse is over a rect
bool isMouseOverRect(Rect rect)
{
    return rect.contains(ctx()->getMousePos());
}

// Standard widget behavior: handle hover/active states
struct WidgetBehavior
{
    bool hovered = false;
    bool held = false;
    bool pressed = false;
    bool released = false;
};

WidgetBehavior widgetBehavior(WidgetId id, Rect rect, bool allowHeld = true)
{
    WidgetBehavior result;
    auto* c = ctx();

    result.hovered = isMouseOverRect(rect);

    if (result.hovered)
    {
        c->setHotWidget(id);
    }

    bool wasActive = c->isWidgetActive(id);

    if (result.hovered && c->isMouseClicked(MouseButton::Left))
    {
        c->setActiveWidget(id);
        result.pressed = true;
    }

    if (wasActive)
    {
        if (c->isMouseDown(MouseButton::Left) && allowHeld)
        {
            result.held = true;
        }
        else
        {
            c->clearActiveWidget();
            if (result.hovered)
            {
                result.released = true;
            }
        }
    }

    return result;
}

}  // anonymous namespace

// ============================================================================
// Text Widgets
// ============================================================================

void text(StringView str)
{
    auto* c = ctx();
    if (!c)
        return;

    auto* font = c->getFont();
    if (!font)
        return;

    Vec2 textSize = font->calcTextSize(str);
    Vec2 pos = c->getCursorScreenPos();

    drawList().addText(pos, str, theme()->text);

    // Advance layout
    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(textSize);
    }
}

void textColored(Color color, StringView str)
{
    auto* c = ctx();
    if (!c)
        return;

    auto* font = c->getFont();
    if (!font)
        return;

    Vec2 textSize = font->calcTextSize(str);
    Vec2 pos = c->getCursorScreenPos();

    drawList().addText(pos, str, color);

    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(textSize);
    }
}

void textDisabled(StringView str)
{
    textColored(theme()->textDisabled, str);
}

void textWrapped(StringView str)
{
    // TODO: Implement text wrapping
    text(str);
}

void bulletText(StringView str)
{
    auto* c = ctx();
    if (!c)
        return;

    Vec2 pos = c->getCursorScreenPos();
    f32 bulletRadius = 3.0f;

    drawList().addCircleFilled(pos + Vec2{bulletRadius, 6.0f}, bulletRadius, theme()->text);

    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.cursor.x += bulletRadius * 2 + 4;
    }

    text(str);
}

void labelText(StringView label, StringView str)
{
    text(label);
    ctx()->sameLine();
    text(str);
}

// ============================================================================
// Button Widgets
// ============================================================================

bool button(StringView label, Vec2 size)
{
    auto* c = ctx();
    if (!c)
        return false;

    WidgetId id = c->getId(label);
    auto* t = theme();

    // Calculate size
    auto* font = c->getFont();
    Vec2 textSize = font ? font->calcTextSize(label) : Vec2{80, 14};
    Vec2 padding = t->framePadding;
    Vec2 widgetSize = calcWidgetSize(size, textSize + padding * 2.0f);

    Vec2 pos = c->getCursorScreenPos();
    Rect rect{pos, pos + widgetSize};

    // Behavior
    auto behavior = widgetBehavior(id, rect);

    // Draw
    Color bgColor = t->button;
    if (behavior.held)
        bgColor = t->buttonActive;
    else if (behavior.hovered)
        bgColor = t->buttonHovered;

    drawList().addRectFilled(rect, bgColor, t->frameRounding);

    if (t->frameBorderSize > 0.0f)
    {
        drawList().addRect(rect, t->border, t->frameRounding, Corner::All, t->frameBorderSize);
    }

    // Draw text centered
    Vec2 textPos = rect.center() - textSize * 0.5f;
    drawList().addText(textPos, label, t->text);

    // Advance layout
    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(widgetSize);
    }

    return behavior.released;
}

bool smallButton(StringView label)
{
    auto* c = ctx();
    if (!c)
        return false;

    auto* font = c->getFont();
    Vec2 textSize = font ? font->calcTextSize(label) : Vec2{40, 12};
    return button(label, textSize + Vec2{4, 2});
}

bool invisibleButton(StringView strId, Vec2 size)
{
    auto* c = ctx();
    if (!c)
        return false;

    WidgetId id = c->getId(strId);

    Vec2 pos = c->getCursorScreenPos();
    Rect rect{pos, pos + size};

    auto behavior = widgetBehavior(id, rect);

    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(size);
    }

    return behavior.released;
}

bool arrowButton(StringView strId, i32 dir)
{
    auto* c = ctx();
    if (!c)
        return false;

    WidgetId id = c->getId(strId);
    auto* t = theme();

    f32 size = 20.0f;
    Vec2 pos = c->getCursorScreenPos();
    Rect rect{pos, pos + Vec2{size, size}};

    auto behavior = widgetBehavior(id, rect);

    Color bgColor = t->button;
    if (behavior.held)
        bgColor = t->buttonActive;
    else if (behavior.hovered)
        bgColor = t->buttonHovered;

    drawList().addRectFilled(rect, bgColor, t->frameRounding);

    // Draw arrow
    Vec2 center = rect.center();
    f32 arrowSize = size * 0.4f;
    Vec2 p1, p2, p3;

    switch (dir)
    {
        case 0:  // Left
            p1 = center + Vec2{-arrowSize, 0};
            p2 = center + Vec2{arrowSize * 0.5f, -arrowSize};
            p3 = center + Vec2{arrowSize * 0.5f, arrowSize};
            break;
        case 1:  // Right
            p1 = center + Vec2{arrowSize, 0};
            p2 = center + Vec2{-arrowSize * 0.5f, -arrowSize};
            p3 = center + Vec2{-arrowSize * 0.5f, arrowSize};
            break;
        case 2:  // Up
            p1 = center + Vec2{0, -arrowSize};
            p2 = center + Vec2{-arrowSize, arrowSize * 0.5f};
            p3 = center + Vec2{arrowSize, arrowSize * 0.5f};
            break;
        case 3:  // Down
        default:
            p1 = center + Vec2{0, arrowSize};
            p2 = center + Vec2{-arrowSize, -arrowSize * 0.5f};
            p3 = center + Vec2{arrowSize, -arrowSize * 0.5f};
            break;
    }

    drawList().addTriangleFilled(p1, p2, p3, t->text);

    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(Vec2{size, size});
    }

    return behavior.released;
}

bool imageButton(uintptr_t textureId, Vec2 size, Vec2 uv0, Vec2 uv1, Color bgColor, Color tintColor)
{
    auto* c = ctx();
    if (!c)
        return false;

    WidgetId id = c->getId(static_cast<i32>(textureId));
    auto* t = theme();

    Vec2 padding = t->framePadding;
    Vec2 widgetSize = size + padding * 2.0f;

    Vec2 pos = c->getCursorScreenPos();
    Rect rect{pos, pos + widgetSize};

    auto behavior = widgetBehavior(id, rect);

    Color bg = bgColor;
    if (behavior.held)
        bg = Color::lerp(bg, t->buttonActive, 0.5f);
    else if (behavior.hovered)
        bg = Color::lerp(bg, t->buttonHovered, 0.5f);

    drawList().addRectFilled(rect, bg, t->frameRounding);

    Rect imageRect{pos + padding, pos + padding + size};
    drawList().addImage(textureId, imageRect, uv0, uv1, tintColor);

    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(widgetSize);
    }

    return behavior.released;
}

// ============================================================================
// Checkbox & Radio
// ============================================================================

bool checkbox(StringView label, bool* value)
{
    auto* c = ctx();
    if (!c || !value)
        return false;

    WidgetId id = c->getId(label);
    auto* t = theme();
    auto* font = c->getFont();

    f32 boxSize = 16.0f;
    Vec2 textSize = font ? font->calcTextSize(label) : Vec2{60, 14};
    Vec2 widgetSize{boxSize + 4 + textSize.x, std::max(boxSize, textSize.y)};

    Vec2 pos = c->getCursorScreenPos();
    Rect boxRect{pos, pos + Vec2{boxSize, boxSize}};
    Rect fullRect{pos, pos + widgetSize};

    auto behavior = widgetBehavior(id, fullRect);

    if (behavior.released)
    {
        *value = !*value;
    }

    // Draw box
    Color bgColor = t->frameBg;
    if (behavior.held)
        bgColor = t->frameBgActive;
    else if (behavior.hovered)
        bgColor = t->frameBgHovered;

    drawList().addRectFilled(boxRect, bgColor, t->frameRounding);
    drawList().addRect(boxRect, t->border, t->frameRounding);

    // Draw checkmark
    if (*value)
    {
        Vec2 center = boxRect.center();
        f32 s = boxSize * 0.25f;
        drawList().addLine(center + Vec2{-s, 0}, center + Vec2{-s * 0.3f, s}, t->checkMark, 2.0f);
        drawList().addLine(center + Vec2{-s * 0.3f, s}, center + Vec2{s, -s}, t->checkMark, 2.0f);
    }

    // Draw label
    Vec2 textPos = pos + Vec2{boxSize + 4, (boxSize - textSize.y) * 0.5f};
    drawList().addText(textPos, label, t->text);

    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(widgetSize);
    }

    return behavior.released;
}

bool checkboxFlags(StringView label, u32* flags, u32 flagsValue)
{
    bool value = (*flags & flagsValue) != 0;
    bool pressed = checkbox(label, &value);
    if (pressed)
    {
        if (value)
        {
            *flags |= flagsValue;
        }
        else
        {
            *flags &= ~flagsValue;
        }
    }
    return pressed;
}

bool radioButton(StringView label, bool active)
{
    auto* c = ctx();
    if (!c)
        return false;

    WidgetId id = c->getId(label);
    auto* t = theme();
    auto* font = c->getFont();

    f32 circleRadius = 8.0f;
    Vec2 textSize = font ? font->calcTextSize(label) : Vec2{60, 14};
    Vec2 widgetSize{circleRadius * 2 + 4 + textSize.x, std::max(circleRadius * 2, textSize.y)};

    Vec2 pos = c->getCursorScreenPos();
    Vec2 circleCenter = pos + Vec2{circleRadius, circleRadius};
    Rect fullRect{pos, pos + widgetSize};

    auto behavior = widgetBehavior(id, fullRect);

    // Draw circle
    Color bgColor = t->frameBg;
    if (behavior.held)
        bgColor = t->frameBgActive;
    else if (behavior.hovered)
        bgColor = t->frameBgHovered;

    drawList().addCircleFilled(circleCenter, circleRadius, bgColor);
    drawList().addCircle(circleCenter, circleRadius, t->border);

    // Draw inner circle if active
    if (active)
    {
        drawList().addCircleFilled(circleCenter, circleRadius * 0.5f, t->checkMark);
    }

    // Draw label
    Vec2 textPos = pos + Vec2{circleRadius * 2 + 4, (circleRadius * 2 - textSize.y) * 0.5f};
    drawList().addText(textPos, label, t->text);

    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(widgetSize);
    }

    return behavior.released;
}

bool radioButton(StringView label, i32* value, i32 buttonValue)
{
    bool pressed = radioButton(label, *value == buttonValue);
    if (pressed)
    {
        *value = buttonValue;
    }
    return pressed;
}

// ============================================================================
// Slider Implementation (generic)
// ============================================================================

namespace
{

template <typename T>
bool sliderScalar(StringView label, T* value, T min, T max, StringView format, f32 width = 0.0f)
{
    auto* c = ctx();
    if (!c || !value)
        return false;

    WidgetId id = c->getId(label);
    auto* t = theme();
    auto* font = c->getFont();

    Vec2 textSize = font ? font->calcTextSize(label) : Vec2{60, 14};
    f32 sliderWidth = width > 0.0f ? width : c->getContentWidth() * 0.65f;
    f32 height = 20.0f;

    Vec2 pos = c->getCursorScreenPos();
    Rect sliderRect{pos, pos + Vec2{sliderWidth, height}};

    auto behavior = widgetBehavior(id, sliderRect);

    bool changed = false;

    if (behavior.held && min != max)
    {
        f32 normalizedPos = (c->getMousePos().x - sliderRect.min.x) / sliderRect.width();
        normalizedPos = std::clamp(normalizedPos, 0.0f, 1.0f);

        T newValue;
        if constexpr (std::is_floating_point_v<T>)
        {
            newValue = min + static_cast<T>(normalizedPos) * (max - min);
        }
        else
        {
            newValue = min + static_cast<T>(normalizedPos * (max - min));
        }

        if (*value != newValue)
        {
            *value = newValue;
            changed = true;
        }
    }

    // Draw background
    Color bgColor = t->frameBg;
    if (behavior.held)
        bgColor = t->frameBgActive;
    else if (behavior.hovered)
        bgColor = t->frameBgHovered;

    drawList().addRectFilled(sliderRect, bgColor, t->frameRounding);

    // Draw grab
    if (min != max)
    {
        f32 normalizedValue;
        if constexpr (std::is_floating_point_v<T>)
        {
            normalizedValue = static_cast<f32>(*value - min) / static_cast<f32>(max - min);
        }
        else
        {
            normalizedValue = static_cast<f32>(*value - min) / static_cast<f32>(max - min);
        }
        normalizedValue = std::clamp(normalizedValue, 0.0f, 1.0f);

        f32 grabWidth = std::max(t->grabMinSize, sliderRect.height());
        f32 grabX = sliderRect.min.x + normalizedValue * (sliderRect.width() - grabWidth);
        Rect grabRect{{grabX, sliderRect.min.y}, {grabX + grabWidth, sliderRect.max.y}};

        Color grabColor = behavior.held ? t->sliderGrabActive : t->sliderGrab;
        drawList().addRectFilled(grabRect, grabColor, t->grabRounding);
    }

    // Draw value text
    char buf[64];
    if constexpr (std::is_floating_point_v<T>)
    {
        std::snprintf(buf, sizeof(buf), format.data(), *value);
    }
    else
    {
        std::snprintf(buf, sizeof(buf), format.data(), *value);
    }
    StringView valueStr(buf);
    Vec2 valueSize = font ? font->calcTextSize(valueStr) : Vec2{30, 14};
    Vec2 valuePos = sliderRect.center() - valueSize * 0.5f;
    drawList().addText(valuePos, valueStr, t->text);

    // Draw label
    c->sameLine();
    text(label);

    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(Vec2{sliderWidth, height});
    }

    return changed;
}

}  // anonymous namespace

bool sliderFloat(StringView label, f32* value, f32 min, f32 max, StringView format)
{
    return sliderScalar(label, value, min, max, format);
}

bool sliderFloat2(StringView label, f32 value[2], f32 min, f32 max, StringView format)
{
    bool changed = false;
    ctx()->pushId(label);
    changed |= sliderFloat("##X", &value[0], min, max, format);
    ctx()->sameLine();
    changed |= sliderFloat("##Y", &value[1], min, max, format);
    ctx()->popId();
    ctx()->sameLine();
    text(label);
    return changed;
}

bool sliderFloat3(StringView label, f32 value[3], f32 min, f32 max, StringView format)
{
    bool changed = false;
    ctx()->pushId(label);
    changed |= sliderFloat("##X", &value[0], min, max, format);
    ctx()->sameLine();
    changed |= sliderFloat("##Y", &value[1], min, max, format);
    ctx()->sameLine();
    changed |= sliderFloat("##Z", &value[2], min, max, format);
    ctx()->popId();
    ctx()->sameLine();
    text(label);
    return changed;
}

bool sliderFloat4(StringView label, f32 value[4], f32 min, f32 max, StringView format)
{
    bool changed = false;
    ctx()->pushId(label);
    changed |= sliderFloat("##X", &value[0], min, max, format);
    ctx()->sameLine();
    changed |= sliderFloat("##Y", &value[1], min, max, format);
    ctx()->sameLine();
    changed |= sliderFloat("##Z", &value[2], min, max, format);
    ctx()->sameLine();
    changed |= sliderFloat("##W", &value[3], min, max, format);
    ctx()->popId();
    ctx()->sameLine();
    text(label);
    return changed;
}

bool sliderInt(StringView label, i32* value, i32 min, i32 max, StringView format)
{
    return sliderScalar(label, value, min, max, format);
}

bool sliderInt2(StringView label, i32 value[2], i32 min, i32 max, StringView format)
{
    bool changed = false;
    ctx()->pushId(label);
    changed |= sliderInt("##X", &value[0], min, max, format);
    ctx()->sameLine();
    changed |= sliderInt("##Y", &value[1], min, max, format);
    ctx()->popId();
    ctx()->sameLine();
    text(label);
    return changed;
}

bool sliderInt3(StringView label, i32 value[3], i32 min, i32 max, StringView format)
{
    bool changed = false;
    ctx()->pushId(label);
    changed |= sliderInt("##X", &value[0], min, max, format);
    ctx()->sameLine();
    changed |= sliderInt("##Y", &value[1], min, max, format);
    ctx()->sameLine();
    changed |= sliderInt("##Z", &value[2], min, max, format);
    ctx()->popId();
    ctx()->sameLine();
    text(label);
    return changed;
}

bool sliderInt4(StringView label, i32 value[4], i32 min, i32 max, StringView format)
{
    bool changed = false;
    ctx()->pushId(label);
    changed |= sliderInt("##X", &value[0], min, max, format);
    ctx()->sameLine();
    changed |= sliderInt("##Y", &value[1], min, max, format);
    ctx()->sameLine();
    changed |= sliderInt("##Z", &value[2], min, max, format);
    ctx()->sameLine();
    changed |= sliderInt("##W", &value[3], min, max, format);
    ctx()->popId();
    ctx()->sameLine();
    text(label);
    return changed;
}

bool sliderAngle(StringView label, f32* radians, f32 minDegrees, f32 maxDegrees)
{
    f32 degrees = *radians * (180.0f / 3.14159265f);
    bool changed = sliderFloat(label, &degrees, minDegrees, maxDegrees, "%.0f deg");
    if (changed)
    {
        *radians = degrees * (3.14159265f / 180.0f);
    }
    return changed;
}

bool vSliderFloat(StringView label, Vec2 size, f32* value, f32 min, f32 max, StringView format)
{
    // TODO: Implement vertical slider
    (void)size;
    return sliderFloat(label, value, min, max, format);
}

bool vSliderInt(StringView label, Vec2 size, i32* value, i32 min, i32 max, StringView format)
{
    // TODO: Implement vertical slider
    (void)size;
    return sliderInt(label, value, min, max, format);
}

// ============================================================================
// Drag Implementation
// ============================================================================

bool dragFloat(StringView label, f32* value, f32 speed, f32 min, f32 max, StringView format)
{
    auto* c = ctx();
    if (!c || !value)
        return false;

    WidgetId id = c->getId(label);
    auto* t = theme();

    f32 width = c->getContentWidth() * 0.65f;
    f32 height = 20.0f;

    Vec2 pos = c->getCursorScreenPos();
    Rect rect{pos, pos + Vec2{width, height}};

    auto behavior = widgetBehavior(id, rect);

    bool changed = false;

    if (behavior.held)
    {
        f32 delta = c->getMouseDelta().x * speed;
        f32 newValue = *value + delta;

        if (min < max)
        {
            newValue = std::clamp(newValue, min, max);
        }

        if (*value != newValue)
        {
            *value = newValue;
            changed = true;
        }
    }

    // Draw
    Color bgColor = t->frameBg;
    if (behavior.held)
        bgColor = t->frameBgActive;
    else if (behavior.hovered)
        bgColor = t->frameBgHovered;

    drawList().addRectFilled(rect, bgColor, t->frameRounding);

    // Draw value
    char buf[64];
    std::snprintf(buf, sizeof(buf), format.data(), *value);
    auto* font = c->getFont();
    Vec2 textSize = font ? font->calcTextSize(buf) : Vec2{30, 14};
    Vec2 textPos = rect.center() - textSize * 0.5f;
    drawList().addText(textPos, buf, t->text);

    c->sameLine();
    text(label);

    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(Vec2{width, height});
    }

    return changed;
}

bool dragFloat2(StringView label, f32 value[2], f32 speed, f32 min, f32 max, StringView format)
{
    bool changed = false;
    ctx()->pushId(label);
    changed |= dragFloat("##X", &value[0], speed, min, max, format);
    ctx()->sameLine();
    changed |= dragFloat("##Y", &value[1], speed, min, max, format);
    ctx()->popId();
    ctx()->sameLine();
    text(label);
    return changed;
}

bool dragFloat3(StringView label, f32 value[3], f32 speed, f32 min, f32 max, StringView format)
{
    bool changed = false;
    ctx()->pushId(label);
    changed |= dragFloat("##X", &value[0], speed, min, max, format);
    ctx()->sameLine();
    changed |= dragFloat("##Y", &value[1], speed, min, max, format);
    ctx()->sameLine();
    changed |= dragFloat("##Z", &value[2], speed, min, max, format);
    ctx()->popId();
    ctx()->sameLine();
    text(label);
    return changed;
}

bool dragFloat4(StringView label, f32 value[4], f32 speed, f32 min, f32 max, StringView format)
{
    bool changed = false;
    ctx()->pushId(label);
    changed |= dragFloat("##X", &value[0], speed, min, max, format);
    ctx()->sameLine();
    changed |= dragFloat("##Y", &value[1], speed, min, max, format);
    ctx()->sameLine();
    changed |= dragFloat("##Z", &value[2], speed, min, max, format);
    ctx()->sameLine();
    changed |= dragFloat("##W", &value[3], speed, min, max, format);
    ctx()->popId();
    ctx()->sameLine();
    text(label);
    return changed;
}

bool dragInt(StringView label, i32* value, f32 speed, i32 min, i32 max, StringView format)
{
    f32 fValue = static_cast<f32>(*value);
    bool changed = dragFloat(label, &fValue, speed, static_cast<f32>(min), static_cast<f32>(max), format);
    if (changed)
    {
        *value = static_cast<i32>(fValue);
    }
    return changed;
}

bool dragInt2(StringView label, i32 value[2], f32 speed, i32 min, i32 max, StringView format)
{
    f32 fValues[2] = {static_cast<f32>(value[0]), static_cast<f32>(value[1])};
    bool changed = dragFloat2(label, fValues, speed, static_cast<f32>(min), static_cast<f32>(max), format);
    if (changed)
    {
        value[0] = static_cast<i32>(fValues[0]);
        value[1] = static_cast<i32>(fValues[1]);
    }
    return changed;
}

bool dragInt3(StringView label, i32 value[3], f32 speed, i32 min, i32 max, StringView format)
{
    f32 fValues[3] = {static_cast<f32>(value[0]), static_cast<f32>(value[1]), static_cast<f32>(value[2])};
    bool changed = dragFloat3(label, fValues, speed, static_cast<f32>(min), static_cast<f32>(max), format);
    if (changed)
    {
        value[0] = static_cast<i32>(fValues[0]);
        value[1] = static_cast<i32>(fValues[1]);
        value[2] = static_cast<i32>(fValues[2]);
    }
    return changed;
}

bool dragInt4(StringView label, i32 value[4], f32 speed, i32 min, i32 max, StringView format)
{
    f32 fValues[4] = {static_cast<f32>(value[0]), static_cast<f32>(value[1]), static_cast<f32>(value[2]),
                      static_cast<f32>(value[3])};
    bool changed = dragFloat4(label, fValues, speed, static_cast<f32>(min), static_cast<f32>(max), format);
    if (changed)
    {
        value[0] = static_cast<i32>(fValues[0]);
        value[1] = static_cast<i32>(fValues[1]);
        value[2] = static_cast<i32>(fValues[2]);
        value[3] = static_cast<i32>(fValues[3]);
    }
    return changed;
}

// ============================================================================
// Input Fields
// ============================================================================

bool inputText(StringView label, char* buf, usize bufSize, InputTextFlags flags)
{
    (void)bufSize;  // TODO: Use for input validation

    auto* c = ctx();
    if (!c || !buf)
        return false;

    WidgetId id = c->getId(label);
    auto* t = theme();

    f32 width = c->getContentWidth() * 0.65f;
    f32 height = 20.0f;

    Vec2 pos = c->getCursorScreenPos();
    Rect rect{pos, pos + Vec2{width, height}};

    auto behavior = widgetBehavior(id, rect);

    bool changed = false;
    bool focused = c->isWidgetFocused(id);

    if (behavior.pressed)
    {
        c->setFocusedWidget(id);
        focused = true;
    }

    // Handle text input when focused
    if (focused)
    {
        // Handle backspace
        if (c->isKeyPressed(Key::Backspace, true))
        {
            usize len = std::strlen(buf);
            if (len > 0)
            {
                buf[len - 1] = '\0';
                changed = true;
            }
        }

        // Handle escape
        if (c->isKeyPressed(Key::Escape))
        {
            c->setFocusedWidget(InvalidWidgetId);
            focused = false;
        }

        // TODO: Handle actual text input from context
    }

    // Draw
    Color bgColor = t->frameBg;
    if (focused)
        bgColor = t->frameBgActive;
    else if (behavior.hovered)
        bgColor = t->frameBgHovered;

    drawList().addRectFilled(rect, bgColor, t->frameRounding);
    drawList().addRect(rect, focused ? t->border : t->borderShadow, t->frameRounding);

    // Draw text
    StringView displayText = hasFlag(flags, InputTextFlags::Password) ? "********" : StringView(buf);
    Vec2 textPos = rect.min + Vec2{4, (height - 14) * 0.5f};
    drawList().addText(textPos, displayText, t->text);

    // Draw cursor when focused
    if (focused)
    {
        auto* font = c->getFont();
        f32 cursorX = textPos.x + (font ? font->calcTextWidth(buf) : 0.0f);
        drawList().addLine({cursorX, rect.min.y + 2}, {cursorX, rect.max.y - 2}, t->text, 1.0f);
    }

    c->sameLine();
    text(label);

    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(Vec2{width, height});
    }

    return changed;
}

bool inputText(StringView label, String& str, InputTextFlags flags)
{
    char buf[256];
    std::strncpy(buf, str.c_str(), sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    bool changed = inputText(label, buf, sizeof(buf), flags);
    if (changed)
    {
        str = buf;
    }
    return changed;
}

bool inputTextMultiline(StringView label, char* buf, usize bufSize, Vec2 size, InputTextFlags flags)
{
    // TODO: Implement multiline text input
    (void)size;
    return inputText(label, buf, bufSize, flags | InputTextFlags::Multiline);
}

bool inputTextMultiline(StringView label, String& str, Vec2 size, InputTextFlags flags)
{
    char buf[4096];
    std::strncpy(buf, str.c_str(), sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    bool changed = inputTextMultiline(label, buf, sizeof(buf), size, flags);
    if (changed)
    {
        str = buf;
    }
    return changed;
}

bool inputFloat(StringView label, f32* value, f32 step, f32 stepFast, StringView format)
{
    (void)step;
    (void)stepFast;
    char buf[64];
    std::snprintf(buf, sizeof(buf), format.data(), *value);

    bool changed = inputText(label, buf, sizeof(buf));
    if (changed)
    {
        f32 newValue;
        if (std::sscanf(buf, "%f", &newValue) == 1)
        {
            *value = newValue;
        }
    }
    return changed;
}

bool inputFloat2(StringView label, f32 value[2], StringView format)
{
    bool changed = false;
    ctx()->pushId(label);
    changed |= inputFloat("##X", &value[0], 0, 0, format);
    ctx()->sameLine();
    changed |= inputFloat("##Y", &value[1], 0, 0, format);
    ctx()->popId();
    ctx()->sameLine();
    text(label);
    return changed;
}

bool inputFloat3(StringView label, f32 value[3], StringView format)
{
    bool changed = false;
    ctx()->pushId(label);
    changed |= inputFloat("##X", &value[0], 0, 0, format);
    ctx()->sameLine();
    changed |= inputFloat("##Y", &value[1], 0, 0, format);
    ctx()->sameLine();
    changed |= inputFloat("##Z", &value[2], 0, 0, format);
    ctx()->popId();
    ctx()->sameLine();
    text(label);
    return changed;
}

bool inputFloat4(StringView label, f32 value[4], StringView format)
{
    bool changed = false;
    ctx()->pushId(label);
    changed |= inputFloat("##X", &value[0], 0, 0, format);
    ctx()->sameLine();
    changed |= inputFloat("##Y", &value[1], 0, 0, format);
    ctx()->sameLine();
    changed |= inputFloat("##Z", &value[2], 0, 0, format);
    ctx()->sameLine();
    changed |= inputFloat("##W", &value[3], 0, 0, format);
    ctx()->popId();
    ctx()->sameLine();
    text(label);
    return changed;
}

bool inputInt(StringView label, i32* value, i32 step, i32 stepFast)
{
    (void)step;
    (void)stepFast;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%d", *value);

    bool changed = inputText(label, buf, sizeof(buf), InputTextFlags::CharsDecimal);
    if (changed)
    {
        i32 newValue;
        if (std::sscanf(buf, "%d", &newValue) == 1)
        {
            *value = newValue;
        }
    }
    return changed;
}

bool inputInt2(StringView label, i32 value[2])
{
    bool changed = false;
    ctx()->pushId(label);
    changed |= inputInt("##X", &value[0]);
    ctx()->sameLine();
    changed |= inputInt("##Y", &value[1]);
    ctx()->popId();
    ctx()->sameLine();
    text(label);
    return changed;
}

bool inputInt3(StringView label, i32 value[3])
{
    bool changed = false;
    ctx()->pushId(label);
    changed |= inputInt("##X", &value[0]);
    ctx()->sameLine();
    changed |= inputInt("##Y", &value[1]);
    ctx()->sameLine();
    changed |= inputInt("##Z", &value[2]);
    ctx()->popId();
    ctx()->sameLine();
    text(label);
    return changed;
}

bool inputInt4(StringView label, i32 value[4])
{
    bool changed = false;
    ctx()->pushId(label);
    changed |= inputInt("##X", &value[0]);
    ctx()->sameLine();
    changed |= inputInt("##Y", &value[1]);
    ctx()->sameLine();
    changed |= inputInt("##Z", &value[2]);
    ctx()->sameLine();
    changed |= inputInt("##W", &value[3]);
    ctx()->popId();
    ctx()->sameLine();
    text(label);
    return changed;
}

// ============================================================================
// Color Editors (Stub implementations)
// ============================================================================

bool colorEdit3(StringView label, f32 col[3], ColorEditFlags flags)
{
    (void)flags;
    return dragFloat3(label, col, 0.01f, 0.0f, 1.0f, "%.2f");
}

bool colorEdit4(StringView label, f32 col[4], ColorEditFlags flags)
{
    (void)flags;
    return dragFloat4(label, col, 0.01f, 0.0f, 1.0f, "%.2f");
}

bool colorPicker3(StringView label, f32 col[3], ColorEditFlags flags)
{
    return colorEdit3(label, col, flags);
}

bool colorPicker4(StringView label, f32 col[4], ColorEditFlags flags)
{
    return colorEdit4(label, col, flags);
}

bool colorButton(StringView descId, Color col, ColorEditFlags flags, Vec2 size)
{
    (void)flags;
    auto* c = ctx();
    if (!c)
        return false;

    Vec2 buttonSize = size.x > 0 && size.y > 0 ? size : Vec2{20, 20};
    Vec2 pos = c->getCursorScreenPos();
    Rect rect{pos, pos + buttonSize};

    drawList().addRectFilled(rect, col);
    drawList().addRect(rect, theme()->border);

    return invisibleButton(descId, buttonSize);
}

// ============================================================================
// Combo & List Boxes
// ============================================================================

bool beginCombo(StringView label, StringView previewValue)
{
    // TODO: Implement combo popup
    auto* c = ctx();
    if (!c)
        return false;

    WidgetId id = c->getId(label);
    auto* t = theme();

    f32 width = c->getContentWidth() * 0.65f;
    f32 height = 20.0f;

    Vec2 pos = c->getCursorScreenPos();
    Rect rect{pos, pos + Vec2{width, height}};

    auto behavior = widgetBehavior(id, rect);

    // Draw
    Color bgColor = t->frameBg;
    if (behavior.held)
        bgColor = t->frameBgActive;
    else if (behavior.hovered)
        bgColor = t->frameBgHovered;

    drawList().addRectFilled(rect, bgColor, t->frameRounding);

    // Draw preview value
    Vec2 textPos = rect.min + Vec2{4, (height - 14) * 0.5f};
    drawList().addText(textPos, previewValue, t->text);

    // Draw dropdown arrow
    Vec2 arrowCenter = rect.max - Vec2{10, height * 0.5f};
    drawList().addTriangleFilled(arrowCenter + Vec2{0, 4}, arrowCenter + Vec2{-4, -2}, arrowCenter + Vec2{4, -2},
                                 t->text);

    c->sameLine();
    text(label);

    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(Vec2{width, height});
    }

    return behavior.released;
}

void endCombo()
{
    // TODO: Close combo popup
}

bool combo(StringView label, i32* currentItem, std::span<const StringView> items)
{
    if (items.empty() || !currentItem)
        return false;

    StringView preview =
        (*currentItem >= 0 && static_cast<usize>(*currentItem) < items.size()) ? items[*currentItem] : "";

    bool changed = false;
    if (beginCombo(label, preview))
    {
        for (usize i = 0; i < items.size(); ++i)
        {
            bool selected = static_cast<i32>(i) == *currentItem;
            if (selectable(items[i], selected))
            {
                *currentItem = static_cast<i32>(i);
                changed = true;
            }
        }
        endCombo();
    }
    return changed;
}

bool combo(StringView label, i32* currentItem, StringView itemsSeparatedByZeros)
{
    // Parse null-separated items
    std::vector<StringView> items;
    const char* ptr = itemsSeparatedByZeros.data();
    const char* end = ptr + itemsSeparatedByZeros.size();

    while (ptr < end && *ptr)
    {
        items.emplace_back(ptr);
        ptr += std::strlen(ptr) + 1;
    }

    return combo(label, currentItem, items);
}

bool beginListBox(StringView label, Vec2 size)
{
    (void)label;
    (void)size;
    // TODO: Implement list box
    return true;
}

void endListBox()
{
    // TODO: End list box
}

bool listBox(StringView label, i32* currentItem, std::span<const StringView> items, i32 heightInItems)
{
    (void)heightInItems;
    bool changed = false;

    if (beginListBox(label))
    {
        for (usize i = 0; i < items.size(); ++i)
        {
            bool selected = static_cast<i32>(i) == *currentItem;
            if (selectable(items[i], selected))
            {
                *currentItem = static_cast<i32>(i);
                changed = true;
            }
        }
        endListBox();
    }

    return changed;
}

// ============================================================================
// Selectable
// ============================================================================

bool selectable(StringView label, bool selected, SelectableFlags flags, Vec2 size)
{
    (void)flags;

    auto* c = ctx();
    if (!c)
        return false;

    WidgetId id = c->getId(label);
    auto* t = theme();
    auto* font = c->getFont();

    Vec2 textSize = font ? font->calcTextSize(label) : Vec2{80, 14};
    Vec2 widgetSize = size.x > 0 && size.y > 0 ? size : Vec2{c->getContentWidth(), textSize.y + 4};

    Vec2 pos = c->getCursorScreenPos();
    Rect rect{pos, pos + widgetSize};

    auto behavior = widgetBehavior(id, rect);

    // Draw
    if (selected || behavior.hovered)
    {
        Color bgColor = selected ? t->header : t->headerHovered;
        drawList().addRectFilled(rect, bgColor);
    }

    Vec2 textPos = rect.min + Vec2{4, (widgetSize.y - textSize.y) * 0.5f};
    drawList().addText(textPos, label, t->text);

    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(widgetSize);
    }

    return behavior.released;
}

bool selectable(StringView label, bool* selected, SelectableFlags flags, Vec2 size)
{
    bool pressed = selectable(label, *selected, flags, size);
    if (pressed)
    {
        *selected = !*selected;
    }
    return pressed;
}

// ============================================================================
// Progress Bar
// ============================================================================

void progressBar(f32 fraction, Vec2 size, StringView overlay)
{
    auto* c = ctx();
    if (!c)
        return;

    auto* t = theme();

    Vec2 widgetSize = Vec2{size.x >= 0.0f ? size.x : c->getContentWidth(), size.y > 0.0f ? size.y : 14.0f};

    Vec2 pos = c->getCursorScreenPos();
    Rect rect{pos, pos + widgetSize};

    // Draw background
    drawList().addRectFilled(rect, t->frameBg, t->frameRounding);

    // Draw progress
    fraction = std::clamp(fraction, 0.0f, 1.0f);
    if (fraction > 0.0f)
    {
        Rect progressRect{rect.min, {rect.min.x + rect.width() * fraction, rect.max.y}};
        drawList().addRectFilled(progressRect, t->plotHistogram, t->frameRounding);
    }

    // Draw overlay text
    if (!overlay.empty())
    {
        auto* font = c->getFont();
        Vec2 textSize = font ? font->calcTextSize(overlay) : Vec2{30, 12};
        Vec2 textPos = rect.center() - textSize * 0.5f;
        drawList().addText(textPos, overlay, t->text);
    }

    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(widgetSize);
    }
}

// ============================================================================
// Tooltip
// ============================================================================

void setTooltip(StringView text)
{
    beginTooltip();
    gui::text(text);
    endTooltip();
}

void beginTooltip()
{
    // TODO: Implement tooltip window
}

void endTooltip()
{
    // TODO: End tooltip
}

// ============================================================================
// Image
// ============================================================================

void image(uintptr_t textureId, Vec2 size, Vec2 uv0, Vec2 uv1, Color tintColor, Color borderColor)
{
    auto* c = ctx();
    if (!c)
        return;

    Vec2 pos = c->getCursorScreenPos();
    Rect rect{pos, pos + size};

    drawList().addImage(textureId, rect, uv0, uv1, tintColor);

    if (borderColor.a > 0)
    {
        drawList().addRect(rect, borderColor);
    }

    auto* window = c->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(size);
    }
}

}  // namespace dakt::gui

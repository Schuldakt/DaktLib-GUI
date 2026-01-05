/**
 * @file Widget.cpp
 * @brief Implementation of Widget base class and concrete widgets
 */

#include "dakt/gui/retained/Widget.hpp"
#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/draw/DrawList.hpp"
#include <algorithm>
#include <cmath>

namespace dakt::gui {

// ============================================================================
// Widget Base Class
// ============================================================================

Widget::Widget() = default;

Widget::Widget(const std::string& id) : id_(id) {}

Widget::~Widget() = default;

Widget::Widget(Widget&& other) noexcept
    : id_(std::move(other.id_)), parent_(other.parent_), children_(std::move(other.children_)), bounds_(other.bounds_), minSize_(other.minSize_), maxSize_(other.maxSize_), preferredSize_(other.preferredSize_), margin_(other.margin_),
      padding_(other.padding_), flags_(other.flags_), visible_(other.visible_), dirty_(other.dirty_), needsLayout_(other.needsLayout_), onClick_(std::move(other.onClick_)), onDoubleClick_(std::move(other.onDoubleClick_)),
      onHover_(std::move(other.onHover_)), onValueChanged_(std::move(other.onValueChanged_)) {
    other.parent_ = nullptr;

    // Update parent pointers in children
    for (auto& child : children_) {
        child->parent_ = this;
    }
}

Widget& Widget::operator=(Widget&& other) noexcept {
    if (this != &other) {
        id_ = std::move(other.id_);
        parent_ = other.parent_;
        children_ = std::move(other.children_);
        bounds_ = other.bounds_;
        minSize_ = other.minSize_;
        maxSize_ = other.maxSize_;
        preferredSize_ = other.preferredSize_;
        margin_ = other.margin_;
        padding_ = other.padding_;
        flags_ = other.flags_;
        visible_ = other.visible_;
        dirty_ = other.dirty_;
        needsLayout_ = other.needsLayout_;
        onClick_ = std::move(other.onClick_);
        onDoubleClick_ = std::move(other.onDoubleClick_);
        onHover_ = std::move(other.onHover_);
        onValueChanged_ = std::move(other.onValueChanged_);

        other.parent_ = nullptr;

        for (auto& child : children_) {
            child->parent_ = this;
        }
    }
    return *this;
}

Widget& Widget::addChild(std::unique_ptr<Widget> child) {
    child->parent_ = this;
    children_.push_back(std::move(child));
    markNeedsLayout();
    return *children_.back();
}

bool Widget::removeChild(Widget* child) {
    auto it = std::find_if(children_.begin(), children_.end(), [child](const std::unique_ptr<Widget>& ptr) { return ptr.get() == child; });
    if (it != children_.end()) {
        (*it)->parent_ = nullptr;
        children_.erase(it);
        markNeedsLayout();
        return true;
    }
    return false;
}

void Widget::clearChildren() {
    for (auto& child : children_) {
        child->parent_ = nullptr;
    }
    children_.clear();
    markNeedsLayout();
}

Widget* Widget::findChild(const std::string& id) {
    for (auto& child : children_) {
        if (child->getId() == id) {
            return child.get();
        }
        if (Widget* found = child->findChild(id)) {
            return found;
        }
    }
    return nullptr;
}

void Widget::setBounds(const Rect& bounds) {
    if (bounds_ != bounds) {
        bounds_ = bounds;
        markDirty();
    }
}

void Widget::setPosition(const Vec2& pos) {
    if (bounds_.x != pos.x || bounds_.y != pos.y) {
        bounds_.x = pos.x;
        bounds_.y = pos.y;
        markDirty();
    }
}

void Widget::setSize(const Vec2& size) {
    if (bounds_.width != size.x || bounds_.height != size.y) {
        bounds_.width = size.x;
        bounds_.height = size.y;
        markNeedsLayout();
    }
}

Vec2 Widget::measureContent() {
    // Default: use preferred size or sum of children
    if (preferredSize_.x > 0 && preferredSize_.y > 0) {
        return preferredSize_;
    }

    Vec2 size{0, 0};
    for (auto& child : children_) {
        Vec2 childSize = child->measureContent();
        size.x = std::max(size.x, childSize.x);
        size.y += childSize.y;
    }

    size.x += padding_.left + padding_.right;
    size.y += padding_.top + padding_.bottom;

    return size;
}

void Widget::layout(const Rect& available) {
    bounds_ = available;

    // Simple vertical stack layout for children
    float y = available.y + padding_.top;
    float contentWidth = available.width - padding_.left - padding_.right;

    for (auto& child : children_) {
        if (!child->isVisible())
            continue;

        Vec2 childSize = child->measureContent();
        childSize.x = std::min(childSize.x, contentWidth);

        Rect childBounds(available.x + padding_.left, y, childSize.x, childSize.y);

        child->layout(childBounds);
        y += childSize.y + 4.0f; // Default spacing
    }

    clearNeedsLayout();
}

void Widget::setFlags(RetainedWidgetFlags state) {
    if (flags_ != state) {
        flags_ = state;
        markDirty();
    }
}

void Widget::addFlag(RetainedWidgetFlags state) {
    RetainedWidgetFlags newState = flags_ | state;
    if (newState != flags_) {
        flags_ = newState;
        markDirty();
    }
}

void Widget::removeFlag(RetainedWidgetFlags state) {
    RetainedWidgetFlags newState = static_cast<RetainedWidgetFlags>(static_cast<uint32_t>(flags_) & ~static_cast<uint32_t>(state));
    if (newState != flags_) {
        flags_ = newState;
        markDirty();
    }
}

bool Widget::hasFlag(RetainedWidgetFlags state) const { return (static_cast<uint32_t>(flags_) & static_cast<uint32_t>(state)) != 0; }

void Widget::setEnabled(bool enabled) {
    if (enabled) {
        removeFlag(RetainedWidgetFlags::Disabled);
    } else {
        addFlag(RetainedWidgetFlags::Disabled);
    }
}

void Widget::markDirty() {
    dirty_ = true;
    propagateDirty();
}

void Widget::markNeedsLayout() {
    needsLayout_ = true;
    markDirty();
}

void Widget::propagateDirty() {
    if (parent_) {
        parent_->dirty_ = true;
        parent_->propagateDirty();
    }
}

bool Widget::handleInput(const WidgetEvent& event) {
    // Default: no handling
    return false;
}

void Widget::build(Context& ctx) {
    if (!visible_)
        return;

    DrawList& drawList = ctx.getDrawList();

    drawBackground(drawList);
    drawContent(drawList);
    drawChildren(ctx);

    clearDirty();
}

void Widget::drawBackground(DrawList& drawList) {
    // Default: no background
}

void Widget::drawContent(DrawList& drawList) {
    // Default: no content
}

void Widget::drawChildren(Context& ctx) {
    for (auto& child : children_) {
        child->build(ctx);
    }
}

void Widget::fireEvent(WidgetEventType type, const WidgetEvent& baseEvent) {
    WidgetEvent event = baseEvent;
    event.type = type;
    event.source = this;

    switch (type) {
    case WidgetEventType::Click:
        if (onClick_)
            onClick_(event);
        break;
    case WidgetEventType::DoubleClick:
        if (onDoubleClick_)
            onDoubleClick_(event);
        break;
    case WidgetEventType::Hover:
        if (onHover_)
            onHover_(event);
        break;
    case WidgetEventType::ValueChanged:
        if (onValueChanged_)
            onValueChanged_(event);
        break;
    default:
        break;
    }
}

// ============================================================================
// Label Widget
// ============================================================================

Label::Label() = default;

Label::Label(const std::string& text) : text_(text) {}

void Label::setText(const std::string& text) {
    if (text_ != text) {
        text_ = text;
        markDirty();
    }
}

Vec2 Label::measureContent() {
    float width = static_cast<float>(text_.length()) * fontSize_ * 0.6f;
    float height = fontSize_ * 1.2f;
    return Vec2(width + padding_.left + padding_.right, height + padding_.top + padding_.bottom);
}

void Label::drawContent(DrawList& drawList) {
    Vec2 textPos(bounds_.x + padding_.left, bounds_.y + padding_.top);
    drawList.drawText(textPos, text_.c_str(), color_, fontSize_);
}

// ============================================================================
// Button Widget
// ============================================================================

Button::Button() { setPadding(Insets(8, 16, 8, 16)); }

Button::Button(const std::string& label) : label_(label) { setPadding(Insets(8, 16, 8, 16)); }

void Button::setLabel(const std::string& label) {
    if (label_ != label) {
        label_ = label;
        markDirty();
    }
}

Vec2 Button::measureContent() {
    float textWidth = static_cast<float>(label_.length()) * 8.0f;
    float textHeight = 16.0f;
    return Vec2(textWidth + padding_.left + padding_.right, textHeight + padding_.top + padding_.bottom);
}

bool Button::handleInput(const WidgetEvent& event) {
    if (isDisabled())
        return false;

    switch (event.type) {
    case WidgetEventType::Hover:
        addFlag(RetainedWidgetFlags::Hovered);
        return true;

    case WidgetEventType::HoverEnd:
        removeFlag(RetainedWidgetFlags::Hovered);
        return true;

    case WidgetEventType::Press:
        addFlag(RetainedWidgetFlags::Active);
        return true;

    case WidgetEventType::Release:
        if (isActive()) {
            removeFlag(RetainedWidgetFlags::Active);
            fireEvent(WidgetEventType::Click, event);
        }
        return true;

    default:
        return false;
    }
}

void Button::drawBackground(DrawList& drawList) {
    Color bgColor = backgroundColor_;
    if (isActive()) {
        bgColor = activeColor_;
    } else if (isHovered()) {
        bgColor = hoverColor_;
    }

    if (borderRadius_ > 0) {
        drawList.drawRectFilledRounded(bounds_, bgColor, borderRadius_);
    } else {
        drawList.drawRectFilled(bounds_, bgColor);
    }
}

void Button::drawContent(DrawList& drawList) {
    // Center the label
    float textWidth = static_cast<float>(label_.length()) * 8.0f;
    float textHeight = 16.0f;

    float x = bounds_.x + (bounds_.width - textWidth) * 0.5f;
    float y = bounds_.y + (bounds_.height - textHeight) * 0.5f;

    Color textColor = isDisabled() ? Color(128, 128, 128, 255) : Color(255, 255, 255, 255);
    drawList.drawText(Vec2(x, y), label_.c_str(), textColor);
}

// ============================================================================
// Checkbox Widget
// ============================================================================

Checkbox::Checkbox() { setPadding(Insets(4, 4, 4, 4)); }

Checkbox::Checkbox(const std::string& label) : label_(label) { setPadding(Insets(4, 4, 4, 4)); }

void Checkbox::setLabel(const std::string& label) {
    if (label_ != label) {
        label_ = label;
        markDirty();
    }
}

void Checkbox::setChecked(bool checked) {
    if (checked) {
        addFlag(RetainedWidgetFlags::Checked);
    } else {
        removeFlag(RetainedWidgetFlags::Checked);
    }
}

Vec2 Checkbox::measureContent() {
    float textWidth = static_cast<float>(label_.length()) * 8.0f;
    float width = checkSize_ + 8.0f + textWidth + padding_.left + padding_.right;
    float height = std::max(checkSize_, 16.0f) + padding_.top + padding_.bottom;
    return Vec2(width, height);
}

bool Checkbox::handleInput(const WidgetEvent& event) {
    if (isDisabled())
        return false;

    switch (event.type) {
    case WidgetEventType::Hover:
        addFlag(RetainedWidgetFlags::Hovered);
        return true;

    case WidgetEventType::HoverEnd:
        removeFlag(RetainedWidgetFlags::Hovered);
        return true;

    case WidgetEventType::Click:
        setChecked(!isChecked());
        fireEvent(WidgetEventType::ValueChanged, event);
        return true;

    default:
        return false;
    }
}

void Checkbox::drawContent(DrawList& drawList) {
    float x = bounds_.x + padding_.left;
    float y = bounds_.y + (bounds_.height - checkSize_) * 0.5f;

    // Draw checkbox box
    Rect boxRect(x, y, checkSize_, checkSize_);
    Color boxColor = isHovered() ? Color(102, 102, 102, 255) : Color(77, 77, 77, 255);
    drawList.drawRectFilled(boxRect, boxColor);
    drawList.drawRect(boxRect, Color(128, 128, 128, 255));

    // Draw checkmark if checked
    if (isChecked()) {
        float inset = 4.0f;
        Rect checkRect(x + inset, y + inset, checkSize_ - inset * 2, checkSize_ - inset * 2);
        drawList.drawRectFilled(checkRect, Color(51, 153, 255, 255));
    }

    // Draw label
    float textX = x + checkSize_ + 8.0f;
    float textY = bounds_.y + (bounds_.height - 16.0f) * 0.5f;
    Color textColor = isDisabled() ? Color(128, 128, 128, 255) : Color(255, 255, 255, 255);
    drawList.drawText(Vec2(textX, textY), label_.c_str(), textColor);
}

// ============================================================================
// Slider Widget
// ============================================================================

Slider::Slider() { setMinSize(Vec2(100, 24)); }

Slider::Slider(float min, float max, float value) : min_(min), max_(max), value_(value) { setMinSize(Vec2(100, 24)); }

void Slider::setValue(float value) {
    value = std::clamp(value, min_, max_);
    if (step_ > 0) {
        value = std::round((value - min_) / step_) * step_ + min_;
    }
    if (value_ != value) {
        value_ = value;
        markDirty();
    }
}

Vec2 Slider::measureContent() {
    float labelWidth = label_.empty() ? 0 : static_cast<float>(label_.length()) * 8.0f + 8.0f;
    return Vec2(labelWidth + 100.0f + padding_.left + padding_.right, 24.0f + padding_.top + padding_.bottom);
}

float Slider::getNormalizedValue() const {
    if (max_ <= min_)
        return 0;
    return (value_ - min_) / (max_ - min_);
}

void Slider::setNormalizedValue(float normalized) { setValue(min_ + normalized * (max_ - min_)); }

bool Slider::handleInput(const WidgetEvent& event) {
    if (isDisabled())
        return false;

    switch (event.type) {
    case WidgetEventType::Hover:
        addFlag(RetainedWidgetFlags::Hovered);
        return true;

    case WidgetEventType::HoverEnd:
        removeFlag(RetainedWidgetFlags::Hovered);
        return true;

    case WidgetEventType::Press:
    case WidgetEventType::DragStart:
        addFlag(RetainedWidgetFlags::Active);
        dragging_ = true;
        // Fall through to update value
        [[fallthrough]];

    case WidgetEventType::DragMove: {
        if (!dragging_)
            return false;

        float labelWidth = label_.empty() ? 0 : static_cast<float>(label_.length()) * 8.0f + 8.0f;
        float trackX = bounds_.x + padding_.left + labelWidth;
        float trackWidth = bounds_.width - padding_.left - padding_.right - labelWidth - thumbRadius_ * 2;

        float localX = event.mousePos.x - trackX - thumbRadius_;
        float normalized = std::clamp(localX / trackWidth, 0.0f, 1.0f);
        float oldValue = value_;
        setNormalizedValue(normalized);

        if (value_ != oldValue) {
            fireEvent(WidgetEventType::ValueChanged, event);
        }
        return true;
    }

    case WidgetEventType::Release:
    case WidgetEventType::DragEnd:
        removeFlag(RetainedWidgetFlags::Active);
        dragging_ = false;
        return true;

    default:
        return false;
    }
}

void Slider::drawContent(DrawList& drawList) {
    float labelWidth = 0;
    if (!label_.empty()) {
        labelWidth = static_cast<float>(label_.length()) * 8.0f + 8.0f;
        float textY = bounds_.y + (bounds_.height - 16.0f) * 0.5f;
        drawList.drawText(Vec2(bounds_.x + padding_.left, textY), label_.c_str(), Color(255, 255, 255, 255));
    }

    float trackX = bounds_.x + padding_.left + labelWidth;
    float trackY = bounds_.y + (bounds_.height - trackHeight_) * 0.5f;
    float trackWidth = bounds_.width - padding_.left - padding_.right - labelWidth - thumbRadius_ * 2;

    // Draw track background
    Rect trackRect(trackX, trackY, trackWidth + thumbRadius_ * 2, trackHeight_);
    drawList.drawRectFilledRounded(trackRect, Color(51, 51, 51, 255), trackHeight_ * 0.5f);

    // Draw filled portion
    float fillWidth = getNormalizedValue() * trackWidth;
    Rect fillRect(trackX, trackY, fillWidth + thumbRadius_, trackHeight_);
    drawList.drawRectFilledRounded(fillRect, Color(51, 153, 255, 255), trackHeight_ * 0.5f);

    // Draw thumb
    float thumbX = trackX + thumbRadius_ + fillWidth;
    float thumbY = bounds_.y + bounds_.height * 0.5f;
    Color thumbColor = isActive() ? Color(255, 255, 255, 255) : Color(230, 230, 230, 255);
    drawList.drawCircleFilled(Vec2(thumbX, thumbY), thumbRadius_, thumbColor);
}

// ============================================================================
// TextInput Widget
// ============================================================================

TextInput::TextInput() {
    setPadding(Insets(6, 8, 6, 8));
    setMinSize(Vec2(100, 28));
}

void TextInput::setText(const std::string& text) {
    if (text_ != text) {
        text_ = text;
        if (cursorPos_ > text_.length()) {
            cursorPos_ = text_.length();
        }
        clearSelection();
        markDirty();
    }
}

void TextInput::setCursorPosition(size_t pos) {
    cursorPos_ = std::min(pos, text_.length());
    markDirty();
}

void TextInput::selectAll() {
    selectionStart_ = 0;
    selectionEnd_ = text_.length();
    cursorPos_ = text_.length();
    markDirty();
}

void TextInput::clearSelection() { selectionStart_ = selectionEnd_ = cursorPos_; }

std::string TextInput::getSelectedText() const {
    if (selectionStart_ == selectionEnd_)
        return "";

    size_t start = std::min(selectionStart_, selectionEnd_);
    size_t end = std::max(selectionStart_, selectionEnd_);
    return text_.substr(start, end - start);
}

void TextInput::insertText(const std::string& str) {
    deleteSelection();

    if (text_.length() + str.length() > maxLength_)
        return;

    text_.insert(cursorPos_, str);
    cursorPos_ += str.length();
    markDirty();
}

void TextInput::deleteSelection() {
    if (selectionStart_ == selectionEnd_)
        return;

    size_t start = std::min(selectionStart_, selectionEnd_);
    size_t end = std::max(selectionStart_, selectionEnd_);
    text_.erase(start, end - start);
    cursorPos_ = start;
    clearSelection();
    markDirty();
}

void TextInput::moveCursor(int delta, bool select) {
    size_t newPos = cursorPos_;
    if (delta < 0 && static_cast<size_t>(-delta) > cursorPos_) {
        newPos = 0;
    } else {
        newPos = static_cast<size_t>(static_cast<int>(cursorPos_) + delta);
    }
    newPos = std::min(newPos, text_.length());

    if (select) {
        if (selectionStart_ == selectionEnd_) {
            selectionStart_ = cursorPos_;
        }
        selectionEnd_ = newPos;
    } else {
        clearSelection();
    }

    cursorPos_ = newPos;
    markDirty();
}

Vec2 TextInput::measureContent() {
    float textWidth = static_cast<float>(std::max(text_.length(), placeholder_.length())) * 8.0f;
    return Vec2(std::max(textWidth, minSize_.x) + padding_.left + padding_.right, 16.0f + padding_.top + padding_.bottom);
}

bool TextInput::handleInput(const WidgetEvent& event) {
    if (isDisabled())
        return false;

    switch (event.type) {
    case WidgetEventType::Hover:
        addFlag(RetainedWidgetFlags::Hovered);
        return true;

    case WidgetEventType::HoverEnd:
        removeFlag(RetainedWidgetFlags::Hovered);
        return true;

    case WidgetEventType::Click:
        addFlag(RetainedWidgetFlags::Focused);
        // TODO: Position cursor based on click position
        return true;

    case WidgetEventType::Blur:
        removeFlag(RetainedWidgetFlags::Focused);
        return true;

    default:
        return false;
    }
}

void TextInput::drawBackground(DrawList& drawList) {
    Color bgColor = isFocused() ? Color(64, 64, 64, 255) : Color(51, 51, 51, 255);
    drawList.drawRectFilledRounded(bounds_, bgColor, 4.0f);

    Color borderColor = isFocused() ? Color(77, 153, 255, 255) : Color(102, 102, 102, 255);
    drawList.drawRectRounded(bounds_, borderColor, 4.0f);
}

void TextInput::drawContent(DrawList& drawList) {
    float textX = bounds_.x + padding_.left;
    float textY = bounds_.y + (bounds_.height - 16.0f) * 0.5f;

    // Draw selection highlight
    if (selectionStart_ != selectionEnd_ && isFocused()) {
        size_t start = std::min(selectionStart_, selectionEnd_);
        size_t end = std::max(selectionStart_, selectionEnd_);

        float selStartX = textX + static_cast<float>(start) * 8.0f;
        float selEndX = textX + static_cast<float>(end) * 8.0f;

        Rect selRect(selStartX, textY, selEndX - selStartX, 16.0f);
        drawList.drawRectFilled(selRect, Color(77, 128, 204, 128));
    }

    // Draw text or placeholder
    if (text_.empty() && !isFocused()) {
        drawList.drawText(Vec2(textX, textY), placeholder_.c_str(), Color(128, 128, 128, 255));
    } else {
        std::string displayText = password_ ? std::string(text_.length(), '*') : text_;
        drawList.drawText(Vec2(textX, textY), displayText.c_str(), Color(255, 255, 255, 255));
    }

    // Draw cursor
    if (isFocused() && cursorVisible_) {
        float cursorX = textX + static_cast<float>(cursorPos_) * 8.0f;
        drawList.drawLine(Vec2(cursorX, textY), Vec2(cursorX, textY + 16.0f), Color(255, 255, 255, 255), 1.0f);
    }
}

// ============================================================================
// Panel Widget
// ============================================================================

Panel::Panel() { setPadding(Insets(8, 8, 8, 8)); }

Panel::Panel(const std::string& title) : title_(title) { setPadding(Insets(8, 8, 8, 8)); }

void Panel::layout(const Rect& available) {
    bounds_ = available;

    float contentTop = available.y + padding_.top;
    if (showHeader_ && !title_.empty()) {
        contentTop += headerHeight_;
    }

    float y = contentTop;
    float contentWidth = available.width - padding_.left - padding_.right;

    for (auto& child : children_) {
        if (!child->isVisible())
            continue;

        Vec2 childSize = child->measureContent();
        childSize.x = std::min(childSize.x, contentWidth);

        Rect childBounds(available.x + padding_.left, y, contentWidth, childSize.y);
        child->layout(childBounds);
        y += childSize.y + 4.0f;
    }

    clearNeedsLayout();
}

void Panel::drawBackground(DrawList& drawList) {
    drawList.drawRectFilledRounded(bounds_, backgroundColor_, borderRadius_);

    if (showHeader_ && !title_.empty()) {
        Rect headerRect(bounds_.x, bounds_.y, bounds_.width, headerHeight_);
        drawList.drawRectFilledRounded(headerRect, Color(51, 51, 51, 255), BorderRadius(borderRadius_, borderRadius_, 0, 0));
    }
}

void Panel::drawContent(DrawList& drawList) {
    if (showHeader_ && !title_.empty()) {
        float textX = bounds_.x + padding_.left;
        float textY = bounds_.y + (headerHeight_ - 14.0f) * 0.5f;
        drawList.drawText(Vec2(textX, textY), title_.c_str(), Color(255, 255, 255, 255), 14.0f);
    }
}

// ============================================================================
// ScrollView Widget
// ============================================================================

ScrollView::ScrollView() { setPadding(Insets(4, 4, 4, 4)); }

void ScrollView::setScrollOffset(const Vec2& offset) {
    scrollOffset_ = offset;
    clampScrollOffset();
    markDirty();
}

void ScrollView::scrollToTop() {
    scrollOffset_.y = 0;
    markDirty();
}

void ScrollView::scrollToBottom() {
    scrollOffset_.y = std::max(0.0f, contentSize_.y - bounds_.height);
    markDirty();
}

void ScrollView::scrollTo(const Vec2& position) {
    scrollOffset_ = position;
    clampScrollOffset();
    markDirty();
}

void ScrollView::layout(const Rect& available) {
    bounds_ = available;
    updateContentSize();

    float y = -scrollOffset_.y;
    float contentWidth = available.width - padding_.left - padding_.right;
    if (verticalScroll_) {
        contentWidth -= scrollbarWidth_;
    }

    for (auto& child : children_) {
        if (!child->isVisible())
            continue;

        Vec2 childSize = child->measureContent();
        Rect childBounds(padding_.left, y, contentWidth, childSize.y);
        child->layout(childBounds);
        y += childSize.y + 4.0f;
    }

    clearNeedsLayout();
}

bool ScrollView::handleInput(const WidgetEvent& event) {
    // Calculate scrollbar bounds for hit testing
    float scrollbarX = bounds_.x + bounds_.width - scrollbarWidth_ - 2;
    float scrollbarY = bounds_.y + 2;
    float scrollbarHeight = bounds_.height - 4;

    bool hasVerticalScrollbar = verticalScroll_ && contentSize_.y > bounds_.height;

    switch (event.type) {
    case WidgetEventType::Press:
    case WidgetEventType::DragStart: {
        if (hasVerticalScrollbar) {
            // Check if clicking on vertical scrollbar track
            if (event.mousePos.x >= scrollbarX && event.mousePos.x <= scrollbarX + scrollbarWidth_ && event.mousePos.y >= scrollbarY && event.mousePos.y <= scrollbarY + scrollbarHeight) {

                scrollbarDragging_ = true;
                addFlag(RetainedWidgetFlags::Active);

                // Calculate thumb position and check if clicking on thumb vs track
                float thumbRatio = bounds_.height / contentSize_.y;
                float thumbHeight = std::max(20.0f, scrollbarHeight * thumbRatio);
                float maxScroll = contentSize_.y - bounds_.height;
                float thumbY = scrollbarY + (scrollbarHeight - thumbHeight) * (scrollOffset_.y / maxScroll);

                // If clicking on track (not thumb), jump to that position
                if (event.mousePos.y < thumbY || event.mousePos.y > thumbY + thumbHeight) {
                    float clickRatio = (event.mousePos.y - scrollbarY - thumbHeight * 0.5f) / (scrollbarHeight - thumbHeight);
                    clickRatio = std::max(0.0f, std::min(1.0f, clickRatio));
                    scrollOffset_.y = clickRatio * maxScroll;
                    clampScrollOffset();
                }

                markDirty();
                return true;
            }
        }
        break;
    }

    case WidgetEventType::DragMove: {
        if (scrollbarDragging_ && hasVerticalScrollbar) {
            float thumbRatio = bounds_.height / contentSize_.y;
            float thumbHeight = std::max(20.0f, scrollbarHeight * thumbRatio);
            float maxScroll = contentSize_.y - bounds_.height;

            // Convert mouse delta to scroll delta
            float scrollableTrack = scrollbarHeight - thumbHeight;
            if (scrollableTrack > 0) {
                float scrollDelta = (event.mouseDelta.y / scrollableTrack) * maxScroll;
                scrollOffset_.y += scrollDelta;
                clampScrollOffset();
            }

            markDirty();
            return true;
        }
        break;
    }

    case WidgetEventType::Release:
    case WidgetEventType::DragEnd:
        if (scrollbarDragging_) {
            scrollbarDragging_ = false;
            removeFlag(RetainedWidgetFlags::Active);
            markDirty();
            return true;
        }
        break;

    case WidgetEventType::Scroll:
        // Handle mouse wheel scrolling
        if (hasVerticalScrollbar) {
            scrollOffset_.y -= event.scrollDelta.y * 30.0f; // 30 pixels per scroll tick
            clampScrollOffset();
            markDirty();
            return true;
        }
        break;

    default:
        break;
    }

    return Widget::handleInput(event);
}

void ScrollView::build(Context& ctx) {
    if (!visible_)
        return;

    DrawList& drawList = ctx.getDrawList();

    drawBackground(drawList);

    // Set clip rect for content
    Rect contentRect(bounds_.x + padding_.left, bounds_.y + padding_.top, bounds_.width - padding_.left - padding_.right - (verticalScroll_ ? scrollbarWidth_ : 0), bounds_.height - padding_.top - padding_.bottom);

    drawList.pushClipRect(contentRect);

    // Offset children by scroll amount
    for (auto& child : children_) {
        Rect childBounds = child->getBounds();
        childBounds.x += bounds_.x;
        childBounds.y += bounds_.y + scrollOffset_.y;
        // Only draw if visible in viewport
        if (childBounds.y + childBounds.height >= bounds_.y && childBounds.y <= bounds_.y + bounds_.height) {
            child->build(ctx);
        }
    }

    drawList.popClipRect();

    drawScrollbars(drawList);
    clearDirty();
}

void ScrollView::updateContentSize() {
    contentSize_ = Vec2(0, 0);
    for (auto& child : children_) {
        Vec2 childSize = child->measureContent();
        contentSize_.x = std::max(contentSize_.x, childSize.x);
        contentSize_.y += childSize.y + 4.0f;
    }
}

void ScrollView::clampScrollOffset() {
    float maxX = std::max(0.0f, contentSize_.x - bounds_.width);
    float maxY = std::max(0.0f, contentSize_.y - bounds_.height);

    scrollOffset_.x = std::clamp(scrollOffset_.x, 0.0f, maxX);
    scrollOffset_.y = std::clamp(scrollOffset_.y, 0.0f, maxY);
}

void ScrollView::drawScrollbars(DrawList& drawList) {
    if (verticalScroll_ && contentSize_.y > bounds_.height) {
        float scrollbarX = bounds_.x + bounds_.width - scrollbarWidth_ - 2;
        float scrollbarY = bounds_.y + 2;
        float scrollbarHeight = bounds_.height - 4;

        // Track
        Rect trackRect(scrollbarX, scrollbarY, scrollbarWidth_, scrollbarHeight);
        drawList.drawRectFilledRounded(trackRect, Color(38, 38, 38, 255), scrollbarWidth_ * 0.5f);

        // Thumb
        float thumbRatio = bounds_.height / contentSize_.y;
        float thumbHeight = std::max(20.0f, scrollbarHeight * thumbRatio);
        float thumbY = scrollbarY + (scrollbarHeight - thumbHeight) * (scrollOffset_.y / (contentSize_.y - bounds_.height));

        Rect thumbRect(scrollbarX, thumbY, scrollbarWidth_, thumbHeight);
        Color thumbColor = scrollbarDragging_ ? Color(153, 153, 153, 255) : Color(102, 102, 102, 255);
        drawList.drawRectFilledRounded(thumbRect, thumbColor, scrollbarWidth_ * 0.5f);
    }
}

// ============================================================================
// TreeNode Widget
// ============================================================================

TreeNode::TreeNode() = default;

TreeNode::TreeNode(const std::string& label) : label_(label) {}

void TreeNode::setExpanded(bool expanded) {
    if (expanded) {
        addFlag(RetainedWidgetFlags::Expanded);
    } else {
        removeFlag(RetainedWidgetFlags::Expanded);
    }
}

Vec2 TreeNode::measureContent() {
    float textWidth = static_cast<float>(label_.length()) * 8.0f;
    float width = indentWidth_ + textWidth + padding_.left + padding_.right;
    float height = rowHeight_;

    if (isExpanded()) {
        for (auto& child : children_) {
            Vec2 childSize = child->measureContent();
            width = std::max(width, indentWidth_ + childSize.x);
            height += childSize.y;
        }
    }

    return Vec2(width, height);
}

void TreeNode::layout(const Rect& available) {
    bounds_ = available;

    if (isExpanded()) {
        float y = available.y + rowHeight_;
        float childX = available.x + indentWidth_;
        float childWidth = available.width - indentWidth_;

        for (auto& child : children_) {
            if (!child->isVisible())
                continue;

            Vec2 childSize = child->measureContent();
            Rect childBounds(childX, y, childWidth, childSize.y);
            child->layout(childBounds);
            y += childSize.y;
        }
    }

    clearNeedsLayout();
}

bool TreeNode::handleInput(const WidgetEvent& event) {
    if (isDisabled())
        return false;

    switch (event.type) {
    case WidgetEventType::Hover:
        addFlag(RetainedWidgetFlags::Hovered);
        return true;

    case WidgetEventType::HoverEnd:
        removeFlag(RetainedWidgetFlags::Hovered);
        return true;

    case WidgetEventType::Click:
        if (!leaf_) {
            setExpanded(!isExpanded());
        }
        addFlag(RetainedWidgetFlags::Selected);
        fireEvent(WidgetEventType::Click, event);
        return true;

    default:
        return false;
    }
}

void TreeNode::build(Context& ctx) {
    if (!visible_)
        return;

    DrawList& drawList = ctx.getDrawList();

    drawContent(drawList);

    if (isExpanded()) {
        drawChildren(ctx);
    }

    clearDirty();
}

void TreeNode::drawContent(DrawList& drawList) {
    // Background for row
    Rect rowRect(bounds_.x, bounds_.y, bounds_.width, rowHeight_);
    if (hasFlag(RetainedWidgetFlags::Selected)) {
        drawList.drawRectFilled(rowRect, Color(77, 128, 204, 77));
    } else if (isHovered()) {
        drawList.drawRectFilled(rowRect, Color(77, 77, 77, 77));
    }

    float x = bounds_.x + padding_.left;
    float y = bounds_.y + (rowHeight_ - 16.0f) * 0.5f;

    // Draw expand arrow if not a leaf
    if (!leaf_ && !children_.empty()) {
        float arrowX = x;
        float arrowY = y + 8.0f;
        float arrowSize = 6.0f;

        if (isExpanded()) {
            // Down arrow
            drawList.drawTriangleFilled(Vec2(arrowX, arrowY - arrowSize * 0.5f), Vec2(arrowX + arrowSize, arrowY - arrowSize * 0.5f), Vec2(arrowX + arrowSize * 0.5f, arrowY + arrowSize * 0.5f), Color(179, 179, 179, 255));
        } else {
            // Right arrow
            drawList.drawTriangleFilled(Vec2(arrowX, arrowY - arrowSize * 0.5f), Vec2(arrowX, arrowY + arrowSize * 0.5f), Vec2(arrowX + arrowSize, arrowY), Color(179, 179, 179, 255));
        }
    }

    // Draw label
    float textX = x + indentWidth_;
    drawList.drawText(Vec2(textX, y), label_.c_str(), Color(255, 255, 255, 255));
}

// ============================================================================
// TabBar Widget
// ============================================================================

TabBar::TabBar() { setMinSize(Vec2(100, tabHeight_)); }

void TabBar::addTab(const std::string& label, const std::string& id, bool closeable) {
    Tab tab;
    tab.label = label;
    tab.id = id.empty() ? label : id;
    tab.closeable = closeable;
    tabs_.push_back(tab);
    markDirty();
}

void TabBar::removeTab(size_t index) {
    if (index < tabs_.size()) {
        tabs_.erase(tabs_.begin() + static_cast<ptrdiff_t>(index));
        if (selectedIndex_ >= tabs_.size() && !tabs_.empty()) {
            selectedIndex_ = tabs_.size() - 1;
        }
        markDirty();
    }
}

void TabBar::removeTab(const std::string& id) {
    for (size_t i = 0; i < tabs_.size(); ++i) {
        if (tabs_[i].id == id) {
            removeTab(i);
            return;
        }
    }
}

void TabBar::setSelectedIndex(size_t index) {
    if (index < tabs_.size() && selectedIndex_ != index) {
        selectedIndex_ = index;
        if (onTabSelected_) {
            onTabSelected_(index);
        }
        markDirty();
    }
}

const std::string& TabBar::getSelectedId() const {
    static std::string empty;
    return tabs_.empty() ? empty : tabs_[selectedIndex_].id;
}

Vec2 TabBar::measureContent() {
    float width = 0;
    for (const auto& tab : tabs_) {
        width += static_cast<float>(tab.label.length()) * 8.0f + tabPadding_ * 2;
        if (tab.closeable) {
            width += 20.0f; // Close button
        }
    }
    return Vec2(width + padding_.left + padding_.right, tabHeight_ + padding_.top + padding_.bottom);
}

bool TabBar::handleInput(const WidgetEvent& event) {
    if (event.type != WidgetEventType::Click)
        return false;

    float x = bounds_.x + padding_.left;
    for (size_t i = 0; i < tabs_.size(); ++i) {
        float tabWidth = static_cast<float>(tabs_[i].label.length()) * 8.0f + tabPadding_ * 2;
        if (tabs_[i].closeable) {
            tabWidth += 20.0f;
        }

        if (event.mousePos.x >= x && event.mousePos.x < x + tabWidth) {
            // Check if close button clicked
            if (tabs_[i].closeable) {
                float closeX = x + tabWidth - 20.0f;
                if (event.mousePos.x >= closeX) {
                    if (onTabClosed_) {
                        onTabClosed_(i);
                    }
                    return true;
                }
            }

            setSelectedIndex(i);
            return true;
        }

        x += tabWidth;
    }

    return false;
}

void TabBar::drawContent(DrawList& drawList) {
    float x = bounds_.x + padding_.left;
    float y = bounds_.y + padding_.top;

    for (size_t i = 0; i < tabs_.size(); ++i) {
        const Tab& tab = tabs_[i];
        float tabWidth = static_cast<float>(tab.label.length()) * 8.0f + tabPadding_ * 2;
        if (tab.closeable) {
            tabWidth += 20.0f;
        }

        Rect tabRect(x, y, tabWidth, tabHeight_);

        // Background
        Color bgColor = (i == selectedIndex_) ? Color(64, 64, 64, 255) : Color(38, 38, 38, 255);
        drawList.drawRectFilledRounded(tabRect, bgColor, BorderRadius(4, 4, 0, 0));

        // Label
        float textX = x + tabPadding_;
        float textY = y + (tabHeight_ - 14.0f) * 0.5f;
        Color textColor = (i == selectedIndex_) ? Color(255, 255, 255, 255) : Color(179, 179, 179, 255);
        drawList.drawText(Vec2(textX, textY), tab.label.c_str(), textColor, 14.0f);

        // Close button
        if (tab.closeable) {
            float closeX = x + tabWidth - 18.0f;
            float closeY = y + (tabHeight_ - 12.0f) * 0.5f;
            drawList.drawText(Vec2(closeX, closeY), "x", Color(153, 153, 153, 255), 12.0f);
        }

        x += tabWidth + 2.0f;
    }

    // Bottom line
    drawList.drawLine(Vec2(bounds_.x, bounds_.y + tabHeight_), Vec2(bounds_.x + bounds_.width, bounds_.y + tabHeight_), Color(77, 77, 77, 255), 1.0f);
}

} // namespace dakt::gui

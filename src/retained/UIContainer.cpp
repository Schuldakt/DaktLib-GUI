/**
 * @file UIContainer.cpp
 * @brief Implementation of UIContainer - retained mode widget tree manager
 */

#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"
#include "dakt/gui/retained/Containers/ContainerBase.hpp"
#include "dakt/gui/retained/Containers/Grid.hpp"
#include "dakt/gui/retained/Containers/HBox.hpp"
#include "dakt/gui/retained/Containers/Splitter.hpp"
#include "dakt/gui/retained/Containers/Stack.hpp"
#include "dakt/gui/retained/Containers/VBox.hpp"
#include "dakt/gui/retained/Containers/Wrap.hpp"
#include "dakt/gui/subsystems/input/Input.hpp"

namespace dakt::gui {

// ============================================================================
// Internal Root Widget
// ============================================================================

/**
 * @brief Internal root widget for UIContainer
 */
class RootWidget : public Widget {
  public:
    RootWidget() { setId("__root__"); }

    void layout(const Rect& available) override {
        bounds_ = available;

        // Layout handled by UIContainer
        for (auto& child : children_) {
            if (child->needsLayout()) {
                child->layout(child->getBounds());
            }
        }

        clearNeedsLayout();
    }

    void drawBackground(DrawList& drawList) override {
        // Root has no background
    }
};

// ============================================================================
// UIContainer Implementation
// ============================================================================

UIContainer::UIContainer() : root_(std::make_unique<RootWidget>()) {}

UIContainer::UIContainer(const std::string& id) : id_(id), root_(std::make_unique<RootWidget>()) {}

UIContainer::~UIContainer() = default;

void UIContainer::setBounds(const Rect& bounds) {
    if (bounds_ != bounds) {
        bounds_ = bounds;
        markDirty();
    }
}

Widget& UIContainer::addWidget(std::unique_ptr<Widget> widget) {
    registerWidget(widget.get());
    return root_->addChild(std::move(widget));
}

bool UIContainer::removeWidget(Widget* widget) {
    if (!widget)
        return false;

    unregisterWidget(widget);
    return root_->removeChild(widget);
}

void UIContainer::clear() {
    widgetLookup_.clear();
    root_->clearChildren();
    focusedWidget_ = nullptr;
    hoveredWidget_ = nullptr;
    activeWidget_ = nullptr;
    markDirty();
}

Widget* UIContainer::findWidget(const std::string& id) {
    auto it = widgetLookup_.find(id);
    if (it != widgetLookup_.end()) {
        return it->second;
    }
    return root_->findChild(id);
}

void UIContainer::registerWidget(Widget* widget) {
    if (!widget || widget->getId().empty())
        return;

    widgetLookup_[widget->getId()] = widget;

    // Register children recursively
    for (auto& child : widget->getChildren()) {
        registerWidget(child.get());
    }
}

void UIContainer::unregisterWidget(Widget* widget) {
    if (!widget)
        return;

    if (!widget->getId().empty()) {
        widgetLookup_.erase(widget->getId());
    }

    // Unregister children recursively
    for (auto& child : widget->getChildren()) {
        unregisterWidget(child.get());
    }
}

void UIContainer::layout() {
    if (!dirty_)
        return;

    root_->setBounds(bounds_);

    switch (direction_) {
    case LayoutDirection::Vertical:
        layoutVertical();
        break;
    case LayoutDirection::Horizontal:
        layoutHorizontal();
        break;
    case LayoutDirection::None:
        // Manual positioning - just layout children in place
        for (auto& child : root_->getChildren()) {
            if (child->needsLayout()) {
                child->layout(child->getBounds());
            }
        }
        break;
    }

    clearDirty();
}

void UIContainer::forceLayout() {
    markDirty();
    root_->markNeedsLayout();
    layout();
}

void UIContainer::layoutVertical() {
    float contentX = bounds_.x + padding_.left;
    float contentY = bounds_.y + padding_.top;
    float contentWidth = bounds_.width - padding_.left - padding_.right;
    float contentHeight = bounds_.height - padding_.top - padding_.bottom;

    // Calculate total content height
    float totalHeight = 0;
    int visibleCount = 0;

    for (auto& child : root_->getChildren()) {
        if (!child->isVisible())
            continue;

        Vec2 size = child->measureContent();
        totalHeight += size.y;
        visibleCount++;
    }

    if (visibleCount > 1) {
        totalHeight += spacing_ * (visibleCount - 1);
    }

    // Determine starting Y based on alignment
    float y = contentY;
    float extraSpace = contentHeight - totalHeight;

    switch (mainAxisAlign_) {
    case Alignment::Center:
        y += extraSpace * 0.5f;
        break;
    case Alignment::End:
        y += extraSpace;
        break;
    case Alignment::SpaceBetween:
        if (visibleCount > 1) {
            spacing_ = extraSpace / (visibleCount - 1);
        }
        break;
    case Alignment::SpaceAround:
        if (visibleCount > 0) {
            float gap = extraSpace / visibleCount;
            y += gap * 0.5f;
            spacing_ = gap;
        }
        break;
    case Alignment::SpaceEvenly:
        if (visibleCount > 0) {
            float gap = extraSpace / (visibleCount + 1);
            y += gap;
            spacing_ = gap;
        }
        break;
    default:
        break;
    }

    // Layout children
    for (auto& child : root_->getChildren()) {
        if (!child->isVisible())
            continue;

        Vec2 size = child->measureContent();
        float childWidth = size.x;
        float childX = contentX;

        // Cross-axis alignment
        switch (crossAxisAlign_) {
        case Alignment::Center:
            childX = contentX + (contentWidth - childWidth) * 0.5f;
            break;
        case Alignment::End:
            childX = contentX + contentWidth - childWidth;
            break;
        case Alignment::Stretch:
            childWidth = contentWidth;
            break;
        default:
            break;
        }

        Rect childBounds(childX, y, childWidth, size.y);
        child->layout(childBounds);

        y += size.y + spacing_;
    }
}

void UIContainer::layoutHorizontal() {
    float contentX = bounds_.x + padding_.left;
    float contentY = bounds_.y + padding_.top;
    float contentWidth = bounds_.width - padding_.left - padding_.right;
    float contentHeight = bounds_.height - padding_.top - padding_.bottom;

    // Calculate total content width
    float totalWidth = 0;
    int visibleCount = 0;

    for (auto& child : root_->getChildren()) {
        if (!child->isVisible())
            continue;

        Vec2 size = child->measureContent();
        totalWidth += size.x;
        visibleCount++;
    }

    if (visibleCount > 1) {
        totalWidth += spacing_ * (visibleCount - 1);
    }

    // Determine starting X based on alignment
    float x = contentX;
    float extraSpace = contentWidth - totalWidth;

    switch (mainAxisAlign_) {
    case Alignment::Center:
        x += extraSpace * 0.5f;
        break;
    case Alignment::End:
        x += extraSpace;
        break;
    case Alignment::SpaceBetween:
        if (visibleCount > 1) {
            spacing_ = extraSpace / (visibleCount - 1);
        }
        break;
    case Alignment::SpaceAround:
        if (visibleCount > 0) {
            float gap = extraSpace / visibleCount;
            x += gap * 0.5f;
            spacing_ = gap;
        }
        break;
    case Alignment::SpaceEvenly:
        if (visibleCount > 0) {
            float gap = extraSpace / (visibleCount + 1);
            x += gap;
            spacing_ = gap;
        }
        break;
    default:
        break;
    }

    // Layout children
    for (auto& child : root_->getChildren()) {
        if (!child->isVisible())
            continue;

        Vec2 size = child->measureContent();
        float childHeight = size.y;
        float childY = contentY;

        // Cross-axis alignment
        switch (crossAxisAlign_) {
        case Alignment::Center:
            childY = contentY + (contentHeight - childHeight) * 0.5f;
            break;
        case Alignment::End:
            childY = contentY + contentHeight - childHeight;
            break;
        case Alignment::Stretch:
            childHeight = contentHeight;
            break;
        default:
            break;
        }

        Rect childBounds(x, childY, size.x, childHeight);
        child->layout(childBounds);

        x += size.x + spacing_;
    }
}

void UIContainer::layoutChildren(Widget* widget, const Rect& available) { widget->layout(available); }

void UIContainer::processInput(InputSystem& input) {
    Vec2 mousePos = input.getMousePosition();
    bool leftDown = input.isMouseButtonDown(MouseButton::Left);
    bool leftPressed = input.isMouseButtonPressed(MouseButton::Left);
    bool leftReleased = input.isMouseButtonReleased(MouseButton::Left);
    float mouseWheel = input.getMouseWheel();

    // Hit test for hover
    Widget* newHovered = hitTest(mousePos);

    // Handle hover state changes
    if (newHovered != hoveredWidget_) {
        if (hoveredWidget_) {
            WidgetEvent event;
            event.type = WidgetEventType::HoverEnd;
            event.mousePos = mousePos;
            hoveredWidget_->handleInput(event);
        }

        hoveredWidget_ = newHovered;

        if (hoveredWidget_) {
            WidgetEvent event;
            event.type = WidgetEventType::Hover;
            event.mousePos = mousePos;
            hoveredWidget_->handleInput(event);
        }
    }

    // Handle scroll wheel
    if (mouseWheel != 0.0f && hoveredWidget_) {
        WidgetEvent event;
        event.type = WidgetEventType::Scroll;
        event.mousePos = mousePos;
        event.scrollDelta = Vec2(0, mouseWheel);
        hoveredWidget_->handleInput(event);
    }

    // Handle mouse press
    if (leftPressed && hoveredWidget_) {
        activeWidget_ = hoveredWidget_;

        WidgetEvent event;
        event.type = WidgetEventType::Press;
        event.mousePos = mousePos;
        event.button = MouseButton::Left;
        activeWidget_->handleInput(event);

        // Set focus on click
        setFocus(activeWidget_);
    }

    // Handle mouse drag
    if (leftDown && activeWidget_) {
        Vec2 delta = mousePos - lastMousePos_;
        if (delta.x != 0 || delta.y != 0) {
            WidgetEvent event;
            event.type = WidgetEventType::DragMove;
            event.mousePos = mousePos;
            event.mouseDelta = delta;
            event.button = MouseButton::Left;
            activeWidget_->handleInput(event);
        }
    }

    // Handle mouse release
    if (leftReleased) {
        if (activeWidget_) {
            WidgetEvent event;
            event.type = WidgetEventType::Release;
            event.mousePos = mousePos;
            event.button = MouseButton::Left;
            activeWidget_->handleInput(event);

            // Fire click if still over same widget
            if (activeWidget_ == hoveredWidget_) {
                event.type = WidgetEventType::Click;
                activeWidget_->handleInput(event);
            }

            activeWidget_ = nullptr;
        }
    }

    lastMousePos_ = mousePos;
}

void UIContainer::setFocus(Widget* widget) {
    if (focusedWidget_ == widget)
        return;

    Widget* oldFocus = focusedWidget_;

    // Blur old widget
    if (oldFocus) {
        WidgetEvent event;
        event.type = WidgetEventType::Blur;
        oldFocus->handleInput(event);
        oldFocus->removeFlag(RetainedWidgetFlags::Focused);
    }

    focusedWidget_ = widget;

    // Focus new widget
    if (focusedWidget_) {
        WidgetEvent event;
        event.type = WidgetEventType::Focus;
        focusedWidget_->handleInput(event);
        focusedWidget_->addFlag(RetainedWidgetFlags::Focused);
    }

    // Fire callback
    if (onFocusChanged_) {
        onFocusChanged_(oldFocus, focusedWidget_);
    }
}

void UIContainer::clearFocus() { setFocus(nullptr); }

void UIContainer::focusNext() {
    buildFocusList();

    if (focusableWidgets_.empty())
        return;

    if (!focusedWidget_) {
        setFocus(focusableWidgets_.front());
        return;
    }

    auto it = std::find(focusableWidgets_.begin(), focusableWidgets_.end(), focusedWidget_);
    if (it != focusableWidgets_.end()) {
        ++it;
        if (it == focusableWidgets_.end()) {
            it = focusableWidgets_.begin();
        }
        setFocus(*it);
    }
}

void UIContainer::focusPrevious() {
    buildFocusList();

    if (focusableWidgets_.empty())
        return;

    if (!focusedWidget_) {
        setFocus(focusableWidgets_.back());
        return;
    }

    auto it = std::find(focusableWidgets_.begin(), focusableWidgets_.end(), focusedWidget_);
    if (it != focusableWidgets_.end()) {
        if (it == focusableWidgets_.begin()) {
            it = focusableWidgets_.end();
        }
        --it;
        setFocus(*it);
    }
}

void UIContainer::render(Context& ctx) {
    // Ensure layout is up to date
    layout();

    // Render all widgets
    root_->build(ctx);
}

void UIContainer::update(float deltaTime) {
    totalTime_ += deltaTime;

    // Update animations, cursor blink, etc.
    // This would be expanded with animation system integration
}

Widget* UIContainer::hitTest(const Vec2& point) { return hitTestRecursive(root_.get(), point); }

Widget* UIContainer::hitTestRecursive(Widget* widget, const Vec2& point) {
    if (!widget || !widget->isVisible())
        return nullptr;

    // Check children first (front to back in reverse order for correct z-order)
    const auto& children = widget->getChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        if (Widget* hit = hitTestRecursive(it->get(), point)) {
            return hit;
        }
    }

    // Check this widget
    const Rect& bounds = widget->getBounds();
    if (point.x >= bounds.x && point.x < bounds.x + bounds.width && point.y >= bounds.y && point.y < bounds.y + bounds.height) {

        // Don't return root widget
        if (widget != root_.get() && widget->isEnabled()) {
            return widget;
        }
    }

    return nullptr;
}

void UIContainer::buildFocusList() {
    focusableWidgets_.clear();
    buildFocusListRecursive(root_.get(), focusableWidgets_);
}

void UIContainer::buildFocusListRecursive(Widget* widget, std::vector<Widget*>& list) {
    if (!widget || !widget->isVisible() || widget->isDisabled())
        return;

    // Add widget if it's focusable (buttons, inputs, etc.)
    // For now, add all non-root widgets
    if (widget != root_.get()) {
        list.push_back(widget);
    }

    // Add children
    for (auto& child : widget->getChildren()) {
        buildFocusListRecursive(child.get(), list);
    }
}

} // namespace dakt::gui

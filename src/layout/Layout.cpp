#include "dakt/gui/layout/Layout.hpp"
#include <algorithm>
#include <cmath>

namespace dakt::gui {

LayoutNode::LayoutNode() = default;
LayoutNode::~LayoutNode() = default;

void LayoutNode::setSize(float width, float height) {
    if (rect_.width != width || rect_.height != height) {
        rect_.width = width;
        rect_.height = height;
        markDirty();
    }
}

void LayoutNode::setPosition(float x, float y) {
    if (rect_.x != x || rect_.y != y) {
        rect_.x = x;
        rect_.y = y;
    }
}

LayoutNode* LayoutNode::addChild() {
    children_.push_back(std::make_unique<LayoutNode>());
    children_.back()->parent_ = this;
    markDirty();
    return children_.back().get();
}

void LayoutNode::removeChild(LayoutNode* child) {
    auto it = std::find_if(children_.begin(), children_.end(), [child](const auto& ptr) { return ptr.get() == child; });
    if (it != children_.end()) {
        children_.erase(it);
        markDirty();
    }
}

void LayoutNode::markDirty() {
    if (!dirty_) {
        dirty_ = true;
        if (parent_) {
            parent_->markDirty();
        }
    }
}

void LayoutNode::computeLayout(float parentWidth, float parentHeight) {
    if (!dirty_)
        return;

    // Clamp to constraints
    float availWidth = std::clamp(parentWidth, constraints_.minWidth, constraints_.maxWidth);
    float availHeight = std::clamp(parentHeight, constraints_.minHeight, constraints_.maxHeight);

    // Account for padding
    float contentWidth = availWidth - flexProps_.padding.left - flexProps_.padding.right;
    float contentHeight = availHeight - flexProps_.padding.top - flexProps_.padding.bottom;

    contentWidth = std::max(0.0f, contentWidth);
    contentHeight = std::max(0.0f, contentHeight);

    // Measure and layout children based on flex direction
    if (flexProps_.direction == FlexDirection::Row) {
        // Row: horizontal main axis
        layoutChildren();
    } else {
        // Column: vertical main axis
        layoutChildren();
    }

    markClean();
}

void LayoutNode::layoutChildren() {
    // Simple layout: stack children vertically/horizontally
    const auto& padding = flexProps_.padding;
    float x = rect_.x + padding.left;
    float y = rect_.y + padding.top;

    float availWidth = rect_.width - padding.left - padding.right;
    float availHeight = rect_.height - padding.top - padding.bottom;

    for (auto& child : children_) {
        child->setPosition(x, y);

        if (flexProps_.direction == FlexDirection::Row) {
            // Row layout
            float childWidth = child->flexProps_.basis;
            if (std::isnan(childWidth)) {
                childWidth = 100.0f; // Default child width
            }
            child->setSize(childWidth, availHeight);
            x += childWidth + child->flexProps_.margin.right;
        } else {
            // Column layout
            float childHeight = child->flexProps_.basis;
            if (std::isnan(childHeight)) {
                childHeight = 30.0f; // Default child height
            }
            child->setSize(availWidth, childHeight);
            y += childHeight + child->flexProps_.margin.bottom;
        }

        child->computeLayout(child->rect_.width, child->rect_.height);
    }
}

} // namespace dakt::gui

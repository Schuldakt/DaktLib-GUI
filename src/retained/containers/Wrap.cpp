#include "dakt/gui/retained/containers/Wrap.hpp"

namespace dakt::gui {

Wrap::Wrap() : UIContainer() { setLayoutDirection(LayoutDirection::None); }

Wrap::Wrap(const std::string& id) : UIContainer(id) { setLayoutDirection(LayoutDirection::None); }

void Wrap::layout() { layoutWrap(); }

void Wrap::layoutWrap() {
    auto* root = getRoot();
    if (!root)
        return;

    const auto& children = root->getChildren();
    if (children.empty())
        return;

    const auto& padding = getPadding();
    float availableWidth = bounds_.width - padding.left - padding.right;
    float availableHeight = bounds_.height - padding.top - padding.bottom;
    float startX = bounds_.x + padding.left;
    float startY = bounds_.y + padding.top;

    if (flowDirection_ == LayoutDirection::Horizontal) {
        // Flow left-to-right, wrap to next row
        float x = startX;
        float y = startY;
        float lineHeight = 0;

        for (const auto& child : children) {
            if (!child->isVisible())
                continue;

            Vec2 childSize = child->measureContent();

            // Check if we need to wrap
            if (x + childSize.x > startX + availableWidth && x > startX) {
                x = startX;
                y += lineHeight + vSpacing_;
                lineHeight = 0;
            }

            child->setBounds(Rect(x, y, childSize.x, childSize.y));
            child->layout(Rect(x, y, childSize.x, childSize.y));

            x += childSize.x + hSpacing_;
            lineHeight = std::max(lineHeight, childSize.y);
        }
    } else {
        // Flow top-to-bottom, wrap to next column
        float x = startX;
        float y = startY;
        float columnWidth = 0;

        for (const auto& child : children) {
            if (!child->isVisible())
                continue;

            Vec2 childSize = child->measureContent();

            // Check if we need to wrap
            if (y + childSize.y > startY + availableHeight && y > startY) {
                y = startY;
                x += columnWidth + hSpacing_;
                columnWidth = 0;
            }

            child->setBounds(Rect(x, y, childSize.x, childSize.y));
            child->layout(Rect(x, y, childSize.x, childSize.y));

            y += childSize.y + vSpacing_;
            columnWidth = std::max(columnWidth, childSize.x);
        }
    }

    clearDirty();
}

} // namespace dakt::gui

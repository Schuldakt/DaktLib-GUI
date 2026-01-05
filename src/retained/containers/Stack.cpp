#include "dakt/gui/retained/containers/Stack.hpp"

namespace dakt::gui {

Stack::Stack() : UIContainer() { setLayoutDirection(LayoutDirection::None); }

Stack::Stack(const std::string& id) : UIContainer(id) { setLayoutDirection(LayoutDirection::None); }

void Stack::setWidgetAlignment(Widget* widget, Alignment hAlign, Alignment vAlign) {
    widgetAlignments_[widget] = {hAlign, vAlign};
    markDirty();
}

void Stack::layout() { layoutStack(); }

void Stack::layoutStack() {
    auto* root = getRoot();
    if (!root)
        return;

    const auto& children = root->getChildren();
    const auto& padding = getPadding();

    float availableWidth = bounds_.width - padding.left - padding.right;
    float availableHeight = bounds_.height - padding.top - padding.bottom;
    float startX = bounds_.x + padding.left;
    float startY = bounds_.y + padding.top;

    for (const auto& child : children) {
        if (!child->isVisible())
            continue;

        Vec2 childSize = child->measureContent();

        // Clamp to available space
        childSize.x = std::min(childSize.x, availableWidth);
        childSize.y = std::min(childSize.y, availableHeight);

        // Get alignment for this widget
        Alignment hAlign = hAlign_;
        Alignment vAlign = vAlign_;

        auto it = widgetAlignments_.find(child.get());
        if (it != widgetAlignments_.end()) {
            hAlign = it->second.horizontal;
            vAlign = it->second.vertical;
        }

        // Calculate position based on alignment
        float x = startX;
        float y = startY;
        float w = childSize.x;
        float h = childSize.y;

        switch (hAlign) {
        case Alignment::Start:
            x = startX;
            break;
        case Alignment::Center:
            x = startX + (availableWidth - w) / 2;
            break;
        case Alignment::End:
            x = startX + availableWidth - w;
            break;
        case Alignment::Stretch:
            w = availableWidth;
            break;
        default:
            break;
        }

        switch (vAlign) {
        case Alignment::Start:
            y = startY;
            break;
        case Alignment::Center:
            y = startY + (availableHeight - h) / 2;
            break;
        case Alignment::End:
            y = startY + availableHeight - h;
            break;
        case Alignment::Stretch:
            h = availableHeight;
            break;
        default:
            break;
        }

        child->setBounds(Rect(x, y, w, h));
        child->layout(Rect(x, y, w, h));
    }

    clearDirty();
}

} // namespace dakt::gui

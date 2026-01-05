#include "dakt/gui/retained/containers/Splitter.hpp"
#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/draw/DrawList.hpp"
#include <algorithm>

namespace dakt::gui {

Splitter::Splitter() : UIContainer() { setLayoutDirection(LayoutDirection::None); }

Splitter::Splitter(Orientation orientation) : UIContainer(), orientation_(orientation) { setLayoutDirection(LayoutDirection::None); }

Splitter::Splitter(const std::string& id, Orientation orientation) : UIContainer(id), orientation_(orientation) { setLayoutDirection(LayoutDirection::None); }

void Splitter::setSplitRatio(float ratio) {
    splitRatio_ = std::max(0.0f, std::min(1.0f, ratio));
    markDirty();
}

Widget* Splitter::getFirstPanel() const {
    auto* root = getRoot();
    if (root && !root->getChildren().empty()) {
        return root->getChildren()[0].get();
    }
    return nullptr;
}

Widget* Splitter::getSecondPanel() const {
    auto* root = getRoot();
    if (root && root->getChildren().size() >= 2) {
        return root->getChildren()[1].get();
    }
    return nullptr;
}

void Splitter::layout() { layoutSplitter(); }

void Splitter::layoutSplitter() {
    auto* root = getRoot();
    if (!root)
        return;

    const auto& children = root->getChildren();
    if (children.size() < 2)
        return;

    const auto& padding = getPadding();
    float availableWidth = bounds_.width - padding.left - padding.right;
    float availableHeight = bounds_.height - padding.top - padding.bottom;
    float startX = bounds_.x + padding.left;
    float startY = bounds_.y + padding.top;

    if (orientation_ == Orientation::Horizontal) {
        // Left/Right split
        float dividerX = startX + (availableWidth - dividerSize_) * splitRatio_;

        // Clamp to minimum sizes
        float maxFirstWidth = availableWidth - dividerSize_ - minSecondSize_;
        float minFirstWidth = minFirstSize_;
        float firstWidth = (availableWidth - dividerSize_) * splitRatio_;
        firstWidth = std::max(minFirstWidth, std::min(maxFirstWidth, firstWidth));

        float secondWidth = availableWidth - firstWidth - dividerSize_;

        // First panel
        children[0]->setBounds(Rect(startX, startY, firstWidth, availableHeight));
        children[0]->layout(Rect(startX, startY, firstWidth, availableHeight));

        // Divider rect
        dividerRect_ = Rect(startX + firstWidth, startY, dividerSize_, availableHeight);

        // Second panel
        float secondX = startX + firstWidth + dividerSize_;
        children[1]->setBounds(Rect(secondX, startY, secondWidth, availableHeight));
        children[1]->layout(Rect(secondX, startY, secondWidth, availableHeight));
    } else {
        // Top/Bottom split
        float dividerY = startY + (availableHeight - dividerSize_) * splitRatio_;

        // Clamp to minimum sizes
        float maxFirstHeight = availableHeight - dividerSize_ - minSecondSize_;
        float minFirstHeight = minFirstSize_;
        float firstHeight = (availableHeight - dividerSize_) * splitRatio_;
        firstHeight = std::max(minFirstHeight, std::min(maxFirstHeight, firstHeight));

        float secondHeight = availableHeight - firstHeight - dividerSize_;

        // First panel
        children[0]->setBounds(Rect(startX, startY, availableWidth, firstHeight));
        children[0]->layout(Rect(startX, startY, availableWidth, firstHeight));

        // Divider rect
        dividerRect_ = Rect(startX, startY + firstHeight, availableWidth, dividerSize_);

        // Second panel
        float secondY = startY + firstHeight + dividerSize_;
        children[1]->setBounds(Rect(startX, secondY, availableWidth, secondHeight));
        children[1]->layout(Rect(startX, secondY, availableWidth, secondHeight));
    }

    clearDirty();
}

bool Splitter::isDividerHovered(const Vec2& mousePos) const { return dividerRect_.contains(mousePos); }

void Splitter::render(Context& ctx) {
    UIContainer::render(ctx);

    // Draw divider
    auto& drawList = ctx.getDrawList();
    Color color = draggingDivider_ ? dividerHoverColor_ : dividerColor_;
    drawList.drawRectFilled(dividerRect_, color);

    // Draw grip indicator
    float cx = dividerRect_.x + dividerRect_.width / 2;
    float cy = dividerRect_.y + dividerRect_.height / 2;
    Color gripColor = Color{100, 100, 110, 255};

    if (orientation_ == Orientation::Horizontal) {
        for (int i = -2; i <= 2; ++i) {
            drawList.drawCircleFilled(Vec2(cx, cy + i * 8), 2, gripColor, 8);
        }
    } else {
        for (int i = -2; i <= 2; ++i) {
            drawList.drawCircleFilled(Vec2(cx + i * 8, cy), 2, gripColor, 8);
        }
    }
}

} // namespace dakt::gui

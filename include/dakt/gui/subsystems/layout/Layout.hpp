#ifndef DAKTLIB_GUI_LAYOUT_HPP
#define DAKTLIB_GUI_LAYOUT_HPP

#include "../../core/Types.hpp"
#include <limits>
#include <memory>
#include <vector>

namespace dakt::gui {

constexpr float AUTO_SIZE = std::numeric_limits<float>::quiet_NaN();

struct FlexProperties {
    FlexDirection direction = FlexDirection::Row;
    JustifyContent justifyContent = JustifyContent::FlexStart;
    AlignItems alignItems = AlignItems::Stretch;
    FlexWrap wrap = FlexWrap::NoWrap;

    float grow = 0.0f;       // flex-grow
    float shrink = 1.0f;     // flex-shrink
    float basis = AUTO_SIZE; // flex-basis (width/height before flex)

    EdgeInsets padding;
    EdgeInsets margin;
    BorderRadius borderRadius;
};

struct LayoutConstraints {
    float minWidth = 0.0f;
    float maxWidth = std::numeric_limits<float>::max();
    float minHeight = 0.0f;
    float maxHeight = std::numeric_limits<float>::max();
};

class LayoutNode {
  public:
    LayoutNode();
    ~LayoutNode();

    // Size/position
    void setSize(float width, float height);
    void setPosition(float x, float y);
    Rect getRect() const { return rect_; }
    Vec2 getSize() const { return Vec2(rect_.width, rect_.height); }
    Vec2 getPosition() const { return Vec2(rect_.x, rect_.y); }

    // Flex layout properties
    void setFlexProperties(const FlexProperties& props) {
        flexProps_ = props;
        markDirty();
    }

    void setFlexDirection(FlexDirection dir) {
        flexProps_.direction = dir;
        markDirty();
    }

    void setFlexGrow(float grow) {
        flexProps_.grow = grow;
        markDirty();
    }

    void setFlexShrink(float shrink) {
        flexProps_.shrink = shrink;
        markDirty();
    }

    void setFlexBasis(float basis) {
        flexProps_.basis = basis;
        markDirty();
    }

    const FlexProperties& getFlexProperties() const { return flexProps_; }

    // Constraints
    void setConstraints(const LayoutConstraints& constraints) {
        constraints_ = constraints;
        markDirty();
    }

    const LayoutConstraints& getConstraints() const { return constraints_; }

    // Hierarchy
    LayoutNode* addChild();
    void removeChild(LayoutNode* child);
    const std::vector<std::unique_ptr<LayoutNode>>& getChildren() const { return children_; }

    LayoutNode* getParent() { return parent_; }
    const LayoutNode* getParent() const { return parent_; }

    // Dirty tracking
    bool isDirty() const { return dirty_; }
    void markDirty();
    void markClean() { dirty_ = false; }

    // Layout computation
    void computeLayout(float parentWidth, float parentHeight);
    void layoutChildren();

  private:
    Rect rect_;
    FlexProperties flexProps_;
    LayoutConstraints constraints_;
    std::vector<std::unique_ptr<LayoutNode>> children_;
    LayoutNode* parent_ = nullptr;
    bool dirty_ = true;

    // Layout cache
    float desiredWidth_ = 0.0f;
    float desiredHeight_ = 0.0f;
};

} // namespace dakt::gui

#endif

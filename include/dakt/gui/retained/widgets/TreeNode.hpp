#ifndef DAKT_GUI_TREE_NODE_HPP
#define DAKT_GUI_TREE_NODE_HPP

#include "WidgetBase.hpp"

namespace dakt::gui {

/**
 * @brief Expandable tree node widget
 */
class DAKT_GUI_API TreeNode : public Widget {
  public:
    TreeNode();
    explicit TreeNode(const std::string& label);

    const std::string& getLabel() const { return label_; }
    void setLabel(const std::string& label) {
        label_ = label;
        markDirty();
    }

    bool isExpanded() const { return hasFlag(RetainedWidgetFlags::Expanded); }
    void setExpanded(bool expanded);
    void toggle() { setExpanded(!isExpanded()); }

    bool isLeaf() const { return leaf_; }
    void setLeaf(bool leaf) {
        leaf_ = leaf;
        markDirty();
    }

    float getIndentWidth() const { return indentWidth_; }
    void setIndentWidth(float width) {
        indentWidth_ = width;
        markDirty();
    }

    float getRowHeight() const { return rowHeight_; }
    void setRowHeight(float height) {
        rowHeight_ = height;
        markDirty();
    }

    Color getExpandIconColor() const { return expandIconColor_; }
    void setExpandIconColor(const Color& color) {
        expandIconColor_ = color;
        markDirty();
    }

    Vec2 measureContent() override;
    void layout(const Rect& available) override;
    bool handleInput(const WidgetEvent& event) override;
    void build(Context& ctx) override;
    void drawContent(DrawList& drawList) override;

  private:
    std::string label_;
    bool leaf_ = false;
    float indentWidth_ = 20.0f;
    float rowHeight_ = 24.0f;
    Color expandIconColor_{180, 180, 180, 255};
};

} // namespace dakt::gui

#endif // DAKT_GUI_TREE_NODE_HPP

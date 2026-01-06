#ifndef DAKTLIB_GUI_PANEL_HPP
#define DAKTLIB_GUI_PANEL_HPP

#include "WidgetBase.hpp"

namespace dakt::gui {

/**
 * @brief Container panel with optional title/border
 */
class DAKTLIB_GUI_API Panel : public Widget {
  public:
    Panel();
    explicit Panel(const std::string& title);

    const std::string& getTitle() const { return title_; }
    void setTitle(const std::string& title) {
        title_ = title;
        markDirty();
    }

    Color getBackgroundColor() const { return backgroundColor_; }
    void setBackgroundColor(const Color& color) {
        backgroundColor_ = color;
        markDirty();
    }

    Color getHeaderColor() const { return headerColor_; }
    void setHeaderColor(const Color& color) {
        headerColor_ = color;
        markDirty();
    }

    Color getBorderColor() const { return borderColor_; }
    void setBorderColor(const Color& color) {
        borderColor_ = color;
        markDirty();
    }

    float getBorderRadius() const { return borderRadius_; }
    void setBorderRadius(float radius) {
        borderRadius_ = radius;
        markDirty();
    }

    float getBorderWidth() const { return borderWidth_; }
    void setBorderWidth(float width) {
        borderWidth_ = width;
        markDirty();
    }

    bool hasHeader() const { return showHeader_; }
    void setShowHeader(bool show) {
        showHeader_ = show;
        markDirty();
    }

    float getHeaderHeight() const { return headerHeight_; }
    void setHeaderHeight(float height) {
        headerHeight_ = height;
        markDirty();
    }

    bool isCollapsible() const { return collapsible_; }
    void setCollapsible(bool collapsible) {
        collapsible_ = collapsible;
        markDirty();
    }

    bool isCollapsed() const { return collapsed_; }
    void setCollapsed(bool collapsed) {
        collapsed_ = collapsed;
        markDirty();
    }

    void layout(const Rect& available) override;
    void drawBackground(DrawList& drawList) override;
    void drawContent(DrawList& drawList) override;

  private:
    std::string title_;
    Color backgroundColor_{38, 38, 38, 242};
    Color headerColor_{50, 50, 55, 255};
    Color borderColor_{70, 70, 80, 255};
    float borderRadius_ = 8.0f;
    float borderWidth_ = 1.0f;
    float headerHeight_ = 24.0f;
    bool showHeader_ = true;
    bool collapsible_ = false;
    bool collapsed_ = false;
};

} // namespace dakt::gui

#endif // DAKTLIB_GUI_PANEL_HPP

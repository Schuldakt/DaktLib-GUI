#ifndef DAKT_GUI_BUTTON_HPP
#define DAKT_GUI_BUTTON_HPP

#include "WidgetBase.hpp"

namespace dakt::gui {

/**
 * @brief Clickable button widget
 */
class DAKT_GUI_API Button : public Widget {
  public:
    Button();
    explicit Button(const std::string& label);

    const std::string& getLabel() const { return label_; }
    void setLabel(const std::string& label);

    Color getBackgroundColor() const { return backgroundColor_; }
    void setBackgroundColor(const Color& color) {
        backgroundColor_ = color;
        markDirty();
    }

    Color getHoverColor() const { return hoverColor_; }
    void setHoverColor(const Color& color) {
        hoverColor_ = color;
        markDirty();
    }

    Color getActiveColor() const { return activeColor_; }
    void setActiveColor(const Color& color) {
        activeColor_ = color;
        markDirty();
    }

    Color getTextColor() const { return textColor_; }
    void setTextColor(const Color& color) {
        textColor_ = color;
        markDirty();
    }

    float getBorderRadius() const { return borderRadius_; }
    void setBorderRadius(float radius) {
        borderRadius_ = radius;
        markDirty();
    }

    Vec2 measureContent() override;
    bool handleInput(const WidgetEvent& event) override;
    void drawBackground(DrawList& drawList) override;
    void drawContent(DrawList& drawList) override;

  private:
    std::string label_;
    Color backgroundColor_{77, 77, 77, 255};
    Color hoverColor_{102, 102, 102, 255};
    Color activeColor_{51, 51, 51, 255};
    Color textColor_{255, 255, 255, 255};
    float borderRadius_ = 4.0f;
};

} // namespace dakt::gui

#endif // DAKT_GUI_BUTTON_HPP

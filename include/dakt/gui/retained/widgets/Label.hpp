#ifndef DAKTLIB_GUI_LABEL_HPP
#define DAKTLIB_GUI_LABEL_HPP

#include "WidgetBase.hpp"

namespace dakt::gui {

/**
 * @brief Simple text label widget
 */
class DAKTLIB_GUI_API Label : public Widget {
  public:
    Label();
    explicit Label(const std::string& text);

    const std::string& getText() const { return text_; }
    void setText(const std::string& text);

    Color getColor() const { return color_; }
    void setColor(const Color& color) {
        color_ = color;
        markDirty();
    }

    float getFontSize() const { return fontSize_; }
    void setFontSize(float size) {
        fontSize_ = size;
        markDirty();
    }

    Vec2 measureContent() override;
    void drawContent(DrawList& drawList) override;

  private:
    std::string text_;
    Color color_{255, 255, 255, 255};
    float fontSize_ = 14.0f;
};

} // namespace dakt::gui

#endif // DAKTLIB_GUI_LABEL_HPP

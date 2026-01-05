#ifndef DAKT_GUI_CHECKBOX_HPP
#define DAKT_GUI_CHECKBOX_HPP

#include "WidgetBase.hpp"

namespace dakt::gui {

/**
 * @brief Checkbox toggle widget
 */
class DAKT_GUI_API Checkbox : public Widget {
  public:
    Checkbox();
    explicit Checkbox(const std::string& label);

    const std::string& getLabel() const { return label_; }
    void setLabel(const std::string& label);

    bool isChecked() const { return hasFlag(RetainedWidgetFlags::Checked); }
    void setChecked(bool checked);
    void toggle() { setChecked(!isChecked()); }

    float getCheckSize() const { return checkSize_; }
    void setCheckSize(float size) {
        checkSize_ = size;
        markDirty();
    }

    Color getCheckColor() const { return checkColor_; }
    void setCheckColor(const Color& color) {
        checkColor_ = color;
        markDirty();
    }

    Vec2 measureContent() override;
    bool handleInput(const WidgetEvent& event) override;
    void drawContent(DrawList& drawList) override;

  private:
    std::string label_;
    float checkSize_ = 18.0f;
    Color checkColor_{100, 180, 100, 255};
};

} // namespace dakt::gui

#endif // DAKT_GUI_CHECKBOX_HPP

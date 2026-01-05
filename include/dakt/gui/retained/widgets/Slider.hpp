#ifndef DAKT_GUI_SLIDER_HPP
#define DAKT_GUI_SLIDER_HPP

#include "WidgetBase.hpp"

namespace dakt::gui {

/**
 * @brief Slider widget for numeric values
 */
class DAKT_GUI_API Slider : public Widget {
  public:
    Slider();
    Slider(float min, float max, float value = 0.0f);

    float getValue() const { return value_; }
    void setValue(float value);

    float getMin() const { return min_; }
    void setMin(float min) {
        min_ = min;
        markDirty();
    }

    float getMax() const { return max_; }
    void setMax(float max) {
        max_ = max;
        markDirty();
    }

    float getStep() const { return step_; }
    void setStep(float step) { step_ = step; }

    const std::string& getLabel() const { return label_; }
    void setLabel(const std::string& label) {
        label_ = label;
        markDirty();
    }

    bool isShowValue() const { return showValue_; }
    void setShowValue(bool show) {
        showValue_ = show;
        markDirty();
    }

    Color getTrackColor() const { return trackColor_; }
    void setTrackColor(const Color& color) {
        trackColor_ = color;
        markDirty();
    }

    Color getFillColor() const { return fillColor_; }
    void setFillColor(const Color& color) {
        fillColor_ = color;
        markDirty();
    }

    Color getThumbColor() const { return thumbColor_; }
    void setThumbColor(const Color& color) {
        thumbColor_ = color;
        markDirty();
    }

    Vec2 measureContent() override;
    bool handleInput(const WidgetEvent& event) override;
    void drawContent(DrawList& drawList) override;

  private:
    float getNormalizedValue() const;
    void setNormalizedValue(float normalized);

    std::string label_;
    float value_ = 0.0f;
    float min_ = 0.0f;
    float max_ = 1.0f;
    float step_ = 0.0f;
    float trackHeight_ = 4.0f;
    float thumbRadius_ = 8.0f;
    bool dragging_ = false;
    bool showValue_ = false;
    Color trackColor_{60, 60, 70, 255};
    Color fillColor_{80, 140, 200, 255};
    Color thumbColor_{220, 220, 230, 255};
};

} // namespace dakt::gui

#endif // DAKT_GUI_SLIDER_HPP

#ifndef DAKT_GUI_PROGRESS_BAR_HPP
#define DAKT_GUI_PROGRESS_BAR_HPP

#include "WidgetBase.hpp"

namespace dakt::gui {

/**
 * @brief Progress bar widget showing completion status
 */
class DAKT_GUI_API ProgressBar : public Widget {
  public:
    ProgressBar();
    ProgressBar(float min, float max, float value = 0.0f);

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

    float getProgress() const;
    void setProgress(float progress); // 0.0 to 1.0

    const std::string& getLabel() const { return label_; }
    void setLabel(const std::string& label) {
        label_ = label;
        markDirty();
    }

    bool isShowPercentage() const { return showPercentage_; }
    void setShowPercentage(bool show) {
        showPercentage_ = show;
        markDirty();
    }

    bool isIndeterminate() const { return indeterminate_; }
    void setIndeterminate(bool indeterminate) {
        indeterminate_ = indeterminate;
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

    float getBorderRadius() const { return borderRadius_; }
    void setBorderRadius(float radius) {
        borderRadius_ = radius;
        markDirty();
    }

    Vec2 measureContent() override;
    void drawContent(DrawList& drawList) override;

  private:
    float value_ = 0.0f;
    float min_ = 0.0f;
    float max_ = 100.0f;
    std::string label_;
    bool showPercentage_ = true;
    bool indeterminate_ = false;
    float animOffset_ = 0.0f;
    Color trackColor_{50, 50, 60, 255};
    Color fillColor_{80, 160, 80, 255};
    float borderRadius_ = 4.0f;
    float barHeight_ = 8.0f;
};

} // namespace dakt::gui

#endif // DAKT_GUI_PROGRESS_BAR_HPP

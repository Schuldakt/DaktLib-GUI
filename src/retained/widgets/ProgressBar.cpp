#include "dakt/gui/retained/widgets/ProgressBar.hpp"
#include "dakt/gui/draw/DrawList.hpp"
#include <algorithm>
#include <cmath>

namespace dakt::gui {

ProgressBar::ProgressBar() : Widget() { setPreferredSize(Vec2(200, 24)); }

ProgressBar::ProgressBar(float min, float max, float value) : Widget(), min_(min), max_(max), value_(value) { setPreferredSize(Vec2(200, 24)); }

void ProgressBar::setValue(float value) {
    value_ = std::max(min_, std::min(max_, value));
    markDirty();
}

float ProgressBar::getProgress() const {
    if (max_ <= min_)
        return 0.0f;
    return (value_ - min_) / (max_ - min_);
}

void ProgressBar::setProgress(float progress) { setValue(min_ + progress * (max_ - min_)); }

Vec2 ProgressBar::measureContent() { return Vec2(200, barHeight_ + (label_.empty() ? 0 : 18)); }

void ProgressBar::drawContent(DrawList& drawList) {
    float labelHeight = label_.empty() ? 0 : 18.0f;
    float barY = bounds_.y + labelHeight;

    // Draw label
    if (!label_.empty()) {
        drawList.drawText(Vec2(bounds_.x, bounds_.y), label_.c_str(), Color{200, 200, 200, 255});
    }

    // Draw track
    Rect trackRect(bounds_.x, barY, bounds_.width, barHeight_);
    drawList.drawRectFilledRounded(trackRect, trackColor_, borderRadius_);

    // Draw fill
    if (!indeterminate_) {
        float fillWidth = bounds_.width * getProgress();
        if (fillWidth > 0) {
            Rect fillRect(bounds_.x, barY, fillWidth, barHeight_);
            drawList.drawRectFilledRounded(fillRect, fillColor_, borderRadius_);
        }

        // Draw percentage text
        if (showPercentage_) {
            char percentText[16];
            snprintf(percentText, sizeof(percentText), "%.0f%%", getProgress() * 100);
            float textX = bounds_.x + bounds_.width / 2 - 15;
            float textY = barY + (barHeight_ - 12) / 2;
            drawList.drawText(Vec2(textX, textY), percentText, Color{255, 255, 255, 255});
        }
    } else {
        // Indeterminate animation (sliding bar)
        float animWidth = bounds_.width * 0.3f;
        float animX = bounds_.x + (bounds_.width - animWidth) * (0.5f + 0.5f * std::sin(animOffset_));
        Rect animRect(animX, barY, animWidth, barHeight_);
        drawList.drawRectFilledRounded(animRect, fillColor_, borderRadius_);
    }
}

} // namespace dakt::gui

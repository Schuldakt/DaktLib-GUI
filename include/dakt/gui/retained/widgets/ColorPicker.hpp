#ifndef DAKT_GUI_COLOR_PICKER_HPP
#define DAKT_GUI_COLOR_PICKER_HPP

#include "WidgetBase.hpp"

namespace dakt::gui {

/**
 * @brief Color picker widget with HSV/RGB modes
 *
 * Provides a color selection interface with:
 * - Saturation/Value square picker
 * - Hue slider
 * - Alpha slider (optional)
 * - RGB/HSV input fields
 * - Hex color input
 * - Color preview
 */
class DAKT_GUI_API ColorPicker : public Widget {
  public:
    enum class Mode {
        HSV,  // Hue-Saturation-Value
        RGB,  // Red-Green-Blue
        Wheel // Color wheel picker
    };

    ColorPicker();
    explicit ColorPicker(const Color& initialColor);

    // Color access
    Color getColor() const { return color_; }
    void setColor(const Color& color);

    // HSV access
    float getHue() const { return hue_; }
    float getSaturation() const { return saturation_; }
    float getValue() const { return value_; }
    void setHSV(float h, float s, float v);

    // Alpha
    bool isAlphaEnabled() const { return showAlpha_; }
    void setAlphaEnabled(bool enabled) {
        showAlpha_ = enabled;
        markDirty();
    }

    // Mode
    Mode getMode() const { return mode_; }
    void setMode(Mode mode) {
        mode_ = mode;
        markDirty();
    }

    // Display options
    bool isHexInputShown() const { return showHexInput_; }
    void setHexInputShown(bool shown) {
        showHexInput_ = shown;
        markDirty();
    }

    bool isPreviewShown() const { return showPreview_; }
    void setPreviewShown(bool shown) {
        showPreview_ = shown;
        markDirty();
    }

    bool isInputFieldsShown() const { return showInputFields_; }
    void setInputFieldsShown(bool shown) {
        showInputFields_ = shown;
        markDirty();
    }

    // Callbacks
    void setOnColorChanged(WidgetCallback callback) { onColorChanged_ = std::move(callback); }

    // Widget overrides
    Vec2 measureContent() override;
    bool handleInput(const WidgetEvent& event) override;
    void drawContent(DrawList& drawList) override;

  private:
    // Color conversion helpers
    void rgbToHsv(const Color& rgb, float& h, float& s, float& v);
    void hsvToRgb(float h, float s, float v, Color& rgb);
    void updateColorFromHSV();
    void updateHSVFromColor();

    // Hit testing
    bool isInSVSquare(const Vec2& pos) const;
    bool isInHueBar(const Vec2& pos) const;
    bool isInAlphaBar(const Vec2& pos) const;

    // Drawing helpers
    void drawSVSquare(DrawList& drawList, const Rect& rect);
    void drawHueBar(DrawList& drawList, const Rect& rect);
    void drawAlphaBar(DrawList& drawList, const Rect& rect);
    void drawPreview(DrawList& drawList, const Rect& rect);
    void drawColorWheel(DrawList& drawList, const Rect& rect);

    Color color_{255, 255, 255, 255};
    float hue_ = 0.0f;        // 0-360
    float saturation_ = 1.0f; // 0-1
    float value_ = 1.0f;      // 0-1

    Mode mode_ = Mode::HSV;
    bool showAlpha_ = true;
    bool showHexInput_ = true;
    bool showPreview_ = true;
    bool showInputFields_ = true;

    // Interaction state
    bool draggingSV_ = false;
    bool draggingHue_ = false;
    bool draggingAlpha_ = false;

    // Layout dimensions
    float svSquareSize_ = 150.0f;
    float barWidth_ = 20.0f;
    float barSpacing_ = 8.0f;
    float previewHeight_ = 30.0f;

    // Cached rects for hit testing
    Rect svSquareRect_;
    Rect hueBarRect_;
    Rect alphaBarRect_;

    WidgetCallback onColorChanged_;
};

} // namespace dakt::gui

#endif // DAKT_GUI_COLOR_PICKER_HPP

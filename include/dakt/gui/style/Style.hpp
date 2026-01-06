#ifndef DAKTLIB_GUI_STYLE_HPP
#define DAKTLIB_GUI_STYLE_HPP

#include "../core/Types.hpp"
#include <string>
#include <unordered_map>

namespace dakt::gui {

// ============================================================================
// Style Properties
// ============================================================================

struct DAKTLIB_GUI_API WidgetStyle {
    // Background
    Color backgroundColor = Color(40, 40, 40, 255);
    Color backgroundColorHover = Color(60, 60, 60, 255);
    Color backgroundColorActive = Color(80, 80, 80, 255);
    Color backgroundColorDisabled = Color(30, 30, 30, 200);

    // Border
    Color borderColor = Color(80, 80, 80, 255);
    Color borderColorHover = Color(100, 100, 100, 255);
    Color borderColorActive = Color(120, 120, 120, 255);
    float borderWidth = 1.0f;
    BorderRadius borderRadius;

    // Text
    Color textColor = Color(220, 220, 220, 255);
    Color textColorHover = Color(255, 255, 255, 255);
    Color textColorActive = Color(255, 255, 255, 255);
    Color textColorDisabled = Color(128, 128, 128, 255);
    float fontSize = 14.0f;

    // Spacing
    EdgeInsets padding = EdgeInsets(8.0f, 12.0f);
    EdgeInsets margin;

    // Shadow
    Shadow shadow;
    bool hasShadow = false;

    // Get colors based on widget state
    Color getBackgroundColor(WidgetState state) const;
    Color getBorderColor(WidgetState state) const;
    Color getTextColor(WidgetState state) const;
};

// ============================================================================
// Color Scheme (semantic colors)
// ============================================================================

struct DAKTLIB_GUI_API ColorScheme {
    // Primary colors
    Color primary = Color(66, 133, 244, 255); // Blue
    Color primaryHover = Color(88, 155, 255, 255);
    Color primaryActive = Color(44, 111, 222, 255);

    // Secondary colors
    Color secondary = Color(95, 99, 104, 255); // Gray
    Color secondaryHover = Color(115, 119, 124, 255);
    Color secondaryActive = Color(75, 79, 84, 255);

    // Semantic colors
    Color success = Color(52, 168, 83, 255); // Green
    Color warning = Color(251, 188, 4, 255); // Yellow
    Color error = Color(234, 67, 53, 255);   // Red
    Color info = Color(66, 133, 244, 255);   // Blue

    // Surface colors
    Color background = Color(30, 30, 30, 255);
    Color surface = Color(40, 40, 40, 255);
    Color surfaceVariant = Color(50, 50, 50, 255);

    // Text colors
    Color textPrimary = Color(230, 230, 230, 255);
    Color textSecondary = Color(170, 170, 170, 255);
    Color textDisabled = Color(100, 100, 100, 255);

    // Border colors
    Color border = Color(70, 70, 70, 255);
    Color borderFocus = Color(66, 133, 244, 255);
};

// ============================================================================
// Typography
// ============================================================================

struct DAKTLIB_GUI_API Typography {
    std::string fontFamily = "system-ui";

    float fontSizeH1 = 32.0f;
    float fontSizeH2 = 24.0f;
    float fontSizeH3 = 20.0f;
    float fontSizeH4 = 18.0f;
    float fontSizeBody = 14.0f;
    float fontSizeSmall = 12.0f;
    float fontSizeCaption = 10.0f;

    float lineHeight = 1.4f;
    float letterSpacing = 0.0f;
};

// ============================================================================
// Spacing Scale
// ============================================================================

struct DAKTLIB_GUI_API SpacingScale {
    float xs = 4.0f;
    float sm = 8.0f;
    float md = 16.0f;
    float lg = 24.0f;
    float xl = 32.0f;
    float xxl = 48.0f;
};

// ============================================================================
// Theme
// ============================================================================

class DAKTLIB_GUI_API Theme {
  public:
    Theme();
    ~Theme();

    // Color scheme
    ColorScheme& colors() { return colors_; }
    const ColorScheme& colors() const { return colors_; }

    // Typography
    Typography& typography() { return typography_; }
    const Typography& typography() const { return typography_; }

    // Spacing
    SpacingScale& spacing() { return spacing_; }
    const SpacingScale& spacing() const { return spacing_; }

    // Widget styles
    WidgetStyle& getButtonStyle() { return buttonStyle_; }
    const WidgetStyle& getButtonStyle() const { return buttonStyle_; }

    WidgetStyle& getInputStyle() { return inputStyle_; }
    const WidgetStyle& getInputStyle() const { return inputStyle_; }

    WidgetStyle& getPanelStyle() { return panelStyle_; }
    const WidgetStyle& getPanelStyle() const { return panelStyle_; }

    // Named colors (legacy interface)
    void setColor(const char* name, Color color);
    Color getColor(const char* name) const;

    // Apply dark/light mode
    void setDarkMode(bool dark);
    bool isDarkMode() const { return darkMode_; }

    // Create default themes
    static Theme createDark();
    static Theme createLight();

  private:
    ColorScheme colors_;
    Typography typography_;
    SpacingScale spacing_;

    WidgetStyle buttonStyle_;
    WidgetStyle inputStyle_;
    WidgetStyle panelStyle_;

    std::unordered_map<std::string, Color> namedColors_;
    bool darkMode_ = true;

    void initDefaultStyles();
};

} // namespace dakt::gui

#endif

#include "dakt/gui/style/Style.hpp"

namespace dakt::gui {

// ============================================================================
// WidgetStyle Implementation
// ============================================================================

Color WidgetStyle::getBackgroundColor(WidgetState state) const {
    switch (state) {
    case WidgetState::Hover:
        return backgroundColorHover;
    case WidgetState::Active:
        return backgroundColorActive;
    case WidgetState::Disabled:
        return backgroundColorDisabled;
    case WidgetState::Focus:
        return backgroundColorHover;
    default:
        return backgroundColor;
    }
}

Color WidgetStyle::getBorderColor(WidgetState state) const {
    switch (state) {
    case WidgetState::Hover:
        return borderColorHover;
    case WidgetState::Active:
        return borderColorActive;
    case WidgetState::Disabled:
        return borderColor;
    case WidgetState::Focus:
        return borderColorActive;
    default:
        return borderColor;
    }
}

Color WidgetStyle::getTextColor(WidgetState state) const {
    switch (state) {
    case WidgetState::Hover:
        return textColorHover;
    case WidgetState::Active:
        return textColorActive;
    case WidgetState::Disabled:
        return textColorDisabled;
    case WidgetState::Focus:
        return textColorHover;
    default:
        return textColor;
    }
}

// ============================================================================
// Theme Implementation
// ============================================================================

Theme::Theme() { initDefaultStyles(); }

Theme::~Theme() = default;

void Theme::initDefaultStyles() {
    // Button style
    buttonStyle_.backgroundColor = Color(55, 55, 55, 255);
    buttonStyle_.backgroundColorHover = Color(70, 70, 70, 255);
    buttonStyle_.backgroundColorActive = Color(45, 45, 45, 255);
    buttonStyle_.backgroundColorDisabled = Color(40, 40, 40, 200);
    buttonStyle_.borderColor = Color(80, 80, 80, 255);
    buttonStyle_.borderColorHover = colors_.borderFocus;
    buttonStyle_.borderColorActive = colors_.primary;
    buttonStyle_.borderWidth = 1.0f;
    buttonStyle_.borderRadius = BorderRadius(4.0f);
    buttonStyle_.textColor = colors_.textPrimary;
    buttonStyle_.textColorHover = Color(255, 255, 255, 255);
    buttonStyle_.textColorActive = Color(255, 255, 255, 255);
    buttonStyle_.textColorDisabled = colors_.textDisabled;
    buttonStyle_.padding = EdgeInsets(8.0f, 16.0f);

    // Input style
    inputStyle_.backgroundColor = Color(35, 35, 35, 255);
    inputStyle_.backgroundColorHover = Color(40, 40, 40, 255);
    inputStyle_.backgroundColorActive = Color(30, 30, 30, 255);
    inputStyle_.backgroundColorDisabled = Color(30, 30, 30, 200);
    inputStyle_.borderColor = Color(70, 70, 70, 255);
    inputStyle_.borderColorHover = Color(90, 90, 90, 255);
    inputStyle_.borderColorActive = colors_.borderFocus;
    inputStyle_.borderWidth = 1.0f;
    inputStyle_.borderRadius = BorderRadius(4.0f);
    inputStyle_.textColor = colors_.textPrimary;
    inputStyle_.textColorDisabled = colors_.textDisabled;
    inputStyle_.padding = EdgeInsets(8.0f, 12.0f);

    // Panel style
    panelStyle_.backgroundColor = colors_.surface;
    panelStyle_.backgroundColorHover = colors_.surface;
    panelStyle_.backgroundColorActive = colors_.surface;
    panelStyle_.borderColor = colors_.border;
    panelStyle_.borderWidth = 1.0f;
    panelStyle_.borderRadius = BorderRadius(8.0f);
    panelStyle_.textColor = colors_.textPrimary;
    panelStyle_.padding = EdgeInsets(16.0f);
}

void Theme::setColor(const char* name, Color color) { namedColors_[name] = color; }

Color Theme::getColor(const char* name) const {
    auto it = namedColors_.find(name);
    if (it != namedColors_.end()) {
        return it->second;
    }
    return Color(255, 255, 255, 255);
}

void Theme::setDarkMode(bool dark) {
    darkMode_ = dark;
    if (dark) {
        colors_ = createDark().colors_;
    } else {
        colors_ = createLight().colors_;
    }
    initDefaultStyles();
}

Theme Theme::createDark() {
    Theme theme;
    theme.darkMode_ = true;

    // Dark theme colors
    theme.colors_.primary = Color(66, 133, 244, 255);
    theme.colors_.primaryHover = Color(88, 155, 255, 255);
    theme.colors_.primaryActive = Color(44, 111, 222, 255);

    theme.colors_.secondary = Color(95, 99, 104, 255);
    theme.colors_.success = Color(52, 168, 83, 255);
    theme.colors_.warning = Color(251, 188, 4, 255);
    theme.colors_.error = Color(234, 67, 53, 255);
    theme.colors_.info = Color(66, 133, 244, 255);

    theme.colors_.background = Color(30, 30, 30, 255);
    theme.colors_.surface = Color(40, 40, 40, 255);
    theme.colors_.surfaceVariant = Color(50, 50, 50, 255);

    theme.colors_.textPrimary = Color(230, 230, 230, 255);
    theme.colors_.textSecondary = Color(170, 170, 170, 255);
    theme.colors_.textDisabled = Color(100, 100, 100, 255);

    theme.colors_.border = Color(70, 70, 70, 255);
    theme.colors_.borderFocus = Color(66, 133, 244, 255);

    theme.initDefaultStyles();
    return theme;
}

Theme Theme::createLight() {
    Theme theme;
    theme.darkMode_ = false;

    // Light theme colors
    theme.colors_.primary = Color(66, 133, 244, 255);
    theme.colors_.primaryHover = Color(55, 122, 233, 255);
    theme.colors_.primaryActive = Color(44, 111, 222, 255);

    theme.colors_.secondary = Color(95, 99, 104, 255);
    theme.colors_.success = Color(52, 168, 83, 255);
    theme.colors_.warning = Color(251, 188, 4, 255);
    theme.colors_.error = Color(234, 67, 53, 255);
    theme.colors_.info = Color(66, 133, 244, 255);

    theme.colors_.background = Color(245, 245, 245, 255);
    theme.colors_.surface = Color(255, 255, 255, 255);
    theme.colors_.surfaceVariant = Color(240, 240, 240, 255);

    theme.colors_.textPrimary = Color(32, 33, 36, 255);
    theme.colors_.textSecondary = Color(95, 99, 104, 255);
    theme.colors_.textDisabled = Color(180, 180, 180, 255);

    theme.colors_.border = Color(218, 220, 224, 255);
    theme.colors_.borderFocus = Color(66, 133, 244, 255);

    theme.initDefaultStyles();
    return theme;
}

} // namespace dakt::gui

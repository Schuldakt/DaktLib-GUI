// Theme implementation stubs
#include "dakt/gui/style/Style.hpp"

namespace dakt::gui {

Theme::Theme() = default;
Theme::~Theme() = default;

void Theme::setColor(const char* name, Color color) {
    // Stub implementation
}

Color Theme::getColor(const char* name) const { return Color(255, 255, 255, 255); }

} // namespace dakt::gui

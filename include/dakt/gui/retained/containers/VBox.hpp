#ifndef DAKTLIB_GUI_VBOX_HPP
#define DAKTLIB_GUI_VBOX_HPP

#include "ContainerBase.hpp"

namespace dakt::gui {

/**
 * @brief Vertical box container - stacks children top to bottom
 */
class DAKTLIB_GUI_API VBox : public UIContainer {
  public:
    VBox() : UIContainer() { setLayoutDirection(LayoutDirection::Vertical); }

    explicit VBox(const std::string& id) : UIContainer(id) { setLayoutDirection(LayoutDirection::Vertical); }

    VBox(const std::string& id, float spacing) : UIContainer(id) {
        setLayoutDirection(LayoutDirection::Vertical);
        setSpacing(spacing);
    }

    // Convenience methods for VBox
    void setVerticalAlignment(Alignment alignment) { setMainAxisAlignment(alignment); }

    void setHorizontalAlignment(Alignment alignment) { setCrossAxisAlignment(alignment); }
};

} // namespace dakt::gui

#endif // DAKTLIB_GUI_VBOX_HPP

#ifndef DAKTLIB_GUI_HBOX_HPP
#define DAKTLIB_GUI_HBOX_HPP

#include "ContainerBase.hpp"

namespace dakt::gui {

/**
 * @brief Horizontal box container - stacks children left to right
 */
class DAKTLIB_GUI_API HBox : public UIContainer {
  public:
    HBox() : UIContainer() { setLayoutDirection(LayoutDirection::Horizontal); }

    explicit HBox(const std::string& id) : UIContainer(id) { setLayoutDirection(LayoutDirection::Horizontal); }

    HBox(const std::string& id, float spacing) : UIContainer(id) {
        setLayoutDirection(LayoutDirection::Horizontal);
        setSpacing(spacing);
    }

    // Convenience methods for HBox
    void setHorizontalAlignment(Alignment alignment) { setMainAxisAlignment(alignment); }

    void setVerticalAlignment(Alignment alignment) { setCrossAxisAlignment(alignment); }
};

} // namespace dakt::gui

#endif // DAKTLIB_GUI_HBOX_HPP

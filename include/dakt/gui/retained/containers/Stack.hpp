#ifndef DAKT_GUI_STACK_HPP
#define DAKT_GUI_STACK_HPP

#include "ContainerBase.hpp"

namespace dakt::gui {

/**
 * @brief Stack container - overlays children on top of each other
 *
 * Children are rendered in order (first child at bottom, last at top).
 * Useful for overlays, modals, and layered interfaces.
 */
class DAKT_GUI_API Stack : public UIContainer {
  public:
    Stack();
    explicit Stack(const std::string& id);

    // Alignment for all children (relative to stack bounds)
    Alignment getHorizontalAlignment() const { return hAlign_; }
    void setHorizontalAlignment(Alignment align) {
        hAlign_ = align;
        markDirty();
    }

    Alignment getVerticalAlignment() const { return vAlign_; }
    void setVerticalAlignment(Alignment align) {
        vAlign_ = align;
        markDirty();
    }

    // Per-widget alignment override
    void setWidgetAlignment(Widget* widget, Alignment hAlign, Alignment vAlign);

    void layout();

  private:
    struct WidgetAlignment {
        Alignment horizontal = Alignment::Center;
        Alignment vertical = Alignment::Center;
    };

    void layoutStack();

    Alignment hAlign_ = Alignment::Center;
    Alignment vAlign_ = Alignment::Center;
    std::unordered_map<Widget*, WidgetAlignment> widgetAlignments_;
};

} // namespace dakt::gui

#endif // DAKT_GUI_STACK_HPP

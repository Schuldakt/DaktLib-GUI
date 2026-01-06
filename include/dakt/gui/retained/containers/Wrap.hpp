#ifndef DAKTLIB_GUI_WRAP_HPP
#define DAKTLIB_GUI_WRAP_HPP

#include "ContainerBase.hpp"

namespace dakt::gui {

/**
 * @brief Wrap container - flows children and wraps to next line when full
 *
 * Similar to CSS flexbox with flex-wrap: wrap
 */
class DAKTLIB_GUI_API Wrap : public UIContainer {
  public:
    Wrap();
    explicit Wrap(const std::string& id);

    // Direction children flow before wrapping
    LayoutDirection getFlowDirection() const { return flowDirection_; }
    void setFlowDirection(LayoutDirection direction) {
        flowDirection_ = direction;
        markDirty();
    }

    // Spacing between items
    float getHorizontalSpacing() const { return hSpacing_; }
    void setHorizontalSpacing(float spacing) {
        hSpacing_ = spacing;
        markDirty();
    }

    float getVerticalSpacing() const { return vSpacing_; }
    void setVerticalSpacing(float spacing) {
        vSpacing_ = spacing;
        markDirty();
    }

    void setSpacing(float spacing) {
        hSpacing_ = vSpacing_ = spacing;
        markDirty();
    }

    // Alignment
    Alignment getLineAlignment() const { return lineAlignment_; }
    void setLineAlignment(Alignment alignment) {
        lineAlignment_ = alignment;
        markDirty();
    }

    void layout();

  private:
    void layoutWrap();

    LayoutDirection flowDirection_ = LayoutDirection::Horizontal;
    float hSpacing_ = 8.0f;
    float vSpacing_ = 8.0f;
    Alignment lineAlignment_ = Alignment::Start;
};

} // namespace dakt::gui

#endif // DAKTLIB_GUI_WRAP_HPP

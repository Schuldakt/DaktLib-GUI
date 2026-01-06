#ifndef DAKTLIB_GUI_SPLITTER_HPP
#define DAKTLIB_GUI_SPLITTER_HPP

#include "ContainerBase.hpp"

namespace dakt::gui {

/**
 * @brief Splitter container - divides space between two children with draggable divider
 */
class DAKTLIB_GUI_API Splitter : public UIContainer {
  public:
    enum class Orientation {
        Horizontal, // Left/Right split
        Vertical    // Top/Bottom split
    };

    Splitter();
    explicit Splitter(Orientation orientation);
    Splitter(const std::string& id, Orientation orientation);

    Orientation getOrientation() const { return orientation_; }
    void setOrientation(Orientation orientation) {
        orientation_ = orientation;
        markDirty();
    }

    // Split ratio (0.0 to 1.0, position of divider)
    float getSplitRatio() const { return splitRatio_; }
    void setSplitRatio(float ratio);

    // Minimum size for each panel (in pixels)
    float getMinFirstSize() const { return minFirstSize_; }
    void setMinFirstSize(float size) {
        minFirstSize_ = size;
        markDirty();
    }

    float getMinSecondSize() const { return minSecondSize_; }
    void setMinSecondSize(float size) {
        minSecondSize_ = size;
        markDirty();
    }

    // Divider appearance
    float getDividerSize() const { return dividerSize_; }
    void setDividerSize(float size) {
        dividerSize_ = size;
        markDirty();
    }

    Color getDividerColor() const { return dividerColor_; }
    void setDividerColor(const Color& color) {
        dividerColor_ = color;
        markDirty();
    }

    Color getDividerHoverColor() const { return dividerHoverColor_; }
    void setDividerHoverColor(const Color& color) {
        dividerHoverColor_ = color;
        markDirty();
    }

    bool isResizable() const { return resizable_; }
    void setResizable(bool resizable) { resizable_ = resizable; }

    // Get first and second panels
    Widget* getFirstPanel() const;
    Widget* getSecondPanel() const;

    void layout();
    void render(Context& ctx);

  private:
    void layoutSplitter();
    bool isDividerHovered(const Vec2& mousePos) const;

    Orientation orientation_ = Orientation::Horizontal;
    float splitRatio_ = 0.5f;
    float minFirstSize_ = 50.0f;
    float minSecondSize_ = 50.0f;
    float dividerSize_ = 6.0f;
    Color dividerColor_{60, 60, 70, 255};
    Color dividerHoverColor_{80, 80, 100, 255};
    bool resizable_ = true;
    bool draggingDivider_ = false;
    Rect dividerRect_;
};

} // namespace dakt::gui

#endif // DAKTLIB_GUI_SPLITTER_HPP

#ifndef DAKTLIB_GUI_TAB_BAR_HPP
#define DAKTLIB_GUI_TAB_BAR_HPP

#include "WidgetBase.hpp"

namespace dakt::gui {

/**
 * @brief Tab bar with multiple tabs
 */
class DAKTLIB_GUI_API TabBar : public Widget {
  public:
    struct Tab {
        std::string label;
        std::string id;
        bool closeable = false;
        bool enabled = true;
    };

    TabBar();

    void addTab(const std::string& label, const std::string& id = "", bool closeable = false);
    void removeTab(size_t index);
    void removeTab(const std::string& id);
    void clearTabs();

    size_t getTabCount() const { return tabs_.size(); }
    const Tab& getTab(size_t index) const { return tabs_[index]; }
    Tab& getTab(size_t index) { return tabs_[index]; }

    size_t getSelectedIndex() const { return selectedIndex_; }
    void setSelectedIndex(size_t index);

    const std::string& getSelectedId() const;

    void setTabEnabled(size_t index, bool enabled);

    // Colors and styling
    Color getBackgroundColor() const { return backgroundColor_; }
    void setBackgroundColor(const Color& color) {
        backgroundColor_ = color;
        markDirty();
    }

    Color getSelectedColor() const { return selectedColor_; }
    void setSelectedColor(const Color& color) {
        selectedColor_ = color;
        markDirty();
    }

    Color getHoverColor() const { return hoverColor_; }
    void setHoverColor(const Color& color) {
        hoverColor_ = color;
        markDirty();
    }

    float getTabHeight() const { return tabHeight_; }
    void setTabHeight(float height) {
        tabHeight_ = height;
        markDirty();
    }

    float getTabPadding() const { return tabPadding_; }
    void setTabPadding(float padding) {
        tabPadding_ = padding;
        markDirty();
    }

    // Callbacks
    void setOnTabSelected(std::function<void(size_t)> callback) { onTabSelected_ = std::move(callback); }
    void setOnTabClosed(std::function<void(size_t)> callback) { onTabClosed_ = std::move(callback); }

    Vec2 measureContent() override;
    bool handleInput(const WidgetEvent& event) override;
    void drawContent(DrawList& drawList) override;

  private:
    std::vector<Tab> tabs_;
    size_t selectedIndex_ = 0;
    float tabHeight_ = 28.0f;
    float tabPadding_ = 12.0f;
    Color backgroundColor_{40, 40, 45, 255};
    Color selectedColor_{60, 60, 70, 255};
    Color hoverColor_{50, 50, 55, 255};
    std::function<void(size_t)> onTabSelected_;
    std::function<void(size_t)> onTabClosed_;
};

} // namespace dakt::gui

#endif // DAKTLIB_GUI_TAB_BAR_HPP

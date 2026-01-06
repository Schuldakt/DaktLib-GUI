#ifndef DAKTLIB_GUI_DROPDOWN_HPP
#define DAKTLIB_GUI_DROPDOWN_HPP

#include "WidgetBase.hpp"

namespace dakt::gui {

/**
 * @brief Dropdown/ComboBox widget
 */
class DAKTLIB_GUI_API Dropdown : public Widget {
  public:
    struct Item {
        std::string label;
        std::string value;
        bool enabled = true;
        bool separator = false;
    };

    Dropdown();
    explicit Dropdown(const std::string& placeholder);

    // Items
    void addItem(const std::string& label, const std::string& value = "");
    void addSeparator();
    void removeItem(size_t index);
    void clearItems();

    size_t getItemCount() const { return items_.size(); }
    const Item& getItem(size_t index) const { return items_[index]; }

    // Selection
    int getSelectedIndex() const { return selectedIndex_; }
    void setSelectedIndex(int index);

    const std::string& getSelectedValue() const;
    const std::string& getSelectedLabel() const;
    void selectByValue(const std::string& value);

    // Appearance
    const std::string& getPlaceholder() const { return placeholder_; }
    void setPlaceholder(const std::string& placeholder) {
        placeholder_ = placeholder;
        markDirty();
    }

    bool isOpen() const { return open_; }
    void setOpen(bool open) {
        open_ = open;
        markDirty();
    }

    float getMaxDropdownHeight() const { return maxDropdownHeight_; }
    void setMaxDropdownHeight(float height) { maxDropdownHeight_ = height; }

    Color getBackgroundColor() const { return backgroundColor_; }
    void setBackgroundColor(const Color& color) {
        backgroundColor_ = color;
        markDirty();
    }

    Color getDropdownColor() const { return dropdownColor_; }
    void setDropdownColor(const Color& color) {
        dropdownColor_ = color;
        markDirty();
    }

    Color getHoverColor() const { return hoverColor_; }
    void setHoverColor(const Color& color) {
        hoverColor_ = color;
        markDirty();
    }

    // Callbacks
    void setOnSelectionChanged(std::function<void(int)> callback) { onSelectionChanged_ = std::move(callback); }

    Vec2 measureContent() override;
    bool handleInput(const WidgetEvent& event) override;
    void drawBackground(DrawList& drawList) override;
    void drawContent(DrawList& drawList) override;

  private:
    std::vector<Item> items_;
    int selectedIndex_ = -1;
    int hoveredIndex_ = -1;
    std::string placeholder_ = "Select...";
    bool open_ = false;
    float maxDropdownHeight_ = 200.0f;
    float itemHeight_ = 24.0f;
    Color backgroundColor_{50, 50, 60, 255};
    Color dropdownColor_{40, 40, 50, 255};
    Color hoverColor_{60, 60, 70, 255};
    std::function<void(int)> onSelectionChanged_;
};

} // namespace dakt::gui

#endif // DAKTLIB_GUI_DROPDOWN_HPP

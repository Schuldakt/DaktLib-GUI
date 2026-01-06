#ifndef DAKTLIB_GUI_RADIO_BUTTON_HPP
#define DAKTLIB_GUI_RADIO_BUTTON_HPP

#include "WidgetBase.hpp"

namespace dakt::gui {

class RadioGroup;

/**
 * @brief Radio button widget (mutually exclusive selection)
 */
class DAKTLIB_GUI_API RadioButton : public Widget {
  public:
    RadioButton();
    explicit RadioButton(const std::string& label);
    RadioButton(const std::string& label, const std::string& value);

    const std::string& getLabel() const { return label_; }
    void setLabel(const std::string& label);

    const std::string& getValue() const { return value_; }
    void setValue(const std::string& value) { value_ = value; }

    bool isSelected() const { return hasFlag(RetainedWidgetFlags::Selected); }
    void setSelected(bool selected);

    RadioGroup* getGroup() const { return group_; }
    void setGroup(RadioGroup* group) { group_ = group; }

    float getRadioSize() const { return radioSize_; }
    void setRadioSize(float size) {
        radioSize_ = size;
        markDirty();
    }

    Color getSelectedColor() const { return selectedColor_; }
    void setSelectedColor(const Color& color) {
        selectedColor_ = color;
        markDirty();
    }

    Color getOutlineColor() const { return outlineColor_; }
    void setOutlineColor(const Color& color) {
        outlineColor_ = color;
        markDirty();
    }

    Vec2 measureContent() override;
    bool handleInput(const WidgetEvent& event) override;
    void drawContent(DrawList& drawList) override;

  private:
    std::string label_;
    std::string value_;
    RadioGroup* group_ = nullptr;
    float radioSize_ = 18.0f;
    Color selectedColor_{80, 140, 200, 255};
    Color outlineColor_{120, 120, 130, 255};
};

/**
 * @brief Group for managing mutually exclusive radio buttons
 */
class DAKTLIB_GUI_API RadioGroup {
  public:
    RadioGroup() = default;
    explicit RadioGroup(const std::string& id) : id_(id) {}

    const std::string& getId() const { return id_; }

    void addButton(RadioButton* button);
    void removeButton(RadioButton* button);

    RadioButton* getSelectedButton() const { return selectedButton_; }
    const std::string& getSelectedValue() const;

    void selectButton(RadioButton* button);
    void selectByValue(const std::string& value);

    void setOnSelectionChanged(std::function<void(RadioButton*)> callback) { onSelectionChanged_ = std::move(callback); }

  private:
    std::string id_;
    std::vector<RadioButton*> buttons_;
    RadioButton* selectedButton_ = nullptr;
    std::function<void(RadioButton*)> onSelectionChanged_;
};

} // namespace dakt::gui

#endif // DAKTLIB_GUI_RADIO_BUTTON_HPP

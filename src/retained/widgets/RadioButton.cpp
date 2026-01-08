#include "dakt/gui/retained/widgets/RadioButton.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"
#include <algorithm>

namespace dakt::gui {

// ============================================================================
// RadioButton
// ============================================================================

RadioButton::RadioButton() : Widget() { setPreferredSize(Vec2(150, 24)); }

RadioButton::RadioButton(const std::string& label) : Widget(), label_(label) { setPreferredSize(Vec2(150, 24)); }

RadioButton::RadioButton(const std::string& label, const std::string& value) : Widget(), label_(label), value_(value) { setPreferredSize(Vec2(150, 24)); }

void RadioButton::setLabel(const std::string& label) {
    label_ = label;
    markDirty();
}

void RadioButton::setSelected(bool selected) {
    if (selected) {
        addFlag(RetainedWidgetFlags::Selected);
        // Deselect others in group
        if (group_) {
            group_->selectButton(this);
        }
    } else {
        removeFlag(RetainedWidgetFlags::Selected);
    }
    markDirty();
}

Vec2 RadioButton::measureContent() {
    float labelWidth = label_.size() * 8.0f;
    return Vec2(radioSize_ + 8 + labelWidth, radioSize_);
}

bool RadioButton::handleInput(const WidgetEvent& event) {
    if (event.type == WidgetEventType::Click && isEnabled()) {
        setSelected(true);
        fireEvent(WidgetEventType::ValueChanged, event);
        return true;
    }
    return Widget::handleInput(event);
}

void RadioButton::drawContent(DrawList& drawList) {
    float cx = bounds_.x + radioSize_ / 2;
    float cy = bounds_.y + bounds_.height / 2;
    float radius = radioSize_ / 2;

    // Draw outer circle
    drawList.drawCircle(Vec2(cx, cy), radius, outlineColor_, 24);

    // Draw inner circle if selected
    if (isSelected()) {
        drawList.drawCircleFilled(Vec2(cx, cy), radius * 0.5f, selectedColor_, 24);
    }

    // Draw label
    if (!label_.empty()) {
        float textX = bounds_.x + radioSize_ + 8;
        float textY = bounds_.y + (bounds_.height - 14) / 2;
        drawList.drawText(Vec2(textX, textY), label_.c_str(), Color{220, 220, 220, 255});
    }
}

// ============================================================================
// RadioGroup
// ============================================================================

void RadioGroup::addButton(RadioButton* button) {
    if (!button)
        return;

    buttons_.push_back(button);
    button->setGroup(this);

    // If this is the first button and no selection, select it
    if (buttons_.size() == 1 && !selectedButton_) {
        selectButton(button);
    }
}

void RadioGroup::removeButton(RadioButton* button) {
    if (!button)
        return;

    auto it = std::find(buttons_.begin(), buttons_.end(), button);
    if (it != buttons_.end()) {
        buttons_.erase(it);
        button->setGroup(nullptr);

        if (selectedButton_ == button) {
            selectedButton_ = nullptr;
            if (!buttons_.empty()) {
                selectButton(buttons_[0]);
            }
        }
    }
}

const std::string& RadioGroup::getSelectedValue() const {
    static const std::string empty;
    return selectedButton_ ? selectedButton_->getValue() : empty;
}

void RadioGroup::selectButton(RadioButton* button) {
    if (button == selectedButton_)
        return;

    // Deselect current
    if (selectedButton_) {
        selectedButton_->removeFlag(RetainedWidgetFlags::Selected);
        selectedButton_->markDirty();
    }

    // Select new
    selectedButton_ = button;
    if (selectedButton_) {
        selectedButton_->addFlag(RetainedWidgetFlags::Selected);
        selectedButton_->markDirty();
    }

    if (onSelectionChanged_) {
        onSelectionChanged_(selectedButton_);
    }
}

void RadioGroup::selectByValue(const std::string& value) {
    for (auto* button : buttons_) {
        if (button->getValue() == value) {
            selectButton(button);
            return;
        }
    }
}

} // namespace dakt::gui

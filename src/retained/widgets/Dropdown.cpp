#include "dakt/gui/retained/widgets/Dropdown.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"

namespace dakt::gui {

Dropdown::Dropdown() : Widget() { setPreferredSize(Vec2(180, 28)); }

Dropdown::Dropdown(const std::string& placeholder) : Widget(), placeholder_(placeholder) { setPreferredSize(Vec2(180, 28)); }

void Dropdown::addItem(const std::string& label, const std::string& value) {
    Item item;
    item.label = label;
    item.value = value.empty() ? label : value;
    items_.push_back(item);
    markDirty();
}

void Dropdown::addSeparator() {
    Item item;
    item.separator = true;
    items_.push_back(item);
    markDirty();
}

void Dropdown::removeItem(size_t index) {
    if (index < items_.size()) {
        items_.erase(items_.begin() + index);
        if (selectedIndex_ == static_cast<int>(index)) {
            selectedIndex_ = -1;
        } else if (selectedIndex_ > static_cast<int>(index)) {
            --selectedIndex_;
        }
        markDirty();
    }
}

void Dropdown::clearItems() {
    items_.clear();
    selectedIndex_ = -1;
    markDirty();
}

void Dropdown::setSelectedIndex(int index) {
    if (index >= -1 && index < static_cast<int>(items_.size())) {
        int oldIndex = selectedIndex_;
        selectedIndex_ = index;
        markDirty();

        if (oldIndex != selectedIndex_ && onSelectionChanged_) {
            onSelectionChanged_(selectedIndex_);
        }
    }
}

const std::string& Dropdown::getSelectedValue() const {
    static const std::string empty;
    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(items_.size())) {
        return items_[selectedIndex_].value;
    }
    return empty;
}

const std::string& Dropdown::getSelectedLabel() const {
    static const std::string empty;
    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(items_.size())) {
        return items_[selectedIndex_].label;
    }
    return empty;
}

void Dropdown::selectByValue(const std::string& value) {
    for (size_t i = 0; i < items_.size(); ++i) {
        if (items_[i].value == value) {
            setSelectedIndex(static_cast<int>(i));
            return;
        }
    }
}

Vec2 Dropdown::measureContent() { return Vec2(180, 28); }

bool Dropdown::handleInput(const WidgetEvent& event) {
    if (event.type == WidgetEventType::Click && isEnabled()) {
        if (!open_) {
            open_ = true;
        } else {
            // Check if clicking on an item
            if (hoveredIndex_ >= 0 && hoveredIndex_ < static_cast<int>(items_.size())) {
                if (!items_[hoveredIndex_].separator && items_[hoveredIndex_].enabled) {
                    setSelectedIndex(hoveredIndex_);
                    open_ = false;
                }
            } else {
                open_ = false;
            }
        }
        markDirty();
        return true;
    }

    if (event.type == WidgetEventType::Hover && open_) {
        // Update hovered item
        float dropdownY = bounds_.y + bounds_.height;
        float relativeY = event.mousePos.y - dropdownY;
        if (relativeY >= 0) {
            hoveredIndex_ = static_cast<int>(relativeY / itemHeight_);
            if (hoveredIndex_ >= static_cast<int>(items_.size())) {
                hoveredIndex_ = -1;
            }
        } else {
            hoveredIndex_ = -1;
        }
        markDirty();
        return true;
    }

    return Widget::handleInput(event);
}

void Dropdown::drawBackground(DrawList& drawList) {
    // Main button background
    Color bgColor = isHovered() ? hoverColor_ : backgroundColor_;
    drawList.drawRectFilledRounded(bounds_, bgColor, 4.0f);
    drawList.drawRectRounded(bounds_, Color{80, 80, 90, 255}, 4.0f);
}

void Dropdown::drawContent(DrawList& drawList) {
    // Draw selected text or placeholder
    const std::string& displayText = selectedIndex_ >= 0 ? items_[selectedIndex_].label : placeholder_;
    Color textColor = selectedIndex_ >= 0 ? Color{220, 220, 220, 255} : Color{120, 120, 130, 255};

    float textY = bounds_.y + (bounds_.height - 14) / 2;
    drawList.drawText(Vec2(bounds_.x + 8, textY), displayText.c_str(), textColor);

    // Draw dropdown arrow
    float arrowX = bounds_.x + bounds_.width - 20;
    float arrowY = bounds_.y + bounds_.height / 2;
    float arrowSize = 6;

    if (open_) {
        // Up arrow
        drawList.drawTriangleFilled(Vec2(arrowX, arrowY + arrowSize / 2), Vec2(arrowX + arrowSize, arrowY + arrowSize / 2), Vec2(arrowX + arrowSize / 2, arrowY - arrowSize / 2), Color{180, 180, 180, 255});
    } else {
        // Down arrow
        drawList.drawTriangleFilled(Vec2(arrowX, arrowY - arrowSize / 2), Vec2(arrowX + arrowSize, arrowY - arrowSize / 2), Vec2(arrowX + arrowSize / 2, arrowY + arrowSize / 2), Color{180, 180, 180, 255});
    }

    // Draw dropdown list if open
    if (open_ && !items_.empty()) {
        float dropdownY = bounds_.y + bounds_.height + 2;
        float dropdownHeight = std::min(maxDropdownHeight_, static_cast<float>(items_.size()) * itemHeight_);

        Rect dropdownRect(bounds_.x, dropdownY, bounds_.width, dropdownHeight);

        // Background
        drawList.drawRectFilledRounded(dropdownRect, dropdownColor_, 4.0f);
        drawList.drawRectRounded(dropdownRect, Color{70, 70, 80, 255}, 4.0f);

        // Items
        for (size_t i = 0; i < items_.size(); ++i) {
            const auto& item = items_[i];
            float itemY = dropdownY + i * itemHeight_;

            if (item.separator) {
                // Draw separator line
                drawList.drawLine(Vec2(bounds_.x + 8, itemY + itemHeight_ / 2), Vec2(bounds_.x + bounds_.width - 8, itemY + itemHeight_ / 2), Color{60, 60, 70, 255});
            } else {
                // Highlight hovered item
                if (static_cast<int>(i) == hoveredIndex_) {
                    Rect highlightRect(bounds_.x, itemY, bounds_.width, itemHeight_);
                    drawList.drawRectFilled(highlightRect, hoverColor_);
                }

                // Item text
                Color itemTextColor = item.enabled ? Color{220, 220, 220, 255} : Color{100, 100, 110, 255};
                if (static_cast<int>(i) == selectedIndex_) {
                    itemTextColor = Color{100, 180, 255, 255};
                }

                float itemTextY = itemY + (itemHeight_ - 14) / 2;
                drawList.drawText(Vec2(bounds_.x + 12, itemTextY), item.label.c_str(), itemTextColor);
            }
        }
    }
}

} // namespace dakt::gui

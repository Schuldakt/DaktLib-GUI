#include "dakt/gui/retained/widgets/Menu.hpp"
#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/draw/DrawList.hpp"
#include <algorithm>

namespace dakt::gui {

// ============================================================================
// Menu
// ============================================================================

Menu::Menu() : Widget() {}

Menu::Menu(const std::string& title) : Widget(), title_(title) {}

void Menu::addItem(const MenuItem& item) {
    items_.push_back(item);
    markDirty();
}

void Menu::addItem(const std::string& label, WidgetCallback callback, const std::string& shortcut) {
    MenuItem item;
    item.label = label;
    item.shortcut = shortcut;
    item.onSelect = std::move(callback);
    items_.push_back(item);
    markDirty();
}

void Menu::addSeparator() {
    items_.push_back(MenuItem::Separator());
    markDirty();
}

Menu& Menu::addSubmenu(const std::string& label) {
    auto submenu = std::make_unique<Menu>(label);
    Menu* ptr = submenu.get();
    submenus_.push_back(std::move(submenu));

    MenuItem item;
    item.label = label;
    item.submenu = ptr;
    items_.push_back(item);
    markDirty();

    return *ptr;
}

void Menu::clearItems() {
    items_.clear();
    submenus_.clear();
    markDirty();
}

void Menu::open(const Vec2& position) {
    open_ = true;
    openPosition_ = position;
    hoveredIndex_ = -1;
    openSubmenu_ = nullptr;
    markDirty();
}

void Menu::close() {
    open_ = false;
    hoveredIndex_ = -1;
    if (openSubmenu_) {
        openSubmenu_->close();
        openSubmenu_ = nullptr;
    }
    markDirty();
}

Vec2 Menu::measureContent() {
    if (!open_)
        return Vec2(0, 0);

    float width = minWidth_;
    float height = 0;

    for (const auto& item : items_) {
        if (item.separator) {
            height += 8.0f;
        } else {
            height += itemHeight_;
            float textWidth = static_cast<float>(item.label.length()) * 8.0f + 40.0f;
            if (!item.shortcut.empty()) {
                textWidth += static_cast<float>(item.shortcut.length()) * 7.0f + 20.0f;
            }
            width = std::max(width, textWidth);
        }
    }

    return Vec2(width, height);
}

Rect Menu::getItemRect(int index) const {
    float y = openPosition_.y;
    for (int i = 0; i < index && i < static_cast<int>(items_.size()); ++i) {
        y += items_[i].separator ? 8.0f : itemHeight_;
    }

    float height = items_[index].separator ? 8.0f : itemHeight_;
    return Rect(openPosition_.x, y, minWidth_, height);
}

void Menu::updateHoveredItem(const Vec2& mousePos) {
    int newHovered = -1;
    float y = openPosition_.y;

    for (size_t i = 0; i < items_.size(); ++i) {
        float itemHeight = items_[i].separator ? 8.0f : itemHeight_;

        if (!items_[i].separator && mousePos.x >= openPosition_.x && mousePos.x <= openPosition_.x + minWidth_ && mousePos.y >= y && mousePos.y < y + itemHeight) {
            newHovered = static_cast<int>(i);
            break;
        }
        y += itemHeight;
    }

    if (newHovered != hoveredIndex_) {
        hoveredIndex_ = newHovered;

        // Handle submenu opening
        if (hoveredIndex_ >= 0 && items_[hoveredIndex_].submenu) {
            if (openSubmenu_ && openSubmenu_ != items_[hoveredIndex_].submenu) {
                openSubmenu_->close();
            }
            Rect itemRect = getItemRect(hoveredIndex_);
            items_[hoveredIndex_].submenu->open(Vec2(itemRect.x + itemRect.width - 4, itemRect.y));
            openSubmenu_ = items_[hoveredIndex_].submenu;
        } else if (openSubmenu_ && (hoveredIndex_ < 0 || !items_[hoveredIndex_].submenu)) {
            openSubmenu_->close();
            openSubmenu_ = nullptr;
        }

        markDirty();
    }
}

bool Menu::handleInput(const WidgetEvent& event) {
    if (!open_)
        return false;

    // First, let open submenu handle input
    if (openSubmenu_ && openSubmenu_->handleInput(event)) {
        return true;
    }

    switch (event.type) {
    case WidgetEventType::Hover:
    case WidgetEventType::DragMove:
        updateHoveredItem(event.mousePos);
        return true;

    case WidgetEventType::Click:
        if (hoveredIndex_ >= 0 && hoveredIndex_ < static_cast<int>(items_.size())) {
            const auto& item = items_[hoveredIndex_];
            if (!item.separator && !item.submenu && item.enabled) {
                if (item.onSelect) {
                    WidgetEvent e = event;
                    e.source = this;
                    item.onSelect(e);
                }
                close();
                return true;
            }
        } else {
            close();
        }
        return true;

    default:
        break;
    }

    return Widget::handleInput(event);
}

void Menu::drawContent(DrawList& drawList) {
    if (!open_)
        return;

    Vec2 size = measureContent();
    Rect menuRect(openPosition_.x, openPosition_.y, size.x, size.y);

    // Draw background with shadow
    Rect shadowRect(menuRect.x + 3, menuRect.y + 3, menuRect.width, menuRect.height);
    drawList.drawRectFilledRounded(shadowRect, Color{0, 0, 0, 80}, 4.0f);

    drawList.drawRectFilledRounded(menuRect, backgroundColor_, 4.0f);
    drawList.drawRectRounded(menuRect, Color{60, 60, 63, 255}, 4.0f);

    // Draw items
    float y = openPosition_.y;
    for (size_t i = 0; i < items_.size(); ++i) {
        const auto& item = items_[i];

        if (item.separator) {
            float sepY = y + 4;
            drawList.drawLine(Vec2(openPosition_.x + 8, sepY), Vec2(openPosition_.x + size.x - 8, sepY), separatorColor_);
            y += 8.0f;
        } else {
            Rect itemRect(openPosition_.x, y, size.x, itemHeight_);

            if (static_cast<int>(i) == hoveredIndex_ && item.enabled) {
                drawList.drawRectFilled(itemRect, hoverColor_);
            }

            Color textColor = item.enabled ? textColor_ : disabledColor_;
            drawList.drawText(Vec2(itemRect.x + 12, itemRect.y + (itemHeight_ - 14) / 2), item.label.c_str(), textColor);

            if (!item.shortcut.empty()) {
                float shortcutWidth = static_cast<float>(item.shortcut.length()) * 7.0f;
                drawList.drawText(Vec2(itemRect.x + itemRect.width - shortcutWidth - 12, itemRect.y + (itemHeight_ - 14) / 2), item.shortcut.c_str(), shortcutColor_);
            }

            if (item.checked) {
                float checkY = itemRect.y + itemHeight_ / 2;
                drawList.drawText(Vec2(itemRect.x + 2, checkY - 7), "✓", textColor);
            }

            if (item.submenu) {
                float arrowX = itemRect.x + itemRect.width - 16;
                float arrowY = itemRect.y + itemHeight_ / 2;
                drawList.drawTriangleFilled(Vec2(arrowX, arrowY - 4), Vec2(arrowX, arrowY + 4), Vec2(arrowX + 6, arrowY), textColor);
            }

            y += itemHeight_;
        }
    }

    if (openSubmenu_) {
        openSubmenu_->drawContent(drawList);
    }
}

// ============================================================================
// MenuBar
// ============================================================================

MenuBar::MenuBar() : Widget() { setPreferredSize(Vec2(0, height_)); }

Menu& MenuBar::addMenu(const std::string& title) {
    auto menu = std::make_unique<Menu>(title);
    Menu* ptr = menu.get();
    menus_.push_back(std::move(menu));
    markDirty();
    return *ptr;
}

void MenuBar::removeMenu(const std::string& title) {
    menus_.erase(std::remove_if(menus_.begin(), menus_.end(), [&title](const auto& m) { return m->getTitle() == title; }), menus_.end());
    markDirty();
}

Menu* MenuBar::getMenu(const std::string& title) {
    for (auto& menu : menus_) {
        if (menu->getTitle() == title) {
            return menu.get();
        }
    }
    return nullptr;
}

Rect MenuBar::getMenuTitleRect(int index) const {
    float x = bounds_.x;
    for (int i = 0; i < index && i < static_cast<int>(menus_.size()); ++i) {
        float width = static_cast<float>(menus_[i]->getTitle().length()) * 8.0f + itemPadding_ * 2;
        x += width;
    }
    float width = static_cast<float>(menus_[index]->getTitle().length()) * 8.0f + itemPadding_ * 2;
    return Rect(x, bounds_.y, width, height_);
}

int MenuBar::getMenuIndexAt(const Vec2& pos) const {
    if (pos.y < bounds_.y || pos.y > bounds_.y + height_)
        return -1;

    float x = bounds_.x;
    for (size_t i = 0; i < menus_.size(); ++i) {
        float width = static_cast<float>(menus_[i]->getTitle().length()) * 8.0f + itemPadding_ * 2;
        if (pos.x >= x && pos.x < x + width) {
            return static_cast<int>(i);
        }
        x += width;
    }
    return -1;
}

Vec2 MenuBar::measureContent() {
    float totalWidth = 0;
    for (const auto& menu : menus_) {
        totalWidth += static_cast<float>(menu->getTitle().length()) * 8.0f + itemPadding_ * 2;
    }
    return Vec2(totalWidth, height_);
}

bool MenuBar::handleInput(const WidgetEvent& event) {
    // First check if any open menu handles the event
    if (activeMenuIndex_ >= 0 && menus_[activeMenuIndex_]->handleInput(event)) {
        if (!menus_[activeMenuIndex_]->isOpen()) {
            activeMenuIndex_ = -1;
            menuBarActive_ = false;
        }
        return true;
    }

    switch (event.type) {
    case WidgetEventType::Hover: {
        int index = getMenuIndexAt(event.mousePos);
        if (menuBarActive_ && index >= 0 && index != activeMenuIndex_) {
            if (activeMenuIndex_ >= 0) {
                menus_[activeMenuIndex_]->close();
            }
            activeMenuIndex_ = index;
            Rect titleRect = getMenuTitleRect(index);
            menus_[index]->open(Vec2(titleRect.x, titleRect.y + height_));
        }
        markDirty();
        return true;
    }

    case WidgetEventType::Click: {
        int index = getMenuIndexAt(event.mousePos);
        if (index >= 0) {
            if (activeMenuIndex_ == index && menus_[index]->isOpen()) {
                menus_[index]->close();
                activeMenuIndex_ = -1;
                menuBarActive_ = false;
            } else {
                if (activeMenuIndex_ >= 0) {
                    menus_[activeMenuIndex_]->close();
                }
                activeMenuIndex_ = index;
                menuBarActive_ = true;
                Rect titleRect = getMenuTitleRect(index);
                menus_[index]->open(Vec2(titleRect.x, titleRect.y + height_));
            }
            markDirty();
            return true;
        }
        break;
    }

    default:
        break;
    }

    return Widget::handleInput(event);
}

void MenuBar::drawContent(DrawList& drawList) {
    drawList.drawRectFilled(bounds_, backgroundColor_);

    float x = bounds_.x;
    for (size_t i = 0; i < menus_.size(); ++i) {
        float width = static_cast<float>(menus_[i]->getTitle().length()) * 8.0f + itemPadding_ * 2;
        Rect titleRect(x, bounds_.y, width, height_);

        if (static_cast<int>(i) == activeMenuIndex_) {
            drawList.drawRectFilled(titleRect, activeColor_);
        }

        drawList.drawText(Vec2(x + itemPadding_, bounds_.y + (height_ - 14) / 2), menus_[i]->getTitle().c_str(), textColor_);
        x += width;
    }

    drawList.drawLine(Vec2(bounds_.x, bounds_.y + height_ - 1), Vec2(bounds_.x + bounds_.width, bounds_.y + height_ - 1), Color{60, 60, 63, 255});

    if (activeMenuIndex_ >= 0 && menus_[activeMenuIndex_]->isOpen()) {
        menus_[activeMenuIndex_]->drawContent(drawList);
    }
}

// ============================================================================
// ContextMenu
// ============================================================================

ContextMenu::ContextMenu() : Menu() {}

void ContextMenu::showAtCursor(const Vec2& cursorPos) { open(cursorPos); }

// ============================================================================
// Popup
// ============================================================================

Popup::Popup() : Widget() { setPreferredSize(Vec2(300, 200)); }

Popup::Popup(const std::string& title) : Widget(), title_(title) { setPreferredSize(Vec2(300, 200)); }

void Popup::open() {
    if (!open_) {
        open_ = true;
        if (onOpen_) {
            WidgetEvent e;
            e.source = this;
            onOpen_(e);
        }
        markDirty();
    }
}

void Popup::open(const Vec2& position, Anchor anchor) {
    position_ = calculateAnchoredPosition(position, anchor, preferredSize_);
    open();
}

void Popup::close() {
    if (open_) {
        open_ = false;
        dragging_ = false;
        if (onClose_) {
            WidgetEvent e;
            e.source = this;
            onClose_(e);
        }
        markDirty();
    }
}

void Popup::toggle() {
    if (open_) {
        close();
    } else {
        open();
    }
}

Vec2 Popup::calculateAnchoredPosition(const Vec2& position, Anchor anchor, const Vec2& size) {
    Vec2 result = position;

    switch (anchor) {
    case Anchor::TopLeft:
        break;
    case Anchor::TopCenter:
        result.x -= size.x / 2;
        break;
    case Anchor::TopRight:
        result.x -= size.x;
        break;
    case Anchor::CenterLeft:
        result.y -= size.y / 2;
        break;
    case Anchor::Center:
        result.x -= size.x / 2;
        result.y -= size.y / 2;
        break;
    case Anchor::CenterRight:
        result.x -= size.x;
        result.y -= size.y / 2;
        break;
    case Anchor::BottomLeft:
        result.y -= size.y;
        break;
    case Anchor::BottomCenter:
        result.x -= size.x / 2;
        result.y -= size.y;
        break;
    case Anchor::BottomRight:
        result.x -= size.x;
        result.y -= size.y;
        break;
    }

    return result;
}

Vec2 Popup::measureContent() { return preferredSize_; }

bool Popup::handleInput(const WidgetEvent& event) {
    if (!open_)
        return false;

    Rect popupRect(position_.x, position_.y, preferredSize_.x, preferredSize_.y);
    Rect titleBarRect(position_.x, position_.y, preferredSize_.x, showTitle_ ? titleBarHeight_ : 0);

    switch (event.type) {
    case WidgetEventType::Press:
        if (draggable_ && showTitle_ && titleBarRect.contains(event.mousePos)) {
            dragging_ = true;
            dragOffset_ = event.mousePos - position_;
            addFlag(RetainedWidgetFlags::Active);
            return true;
        }
        break;

    case WidgetEventType::DragMove:
        if (dragging_) {
            position_ = event.mousePos - dragOffset_;
            markDirty();
            return true;
        }
        break;

    case WidgetEventType::Release:
    case WidgetEventType::DragEnd:
        if (dragging_) {
            dragging_ = false;
            removeFlag(RetainedWidgetFlags::Active);
            return true;
        }
        break;

    case WidgetEventType::Click:
        if (closeOnOutsideClick_ && !popupRect.contains(event.mousePos)) {
            close();
            return true;
        }
        break;

    default:
        break;
    }

    return Widget::handleInput(event);
}

void Popup::build(Context& ctx) {
    if (!open_ || !visible_)
        return;

    DrawList& drawList = ctx.getDrawList();
    bounds_ = Rect(position_.x, position_.y, preferredSize_.x, preferredSize_.y);

    drawContent(drawList);

    for (auto& child : children_) {
        child->build(ctx);
    }

    clearDirty();
}

void Popup::drawContent(DrawList& drawList) {
    Rect popupRect(position_.x, position_.y, preferredSize_.x, preferredSize_.y);

    // Draw shadow
    Rect shadowRect(popupRect.x + 4, popupRect.y + 4, popupRect.width, popupRect.height);
    drawList.drawRectFilledRounded(shadowRect, Color{0, 0, 0, 60}, borderRadius_);

    // Draw background
    drawList.drawRectFilledRounded(popupRect, backgroundColor_, borderRadius_);

    // Draw title bar
    if (showTitle_) {
        Rect titleBarRect(position_.x, position_.y, preferredSize_.x, titleBarHeight_);
        drawList.drawRectFilledRounded(titleBarRect, titleBarColor_, borderRadius_);

        drawList.drawText(Vec2(titleBarRect.x + 10, titleBarRect.y + (titleBarHeight_ - 14) / 2), title_.c_str(), Color{220, 220, 220, 255});

        float closeX = titleBarRect.x + titleBarRect.width - 24;
        float closeY = titleBarRect.y + (titleBarHeight_ - 12) / 2;
        drawList.drawText(Vec2(closeX, closeY), "×", Color{180, 180, 180, 255});
    }

    // Draw border
    drawList.drawRectRounded(popupRect, borderColor_, borderRadius_);
}

// ============================================================================
// Tooltip
// ============================================================================

Tooltip::Tooltip() : Widget() {}

Tooltip::Tooltip(const std::string& text) : Widget(), text_(text) {}

void Tooltip::show(const Vec2& position) {
    position_ = position;
    visible_ = true;
    markDirty();
}

void Tooltip::hide() {
    visible_ = false;
    markDirty();
}

Vec2 Tooltip::measureContent() {
    float width = static_cast<float>(text_.length()) * 7.0f + 16.0f;
    float height = 24.0f;
    return Vec2(width, height);
}

void Tooltip::drawContent(DrawList& drawList) {
    if (!visible_)
        return;

    Vec2 size = measureContent();
    Rect tooltipRect(position_.x, position_.y, size.x, size.y);

    Rect shadowRect(tooltipRect.x + 2, tooltipRect.y + 2, tooltipRect.width, tooltipRect.height);
    drawList.drawRectFilledRounded(shadowRect, Color{0, 0, 0, 60}, 4.0f);

    drawList.drawRectFilledRounded(tooltipRect, backgroundColor_, 4.0f);
    drawList.drawRectRounded(tooltipRect, borderColor_, 4.0f);

    drawList.drawText(Vec2(tooltipRect.x + 8, tooltipRect.y + 5), text_.c_str(), textColor_);
}

} // namespace dakt::gui

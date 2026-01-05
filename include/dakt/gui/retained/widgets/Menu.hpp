#ifndef DAKT_GUI_MENU_HPP
#define DAKT_GUI_MENU_HPP

#include "WidgetBase.hpp"
#include <vector>

namespace dakt::gui {

class Menu;
class MenuBar;
class ContextMenu;

/**
 * @brief Individual menu item
 */
struct MenuItem {
    std::string label;
    std::string shortcut; // Display text like "Ctrl+S"
    std::string icon;     // Icon identifier (optional)
    bool enabled = true;
    bool checked = false;
    bool separator = false; // If true, render as separator line

    Menu* submenu = nullptr; // For nested menus
    WidgetCallback onSelect; // Called when item is clicked

    // Convenience constructors
    static MenuItem Separator() {
        MenuItem item;
        item.separator = true;
        return item;
    }

    static MenuItem Action(const std::string& label, WidgetCallback callback, const std::string& shortcut = "") {
        MenuItem item;
        item.label = label;
        item.shortcut = shortcut;
        item.onSelect = std::move(callback);
        return item;
    }

    static MenuItem Checkable(const std::string& label, bool* checked, WidgetCallback callback = nullptr) {
        MenuItem item;
        item.label = label;
        item.checked = checked ? *checked : false;
        item.onSelect = std::move(callback);
        return item;
    }
};

/**
 * @brief Popup menu that can be shown anywhere
 */
class DAKT_GUI_API Menu : public Widget {
  public:
    Menu();
    explicit Menu(const std::string& title);

    const std::string& getTitle() const { return title_; }
    void setTitle(const std::string& title) { title_ = title; }

    // Item management
    void addItem(const MenuItem& item);
    void addItem(const std::string& label, WidgetCallback callback, const std::string& shortcut = "");
    void addSeparator();
    Menu& addSubmenu(const std::string& label);
    void clearItems();

    const std::vector<MenuItem>& getItems() const { return items_; }

    // Visibility
    bool isOpen() const { return open_; }
    void open(const Vec2& position);
    void close();

    // Styling
    float getItemHeight() const { return itemHeight_; }
    void setItemHeight(float height) { itemHeight_ = height; }

    Color getBackgroundColor() const { return backgroundColor_; }
    void setBackgroundColor(const Color& color) { backgroundColor_ = color; }

    Color getHoverColor() const { return hoverColor_; }
    void setHoverColor(const Color& color) { hoverColor_ = color; }

    Vec2 measureContent() override;
    bool handleInput(const WidgetEvent& event) override;
    void drawContent(DrawList& drawList) override;

  private:
    void updateHoveredItem(const Vec2& mousePos);
    Rect getItemRect(int index) const;

    std::string title_;
    std::vector<MenuItem> items_;
    std::vector<std::unique_ptr<Menu>> submenus_; // Owned submenus

    bool open_ = false;
    Vec2 openPosition_;
    int hoveredIndex_ = -1;
    Menu* openSubmenu_ = nullptr;

    float itemHeight_ = 24.0f;
    float minWidth_ = 150.0f;
    float submenuDelay_ = 0.3f; // Seconds before opening submenu

    Color backgroundColor_{45, 45, 48, 245};
    Color hoverColor_{62, 62, 66, 255};
    Color textColor_{220, 220, 220, 255};
    Color disabledColor_{128, 128, 128, 255};
    Color separatorColor_{60, 60, 63, 255};
    Color shortcutColor_{150, 150, 150, 255};
};

/**
 * @brief Horizontal menu bar (File, Edit, View, etc.)
 */
class DAKT_GUI_API MenuBar : public Widget {
  public:
    MenuBar();

    Menu& addMenu(const std::string& title);
    void removeMenu(const std::string& title);
    Menu* getMenu(const std::string& title);

    const std::vector<std::unique_ptr<Menu>>& getMenus() const { return menus_; }

    // Styling
    float getHeight() const { return height_; }
    void setHeight(float height) {
        height_ = height;
        markDirty();
    }

    Color getBackgroundColor() const { return backgroundColor_; }
    void setBackgroundColor(const Color& color) { backgroundColor_ = color; }

    Vec2 measureContent() override;
    bool handleInput(const WidgetEvent& event) override;
    void drawContent(DrawList& drawList) override;

  private:
    Rect getMenuTitleRect(int index) const;
    int getMenuIndexAt(const Vec2& pos) const;

    std::vector<std::unique_ptr<Menu>> menus_;
    int activeMenuIndex_ = -1;
    bool menuBarActive_ = false; // True when any menu is open

    float height_ = 28.0f;
    float itemPadding_ = 12.0f;

    Color backgroundColor_{45, 45, 48, 255};
    Color hoverColor_{62, 62, 66, 255};
    Color activeColor_{0, 122, 204, 255};
    Color textColor_{220, 220, 220, 255};
};

/**
 * @brief Context menu (right-click menu)
 */
class DAKT_GUI_API ContextMenu : public Menu {
  public:
    ContextMenu();

    // Show at mouse position
    void showAtCursor(const Vec2& cursorPos);

    // Auto-close when clicking outside
    bool shouldCloseOnOutsideClick() const { return closeOnOutsideClick_; }
    void setCloseOnOutsideClick(bool close) { closeOnOutsideClick_ = close; }

  private:
    bool closeOnOutsideClick_ = true;
};

/**
 * @brief Popup window that can contain any widgets
 */
class DAKT_GUI_API Popup : public Widget {
  public:
    enum class Anchor { TopLeft, TopCenter, TopRight, CenterLeft, Center, CenterRight, BottomLeft, BottomCenter, BottomRight };

    Popup();
    explicit Popup(const std::string& title);

    const std::string& getTitle() const { return title_; }
    void setTitle(const std::string& title) {
        title_ = title;
        markDirty();
    }

    bool hasTitle() const { return showTitle_; }
    void setShowTitle(bool show) {
        showTitle_ = show;
        markDirty();
    }

    // Visibility
    bool isOpen() const { return open_; }
    void open();
    void open(const Vec2& position, Anchor anchor = Anchor::TopLeft);
    void close();
    void toggle();

    // Modal behavior
    bool isModal() const { return modal_; }
    void setModal(bool modal) { modal_ = modal; }

    // Close behavior
    bool closesOnOutsideClick() const { return closeOnOutsideClick_; }
    void setCloseOnOutsideClick(bool close) { closeOnOutsideClick_ = close; }

    bool closesOnEscape() const { return closeOnEscape_; }
    void setCloseOnEscape(bool close) { closeOnEscape_ = close; }

    // Draggable
    bool isDraggable() const { return draggable_; }
    void setDraggable(bool draggable) { draggable_ = draggable; }

    // Callbacks
    void setOnOpen(WidgetCallback callback) { onOpen_ = std::move(callback); }
    void setOnClose(WidgetCallback callback) { onClose_ = std::move(callback); }

    Vec2 measureContent() override;
    bool handleInput(const WidgetEvent& event) override;
    void build(Context& ctx) override;
    void drawContent(DrawList& drawList) override;

  private:
    Vec2 calculateAnchoredPosition(const Vec2& position, Anchor anchor, const Vec2& size);

    std::string title_;
    bool showTitle_ = true;
    bool open_ = false;
    bool modal_ = false;
    bool closeOnOutsideClick_ = true;
    bool closeOnEscape_ = true;
    bool draggable_ = true;

    Vec2 position_;
    bool dragging_ = false;
    Vec2 dragOffset_;

    float titleBarHeight_ = 28.0f;
    float borderRadius_ = 4.0f;

    Color backgroundColor_{50, 50, 54, 250};
    Color titleBarColor_{62, 62, 66, 255};
    Color borderColor_{70, 70, 74, 255};

    WidgetCallback onOpen_;
    WidgetCallback onClose_;
};

/**
 * @brief Tooltip popup that appears on hover
 */
class DAKT_GUI_API Tooltip : public Widget {
  public:
    Tooltip();
    explicit Tooltip(const std::string& text);

    const std::string& getText() const { return text_; }
    void setText(const std::string& text) {
        text_ = text;
        markDirty();
    }

    // Show/hide
    void show(const Vec2& position);
    void hide();
    bool isVisible() const { return visible_; }

    // Timing
    float getDelay() const { return delay_; }
    void setDelay(float seconds) { delay_ = seconds; }

    float getDuration() const { return duration_; }
    void setDuration(float seconds) { duration_ = seconds; }

    Vec2 measureContent() override;
    void drawContent(DrawList& drawList) override;

  private:
    std::string text_;
    bool visible_ = false;
    Vec2 position_;

    float delay_ = 0.5f;    // Time before showing
    float duration_ = 0.0f; // 0 = infinite
    float showTimer_ = 0.0f;

    Color backgroundColor_{60, 60, 64, 240};
    Color textColor_{220, 220, 220, 255};
    Color borderColor_{80, 80, 84, 255};
};

} // namespace dakt::gui

#endif // DAKT_GUI_MENU_HPP

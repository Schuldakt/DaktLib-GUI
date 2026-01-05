#ifndef DAKT_GUI_CONTAINER_BASE_HPP
#define DAKT_GUI_CONTAINER_BASE_HPP

#include "../widgets/WidgetBase.hpp"
#include <functional>
#include <unordered_map>

namespace dakt::gui {

class Context;
class DrawList;
class InputSystem;

// ============================================================================
// Layout Direction
// ============================================================================

enum class LayoutDirection {
    Vertical,   // Stack children top to bottom
    Horizontal, // Stack children left to right
    None,       // Manual positioning
};

// ============================================================================
// Layout Alignment
// ============================================================================

enum class Alignment {
    Start,
    Center,
    End,
    Stretch,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly,
};

// ============================================================================
// UIContainer - Base Container Class
// ============================================================================

/**
 * @brief Root container for retained-mode widget trees
 *
 * UIContainer manages a tree of persistent Widget objects, handles
 * input routing, layout computation, and rendering via the immediate
 * mode API bridge.
 */
class DAKT_GUI_API UIContainer {
  public:
    UIContainer();
    explicit UIContainer(const std::string& id);
    ~UIContainer();

    // Non-copyable
    UIContainer(const UIContainer&) = delete;
    UIContainer& operator=(const UIContainer&) = delete;

    // ========================================================================
    // Identity & Configuration
    // ========================================================================

    const std::string& getId() const { return id_; }
    void setId(const std::string& id) { id_ = id; }

    void setBounds(const Rect& bounds);
    const Rect& getBounds() const { return bounds_; }

    void setLayoutDirection(LayoutDirection direction) {
        direction_ = direction;
        markDirty();
    }
    LayoutDirection getLayoutDirection() const { return direction_; }

    void setMainAxisAlignment(Alignment alignment) {
        mainAxisAlign_ = alignment;
        markDirty();
    }
    Alignment getMainAxisAlignment() const { return mainAxisAlign_; }

    void setCrossAxisAlignment(Alignment alignment) {
        crossAxisAlign_ = alignment;
        markDirty();
    }
    Alignment getCrossAxisAlignment() const { return crossAxisAlign_; }

    void setSpacing(float spacing) {
        spacing_ = spacing;
        markDirty();
    }
    float getSpacing() const { return spacing_; }

    void setPadding(const Insets& padding) {
        padding_ = padding;
        markDirty();
    }
    const Insets& getPadding() const { return padding_; }

    // ========================================================================
    // Widget Management
    // ========================================================================

    Widget* getRoot() { return root_.get(); }
    const Widget* getRoot() const { return root_.get(); }

    Widget& addWidget(std::unique_ptr<Widget> widget);

    template <typename T, typename... Args> T& addWidget(Args&&... args) {
        auto widget = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *widget;
        addWidget(std::move(widget));
        return ref;
    }

    bool removeWidget(Widget* widget);
    void clear();
    Widget* findWidget(const std::string& id);
    void registerWidget(Widget* widget);
    void unregisterWidget(Widget* widget);

    // ========================================================================
    // Dirty Tracking & Layout
    // ========================================================================

    bool isDirty() const { return dirty_; }
    void markDirty() { dirty_ = true; }
    void clearDirty() { dirty_ = false; }

    void layout();
    void forceLayout();

    // ========================================================================
    // Input Processing
    // ========================================================================

    void processInput(InputSystem& input);
    void setFocus(Widget* widget);
    Widget* getFocusedWidget() const { return focusedWidget_; }
    void clearFocus();
    void focusNext();
    void focusPrevious();

    // ========================================================================
    // Rendering
    // ========================================================================

    void render(Context& ctx);
    void update(float deltaTime);

    // ========================================================================
    // Event Callbacks
    // ========================================================================

    using FocusCallback = std::function<void(Widget* oldFocus, Widget* newFocus)>;
    void setOnFocusChanged(FocusCallback callback) { onFocusChanged_ = std::move(callback); }

  protected:
    void layoutVertical();
    void layoutHorizontal();
    void layoutChildren(Widget* widget, const Rect& available);

    Widget* hitTest(const Vec2& point);
    Widget* hitTestRecursive(Widget* widget, const Vec2& point);

    void buildFocusList();
    void buildFocusListRecursive(Widget* widget, std::vector<Widget*>& list);

    std::string id_;
    Rect bounds_;
    std::unique_ptr<Widget> root_;

    LayoutDirection direction_ = LayoutDirection::Vertical;
    Alignment mainAxisAlign_ = Alignment::Start;
    Alignment crossAxisAlign_ = Alignment::Stretch;
    float spacing_ = 4.0f;
    Insets padding_;

    bool dirty_ = true;
    float totalTime_ = 0.0f;

    std::unordered_map<std::string, Widget*> widgetLookup_;

    Widget* focusedWidget_ = nullptr;
    Widget* hoveredWidget_ = nullptr;
    Widget* activeWidget_ = nullptr;
    std::vector<Widget*> focusableWidgets_;
    size_t focusIndex_ = 0;

    FocusCallback onFocusChanged_;

    Vec2 lastMousePos_;
    bool mouseDown_[5] = {false};
};

} // namespace dakt::gui

#endif // DAKT_GUI_CONTAINER_BASE_HPP

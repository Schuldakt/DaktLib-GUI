#ifndef DAKT_GUI_WIDGET_BASE_HPP
#define DAKT_GUI_WIDGET_BASE_HPP

#include "../../core/Types.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace dakt::gui {

class Context;
class DrawList;
class Widget;
class UIContainer;

// Use EdgeInsets as Insets alias for convenience
using Insets = EdgeInsets;

// ============================================================================
// Retained Widget State
// ============================================================================

/**
 * @brief Extended state flags for retained-mode widgets
 */
enum class RetainedWidgetFlags : uint32_t {
    None = 0,
    Hovered = 1 << 0,
    Active = 1 << 1,  // Being interacted with
    Focused = 1 << 2, // Has keyboard focus
    Disabled = 1 << 3,
    Selected = 1 << 4,
    Checked = 1 << 5,
    Expanded = 1 << 6, // For tree nodes, collapsibles
};

inline RetainedWidgetFlags operator|(RetainedWidgetFlags a, RetainedWidgetFlags b) { return static_cast<RetainedWidgetFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }

inline RetainedWidgetFlags operator&(RetainedWidgetFlags a, RetainedWidgetFlags b) { return static_cast<RetainedWidgetFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b)); }

inline bool hasFlag(RetainedWidgetFlags flags, RetainedWidgetFlags check) { return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(check)) != 0; }

// ============================================================================
// Widget Events
// ============================================================================

/**
 * @brief Event types for widget callbacks
 */
enum class WidgetEventType {
    Click,
    DoubleClick,
    Press,
    Release,
    Hover,
    HoverEnd,
    Focus,
    Blur,
    ValueChanged,
    Submit, // Enter pressed in text input
    DragStart,
    DragMove,
    DragEnd,
    Scroll, // Mouse wheel scroll
};

/**
 * @brief Event data passed to callbacks
 */
struct WidgetEvent {
    WidgetEventType type;
    Widget* source = nullptr;
    Vec2 mousePos;
    Vec2 mouseDelta;
    Vec2 scrollDelta; // For scroll wheel events
    MouseButton button = MouseButton::Left;
    int keyCode = 0;
    char character = 0;
};

using WidgetCallback = std::function<void(const WidgetEvent&)>;

// ============================================================================
// Widget Base Class
// ============================================================================

/**
 * @brief Base class for all retained-mode widgets
 *
 * Widgets maintain persistent state and generate immediate-mode
 * draw commands when build() is called.
 */
class DAKT_GUI_API Widget {
  public:
    Widget();
    explicit Widget(const std::string& id);
    virtual ~Widget();

    // Non-copyable, movable
    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget(Widget&&) noexcept;
    Widget& operator=(Widget&&) noexcept;

    // ========================================================================
    // Identity
    // ========================================================================

    const std::string& getId() const { return id_; }
    void setId(const std::string& id) { id_ = id; }

    // ========================================================================
    // Hierarchy
    // ========================================================================

    Widget* getParent() const { return parent_; }
    const std::vector<std::unique_ptr<Widget>>& getChildren() const { return children_; }

    Widget& addChild(std::unique_ptr<Widget> child);

    template <typename T, typename... Args> T& addChild(Args&&... args) {
        auto widget = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *widget;
        addChild(std::move(widget));
        return ref;
    }

    bool removeChild(Widget* child);
    void clearChildren();
    Widget* findChild(const std::string& id);

    // ========================================================================
    // Layout
    // ========================================================================

    const Rect& getBounds() const { return bounds_; }
    void setBounds(const Rect& bounds);

    Vec2 getPosition() const { return Vec2(bounds_.x, bounds_.y); }
    void setPosition(const Vec2& pos);

    Vec2 getSize() const { return Vec2(bounds_.width, bounds_.height); }
    void setSize(const Vec2& size);

    Vec2 getMinSize() const { return minSize_; }
    void setMinSize(const Vec2& size) {
        minSize_ = size;
        markDirty();
    }

    Vec2 getMaxSize() const { return maxSize_; }
    void setMaxSize(const Vec2& size) {
        maxSize_ = size;
        markDirty();
    }

    Vec2 getPreferredSize() const { return preferredSize_; }
    void setPreferredSize(const Vec2& size) {
        preferredSize_ = size;
        markDirty();
    }

    const Insets& getMargin() const { return margin_; }
    void setMargin(const Insets& margin) {
        margin_ = margin;
        markDirty();
    }

    const Insets& getPadding() const { return padding_; }
    void setPadding(const Insets& padding) {
        padding_ = padding;
        markDirty();
    }

    virtual Vec2 measureContent();
    virtual void layout(const Rect& available);

    // ========================================================================
    // State
    // ========================================================================

    RetainedWidgetFlags getFlags() const { return flags_; }
    void setFlags(RetainedWidgetFlags state);
    void addFlag(RetainedWidgetFlags state);
    void removeFlag(RetainedWidgetFlags state);
    bool hasFlag(RetainedWidgetFlags state) const;

    bool isHovered() const { return hasFlag(RetainedWidgetFlags::Hovered); }
    bool isActive() const { return hasFlag(RetainedWidgetFlags::Active); }
    bool isFocused() const { return hasFlag(RetainedWidgetFlags::Focused); }
    bool isDisabled() const { return hasFlag(RetainedWidgetFlags::Disabled); }
    bool isEnabled() const { return !isDisabled(); }

    void setEnabled(bool enabled);
    void setVisible(bool visible) {
        visible_ = visible;
        markDirty();
    }
    bool isVisible() const { return visible_; }

    // ========================================================================
    // Dirty Tracking
    // ========================================================================

    bool isDirty() const { return dirty_; }
    void markDirty();
    void clearDirty() { dirty_ = false; }

    bool needsLayout() const { return needsLayout_; }
    void markNeedsLayout();
    void clearNeedsLayout() { needsLayout_ = false; }

    // ========================================================================
    // Events
    // ========================================================================

    void setOnClick(WidgetCallback callback) { onClick_ = std::move(callback); }
    void setOnDoubleClick(WidgetCallback callback) { onDoubleClick_ = std::move(callback); }
    void setOnHover(WidgetCallback callback) { onHover_ = std::move(callback); }
    void setOnValueChanged(WidgetCallback callback) { onValueChanged_ = std::move(callback); }

    virtual bool handleInput(const WidgetEvent& event);

    // ========================================================================
    // Rendering
    // ========================================================================

    virtual void build(Context& ctx);
    virtual void drawBackground(DrawList& drawList);
    virtual void drawContent(DrawList& drawList);
    virtual void drawChildren(Context& ctx);

  protected:
    void fireEvent(WidgetEventType type, const WidgetEvent& baseEvent = {});
    void propagateDirty();

    std::string id_;
    Widget* parent_ = nullptr;
    std::vector<std::unique_ptr<Widget>> children_;

    Rect bounds_;
    Vec2 minSize_{0, 0};
    Vec2 maxSize_{10000, 10000};
    Vec2 preferredSize_{0, 0};
    Insets margin_;
    Insets padding_;

    RetainedWidgetFlags flags_ = RetainedWidgetFlags::None;
    bool visible_ = true;
    bool dirty_ = true;
    bool needsLayout_ = true;

    WidgetCallback onClick_;
    WidgetCallback onDoubleClick_;
    WidgetCallback onHover_;
    WidgetCallback onValueChanged_;
};

} // namespace dakt::gui

#endif // DAKT_GUI_WIDGET_BASE_HPP

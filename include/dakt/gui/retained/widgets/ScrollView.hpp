#ifndef DAKT_GUI_SCROLL_VIEW_HPP
#define DAKT_GUI_SCROLL_VIEW_HPP

#include "WidgetBase.hpp"

namespace dakt::gui {

/**
 * @brief Scrollable container widget
 */
class DAKT_GUI_API ScrollView : public Widget {
  public:
    ScrollView();

    Vec2 getScrollOffset() const { return scrollOffset_; }
    void setScrollOffset(const Vec2& offset);

    Vec2 getContentSize() const { return contentSize_; }

    bool isHorizontalScrollEnabled() const { return horizontalScroll_; }
    void setHorizontalScrollEnabled(bool enabled) {
        horizontalScroll_ = enabled;
        markDirty();
    }

    bool isVerticalScrollEnabled() const { return verticalScroll_; }
    void setVerticalScrollEnabled(bool enabled) {
        verticalScroll_ = enabled;
        markDirty();
    }

    float getScrollbarWidth() const { return scrollbarWidth_; }
    void setScrollbarWidth(float width) {
        scrollbarWidth_ = width;
        markDirty();
    }

    Color getScrollbarColor() const { return scrollbarColor_; }
    void setScrollbarColor(const Color& color) {
        scrollbarColor_ = color;
        markDirty();
    }

    Color getScrollbarTrackColor() const { return scrollbarTrackColor_; }
    void setScrollbarTrackColor(const Color& color) {
        scrollbarTrackColor_ = color;
        markDirty();
    }

    void scrollToTop();
    void scrollToBottom();
    void scrollTo(const Vec2& position);
    void scrollBy(const Vec2& delta);

    void layout(const Rect& available) override;
    bool handleInput(const WidgetEvent& event) override;
    void build(Context& ctx) override;

  private:
    void updateContentSize();
    void clampScrollOffset();
    void drawScrollbars(DrawList& drawList);

    Vec2 scrollOffset_{0, 0};
    Vec2 contentSize_{0, 0};
    bool horizontalScroll_ = false;
    bool verticalScroll_ = true;
    float scrollbarWidth_ = 8.0f;
    bool scrollbarDragging_ = false;
    bool scrollbarHorizontalDragging_ = false;
    Color scrollbarColor_{100, 100, 110, 200};
    Color scrollbarTrackColor_{40, 40, 45, 100};
};

} // namespace dakt::gui

#endif // DAKT_GUI_SCROLL_VIEW_HPP

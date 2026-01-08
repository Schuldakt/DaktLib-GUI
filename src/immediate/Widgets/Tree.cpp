#include "dakt/gui/immediate/Widgets/Tree.hpp"
#include "dakt/gui/immediate/Immediate.hpp"
#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/WidgetBase.hpp"

#include <cstring>

namespace dakt::gui {

    bool treeNode(const char* label, TreeNodeFlags flags) {
        auto w = widgetSetup();
        if (!w) return false;

        Vec2 pos = getCursorPos();
        Vec2 windowPos = getWindowPos();
        Vec2 nodePos = windowPos + pos;

        bool hovered = isMouseHoveringRect(nodePos, nodePos + Vec2(200, 20));
        bool clicked = hovered && isMouseReleased(MouseButton::Left);

        w.dl->drawText(nodePos, ">", w.colors->textPrimary);
        w.dl->drawText(Vec2(nodePos.x + 16, nodePos.y), label, w.colors->textPrimary);

        setCursorPos(Vec2(pos.x, pos.y + 20 +4.0f));

        return clicked;
    }

    bool treeNodeEx(const char* strId, TreeNodeFlags flags, const char* fmt, ...) {
        return treeNode(strId, flags);
    }

    void treePop() {
        unindent();
    }

    bool collapsingHeader(const char* label, TreeNodeFlags flags) {
        return treeNode(label, flags);
    }

    bool selectable(const char* label, bool selected, Vec2 size) {
        auto w = widgetSetup();
        if (!w) return false;

        ID id = getID(label);

        Vec2 pos = getCursorPos();
        Vec2 windowPos = getWindowPos();
        Vec2 itemPos = windowPos + pos;

        Vec2 itemSize = size;
        if (itemSize.x <= 0)
            itemSize.x = getContentRegionAvail().x;
        if (itemSize.y <= 0)
            itemSize.y = 20.0f;

        Rect itemRect(itemPos.x, itemPos.y, itemSize.x, itemSize.y);

        bool hovered = isMouseHoveringRect(itemPos, itemPos + itemSize);
        bool clicked = hovered && isMouseReleased(MouseButton::Left);

        if (selected || hovered) {
            Color bgColor = selected ? w.colors->primary : w.colors->surfaceVariant;
            w.dl->drawRectFilled(itemRect, bgColor);
        }

        w.dl->drawText(Vec2(itemPos.x + 4, itemPos.y + 2), label, selected ? Color::white() : w.colors->textPrimary);

        w.state->lastItemId = id;
        w.state->lastItemHovered = hovered;

        setCursorPos(Vec2(pos.x, pos.y + itemSize.y));

        return clicked;
    }

    bool selectable(const char* label, bool* selected, Vec2 size) {
        if (selectable(label, *selected, size)) {
            *selected = !(*selected);
            return true;
        }
        return false;
    }

    bool listBox(const char* label, int* currentItem, const char* const items[], int itemCount, int heightInItems) {
        if (!currentItem || !items)
            return false;

        text("%s", label);
        bool changed = false;
        for (int i = 0; i < itemCount; i++) {
            if (selectable(items[i], i == *currentItem, Vec2(0, 0))) {
                *currentItem = i;
                changed = true;
            }
        }
        return changed;
    }

} // namespace dakt::gui
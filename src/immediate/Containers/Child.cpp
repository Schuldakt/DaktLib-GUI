#include "dakt/gui/immediate/Containers/Child.hpp"

#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"

#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"

namespace dakt::gui {

    bool beginChild(const char* id, Vec2 size, bool border) {
        Context* ctx = getCurrentContext();
        if (!ctx || !id) return false;

        ImmediateState& s = getState();
        if (!s.currentWindow) return false;

        WindowState* parent = s.currentWindow;

        // Backup parent window and layout state
        ChildState cs;
        cs.parentWindow = parent;
        cs.parentCursorBackup = parent->cursorPos;
        cs.parentCursorStartBackup = parent->cursorStartPos;
        cs.parentLastItemRectBackup = s.lastItemRect;

        // Determine child size defaults based on available content region
        const float padding = 8.0f;
        Vec2 parentMax = parent->pos + parent->size - Vec2(padding, padding);
        Vec2 avail = parentMax - parent->cursorPos;

        if (size.x <= 0.0f) size.x = avail.x;
        if (size.y <= 0.0f) size.y = avail.y;

        // Generate a stabled child ID: combine parent window id + child id
        pushID(parent->id);
        ID childId = getID(id);
        popID();

        // Create/retrieve persistent child window
        WindowState* child = nullptr;
        auto it = s.windowsById.find(childId);
        if (it == s.windowsById.end()) {
            auto ws = std::make_unique<WindowState>();
            ws->id = childId;
            ws->name = id;
            ws->flags = WindowFlags::NoTitleBar;
            ws->collapsed = false;
            ws->skipItems = false;
            ws->pos = parent->cursorPos;
            ws->size = size;
            ws->cursorStartPos = ws->pos + Vec2(padding, padding);
            ws->cursorPos = ws->cursorStartPos;

            child = ws.get();
            s.windowsById.emplace(childId, std::move(ws));
        } else {
            child = it->second.get();
        }

        // Configure child geometry
        child->pos = parent->cursorPos;
        child->size = size;
        child->cursorStartPos = child->pos + Vec2(padding, padding);
        child->cursorPos = child->cursorStartPos;
        child->skipItems = false;

        // Track child geometry for endChild()
        cs.childPos = child->pos;
        cs.childSize = child->size;
        cs.childWindow = child;
        
        // Push child container state
        s.childStack.push_back(cs);

        // Switch active window to child
        s.windowStack.push_back(child);
        s.currentWindow = child;


        // Draw child background/border if requested
        DrawList dl = ctx->getDrawList();
        dl.drawRectFilledRounded(
            Rect(child->pos.x, child->pos.y, child->size.x, child->size.y),
           Color::fromFloats(0.12f, 0.12f, 0.12f, 1.0f),
          3.0f
        );

        if (border) {
            dl.drawRectRounded(
                Rect(child->pos.x, child->pos.y, child->size.x, child->size.y),
               Color::fromFloats(0.25f, 0.25f, 0.25f, 1.0f),
              3.0f
            );
        }

        // Child is now active window
        pushID(id);

        return !child->collapsed;
    }

    void endChild() {
        ImmediateState& s = getState();
        if (s.childStack.empty()) return;

        // Pop child ID scope
        popID();

        // Current window is child; pop it
        if (!s.windowStack.empty()) {
            s.windowStack.pop_back();
        }

        ChildState cs = s.childStack.back();
        s.childStack.pop_back();

        if (!s.currentWindow) return;

        WindowState* parent = cs.parentWindow;

        // Advance parent cursor as if child is one item
        const float spacingY = 8.0f;
        parent->cursorPos.x = parent->cursorStartPos.x;
        parent->cursorPos.y = cs.childPos.y + cs.childSize.y + spacingY;

        s.lastItemRect = Rect(cs.childPos.x, cs.childPos.y, cs.childSize.x, cs.childSize.y);
    }

} // namespace dakt::gui
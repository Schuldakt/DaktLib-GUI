#include "dakt/gui/immediate/Containers/Window.hpp"

#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"

#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"

namespace dakt::gui {

    static bool hasFlag(WindowFlags flags, WindowFlags flag) {
        return (static_cast<std::uint32_t>(flags) & static_cast<std::uint32_t>(flag)) != 0;
    }

    bool beginWindow(const char* title, bool* open, WindowFlags flags) {
        Context* ctx = getCurrentContext();
        if (!ctx || !title) {
            return false;
        }

        ImmediateState& s = getState();

        // If caller controls open state and it's false, skip
        if (open && !(*open)) {
            return false;
        }

        // Stable window ID
        ID id = getID(title);

        // Fetch or create persistent window storage
        WindowState* window = nullptr;

        auto it = s.windowsById.find(id);
        if (it == s.windowsById.end()) {
            auto ws = std::make_unique<WindowState>();
            ws->id = id;
            ws->name = title;
            ws->pos = Vec2(100, 100);
            ws->size = Vec2(400, 300);
            ws->contentSize = Vec2(0, 0);
            ws->cursorPos = Vec2(0, 0);
            ws->cursorStartPos = Vec2(0, 0);
            ws->flags = flags;
            ws->collapsed = false;
            ws->skipItems = false;

            window = ws.get();
            s.windowsById.emplace(id, std::move(ws));
        } else {
            window = it->second.get();
        }

        // Update runtime properties
        window->name = title;
        window->flags = flags;
        window->skipItems = false;

        // Apply next-window hints
        if (s.nextWindowPosSet) {
            window->pos = s.nextWindowPos;
            s.nextWindowPosSet = false;
        }
        if (s.nextWindowSizeSet) {
            window->size = s.nextWindowSize;
            s.nextWindowSizeSet = false;
        }

        // COmpute cursor start (simple default padding + title bar)
        const float paddingX = 8.0f;
        const float paddingY = 8.0f;
        const float titleBarHeight = hasFlag(flags, WindowFlags::NoTitleBar) ? 0.0f : 24.0f;

        window->cursorStartPos = Vec2(
            window->pos.x + paddingX,
            window->pos.y + titleBarHeight + paddingY
        );
        window->cursorPos = window->cursorStartPos;

        // Push onto window stack and set current pointer (NO COPY)
        s.windowStack.push_back(window);
        s.currentWindow = window;

        // Push window ID scope
        pushID(title);

        // Basic draw for window (optional, can be expanded later)
        // This assumes ctx->getDrawList() exists and can draw rect/text.
        DrawList& dl = ctx->getDrawList();

        if (!hasFlag(flags, WindowFlags::NoBackground)) {
            dl.drawRectFilledRounded(Rect(window->pos.x, window->pos.y, window->size.x, window->size.y),
                                     Color::fromFloats(0.15f, 0.15f, 0.15f, 4.0f), 4.0f);
        }

        if (!hasFlag(flags, WindowFlags::NoTitleBar)) {
            dl.drawRectFilledRounded(Rect(window->pos.x, window->pos.y, window->size.x, titleBarHeight),
                                     Color::fromFloats(0.20f, 0.20f, 0.20f, 1.0f), 4.0f);
            
            dl.drawText(Vec2(window->pos.x + paddingX, window->pos.y + 4.0f),
                        title, Color::fromFloats(1, 1, 1, 1));
        }

        // If window is collapsed, you might want to skip content
        if (window->collapsed) {
            window->skipItems = true;
            return false;
        }

        return true;
    }

    void endWindow() {
        ImmediateState& s = getState();

        if (s.windowStack.empty()) {
            return;
        }

        // Pop ID scope
        popID();

        // Pop window stack
        s.windowStack.pop_back();

        // Restore current window pointer
        s.currentWindow = s.windowStack.empty() ? nullptr : s.windowStack.back();
    }

    void setNextWindowPos(Vec2 pos) {
        ImmediateState& s = getState();
        s.nextWindowPosSet = true;
        s.nextWindowPos = pos;
    }

    void setNextWindowSize(Vec2 size) {
        ImmediateState& s = getState();
        s.nextWindowSizeSet = true;
        s.nextWindowSize = size;
    }

    void setNextWindowCollapsed(bool /*collapsed*/) {
        // NOTE: Your ImmediateState currently has no nextWindowCollapsedSet.
        // Add it if you want this feature; otherwise this becomes a no-op.
        // Leaving no-op is acceptable until you implement collapsed windows.
    }

    Vec2 getWindowPos() {
        ImmediateState& s = getState();
        return s.currentWindow ? s.currentWindow->pos : Vec2(0, 0);
    }

    Vec2 getContentRegionAvail() {
        ImmediateState& s = getState();
        if (!s.currentWindow) return Vec2(0, 0);

        // Compute bottom-right content corner (simple padding)
        const float paddingX = 8.0f;
        const float paddingY = 8.0f;
        Vec2 max = s.currentWindow->pos + s.currentWindow->size - Vec2(paddingX, paddingY);

        return max - s.currentWindow->cursorPos;
    }

    Vec2 getCursorPos() {
        ImmediateState& s = getState();
        if (!s.currentWindow) return Vec2(0, 0);
        // Return cursor local to window origin
        return s.currentWindow->cursorPos - s.currentWindow->pos;
    }

    void setCursorPos(Vec2 pos) {
        ImmediateState& s = getState();
        if (!s.currentWindow) return;
        // Set cursor absolute
        s.currentWindow->cursorPos = s.currentWindow->pos + pos;
    }

} // namespace dakt::gui
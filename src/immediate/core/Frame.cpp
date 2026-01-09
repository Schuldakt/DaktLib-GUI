#include "dakt/gui/immediate/core/Frame.hpp"

#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"

namespace dakt::gui {

    void beginFrame(Context& ctx, float deltaTime) {
        // Establish current context for this thread
        setCurrentContext(&ctx);

        // Grab context-owned ImmediateState
        ImmediateState& s = getState();

        // Frame bookkeeping
        s.deltaTime = deltaTime;
        s.frameIndex++;

        // Input capture (once per frame)
        // Context owns authoritative mouse input.
        s.mouse = ctx.getMouseInput();

        // Per-frame resets
        s.hotId = 0;
        s.lastItemHovered = false;
        s.lastItemActive = false;
        s.lastItemClicked = false;

        s.windowStack.clear();
        s.currentWindow = nullptr;

        s.groupStack.clear();
        s.childStack.clear();

        s.idStack.clear();

        s.lastItemId = 0;
        s.lastItemRect = Rect(0, 0, 0, 0);
    }

    void endFrame(Context& ctx) {
        (void)ctx;
        setCurrentContext(nullptr);
    }
}
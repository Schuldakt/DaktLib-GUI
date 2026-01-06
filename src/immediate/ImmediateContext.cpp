#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"

namespace dakt::gui {

    void beginFrame(Context& ctx, float deltaTime) {
        auto& s = ctx.getImmediateState();
        setCurrent(&s);

        s.ctx = &ctx;
        s.deltaTime = deltaTime;
        s.frameIndex++;

        // Reset per-frame stacks
        s.idStack.clear();
        s.windowStack.clear();
        s.groupStack.clear();
        s.lastItem = {};
        // Keep s.windowById and s.itemStates persistent across frames.
    }

    void endFrame(Context& ctx) {
        auto& s = ctx.getImmediateState();
        (void)s;

        setCurrent(nullptr);
    }

} // namespace dakt::gui
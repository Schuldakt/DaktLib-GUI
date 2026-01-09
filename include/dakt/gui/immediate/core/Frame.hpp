#pragma once

#include "dakt/gui/core/Types.hpp"

namespace dakt::gui {

    class Context;

    /**
     * Immediate-mode frame lifecycle.
     * 
     * beginFrame:
     * - sets the current thread-local context
     * - copies input into ImmediateState
     * - clears per-frame values (hot item, stacks, last item flags)
     *
     * endFrame:
     * - clears the thread-local current context
     */
    DAKTLIB_GUI_API void beginFrame(Context& ctx, float deltaTime);
    DAKTLIB_GUI_API void endFrame(Context& ctx);

} // namespace dakt::gui
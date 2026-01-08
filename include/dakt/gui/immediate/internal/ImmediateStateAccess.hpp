#pragma once

#include "dakt/gui/core/Types.hpp"

namespace dakt::gui {

    class Context;
    struct ImmediateState;

    // ========================================================================
    // Context-based API (preferred)
    // ========================================================================

    // Set/clear the current context for this thread (called by beginFrame/endFrame)
    DAKTLIB_GUI_API void setCurrentContext(Context* ctx);

    // Get current context. Returns nullptr if called outside beginFrame/endFrame.
    DAKTLIB_GUI_API Context* getCurrentContext();

    // Safe check if a context is active.
    DAKTLIB_GUI_API bool hasCurrentContext();

    // ========================================================================
    // Convenience accessors
    // ========================================================================

    // Get current immediate state. Undefined behavior if no context is active.
    DAKTLIB_GUI_API ImmediateState& getState();

    // Safe check if state is available
    DAKTLIB_GUI_API bool hasState();

} // namespace dakt::gui

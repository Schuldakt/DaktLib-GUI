#pragma once

namespace dakt::gui {

    struct ImmediateState;

    // Set/clear the current immediate state for this thread.
    void setCurrent(ImmediateState* state);

    // Get current stat. Undefined behavior if called outside beginFrame/endFrame.
    ImmediateState& current();

    // Safe check if a frame is active.
    bool hasCurrent();

} // namespace dakt::gui
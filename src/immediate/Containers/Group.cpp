#include "dakt/gui/immediate/Containers/Group.hpp"

#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"

namespace dakt::gui {

    void beginGroup() {
        ImmediateState& s = getState();
        if (!s.currentWindow) return;

        GroupState gs;
        gs.cursorPosBackup = s.currentWindow->cursorPos;
        gs.cursorStartBackup = s.currentWindow->cursorStartPos;
        gs.lastItemRectBackup = s.lastItemRect;

        s.groupStack.push_back(gs);
    }

    void endGroup() {
        ImmediateState& s = getState();
        if (!s.currentWindow) return;
        if (s.groupStack.empty()) return;

        GroupState gs = s.groupStack.back();
        s.groupStack.pop_back();

        // Restore layout state
        s.currentWindow->cursorPos = gs.cursorPosBackup;
        s.currentWindow->cursorStartPos = gs.cursorStartBackup;
        s.lastItemRect = gs.lastItemRectBackup;
    }

} // namespace dakt::gui
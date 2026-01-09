#include "dakt/gui/immediate/Containers/Layout.hpp"

#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"
#include "dakt/gui/subsystems/style/Style.hpp"

#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"

namespace dakt::gui {

    void sameLine(float offsetX, float spacing) {
        ImmediateState& state = getState();
        if (!state.currentWindow) {
            return;
        }

        float sp = (spacing < 0.0f) ? 8.0f : spacing;

        // Place cursor at the end of the last item + spacing + optional offset
        state.currentWindow->cursorPos.x = state.lastItemRect.right() + sp + offsetX;
        state.currentWindow->cursorPos.y = state.lastItemRect.y;
    }

    void newLine() {
        ImmediateState& state = getState();
        if (!state.currentWindow) {
            return;
        }

        // Reset X back to line start, move down by line height.
        state.currentWindow->cursorPos.x = state.currentWindow->cursorStartPos.x;
        state.currentWindow->cursorPos.y += 20.0f;
    }

    void spacing() {
        ImmediateState& state = getState();
        if (!state.currentWindow) {
            return;
        }

        state.currentWindow->cursorPos.y += 8.0f;
    }

    void separator() {
        Context* ctx = getCurrentContext();
        if (!ctx) return;

        ImmediateState& state = getState();
        if (!state.currentWindow) 
            return;

        // Draw simple horizontal line across the content region
        DrawList& dl = ctx->getDrawList();
        const Theme& theme = ctx->getTheme();

        Vec2 p1 = state.currentWindow->cursorPos;
        Vec2 p2 = Vec2(state.currentWindow->pos.x + state.currentWindow->size.x - 8.0f, p1.y);

        dl.drawLine(p1, p2, theme.colors().border, 1.0f);

        // Advance cursor down after separator
        state.currentWindow->cursorPos.y += 8.0f;
    }

    void indent(float indentW) {
        ImmediateState& state = getState();
        if (!state.currentWindow)
            return;

        float w = (indentW == 0.0f) ? 20.0f : indentW;
        state.currentWindow->cursorPos.x += w;
        state.currentWindow->cursorStartPos.x += w;
    }

    void unindent(float indentW) {
        ImmediateState& state = getState();
        if (!state.currentWindow)
            return;

        float w = (indentW == 0.0f) ? 20.0f : indentW;
        state.currentWindow->cursorPos.x -= w;
        state.currentWindow->cursorStartPos.x -= w;
    }

} // namespace dakt::gui

#include "dakt/gui/immediate/Widgets/ProgressBar.hpp"

#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"

#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"

#include <algorithm>
#include <cstring>

namespace dakt::gui {

    void progressBar(float fraction, Vec2 size, const char* overlay) {
        Context* ctx = getCurrentContext();
        if (!ctx) return;

        ImmediateState& s = getState();
        if (!s.currentWindow) return;
        if (s.currentWindow->skipItems) return;

        WindowState* win = s.currentWindow;

        fraction = std::clamp(fraction, 0.0f, 1.0f);

        const float spacingY = 4.0f;
        const float defaultHeight = 16.0f;

        Vec2 barSize = size;
        if (barSize.x < 0) {
            barSize.x = 200.0f;  // Default width if negative
        }
        if (barSize.y <= 0) {
            barSize.y = defaultHeight;
        }

        Vec2 pos = win->cursorPos;
        Rect bb(pos.x, pos.y, barSize.x, barSize.y);

        win->cursorPos.x = win->cursorStartPos.x;
        win->cursorPos.y += barSize.y + spacingY;

        DrawList* dl = getWindowDrawList();
        if (dl) {
            Color bgColor = Color::fromFloats(0.20f, 0.20f, 0.20f, 1.0f);
            dl->drawRectFilledRounded(bb, bgColor, 2.0f);

            if (fraction > 0) {
                Rect fillRect(pos.x, pos.y, barSize.x * fraction, barSize.y);
                Color fillColor = Color::fromFloats(0.30f, 0.50f, 0.80f, 1.0f);
                dl->drawRectFilledRounded(fillRect, fillColor, 2.0f);
            }

            dl->drawRectRounded(bb, Color::fromFloats(0.30f, 0.30f, 0.30f, 1.0f), 2.0f);

            if (overlay) {
                float textWidth = static_cast<float>(strlen(overlay)) * 8.0f;
                Vec2 textPos(pos.x + (barSize.x - textWidth) * 0.5f, pos.y + (barSize.y - 14) * 0.5f);
                dl->drawText(textPos, overlay, Color::fromFloats(1.0f, 1.0f, 1.0f, 1.0f));
            }
        }
    }
} // namespace dakt::gui
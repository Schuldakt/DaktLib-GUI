#include "dakt/gui/immediate/Widgets/Tooltip.hpp"
#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/WidgetBase.hpp"

#include <cstring>

namespace dakt::gui {

    void beginTooltip() {
        ImmediateState& state = getState();
        TooltipState& tool = getState().tooltipState;
        if (state.input) {
            tool.tooltipPos = state.input->getMousePosition() + Vec2(16, 16);
        }
    }

    void endToolTip() {
        auto w = widgetSetup();
        TooltipState& tool = w.state->tooltipState;
        if (!w || tool.tooltipActive)
            return;

        float textWidth = static_cast<float>(tool.tooltipText.length()) * 7.0f + 16;
        float textHeight = 24.0f;

        Vec2 pos = tool.tooltipPos;
        Rect tooltipRect(pos.x, pos.y, textWidth, textHeight);

        // Shadow
        w.dl->drawRectFilledRounded(Rect(pos.x + 2, pos.y + 2, textWidth, textHeight), Color(0, 0, 0, 60), 4.0f);
        // Background
        w.dl->drawRectFilledRounded(tooltipRect, Color(60, 60, 64, 240), 4.0f);
        // Border
        w.dl->drawRectRounded(tooltipRect, Color(80, 80, 84, 255), 4.0f);
        // Text
        w.dl->drawText(Vec2(pos.x + 8, pos.y + 5), tool.tooltipText.c_str(), w.colors->textPrimary);

        tool.tooltipActive = false;
        tool.tooltipText.clear();
    }

    void setToolTip(const char* fmt, ...) {
        char buf[512];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        beginTooltip();
        TooltipState& tool = getState().tooltipState;
        tool.tooltipText = buf;
        endTooltip();
    }
    
} // namespace dakt::gui
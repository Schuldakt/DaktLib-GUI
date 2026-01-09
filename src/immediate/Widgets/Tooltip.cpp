#include "dakt/gui/immediate/Widgets/Tooltip.hpp"

#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"

#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"

#include <cstring>
#include <cstdarg>

namespace dakt::gui {

    void beginTooltip() {
        ImmediateState& s = getState();
        TooltipState& tool = s.tooltipState;
        
        Vec2 mousePos = getMousePos();
        tool.tooltipPos = Vec2(mousePos.x + 16, mousePos.y + 16);
    }

    void endToolTip() {
        Context* ctx = getCurrentContext();
        if (!ctx) return;

        ImmediateState& s = getState();
        TooltipState& tool = s.tooltipState;
        
        if (tool.tooltipActive || tool.tooltipText.empty()) {
            return;
        }

        DrawList* dl = getWindowDrawList();
        if (!dl) return;

        float textWidth = static_cast<float>(tool.tooltipText.length()) * 7.0f + 16;
        float textHeight = 24.0f;

        Vec2 pos = tool.tooltipPos;
        Rect tooltipRect(pos.x, pos.y, textWidth, textHeight);

        // Shadow
        dl->drawRectFilledRounded(Rect(pos.x + 2, pos.y + 2, textWidth, textHeight), Color(0, 0, 0, 60), 4.0f);
        // Background
        dl->drawRectFilledRounded(tooltipRect, Color(60, 60, 64, 240), 4.0f);
        // Border
        dl->drawRectRounded(tooltipRect, Color(80, 80, 84, 255), 4.0f);
        // Text
        dl->drawText(Vec2(pos.x + 8, pos.y + 5), tool.tooltipText.c_str(), Color::fromFloats(1.0f, 1.0f, 1.0f, 1.0f));

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
        
        ImmediateState& s = getState();
        TooltipState& tool = s.tooltipState;
        tool.tooltipText = buf;
        
        endToolTip();
    }
    
} // namespace dakt::gui
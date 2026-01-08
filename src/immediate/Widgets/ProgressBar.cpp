#include "dakt/gui/immediate/Widgets/ProgressBar.hpp"
#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/WidgetBase.hpp"

#include <algorithm>
#include <cstring>

namespace dakt::gui {

    void progressBar(float fraction, Vec2 size, const char* overlay) {
        auto w = widgetSetup();
        if (!w) return;

        Vec2 pos = getCursorPos();
        Vec2 windowPos = getWindowPos();
        Vec2 barPos = windowPos + pos;

        Vec2 barSize = size;
        if (barSize.x < 0)
            barSize.x = getContentRegionAvail().x;
        if (barSize.y <= 0)
            barSize.y = 16.0f;

        fraction = std::clamp(fraction, 0.0f, 1.0f);

        Rect bgRect(barPos.x, barPos.y, barSize.x, barSize.y);

        w.dl->drawRectFilledRounded(bgRect, w.colors->surface, 2.0f);

        if (fraction > 0) {
            Rect fillRect(barPos.x, barPos.y, barSize.x * fraction, barSize.y);
            w.dl->drawRectFilledRounded(fillRect, w.colors->primary, 2.0f);
        }

        w.dl->drawRectRounded(bgRect, w.colors->border, 2.0f);

        if (overlay) {
            float textWidth = static_cast<float>(strlen(overlay)) * 8.0f;
            Vec2 textPos = barPos + Vec2((barSize.x - textWidth) * 0.5f, (barSize.y - 14) * 0.5f);
            w.dl->drawText(textPos, overlay, w.colors->textPrimary);
        }

        setCursorPos(Vec2(pos.x, pos.y + barSize.y + 4.0f));
    }
} // namespace dakt::gui
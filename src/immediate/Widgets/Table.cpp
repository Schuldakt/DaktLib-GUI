#include "dakt/gui/immediate/Widgets/Table.hpp"
#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/WidgetBase.hpp"

#include <cstring>

namespace dakt::gui {
    
    bool beginTable(const char* strId, int columns, TableFlags flags, Vec2 outerSize) {
        auto w = widgetSetup();
        TableState& tbl = w.state->tableState;
        if (!w || tbl.tableActive)
            return false;

        tbl.tableActive = true;
        tbl.tableId = strId;
        tbl.columns = columns;
        tbl.flags = flags;
        tbl.currentColumn = -1;
        tbl.currentRow = -1;

        tbl.tablePos = getWindowPos() + getCursorPos();
        tbl.tableSize = outerSize;
        if (outerSize.x <= 0)
            tbl.tableSize.y = 200.0f;
        if (outerSize.y <= 0)
            tbl.tableSize.y = 200.0f;

        // Initialize column widths
        tbl.columnWidths.clear();
        tbl.columnLabels.clear();
        float defaultWidth = tbl.tableSize.x / columns;
        for (int i = 0; i < columns; i++) {
            tbl.columnWidths.push_back(defaultWidth);
            tbl.columnLabels.push_back("");
        }

        // Draw table background
        Rect tableRect(tbl.tablePos.x, tbl.tablePos.y, tbl.tableSize.x, tbl.tableSize.y);
        w.dl->drawRectFilled(tableRect, w.colors->surface);

        if (static_cast<uint32_t>(flags) & static_cast<uint32_t>(TableFlags::Borders)) {
            w.dl->drawRect(tableRect, w.colors->border);
        }

        return true;
    }

    void endTable() {
        TableState& tbl = getState().tableState;
        if (!tbl.tableActive)
            return;

        // Move cursor past the table
        Vec2 pos = getCursorPos();
        float tableHeight = tbl.headerHeight + (tbl.currentRow + 1) * tbl.rowHeight;
        setCursorPos(Vec2(pos.x, pos.y + tableHeight + 4));

        tbl.tableActive = false;
        tbl.columnWidths.clear();
        tbl.columnLabels.clear();
    }

    void tableNextRow() {
        TableState& tbl = getState().tableState;
        tbl.currentRow++;
        tbl.currentColumn = -1;
    }

    bool tableNextColumn() {
        TableState& tbl = getState().tableState;
        if (!tbl.tableActive)
            return false;

        tbl.currentColumn++;
        return tbl.currentColumn < tbl.columns;
    }

    bool tableSetColumnIndex(int columnIndex) {
        TableState& tbl = getState().tableState;
        if (!tbl.tableActive || columnIndex < 0 || columnIndex >= tbl.columns) {
            return false;
        }
        tbl.currentColumn = columnIndex;
        return true;
    }

    void tableSetupColumn(const char* label, float initWidth) {
        TableState& tbl = getState().tableState;
        if (!tbl.tableActive)
            return;

        static int setupColumn = 0;
        if (setupColumn < tbl.columns) {
            tbl.columnLabels[setupColumn] = label;
            if (initWidth > 0) {
                tbl.columnWidths[setupColumn] = initWidth;
            }
            setupColumn++;
        } else {
            setupColumn = 0;
        }
    }

    void tableHeadersRow() {
        auto w = widgetSetup();
        TableState& tbl = w.state->tableState;
        if (!w || tbl.tableActive)
            return;

        float x = tbl.tablePos.x;
        float y = tbl.tablePos.y;

        // Draw header background
        Rect headerRect(x, y, tbl.tableSize.x, tbl.headerHeight);
        w.dl->drawRectFilled(headerRect, Color(55, 55, 60, 255));

        // Draw column headers
        for (int i = 0; i < tbl.columns; i++) {
            float colWidth = tbl.columnWidths[i];

            if (tbl.columnLabels[i] && strlen(tbl.columnLabels[i]) > 0) {
                w.dl->drawText(Vec2(x + 8, y + 6), tbl.columnLabels[i], w.colors->textPrimary);
            }

            // Column separator
            if (static_cast<uint32_t>(tbl.flags) & static_cast<uint32_t>(TableFlags::Borders)) {
                w.dl->drawLine(Vec2(x + colWidth, y), Vec2(x + colWidth, y + tbl.headerHeight), w.colors->border);
            }

            x += colWidth;
        }

        // Header bottom border
        w.dl->drawLine(Vec2(tbl.tablePos.x, y + tbl.headerHeight), Vec2(tbl.tablePos.x + tbl.tableSize.x, y + tbl.headerHeight), w.colors->border);

        tbl.currentRow = -1;
    }

} // namespace dakt::gui
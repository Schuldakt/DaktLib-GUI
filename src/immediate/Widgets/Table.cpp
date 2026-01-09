#include "dakt/gui/immediate/Widgets/Table.hpp"

#include "dakt/gui/core/Context.hpp"

#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"

#include <cstring>

namespace dakt::gui {

    bool beginTable(const char* strId, int columns, TableFlags flags, Vec2 outerSize) {
        Context* ctx = getCurrentContext();
        if (!ctx || !strId) return false;

        ImmediateState& s = getState();
        TableState& tbl = s.tableState;
        
        if (tbl.tableActive)
            return false;

        tbl.tableActive = true;
        tbl.tableId = strId;
        tbl.columns = columns;
        tbl.flags = flags;
        tbl.currentColumn = -1;
        tbl.currentRow = -1;

        tbl.tablePos = Vec2(0, 0);
        tbl.tableSize = outerSize;
        if (outerSize.x <= 0)
            tbl.tableSize.x = 200.0f;
        if (outerSize.y <= 0)
            tbl.tableSize.y = 200.0f;

        tbl.columnWidths.clear();
        tbl.columnLabels.clear();
        float defaultWidth = tbl.tableSize.x / columns;
        for (int i = 0; i < columns; i++) {
            tbl.columnWidths.push_back(defaultWidth);
            tbl.columnLabels.push_back("");
        }

        return true;
    }

    void endTable() {
        ImmediateState& s = getState();
        TableState& tbl = s.tableState;
        if (!tbl.tableActive)
            return;

        tbl.tableActive = false;
        tbl.columnWidths.clear();
        tbl.columnLabels.clear();
    }

    void tableNextRow() {
        ImmediateState& s = getState();
        TableState& tbl = s.tableState;
        tbl.currentRow++;
        tbl.currentColumn = -1;
    }

    bool tableNextColumn() {
        ImmediateState& s = getState();
        TableState& tbl = s.tableState;
        if (!tbl.tableActive)
            return false;

        tbl.currentColumn++;
        return tbl.currentColumn < tbl.columns;
    }

    bool tableSetColumnIndex(int columnIndex) {
        ImmediateState& s = getState();
        TableState& tbl = s.tableState;
        if (!tbl.tableActive || columnIndex < 0 || columnIndex >= tbl.columns) {
            return false;
        }
        tbl.currentColumn = columnIndex;
        return true;
    }

    void tableSetupColumn(const char* label, float initWidth) {
        ImmediateState& s = getState();
        TableState& tbl = s.tableState;
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
        ImmediateState& s = getState();
        TableState& tbl = s.tableState;
        if (!tbl.tableActive)
            return;

        tbl.currentRow = -1;
    }

} // namespace dakt::gui
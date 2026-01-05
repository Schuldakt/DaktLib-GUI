#ifndef DAKT_GUI_TABLE_HPP
#define DAKT_GUI_TABLE_HPP

#include "WidgetBase.hpp"
#include <functional>
#include <variant>
#include <vector>

namespace dakt::gui {

/**
 * @brief Table column definition
 */
struct TableColumn {
    std::string header;
    float width = 100.0f;
    float minWidth = 50.0f;
    float maxWidth = 500.0f;
    bool resizable = true;
    bool sortable = true;
    bool visible = true;

    enum class Alignment { Left, Center, Right };
    Alignment headerAlign = Alignment::Left;
    Alignment contentAlign = Alignment::Left;
};

/**
 * @brief Table cell value (can hold different types)
 */
using TableCellValue = std::variant<std::string, int, float, double, bool>;

/**
 * @brief Table row data
 */
struct TableRow {
    std::vector<TableCellValue> cells;
    bool selected = false;
    bool expanded = false; // For tree tables
    int indentLevel = 0;   // For tree tables
    void* userData = nullptr;
};

/**
 * @brief Sort direction
 */
enum class SortDirection { None, Ascending, Descending };

/**
 * @brief Table selection mode
 */
enum class TableSelectionMode { None, Single, Multiple };

/**
 * @brief Table widget for displaying tabular data
 *
 * Features:
 * - Column headers with sorting
 * - Column resizing
 * - Row selection (single/multiple)
 * - Virtual scrolling for large datasets
 * - Custom cell rendering
 * - Tree table support (expandable rows)
 */
class DAKT_GUI_API Table : public Widget {
  public:
    using CellRenderer = std::function<void(DrawList&, const Rect&, int row, int col, const TableCellValue&)>;
    using SortCallback = std::function<void(int column, SortDirection direction)>;
    using SelectionCallback = std::function<void(const std::vector<int>& selectedRows)>;

    Table();

    // Column management
    void addColumn(const TableColumn& column);
    void addColumn(const std::string& header, float width = 100.0f);
    void removeColumn(int index);
    void clearColumns();

    TableColumn& getColumn(int index) { return columns_[index]; }
    const TableColumn& getColumn(int index) const { return columns_[index]; }
    int getColumnCount() const { return static_cast<int>(columns_.size()); }

    // Row/data management
    void addRow(const TableRow& row);
    void addRow(const std::vector<TableCellValue>& cells);
    void insertRow(int index, const TableRow& row);
    void removeRow(int index);
    void clearRows();
    void setRows(const std::vector<TableRow>& rows);

    TableRow& getRow(int index) { return rows_[index]; }
    const TableRow& getRow(int index) const { return rows_[index]; }
    int getRowCount() const { return static_cast<int>(rows_.size()); }

    // Cell access
    void setCellValue(int row, int col, const TableCellValue& value);
    const TableCellValue& getCellValue(int row, int col) const;

    // Selection
    TableSelectionMode getSelectionMode() const { return selectionMode_; }
    void setSelectionMode(TableSelectionMode mode) { selectionMode_ = mode; }

    void selectRow(int index, bool addToSelection = false);
    void deselectRow(int index);
    void selectAll();
    void deselectAll();
    std::vector<int> getSelectedRows() const;
    bool isRowSelected(int index) const;

    // Sorting
    int getSortColumn() const { return sortColumn_; }
    SortDirection getSortDirection() const { return sortDirection_; }
    void sortByColumn(int column, SortDirection direction = SortDirection::Ascending);
    void clearSort();

    // Scrolling
    void scrollToRow(int index);
    void scrollToTop();
    void scrollToBottom();
    int getFirstVisibleRow() const;
    int getLastVisibleRow() const;

    // Styling
    float getRowHeight() const { return rowHeight_; }
    void setRowHeight(float height) {
        rowHeight_ = height;
        markDirty();
    }

    float getHeaderHeight() const { return headerHeight_; }
    void setHeaderHeight(float height) {
        headerHeight_ = height;
        markDirty();
    }

    bool isShowHeader() const { return showHeader_; }
    void setShowHeader(bool show) {
        showHeader_ = show;
        markDirty();
    }

    bool isShowRowNumbers() const { return showRowNumbers_; }
    void setShowRowNumbers(bool show) {
        showRowNumbers_ = show;
        markDirty();
    }

    bool isAlternatingRowColors() const { return alternatingRows_; }
    void setAlternatingRowColors(bool alternate) {
        alternatingRows_ = alternate;
        markDirty();
    }

    bool isShowGridLines() const { return showGridLines_; }
    void setShowGridLines(bool show) {
        showGridLines_ = show;
        markDirty();
    }

    // Colors
    Color getHeaderColor() const { return headerColor_; }
    void setHeaderColor(const Color& color) { headerColor_ = color; }

    Color getRowColor() const { return rowColor_; }
    void setRowColor(const Color& color) { rowColor_ = color; }

    Color getAlternateRowColor() const { return alternateRowColor_; }
    void setAlternateRowColor(const Color& color) { alternateRowColor_ = color; }

    Color getSelectedRowColor() const { return selectedRowColor_; }
    void setSelectedRowColor(const Color& color) { selectedRowColor_ = color; }

    Color getHoverRowColor() const { return hoverRowColor_; }
    void setHoverRowColor(const Color& color) { hoverRowColor_ = color; }

    Color getGridLineColor() const { return gridLineColor_; }
    void setGridLineColor(const Color& color) { gridLineColor_ = color; }

    // Custom rendering
    void setCellRenderer(CellRenderer renderer) { cellRenderer_ = std::move(renderer); }

    // Callbacks
    void setOnSort(SortCallback callback) { onSort_ = std::move(callback); }
    void setOnSelectionChanged(SelectionCallback callback) { onSelectionChanged_ = std::move(callback); }
    void setOnRowDoubleClick(std::function<void(int row)> callback) { onRowDoubleClick_ = std::move(callback); }
    void setOnCellClick(std::function<void(int row, int col)> callback) { onCellClick_ = std::move(callback); }

    // Widget overrides
    Vec2 measureContent() override;
    bool handleInput(const WidgetEvent& event) override;
    void drawContent(DrawList& drawList) override;

  private:
    // Hit testing
    int getRowAt(float y) const;
    int getColumnAt(float x) const;
    int getColumnResizeHandle(const Vec2& pos) const;

    // Drawing helpers
    void drawHeader(DrawList& drawList);
    void drawRows(DrawList& drawList);
    void drawCell(DrawList& drawList, const Rect& cellRect, int row, int col);
    void drawSortIndicator(DrawList& drawList, const Rect& headerRect, SortDirection dir);
    void drawResizeHandle(DrawList& drawList, float x);

    // Internal
    void updateColumnWidths();
    float getTotalWidth() const;
    void fireSelectionChanged();

    std::vector<TableColumn> columns_;
    std::vector<TableRow> rows_;

    // Selection
    TableSelectionMode selectionMode_ = TableSelectionMode::Single;
    int lastSelectedRow_ = -1;

    // Sorting
    int sortColumn_ = -1;
    SortDirection sortDirection_ = SortDirection::None;

    // Scrolling
    float scrollY_ = 0.0f;
    float scrollX_ = 0.0f;

    // Interaction state
    int hoveredRow_ = -1;
    int hoveredColumn_ = -1;
    int resizingColumn_ = -1;
    float resizeStartX_ = 0.0f;
    float resizeStartWidth_ = 0.0f;
    bool draggingScrollbar_ = false;

    // Dimensions
    float rowHeight_ = 24.0f;
    float headerHeight_ = 28.0f;
    float rowNumberWidth_ = 40.0f;
    float scrollbarWidth_ = 12.0f;
    float resizeHandleWidth_ = 6.0f;

    // Options
    bool showHeader_ = true;
    bool showRowNumbers_ = false;
    bool alternatingRows_ = true;
    bool showGridLines_ = true;
    bool showHorizontalScrollbar_ = true;
    bool showVerticalScrollbar_ = true;

    // Colors
    Color headerColor_{55, 55, 60, 255};
    Color rowColor_{40, 40, 44, 255};
    Color alternateRowColor_{45, 45, 50, 255};
    Color selectedRowColor_{0, 90, 158, 255};
    Color hoverRowColor_{55, 55, 60, 255};
    Color gridLineColor_{60, 60, 65, 255};
    Color textColor_{220, 220, 220, 255};
    Color headerTextColor_{200, 200, 200, 255};

    // Callbacks
    CellRenderer cellRenderer_;
    SortCallback onSort_;
    SelectionCallback onSelectionChanged_;
    std::function<void(int row)> onRowDoubleClick_;
    std::function<void(int row, int col)> onCellClick_;
};

} // namespace dakt::gui

#endif // DAKT_GUI_TABLE_HPP

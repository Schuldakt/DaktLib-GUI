#include "dakt/gui/retained/widgets/Table.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"
#include <algorithm>
#include <sstream>

namespace dakt::gui {

Table::Table() : Widget() { setPreferredSize(Vec2(400, 300)); }

// ============================================================================
// Column Management
// ============================================================================

void Table::addColumn(const TableColumn& column) {
    columns_.push_back(column);
    markDirty();
}

void Table::addColumn(const std::string& header, float width) {
    TableColumn col;
    col.header = header;
    col.width = width;
    columns_.push_back(col);
    markDirty();
}

void Table::removeColumn(int index) {
    if (index >= 0 && index < static_cast<int>(columns_.size())) {
        columns_.erase(columns_.begin() + index);
        // Also remove corresponding cells from all rows
        for (auto& row : rows_) {
            if (index < static_cast<int>(row.cells.size())) {
                row.cells.erase(row.cells.begin() + index);
            }
        }
        markDirty();
    }
}

void Table::clearColumns() {
    columns_.clear();
    markDirty();
}

// ============================================================================
// Row/Data Management
// ============================================================================

void Table::addRow(const TableRow& row) {
    rows_.push_back(row);
    markDirty();
}

void Table::addRow(const std::vector<TableCellValue>& cells) {
    TableRow row;
    row.cells = cells;
    rows_.push_back(row);
    markDirty();
}

void Table::insertRow(int index, const TableRow& row) {
    if (index >= 0 && index <= static_cast<int>(rows_.size())) {
        rows_.insert(rows_.begin() + index, row);
        markDirty();
    }
}

void Table::removeRow(int index) {
    if (index >= 0 && index < static_cast<int>(rows_.size())) {
        rows_.erase(rows_.begin() + index);
        markDirty();
    }
}

void Table::clearRows() {
    rows_.clear();
    scrollY_ = 0;
    markDirty();
}

void Table::setRows(const std::vector<TableRow>& rows) {
    rows_ = rows;
    scrollY_ = 0;
    markDirty();
}

void Table::setCellValue(int row, int col, const TableCellValue& value) {
    if (row >= 0 && row < static_cast<int>(rows_.size()) && col >= 0 && col < static_cast<int>(rows_[row].cells.size())) {
        rows_[row].cells[col] = value;
        markDirty();
    }
}

const TableCellValue& Table::getCellValue(int row, int col) const {
    static TableCellValue empty{""};
    if (row >= 0 && row < static_cast<int>(rows_.size()) && col >= 0 && col < static_cast<int>(rows_[row].cells.size())) {
        return rows_[row].cells[col];
    }
    return empty;
}

// ============================================================================
// Selection
// ============================================================================

void Table::selectRow(int index, bool addToSelection) {
    if (index < 0 || index >= static_cast<int>(rows_.size()))
        return;

    if (selectionMode_ == TableSelectionMode::None)
        return;

    if (selectionMode_ == TableSelectionMode::Single || !addToSelection) {
        deselectAll();
    }

    rows_[index].selected = true;
    lastSelectedRow_ = index;
    markDirty();
    fireSelectionChanged();
}

void Table::deselectRow(int index) {
    if (index >= 0 && index < static_cast<int>(rows_.size())) {
        rows_[index].selected = false;
        markDirty();
        fireSelectionChanged();
    }
}

void Table::selectAll() {
    if (selectionMode_ != TableSelectionMode::Multiple)
        return;

    for (auto& row : rows_) {
        row.selected = true;
    }
    markDirty();
    fireSelectionChanged();
}

void Table::deselectAll() {
    for (auto& row : rows_) {
        row.selected = false;
    }
    markDirty();
    // Don't fire callback here - it's called by the higher-level functions
}

std::vector<int> Table::getSelectedRows() const {
    std::vector<int> selected;
    for (size_t i = 0; i < rows_.size(); ++i) {
        if (rows_[i].selected) {
            selected.push_back(static_cast<int>(i));
        }
    }
    return selected;
}

bool Table::isRowSelected(int index) const {
    if (index >= 0 && index < static_cast<int>(rows_.size())) {
        return rows_[index].selected;
    }
    return false;
}

void Table::fireSelectionChanged() {
    if (onSelectionChanged_) {
        onSelectionChanged_(getSelectedRows());
    }
}

// ============================================================================
// Sorting
// ============================================================================

void Table::sortByColumn(int column, SortDirection direction) {
    if (column < 0 || column >= static_cast<int>(columns_.size()))
        return;
    if (!columns_[column].sortable)
        return;

    sortColumn_ = column;
    sortDirection_ = direction;

    if (direction == SortDirection::None) {
        // No sorting
        return;
    }

    std::sort(rows_.begin(), rows_.end(), [column, direction](const TableRow& a, const TableRow& b) {
        if (column >= static_cast<int>(a.cells.size()) || column >= static_cast<int>(b.cells.size())) {
            return false;
        }

        const auto& cellA = a.cells[column];
        const auto& cellB = b.cells[column];

        bool result = false;

        // Compare based on type
        if (std::holds_alternative<std::string>(cellA) && std::holds_alternative<std::string>(cellB)) {
            result = std::get<std::string>(cellA) < std::get<std::string>(cellB);
        } else if (std::holds_alternative<int>(cellA) && std::holds_alternative<int>(cellB)) {
            result = std::get<int>(cellA) < std::get<int>(cellB);
        } else if (std::holds_alternative<float>(cellA) && std::holds_alternative<float>(cellB)) {
            result = std::get<float>(cellA) < std::get<float>(cellB);
        } else if (std::holds_alternative<double>(cellA) && std::holds_alternative<double>(cellB)) {
            result = std::get<double>(cellA) < std::get<double>(cellB);
        } else if (std::holds_alternative<bool>(cellA) && std::holds_alternative<bool>(cellB)) {
            result = !std::get<bool>(cellA) && std::get<bool>(cellB);
        }

        return direction == SortDirection::Ascending ? result : !result;
    });

    if (onSort_) {
        onSort_(column, direction);
    }

    markDirty();
}

void Table::clearSort() {
    sortColumn_ = -1;
    sortDirection_ = SortDirection::None;
    markDirty();
}

// ============================================================================
// Scrolling
// ============================================================================

void Table::scrollToRow(int index) {
    if (index < 0 || index >= static_cast<int>(rows_.size()))
        return;

    float targetY = index * rowHeight_;
    float viewportHeight = bounds_.height - (showHeader_ ? headerHeight_ : 0);

    if (targetY < scrollY_) {
        scrollY_ = targetY;
    } else if (targetY + rowHeight_ > scrollY_ + viewportHeight) {
        scrollY_ = targetY + rowHeight_ - viewportHeight;
    }

    markDirty();
}

void Table::scrollToTop() {
    scrollY_ = 0;
    markDirty();
}

void Table::scrollToBottom() {
    float contentHeight = rows_.size() * rowHeight_;
    float viewportHeight = bounds_.height - (showHeader_ ? headerHeight_ : 0);
    scrollY_ = std::max(0.0f, contentHeight - viewportHeight);
    markDirty();
}

int Table::getFirstVisibleRow() const { return static_cast<int>(scrollY_ / rowHeight_); }

int Table::getLastVisibleRow() const {
    float viewportHeight = bounds_.height - (showHeader_ ? headerHeight_ : 0);
    return std::min(static_cast<int>((scrollY_ + viewportHeight) / rowHeight_), static_cast<int>(rows_.size()) - 1);
}

// ============================================================================
// Hit Testing
// ============================================================================

int Table::getRowAt(float y) const {
    float headerOffset = showHeader_ ? headerHeight_ : 0;
    float relativeY = y - bounds_.y - headerOffset + scrollY_;

    if (relativeY < 0)
        return -1;

    int row = static_cast<int>(relativeY / rowHeight_);
    return (row >= 0 && row < static_cast<int>(rows_.size())) ? row : -1;
}

int Table::getColumnAt(float x) const {
    float relativeX = x - bounds_.x + scrollX_;
    if (showRowNumbers_)
        relativeX -= rowNumberWidth_;

    float colX = 0;
    for (size_t i = 0; i < columns_.size(); ++i) {
        if (columns_[i].visible) {
            if (relativeX >= colX && relativeX < colX + columns_[i].width) {
                return static_cast<int>(i);
            }
            colX += columns_[i].width;
        }
    }
    return -1;
}

int Table::getColumnResizeHandle(const Vec2& pos) const {
    if (!showHeader_)
        return -1;
    if (pos.y < bounds_.y || pos.y > bounds_.y + headerHeight_)
        return -1;

    float x = bounds_.x;
    if (showRowNumbers_)
        x += rowNumberWidth_;

    for (size_t i = 0; i < columns_.size(); ++i) {
        if (columns_[i].visible && columns_[i].resizable) {
            x += columns_[i].width;
            if (std::abs(pos.x - x) <= resizeHandleWidth_ / 2) {
                return static_cast<int>(i);
            }
        }
    }
    return -1;
}

float Table::getTotalWidth() const {
    float total = showRowNumbers_ ? rowNumberWidth_ : 0;
    for (const auto& col : columns_) {
        if (col.visible)
            total += col.width;
    }
    return total;
}

// ============================================================================
// Widget Overrides
// ============================================================================

Vec2 Table::measureContent() {
    float width = getTotalWidth() + scrollbarWidth_;
    float height = (showHeader_ ? headerHeight_ : 0) + rows_.size() * rowHeight_;
    return Vec2(width, height);
}

bool Table::handleInput(const WidgetEvent& event) {
    if (isDisabled())
        return false;

    switch (event.type) {
    case WidgetEventType::Hover: {
        hoveredRow_ = getRowAt(event.mousePos.y);
        hoveredColumn_ = getColumnAt(event.mousePos.x);
        markDirty();
        return true;
    }

    case WidgetEventType::HoverEnd:
        hoveredRow_ = -1;
        hoveredColumn_ = -1;
        markDirty();
        return true;

    case WidgetEventType::Press: {
        // Check for column resize
        int resizeCol = getColumnResizeHandle(event.mousePos);
        if (resizeCol >= 0) {
            resizingColumn_ = resizeCol;
            resizeStartX_ = event.mousePos.x;
            resizeStartWidth_ = columns_[resizeCol].width;
            addFlag(RetainedWidgetFlags::Active);
            return true;
        }

        // Check for header click (sorting)
        if (showHeader_ && event.mousePos.y >= bounds_.y && event.mousePos.y < bounds_.y + headerHeight_) {
            int col = getColumnAt(event.mousePos.x);
            if (col >= 0 && columns_[col].sortable) {
                SortDirection newDir = SortDirection::Ascending;
                if (sortColumn_ == col) {
                    newDir = (sortDirection_ == SortDirection::Ascending) ? SortDirection::Descending : SortDirection::Ascending;
                }
                sortByColumn(col, newDir);
                return true;
            }
        }

        // Row selection
        int row = getRowAt(event.mousePos.y);
        if (row >= 0) {
            // TODO: Check for Ctrl/Shift modifiers for multi-select
            selectRow(row, false);

            if (onCellClick_) {
                int col = getColumnAt(event.mousePos.x);
                if (col >= 0) {
                    onCellClick_(row, col);
                }
            }
            return true;
        }
        break;
    }

    case WidgetEventType::DragMove:
        if (resizingColumn_ >= 0) {
            float delta = event.mousePos.x - resizeStartX_;
            float newWidth = resizeStartWidth_ + delta;
            newWidth = std::max(columns_[resizingColumn_].minWidth, std::min(columns_[resizingColumn_].maxWidth, newWidth));
            columns_[resizingColumn_].width = newWidth;
            markDirty();
            return true;
        }
        if (draggingScrollbar_) {
            // Handle scrollbar drag
            float contentHeight = rows_.size() * rowHeight_;
            float viewportHeight = bounds_.height - (showHeader_ ? headerHeight_ : 0);
            float scrollableHeight = contentHeight - viewportHeight;

            if (scrollableHeight > 0) {
                float scrollbarTrackHeight = viewportHeight - 20; // Account for thumb min size
                float ratio = event.mouseDelta.y / scrollbarTrackHeight;
                scrollY_ += ratio * scrollableHeight;
                scrollY_ = std::max(0.0f, std::min(scrollableHeight, scrollY_));
                markDirty();
            }
            return true;
        }
        break;

    case WidgetEventType::Release:
    case WidgetEventType::DragEnd:
        if (resizingColumn_ >= 0) {
            resizingColumn_ = -1;
            removeFlag(RetainedWidgetFlags::Active);
            return true;
        }
        if (draggingScrollbar_) {
            draggingScrollbar_ = false;
            removeFlag(RetainedWidgetFlags::Active);
            return true;
        }
        break;

    case WidgetEventType::DoubleClick: {
        int row = getRowAt(event.mousePos.y);
        if (row >= 0 && onRowDoubleClick_) {
            onRowDoubleClick_(row);
            return true;
        }
        break;
    }

    case WidgetEventType::Scroll: {
        float contentHeight = rows_.size() * rowHeight_;
        float viewportHeight = bounds_.height - (showHeader_ ? headerHeight_ : 0);
        float maxScroll = std::max(0.0f, contentHeight - viewportHeight);

        scrollY_ -= event.scrollDelta.y * 30.0f;
        scrollY_ = std::max(0.0f, std::min(maxScroll, scrollY_));
        markDirty();
        return true;
    }

    default:
        break;
    }

    return Widget::handleInput(event);
}

void Table::drawContent(DrawList& drawList) {
    // Draw background
    drawList.drawRectFilled(bounds_, rowColor_);

    // Set clip rect
    drawList.pushClipRect(bounds_);

    // Draw header
    if (showHeader_) {
        drawHeader(drawList);
    }

    // Draw rows
    drawRows(drawList);

    // Draw scrollbar
    float contentHeight = rows_.size() * rowHeight_;
    float viewportHeight = bounds_.height - (showHeader_ ? headerHeight_ : 0);

    if (contentHeight > viewportHeight && showVerticalScrollbar_) {
        float scrollbarX = bounds_.x + bounds_.width - scrollbarWidth_;
        float scrollbarY = bounds_.y + (showHeader_ ? headerHeight_ : 0);

        // Track
        Rect trackRect(scrollbarX, scrollbarY, scrollbarWidth_, viewportHeight);
        drawList.drawRectFilled(trackRect, Color{30, 30, 34, 255});

        // Thumb
        float thumbRatio = viewportHeight / contentHeight;
        float thumbHeight = std::max(20.0f, viewportHeight * thumbRatio);
        float thumbY = scrollbarY + (viewportHeight - thumbHeight) * (scrollY_ / (contentHeight - viewportHeight));

        Rect thumbRect(scrollbarX + 2, thumbY, scrollbarWidth_ - 4, thumbHeight);
        Color thumbColor = draggingScrollbar_ ? Color{120, 120, 124, 255} : Color{80, 80, 84, 255};
        drawList.drawRectFilledRounded(thumbRect, thumbColor, 4.0f);
    }

    drawList.popClipRect();
}

void Table::drawHeader(DrawList& drawList) {
    Rect headerRect(bounds_.x, bounds_.y, bounds_.width - scrollbarWidth_, headerHeight_);
    drawList.drawRectFilled(headerRect, headerColor_);

    float x = bounds_.x;
    if (showRowNumbers_) {
        Rect numHeader(x, bounds_.y, rowNumberWidth_, headerHeight_);
        drawList.drawRectFilled(numHeader, headerColor_);
        drawList.drawText(Vec2(x + 4, bounds_.y + (headerHeight_ - 14) / 2), "#", headerTextColor_);
        x += rowNumberWidth_;
    }

    for (size_t i = 0; i < columns_.size(); ++i) {
        if (!columns_[i].visible)
            continue;

        Rect colHeader(x, bounds_.y, columns_[i].width, headerHeight_);

        // Hover highlight
        if (hoveredColumn_ == static_cast<int>(i) && hoveredRow_ < 0) { // Only when hovering header
            drawList.drawRectFilled(colHeader, hoverRowColor_);
        }

        // Header text
        float textX = x + 8;
        if (columns_[i].headerAlign == TableColumn::Alignment::Center) {
            float textWidth = static_cast<float>(columns_[i].header.length()) * 8;
            textX = x + (columns_[i].width - textWidth) / 2;
        } else if (columns_[i].headerAlign == TableColumn::Alignment::Right) {
            float textWidth = static_cast<float>(columns_[i].header.length()) * 8;
            textX = x + columns_[i].width - textWidth - 8;
        }

        drawList.drawText(Vec2(textX, bounds_.y + (headerHeight_ - 14) / 2), columns_[i].header.c_str(), headerTextColor_);

        // Sort indicator
        if (static_cast<int>(i) == sortColumn_ && sortDirection_ != SortDirection::None) {
            drawSortIndicator(drawList, colHeader, sortDirection_);
        }

        // Column separator
        if (showGridLines_) {
            drawList.drawLine(Vec2(x + columns_[i].width - 1, bounds_.y), Vec2(x + columns_[i].width - 1, bounds_.y + headerHeight_), gridLineColor_);
        }

        // Resize handle highlight
        if (columns_[i].resizable && resizingColumn_ == static_cast<int>(i)) {
            drawResizeHandle(drawList, x + columns_[i].width);
        }

        x += columns_[i].width;
    }

    // Header bottom border
    drawList.drawLine(Vec2(bounds_.x, bounds_.y + headerHeight_ - 1), Vec2(bounds_.x + bounds_.width, bounds_.y + headerHeight_ - 1), gridLineColor_);
}

void Table::drawRows(DrawList& drawList) {
    float headerOffset = showHeader_ ? headerHeight_ : 0;
    float viewportHeight = bounds_.height - headerOffset;

    int firstRow = getFirstVisibleRow();
    int lastRow = getLastVisibleRow();

    for (int rowIndex = firstRow; rowIndex <= lastRow && rowIndex < static_cast<int>(rows_.size()); ++rowIndex) {
        const auto& row = rows_[rowIndex];
        float y = bounds_.y + headerOffset + rowIndex * rowHeight_ - scrollY_;

        // Row background
        Color bgColor;
        if (row.selected) {
            bgColor = selectedRowColor_;
        } else if (rowIndex == hoveredRow_) {
            bgColor = hoverRowColor_;
        } else if (alternatingRows_ && rowIndex % 2 == 1) {
            bgColor = alternateRowColor_;
        } else {
            bgColor = rowColor_;
        }

        Rect rowRect(bounds_.x, y, bounds_.width - scrollbarWidth_, rowHeight_);
        drawList.drawRectFilled(rowRect, bgColor);

        // Row number
        float x = bounds_.x;
        if (showRowNumbers_) {
            char numStr[16];
            snprintf(numStr, sizeof(numStr), "%d", rowIndex + 1);
            drawList.drawText(Vec2(x + 4, y + (rowHeight_ - 14) / 2), numStr, Color{128, 128, 128, 255});
            x += rowNumberWidth_;
        }

        // Cells
        for (size_t colIndex = 0; colIndex < columns_.size(); ++colIndex) {
            if (!columns_[colIndex].visible)
                continue;

            Rect cellRect(x, y, columns_[colIndex].width, rowHeight_);
            drawCell(drawList, cellRect, rowIndex, static_cast<int>(colIndex));

            // Grid line
            if (showGridLines_) {
                drawList.drawLine(Vec2(x + columns_[colIndex].width - 1, y), Vec2(x + columns_[colIndex].width - 1, y + rowHeight_), gridLineColor_);
            }

            x += columns_[colIndex].width;
        }

        // Row bottom border
        if (showGridLines_) {
            drawList.drawLine(Vec2(bounds_.x, y + rowHeight_ - 1), Vec2(bounds_.x + bounds_.width, y + rowHeight_ - 1), gridLineColor_);
        }
    }
}

void Table::drawCell(DrawList& drawList, const Rect& cellRect, int row, int col) {
    if (cellRenderer_) {
        cellRenderer_(drawList, cellRect, row, col, getCellValue(row, col));
        return;
    }

    // Default cell rendering
    const auto& value = getCellValue(row, col);
    std::string text;

    std::visit(
        [&text](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>) {
                text = arg;
            } else if constexpr (std::is_same_v<T, bool>) {
                text = arg ? "✓" : "";
            } else {
                std::ostringstream oss;
                oss << arg;
                text = oss.str();
            }
        },
        value);

    float textX = cellRect.x + 8;
    if (columns_[col].contentAlign == TableColumn::Alignment::Center) {
        float textWidth = static_cast<float>(text.length()) * 8;
        textX = cellRect.x + (cellRect.width - textWidth) / 2;
    } else if (columns_[col].contentAlign == TableColumn::Alignment::Right) {
        float textWidth = static_cast<float>(text.length()) * 8;
        textX = cellRect.x + cellRect.width - textWidth - 8;
    }

    drawList.drawText(Vec2(textX, cellRect.y + (rowHeight_ - 14) / 2), text.c_str(), textColor_);
}

void Table::drawSortIndicator(DrawList& drawList, const Rect& headerRect, SortDirection dir) {
    float x = headerRect.x + headerRect.width - 16;
    float y = headerRect.y + headerRect.height / 2;

    if (dir == SortDirection::Ascending) {
        drawList.drawTriangleFilled(Vec2(x, y + 3), Vec2(x + 8, y + 3), Vec2(x + 4, y - 3), headerTextColor_);
    } else {
        drawList.drawTriangleFilled(Vec2(x, y - 3), Vec2(x + 8, y - 3), Vec2(x + 4, y + 3), headerTextColor_);
    }
}

void Table::drawResizeHandle(DrawList& drawList, float x) {
    float y1 = bounds_.y;
    float y2 = bounds_.y + headerHeight_;
    drawList.drawLine(Vec2(x, y1), Vec2(x, y2), Color{0, 122, 204, 255});
}

} // namespace dakt::gui

#include "dakt/gui/retained/containers/Grid.hpp"

namespace dakt::gui {

Grid::Grid() : UIContainer() { setLayoutDirection(LayoutDirection::None); }

Grid::Grid(const std::string& id) : UIContainer(id) { setLayoutDirection(LayoutDirection::None); }

Grid::Grid(int columns, int rows) : UIContainer(), columns_(columns), rows_(rows) { setLayoutDirection(LayoutDirection::None); }

Grid::Grid(const std::string& id, int columns, int rows) : UIContainer(id), columns_(columns), rows_(rows) { setLayoutDirection(LayoutDirection::None); }

void Grid::setCellSpan(Widget* widget, const CellSpan& span) {
    cellSpans_[widget] = span;
    markDirty();
}

Grid::CellSpan Grid::getCellSpan(Widget* widget) const {
    auto it = cellSpans_.find(widget);
    if (it != cellSpans_.end()) {
        return it->second;
    }
    return CellSpan{};
}

void Grid::layout() { layoutGrid(); }

void Grid::layoutGrid() {
    auto* root = getRoot();
    if (!root)
        return;

    const auto& children = root->getChildren();
    if (children.empty())
        return;

    const auto& padding = getPadding();
    float availableWidth = bounds_.width - padding.left - padding.right;
    float availableHeight = bounds_.height - padding.top - padding.bottom;

    int cols = columns_;
    int rows = rows_;

    // Auto-calculate rows if not specified
    if (autoRows_ && rows == 0) {
        rows = (static_cast<int>(children.size()) + cols - 1) / cols;
    }

    // Calculate cell dimensions
    float totalColGap = (cols > 1) ? (cols - 1) * columnGap_ : 0;
    float totalRowGap = (rows > 1) ? (rows - 1) * rowGap_ : 0;

    float cellWidth = (availableWidth - totalColGap) / cols;
    float cellHeight = (availableHeight - totalRowGap) / rows;

    float startX = bounds_.x + padding.left;
    float startY = bounds_.y + padding.top;

    // Layout children
    int index = 0;
    for (const auto& child : children) {
        if (!child->isVisible())
            continue;

        // Check for custom cell span
        CellSpan span = getCellSpan(child.get());

        int col, row;
        if (cellSpans_.count(child.get())) {
            col = span.column;
            row = span.row;
        } else {
            col = index % cols;
            row = index / cols;
        }

        float x = startX + col * (cellWidth + columnGap_);
        float y = startY + row * (cellHeight + rowGap_);
        float w = cellWidth * span.columnSpan + (span.columnSpan - 1) * columnGap_;
        float h = cellHeight * span.rowSpan + (span.rowSpan - 1) * rowGap_;

        child->setBounds(Rect(x, y, w, h));
        child->layout(Rect(x, y, w, h));

        ++index;
    }

    clearDirty();
}

} // namespace dakt::gui

#ifndef DAKTLIB_GUI_GRID_HPP
#define DAKTLIB_GUI_GRID_HPP

#include "ContainerBase.hpp"

namespace dakt::gui {

/**
 * @brief Grid container - arranges children in rows and columns
 */
class DAKTLIB_GUI_API Grid : public UIContainer {
  public:
    Grid();
    explicit Grid(const std::string& id);
    Grid(int columns, int rows = 0);
    Grid(const std::string& id, int columns, int rows = 0);

    int getColumns() const { return columns_; }
    void setColumns(int columns) {
        columns_ = columns;
        markDirty();
    }

    int getRows() const { return rows_; }
    void setRows(int rows) {
        rows_ = rows;
        markDirty();
    }

    float getColumnGap() const { return columnGap_; }
    void setColumnGap(float gap) {
        columnGap_ = gap;
        markDirty();
    }

    float getRowGap() const { return rowGap_; }
    void setRowGap(float gap) {
        rowGap_ = gap;
        markDirty();
    }

    void setGap(float gap) {
        columnGap_ = rowGap_ = gap;
        markDirty();
    }

    bool isAutoRows() const { return autoRows_; }
    void setAutoRows(bool auto_) {
        autoRows_ = auto_;
        markDirty();
    }

    // Cell spanning (for advanced grid layouts)
    struct CellSpan {
        int column = 0;
        int row = 0;
        int columnSpan = 1;
        int rowSpan = 1;
    };

    void setCellSpan(Widget* widget, const CellSpan& span);
    CellSpan getCellSpan(Widget* widget) const;

    void layout();

  private:
    void layoutGrid();

    int columns_ = 2;
    int rows_ = 0; // 0 = auto-calculate from children
    float columnGap_ = 8.0f;
    float rowGap_ = 8.0f;
    bool autoRows_ = true;
    std::unordered_map<Widget*, CellSpan> cellSpans_;
};

} // namespace dakt::gui

#endif // DAKTLIB_GUI_GRID_HPP

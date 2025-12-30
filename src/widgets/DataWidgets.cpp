// ============================================================================
// DaktLib GUI Module - Data Widgets Implementation
// ============================================================================

#include <dakt/gui/core/DrawList.hpp>
#include <dakt/gui/framework/Context.hpp>
#include <dakt/gui/framework/Theme.hpp>
#include <dakt/gui/widgets/Containers.hpp>
#include <dakt/gui/widgets/DataWidgets.hpp>
#include <dakt/gui/widgets/Widgets.hpp>

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace dakt::gui
{

// ============================================================================
// Internal State
// ============================================================================

namespace
{

struct PropertyGridState
{
    WidgetId id;
    PropertyGridConfig config;
    String currentCategory;
    i32 rowIndex = 0;
};

PropertyGridState* g_currentPropertyGrid = nullptr;

struct DataTableInternalState
{
    WidgetId id;
    DataTableConfig config;
    DataTableState* state;
    i32 currentRow = -1;
    i32 currentColumn = -1;
    Vec2 startPos;
};

DataTableInternalState* g_currentDataTable = nullptr;

struct TreeListInternalState
{
    WidgetId id;
    TreeListConfig config;
    TreeListState* state;
    Vec2 startPos;
    f32 currentY = 0;
};

TreeListInternalState* g_currentTreeList = nullptr;

}  // anonymous namespace

// Forward declarations
void propertyCategory(StringView name);

// ============================================================================
// Property Grid
// ============================================================================

bool beginPropertyGrid(StringView id, const PropertyGridConfig& config)
{
    auto* ctx = getContext();
    if (!ctx)
        return false;

    static PropertyGridState state;
    state.id = ctx->getId(id);
    state.config = config;
    state.currentCategory = "";
    state.rowIndex = 0;

    g_currentPropertyGrid = &state;

    return true;
}

void endPropertyGrid()
{
    g_currentPropertyGrid = nullptr;
}

namespace
{

void drawPropertyRow(StringView name, bool useAltBg = false)
{
    auto* ctx = getContext();
    if (!ctx || !g_currentPropertyGrid)
        return;

    auto* theme = ctx->getTheme();

    Vec2 pos = ctx->getCursorScreenPos();
    f32 rowHeight = 22.0f;
    f32 totalWidth = ctx->getContentWidth();

    // Draw alternating background
    if (useAltBg)
    {
        Rect rowRect{pos, pos + Vec2{totalWidth, rowHeight}};
        ctx->getDrawList().addRectFilled(rowRect, g_currentPropertyGrid->config.alternateBg);
    }

    // Draw name column
    Vec2 namePos = pos + Vec2{4, (rowHeight - 14) * 0.5f};
    ctx->getDrawList().addText(namePos, name, theme->text);

    // Move cursor to value column
    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.cursor = pos + Vec2{g_currentPropertyGrid->config.nameColumnWidth, 0};
    }

    g_currentPropertyGrid->rowIndex++;
}

void endPropertyRow(f32 height = 22.0f)
{
    auto* ctx = getContext();
    if (!ctx)
        return;

    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.cursor.x = window->contentRect.min.x;
        window->layout.cursor.y += height;
    }
}

}  // anonymous namespace

bool propertyBool(StringView name, bool* value, StringView tooltip)
{
    (void)tooltip;

    if (!g_currentPropertyGrid || !value)
        return false;

    drawPropertyRow(name, g_currentPropertyGrid->rowIndex % 2 == 1);

    bool changed = checkbox("##value", value);

    endPropertyRow();

    return changed;
}

bool propertyInt(StringView name, i32* value, i32 min, i32 max, StringView tooltip)
{
    (void)tooltip;

    if (!g_currentPropertyGrid || !value)
        return false;

    drawPropertyRow(name, g_currentPropertyGrid->rowIndex % 2 == 1);

    bool changed = sliderInt("##value", value, min, max);

    endPropertyRow();

    return changed;
}

bool propertyFloat(StringView name, f32* value, f32 min, f32 max, StringView tooltip)
{
    (void)tooltip;

    if (!g_currentPropertyGrid || !value)
        return false;

    drawPropertyRow(name, g_currentPropertyGrid->rowIndex % 2 == 1);

    bool changed = sliderFloat("##value", value, min, max);

    endPropertyRow();

    return changed;
}

bool propertyString(StringView name, String& value, StringView tooltip)
{
    (void)tooltip;

    if (!g_currentPropertyGrid)
        return false;

    drawPropertyRow(name, g_currentPropertyGrid->rowIndex % 2 == 1);

    bool changed = inputText("##value", value);

    endPropertyRow();

    return changed;
}

bool propertyString(StringView name, char* buf, usize bufSize, StringView tooltip)
{
    (void)tooltip;

    if (!g_currentPropertyGrid || !buf)
        return false;

    drawPropertyRow(name, g_currentPropertyGrid->rowIndex % 2 == 1);

    bool changed = inputText("##value", buf, bufSize);

    endPropertyRow();

    return changed;
}

bool propertyVec2(StringView name, Vec2* value, StringView tooltip)
{
    (void)tooltip;

    if (!g_currentPropertyGrid || !value)
        return false;

    drawPropertyRow(name, g_currentPropertyGrid->rowIndex % 2 == 1);

    f32 values[2] = {value->x, value->y};
    bool changed = sliderFloat2("##value", values, -1000.0f, 1000.0f);
    if (changed)
    {
        value->x = values[0];
        value->y = values[1];
    }

    endPropertyRow();

    return changed;
}

bool propertyVec4(StringView name, Vec4* value, StringView tooltip)
{
    (void)tooltip;

    if (!g_currentPropertyGrid || !value)
        return false;

    drawPropertyRow(name, g_currentPropertyGrid->rowIndex % 2 == 1);

    f32 values[4] = {value->x, value->y, value->z, value->w};
    bool changed = sliderFloat4("##value", values, -1000.0f, 1000.0f);
    if (changed)
    {
        value->x = values[0];
        value->y = values[1];
        value->z = values[2];
        value->w = values[3];
    }

    endPropertyRow();

    return changed;
}

bool propertyColor(StringView name, Color* value, StringView tooltip)
{
    (void)tooltip;

    if (!g_currentPropertyGrid || !value)
        return false;

    drawPropertyRow(name, g_currentPropertyGrid->rowIndex % 2 == 1);

    f32 col[4] = {value->r / 255.0f, value->g / 255.0f, value->b / 255.0f, value->a / 255.0f};
    bool changed = colorEdit4("##value", col);
    if (changed)
    {
        value->r = static_cast<u8>(col[0] * 255.0f);
        value->g = static_cast<u8>(col[1] * 255.0f);
        value->b = static_cast<u8>(col[2] * 255.0f);
        value->a = static_cast<u8>(col[3] * 255.0f);
    }

    endPropertyRow();

    return changed;
}

bool propertyColor3(StringView name, f32 col[3], StringView tooltip)
{
    (void)tooltip;

    if (!g_currentPropertyGrid || !col)
        return false;

    drawPropertyRow(name, g_currentPropertyGrid->rowIndex % 2 == 1);

    bool changed = colorEdit3("##value", col);

    endPropertyRow();

    return changed;
}

bool propertyColor4(StringView name, f32 col[4], StringView tooltip)
{
    (void)tooltip;

    if (!g_currentPropertyGrid || !col)
        return false;

    drawPropertyRow(name, g_currentPropertyGrid->rowIndex % 2 == 1);

    bool changed = colorEdit4("##value", col);

    endPropertyRow();

    return changed;
}

bool propertyCombo(StringView name, i32* currentItem, std::span<const StringView> items, StringView tooltip)
{
    (void)tooltip;

    if (!g_currentPropertyGrid || !currentItem)
        return false;

    drawPropertyRow(name, g_currentPropertyGrid->rowIndex % 2 == 1);

    bool changed = combo("##value", currentItem, items);

    endPropertyRow();

    return changed;
}

bool propertyFlags(StringView name, u32* flags, std::span<const StringView> flagNames, StringView tooltip)
{
    (void)tooltip;

    if (!g_currentPropertyGrid || !flags)
        return false;

    bool changed = false;

    propertyCategory(name);

    for (usize i = 0; i < flagNames.size(); ++i)
    {
        u32 flagValue = 1u << i;
        bool flagSet = (*flags & flagValue) != 0;

        if (propertyBool(flagNames[i], &flagSet))
        {
            if (flagSet)
            {
                *flags |= flagValue;
            }
            else
            {
                *flags &= ~flagValue;
            }
            changed = true;
        }
    }

    return changed;
}

void propertyCategory(StringView name)
{
    auto* ctx = getContext();
    if (!ctx || !g_currentPropertyGrid)
        return;

    auto* theme = ctx->getTheme();

    Vec2 pos = ctx->getCursorScreenPos();
    f32 rowHeight = 22.0f;
    f32 totalWidth = ctx->getContentWidth();

    // Draw category background
    Rect catRect{pos, pos + Vec2{totalWidth, rowHeight}};
    ctx->getDrawList().addRectFilled(catRect, g_currentPropertyGrid->config.categoryBg);

    // Draw category name
    Vec2 textPos = pos + Vec2{4, (rowHeight - 14) * 0.5f};
    ctx->getDrawList().addText(textPos, name, theme->text);

    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(Vec2{totalWidth, rowHeight});
    }

    g_currentPropertyGrid->currentCategory = String(name);
}

void propertyReadOnly(StringView name, StringView value)
{
    auto* ctx = getContext();
    if (!ctx || !g_currentPropertyGrid)
        return;

    auto* theme = ctx->getTheme();

    drawPropertyRow(name, g_currentPropertyGrid->rowIndex % 2 == 1);

    Vec2 pos = ctx->getCursorScreenPos();
    ctx->getDrawList().addText(pos + Vec2{4, 4}, value, theme->textDisabled);

    endPropertyRow();
}

void propertyReadOnly(StringView name, i32 value)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%d", value);
    propertyReadOnly(name, buf);
}

void propertyReadOnly(StringView name, f32 value)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.3f", value);
    propertyReadOnly(name, buf);
}

void propertyCustom(StringView name, std::function<bool()> valueEditor)
{
    if (!g_currentPropertyGrid || !valueEditor)
        return;

    drawPropertyRow(name, g_currentPropertyGrid->rowIndex % 2 == 1);
    valueEditor();
    endPropertyRow();
}

// ============================================================================
// Data Table
// ============================================================================

bool beginDataTable(StringView id, const DataTableConfig& config, DataTableState& state)
{
    auto* ctx = getContext();
    if (!ctx)
        return false;

    static DataTableInternalState internalState;
    internalState.id = ctx->getId(id);
    internalState.config = config;
    internalState.state = &state;
    internalState.currentRow = -1;
    internalState.currentColumn = -1;
    internalState.startPos = ctx->getCursorScreenPos();

    g_currentDataTable = &internalState;

    auto* theme = ctx->getTheme();

    Vec2 pos = internalState.startPos;
    f32 tableWidth = ctx->getContentWidth();

    // Draw header
    if (config.showHeaders)
    {
        f32 x = pos.x;
        for (usize i = 0; i < config.columns.size(); ++i)
        {
            const auto& col = config.columns[i];
            f32 colWidth = col.width > 0 ? col.width : tableWidth / config.columns.size();

            Rect headerRect{{x, pos.y}, {x + colWidth, pos.y + config.rowHeight}};
            ctx->getDrawList().addRectFilled(headerRect, config.headerBg);

            // Draw sort indicator if this column is sorted
            if (state.sortColumn == static_cast<i32>(i))
            {
                Vec2 arrowPos = headerRect.max - Vec2{12, config.rowHeight * 0.5f};
                if (state.sortDirection == SortDirection::Ascending)
                {
                    ctx->getDrawList().addTriangleFilled(arrowPos + Vec2{0, -4}, arrowPos + Vec2{-4, 2},
                                                         arrowPos + Vec2{4, 2}, theme->text);
                }
                else if (state.sortDirection == SortDirection::Descending)
                {
                    ctx->getDrawList().addTriangleFilled(arrowPos + Vec2{0, 4}, arrowPos + Vec2{-4, -2},
                                                         arrowPos + Vec2{4, -2}, theme->text);
                }
            }

            // Draw header text
            Vec2 textPos = headerRect.min + Vec2{4, (config.rowHeight - 14) * 0.5f};
            ctx->getDrawList().addText(textPos, col.header, theme->text);

            // Handle header click for sorting
            if (col.sortable && headerRect.contains(ctx->getMousePos()) && ctx->isMouseClicked(MouseButton::Left))
            {
                if (state.sortColumn == static_cast<i32>(i))
                {
                    if (state.sortDirection == SortDirection::Ascending)
                    {
                        state.sortDirection = SortDirection::Descending;
                    }
                    else if (state.sortDirection == SortDirection::Descending)
                    {
                        state.sortDirection = SortDirection::None;
                        state.sortColumn = -1;
                    }
                    else
                    {
                        state.sortDirection = SortDirection::Ascending;
                    }
                }
                else
                {
                    state.sortColumn = static_cast<i32>(i);
                    state.sortDirection = SortDirection::Ascending;
                }
            }

            x += colWidth;
        }

        // Advance past header
        auto* window = ctx->getCurrentWindow();
        if (window)
        {
            window->layout.cursor.y += config.rowHeight;
        }
    }

    return true;
}

void endDataTable()
{
    if (!g_currentDataTable)
        return;

    auto* ctx = getContext();
    if (!ctx)
        return;

    // Calculate total height
    f32 totalHeight = (g_currentDataTable->currentRow + 1) * g_currentDataTable->config.rowHeight;
    if (g_currentDataTable->config.showHeaders)
    {
        totalHeight += g_currentDataTable->config.rowHeight;
    }

    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(Vec2{ctx->getContentWidth(), totalHeight});
    }

    g_currentDataTable = nullptr;
}

void dataTableNextRow()
{
    if (!g_currentDataTable)
        return;

    auto* ctx = getContext();
    if (!ctx)
        return;

    auto& state = *g_currentDataTable;
    state.currentRow++;
    state.currentColumn = -1;

    // Draw row background
    Vec2 pos = state.startPos;
    pos.y += (state.currentRow + (state.config.showHeaders ? 1 : 0)) * state.config.rowHeight;

    f32 tableWidth = ctx->getContentWidth();
    Rect rowRect{pos, pos + Vec2{tableWidth, state.config.rowHeight}};

    bool isHovered = rowRect.contains(ctx->getMousePos());
    bool isSelected = std::find(state.state->selectedRows.begin(), state.state->selectedRows.end(), state.currentRow) !=
                      state.state->selectedRows.end();

    Color bgColor;
    if (isSelected)
    {
        bgColor = state.config.selectedBg;
    }
    else if (isHovered)
    {
        bgColor = state.config.hoverBg;
        state.state->hoveredRow = state.currentRow;
    }
    else if (state.config.alternateRowColors && state.currentRow % 2 == 1)
    {
        bgColor = state.config.rowBgAlt;
    }
    else
    {
        bgColor = state.config.rowBg;
    }

    ctx->getDrawList().addRectFilled(rowRect, bgColor);

    // Handle row selection
    if (isHovered && ctx->isMouseClicked(MouseButton::Left))
    {
        if (state.config.selectionMode == SelectionMode::Single)
        {
            state.state->selectedRows.clear();
            state.state->selectedRows.push_back(state.currentRow);
        }
        else if (state.config.selectionMode == SelectionMode::Multiple)
        {
            auto it = std::find(state.state->selectedRows.begin(), state.state->selectedRows.end(), state.currentRow);
            if (it != state.state->selectedRows.end())
            {
                state.state->selectedRows.erase(it);
            }
            else
            {
                state.state->selectedRows.push_back(state.currentRow);
            }
        }
    }
}

namespace
{

void dataTableNextCell()
{
    if (!g_currentDataTable)
        return;

    auto* ctx = getContext();
    if (!ctx)
        return;

    auto& state = *g_currentDataTable;
    state.currentColumn++;

    // Calculate cell position
    f32 x = state.startPos.x;
    for (i32 i = 0; i < state.currentColumn; ++i)
    {
        f32 colWidth = state.config.columns[i].width > 0 ? state.config.columns[i].width
                                                         : ctx->getContentWidth() / state.config.columns.size();
        x += colWidth;
    }

    f32 y = state.startPos.y + (state.currentRow + (state.config.showHeaders ? 1 : 0)) * state.config.rowHeight;

    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.cursor = Vec2{x + 4, y + (state.config.rowHeight - 14) * 0.5f};
    }
}

}  // anonymous namespace

void dataTableCell(StringView textStr)
{
    if (!g_currentDataTable)
        return;

    auto* ctx = getContext();
    if (!ctx)
        return;

    dataTableNextCell();
    ctx->getDrawList().addText(ctx->getCursorScreenPos(), textStr, ctx->getTheme()->text);
}

void dataTableCell(i32 value)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%d", value);
    dataTableCell(StringView(buf));
}

void dataTableCell(f32 value, StringView format)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), format.data(), value);
    dataTableCell(StringView(buf));
}

void dataTableCellCheckbox(bool* value)
{
    if (!value)
        return;
    dataTableNextCell();
    checkbox("##cell", value);
}

void dataTableCellButton(StringView label)
{
    dataTableNextCell();
    button(label, Vec2{60, 16});
}

void dataTableCellProgress(f32 fraction)
{
    if (!g_currentDataTable)
        return;

    auto* ctx = getContext();
    if (!ctx)
        return;

    dataTableNextCell();

    Vec2 pos = ctx->getCursorScreenPos();
    f32 width = 80.0f;
    f32 height = 14.0f;

    Rect bgRect{pos, pos + Vec2{width, height}};
    ctx->getDrawList().addRectFilled(bgRect, ctx->getTheme()->frameBg, 2.0f);

    Rect fillRect{pos, pos + Vec2{width * std::clamp(fraction, 0.0f, 1.0f), height}};
    ctx->getDrawList().addRectFilled(fillRect, ctx->getTheme()->plotHistogram, 2.0f);
}

void dataTableCellColor(Color color)
{
    if (!g_currentDataTable)
        return;

    auto* ctx = getContext();
    if (!ctx)
        return;

    dataTableNextCell();

    Vec2 pos = ctx->getCursorScreenPos();
    Rect colorRect{pos, pos + Vec2{16, 14}};
    ctx->getDrawList().addRectFilled(colorRect, color);
    ctx->getDrawList().addRect(colorRect, ctx->getTheme()->border);
}

void dataTableCellCustom(std::function<void()> renderer)
{
    if (!renderer)
        return;
    dataTableNextCell();
    renderer();
}

bool isDataTableRowSelected(i32 row)
{
    if (!g_currentDataTable)
        return false;
    const auto& selected = g_currentDataTable->state->selectedRows;
    return std::find(selected.begin(), selected.end(), row) != selected.end();
}

std::span<const i32> getDataTableSelectedRows()
{
    if (!g_currentDataTable)
        return {};
    return g_currentDataTable->state->selectedRows;
}

void clearDataTableSelection()
{
    if (g_currentDataTable)
    {
        g_currentDataTable->state->selectedRows.clear();
    }
}

void selectDataTableRow(i32 row, bool addToSelection)
{
    if (!g_currentDataTable)
        return;

    if (!addToSelection)
    {
        g_currentDataTable->state->selectedRows.clear();
    }
    g_currentDataTable->state->selectedRows.push_back(row);
}

// ============================================================================
// Hex View
// ============================================================================

bool hexView(StringView id, std::span<byte> data, const HexViewConfig& config, HexViewState& state)
{
    (void)id;  // Suppress unused warning

    auto* ctx = getContext();
    if (!ctx)
        return false;

    auto* theme = ctx->getTheme();
    auto* font = ctx->getFont();

    Vec2 pos = ctx->getCursorScreenPos();
    f32 charWidth = font ? font->calcTextWidth("0") : 8.0f;
    f32 lineHeight = 16.0f;

    f32 addressWidth = config.showAddresses ? charWidth * 10 : 0;
    f32 hexWidth = charWidth * 3 * config.bytesPerRow;
    f32 asciiWidth = config.showAscii ? charWidth * config.bytesPerRow : 0;
    f32 totalWidth = addressWidth + hexWidth + asciiWidth + 16;

    usize numRows = (data.size() + config.bytesPerRow - 1) / config.bytesPerRow;
    f32 totalHeight = std::min(300.0f, static_cast<f32>(numRows) * lineHeight);

    Rect bounds{pos, pos + Vec2{totalWidth, totalHeight}};
    ctx->getDrawList().addRectFilled(bounds, theme->frameBg);

    // Draw header
    if (config.showHeader)
    {
        f32 x = pos.x + addressWidth;
        for (i32 i = 0; i < config.bytesPerRow; ++i)
        {
            char buf[4];
            std::snprintf(buf, sizeof(buf), "%02X", i);
            ctx->getDrawList().addText({x, pos.y}, buf, theme->textDisabled);
            x += charWidth * 3;
        }
        pos.y += lineHeight;
    }

    // Draw hex content
    bool modified = false;
    f32 y = pos.y - state.scroll.y;

    for (usize row = 0; row < numRows; ++row)
    {
        if (y + lineHeight < pos.y || y > bounds.max.y)
        {
            y += lineHeight;
            continue;
        }

        f32 x = pos.x;
        usize rowOffset = row * config.bytesPerRow;

        // Draw address
        if (config.showAddresses)
        {
            char addrBuf[16];
            std::snprintf(addrBuf, sizeof(addrBuf), "%08zX", rowOffset);
            ctx->getDrawList().addText({x, y}, addrBuf, config.addressColor);
            x += addressWidth;
        }

        // Draw hex bytes
        for (i32 col = 0; col < config.bytesPerRow; ++col)
        {
            usize byteIndex = rowOffset + col;
            if (byteIndex >= data.size())
                break;

            byte b = data[byteIndex];
            bool isSelected = byteIndex >= state.selectionStart && byteIndex <= state.selectionEnd;
            bool isModified = std::find(state.modifiedBytes.begin(), state.modifiedBytes.end(), byteIndex) !=
                              state.modifiedBytes.end();

            if (isSelected)
            {
                Rect selRect{{x, y}, {x + charWidth * 2, y + lineHeight}};
                ctx->getDrawList().addRectFilled(selRect, config.selectionBg);
            }

            char hexBuf[4];
            std::snprintf(hexBuf, sizeof(hexBuf), "%02X", static_cast<u8>(b));

            Color hexColor =
                isModified ? config.modifiedColor : (b == std::byte{0} ? config.zeroColor : config.hexColor);
            ctx->getDrawList().addText({x, y}, hexBuf, hexColor);

            x += charWidth * 3;
        }

        // Draw ASCII
        if (config.showAscii)
        {
            x += charWidth;
            for (i32 col = 0; col < config.bytesPerRow; ++col)
            {
                usize byteIndex = rowOffset + col;
                if (byteIndex >= data.size())
                    break;

                byte b = data[byteIndex];
                u8 bVal = static_cast<u8>(b);
                char ch = (bVal >= 32 && bVal < 127) ? static_cast<char>(bVal) : '.';
                char asciiStr[2] = {ch, '\0'};

                bool isSelected = byteIndex >= state.selectionStart && byteIndex <= state.selectionEnd;
                if (isSelected)
                {
                    Rect selRect{{x, y}, {x + charWidth, y + lineHeight}};
                    ctx->getDrawList().addRectFilled(selRect, config.selectionBg);
                }

                ctx->getDrawList().addText({x, y}, asciiStr, config.asciiColor);
                x += charWidth;
            }
        }

        y += lineHeight;
    }

    // Handle selection
    if (bounds.contains(ctx->getMousePos()))
    {
        if (ctx->isMouseClicked(MouseButton::Left))
        {
            // Calculate byte index from mouse position
            Vec2 relPos = ctx->getMousePos() - pos;
            relPos.y += state.scroll.y;

            i32 row = static_cast<i32>(relPos.y / lineHeight);
            f32 xOffset = relPos.x - addressWidth;
            i32 col = static_cast<i32>(xOffset / (charWidth * 3));

            if (col >= 0 && col < config.bytesPerRow)
            {
                usize byteIndex = row * config.bytesPerRow + col;
                if (byteIndex < data.size())
                {
                    state.selectionStart = byteIndex;
                    state.selectionEnd = byteIndex;
                    state.cursorPos = byteIndex;
                }
            }
        }

        // Handle scroll
        f32 wheel = ctx->getMouseWheel();
        state.scroll.y -= wheel * lineHeight * 3;
        state.scroll.y = std::clamp(state.scroll.y, 0.0f, std::max(0.0f, numRows * lineHeight - totalHeight));
    }

    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(bounds.size());
    }

    return modified;
}

bool hexView(StringView id, std::span<const byte> data, const HexViewConfig& config, HexViewState& state)
{
    // Create a non-const copy for the editable version
    // For read-only view, just wrap and call
    auto* ctx = getContext();
    if (!ctx)
        return false;

    // For const data, we can't modify, so just render
    HexViewConfig readOnlyConfig = config;
    readOnlyConfig.editable = false;

    std::vector<byte> dataCopy(data.begin(), data.end());
    return hexView(id, std::span<byte>(dataCopy), readOnlyConfig, state);
}

void hexViewGoToAddress(HexViewState& state, usize address)
{
    state.cursorPos = address;
    state.selectionStart = address;
    state.selectionEnd = address;
    // Scroll calculation would need bytesPerRow which we don't have here
}

void hexViewSelectRange(HexViewState& state, usize start, usize end)
{
    state.selectionStart = start;
    state.selectionEnd = end;
}

std::span<const byte> hexViewGetSelection(std::span<const byte> data, const HexViewState& state)
{
    if (state.selectionStart >= data.size())
        return {};
    usize end = std::min(state.selectionEnd + 1, data.size());
    return data.subspan(state.selectionStart, end - state.selectionStart);
}

// ============================================================================
// Tree List
// ============================================================================

bool beginTreeList(StringView id, const TreeListConfig& config, TreeListState& state)
{
    auto* ctx = getContext();
    if (!ctx)
        return false;

    static TreeListInternalState internalState;
    internalState.id = ctx->getId(id);
    internalState.config = config;
    internalState.state = &state;
    internalState.startPos = ctx->getCursorScreenPos();
    internalState.currentY = 0;

    g_currentTreeList = &internalState;

    // Draw column headers if we have columns
    if (!config.columns.empty())
    {
        Vec2 pos = internalState.startPos;
        f32 x = pos.x;

        for (const auto& col : config.columns)
        {
            f32 colWidth = col.width > 0 ? col.width : ctx->getContentWidth() / config.columns.size();
            Rect headerRect{{x, pos.y}, {x + colWidth, pos.y + config.rowHeight}};
            ctx->getDrawList().addRectFilled(headerRect, ctx->getTheme()->tableHeaderBg);

            Vec2 textPos = headerRect.min + Vec2{4, (config.rowHeight - 14) * 0.5f};
            ctx->getDrawList().addText(textPos, col.header, ctx->getTheme()->text);

            x += colWidth;
        }

        internalState.currentY = config.rowHeight;
    }

    return true;
}

void endTreeList()
{
    if (!g_currentTreeList)
        return;

    auto* ctx = getContext();
    if (ctx)
    {
        auto* window = ctx->getCurrentWindow();
        if (window)
        {
            window->layout.advanceCursor(Vec2{ctx->getContentWidth(), g_currentTreeList->currentY});
        }
    }

    g_currentTreeList = nullptr;
}

bool treeListNode(TreeListNode& node, i32 depth)
{
    if (!g_currentTreeList)
        return false;

    auto* ctx = getContext();
    if (!ctx)
        return false;

    auto* theme = ctx->getTheme();
    const auto& config = g_currentTreeList->config;

    Vec2 pos = g_currentTreeList->startPos;
    pos.y += g_currentTreeList->currentY;

    f32 indentWidth = depth * config.indentWidth;
    f32 rowWidth = ctx->getContentWidth();

    Rect rowRect{pos, pos + Vec2{rowWidth, config.rowHeight}};
    bool hovered = rowRect.contains(ctx->getMousePos());

    // Draw background
    if (node.selected)
    {
        ctx->getDrawList().addRectFilled(rowRect, theme->header);
    }
    else if (hovered)
    {
        ctx->getDrawList().addRectFilled(rowRect, theme->headerHovered);
        g_currentTreeList->state->hoveredNode = &node;
    }

    // Draw tree lines
    if (config.showLines && depth > 0)
    {
        f32 lineX = pos.x + (depth - 0.5f) * config.indentWidth;
        ctx->getDrawList().addLine({lineX, pos.y}, {lineX, pos.y + config.rowHeight}, config.lineColor);
        ctx->getDrawList().addLine({lineX, pos.y + config.rowHeight * 0.5f},
                                   {pos.x + depth * config.indentWidth, pos.y + config.rowHeight * 0.5f},
                                   config.lineColor);
    }

    f32 x = pos.x + indentWidth;

    // Draw expand/collapse arrow
    bool hasChildren = !node.children.empty();
    if (hasChildren)
    {
        Vec2 arrowCenter{x + 6, pos.y + config.rowHeight * 0.5f};
        if (node.expanded)
        {
            ctx->getDrawList().addTriangleFilled(arrowCenter + Vec2{0, 3}, arrowCenter + Vec2{-4, -2},
                                                 arrowCenter + Vec2{4, -2}, theme->text);
        }
        else
        {
            ctx->getDrawList().addTriangleFilled(arrowCenter + Vec2{3, 0}, arrowCenter + Vec2{-2, -4},
                                                 arrowCenter + Vec2{-2, 4}, theme->text);
        }
    }
    x += 16;

    // Draw icon if enabled
    if (config.showIcons && node.iconTexture != 0)
    {
        Rect iconRect{{x, pos.y + 2}, {x + 16, pos.y + config.rowHeight - 2}};
        ctx->getDrawList().addImage(node.iconTexture, iconRect, {0, 0}, {1, 1}, node.iconColor);
        x += 20;
    }

    // Draw label
    ctx->getDrawList().addText({x, pos.y + (config.rowHeight - 14) * 0.5f}, node.label, theme->text);

    // Draw column values
    if (!config.columns.empty() && !node.columnValues.empty())
    {
        f32 colX = pos.x;
        for (usize i = 0; i < config.columns.size() && i < node.columnValues.size() + 1; ++i)
        {
            f32 colWidth = config.columns[i].width > 0 ? config.columns[i].width : rowWidth / config.columns.size();

            if (i > 0 && i - 1 < node.columnValues.size())
            {
                Vec2 textPos{colX + 4, pos.y + (config.rowHeight - 14) * 0.5f};
                ctx->getDrawList().addText(textPos, node.columnValues[i - 1], theme->text);
            }

            colX += colWidth;
        }
    }

    // Handle click
    bool clicked = false;
    if (hovered && ctx->isMouseClicked(MouseButton::Left))
    {
        // Check if clicking on arrow
        f32 arrowX = pos.x + indentWidth;
        if (hasChildren && ctx->getMousePos().x < arrowX + 16)
        {
            node.expanded = !node.expanded;
        }
        else
        {
            // Select node
            if (!ctx->isKeyDown(Key::LeftCtrl) && !ctx->isKeyDown(Key::RightCtrl))
            {
                treeListClearSelection(*g_currentTreeList->state);
            }
            treeListSelectNode(*g_currentTreeList->state, &node, true);
            clicked = true;
        }
    }

    g_currentTreeList->currentY += config.rowHeight;

    // Render children if expanded
    if (node.expanded)
    {
        for (auto& child : node.children)
        {
            treeListNode(child, depth + 1);
        }
    }

    return clicked;
}

void treeListLeaf(StringView label, std::span<const StringView> columnValues)
{
    TreeListNode node;
    node.label = label;
    node.columnValues.assign(columnValues.begin(), columnValues.end());
    treeListNode(node, 0);
}

void treeListSelectNode(TreeListState& state, TreeListNode* node, bool addToSelection)
{
    if (!node)
        return;

    if (!addToSelection)
    {
        for (auto* n : state.selectedNodes)
        {
            n->selected = false;
        }
        state.selectedNodes.clear();
    }

    node->selected = true;
    state.selectedNodes.push_back(node);
}

void treeListClearSelection(TreeListState& state)
{
    for (auto* node : state.selectedNodes)
    {
        node->selected = false;
    }
    state.selectedNodes.clear();
}

std::span<TreeListNode* const> treeListGetSelectedNodes(const TreeListState& state)
{
    return state.selectedNodes;
}

void treeListExpandAll(TreeListNode& root)
{
    root.expanded = true;
    for (auto& child : root.children)
    {
        treeListExpandAll(child);
    }
}

void treeListCollapseAll(TreeListNode& root)
{
    root.expanded = false;
    for (auto& child : root.children)
    {
        treeListCollapseAll(child);
    }
}

void treeListExpandToNode(TreeListNode& root, TreeListNode* target)
{
    if (&root == target)
        return;

    // DFS to find path to target
    std::function<bool(TreeListNode&)> expandPath = [&](TreeListNode& node) -> bool
    {
        if (&node == target)
            return true;

        for (auto& child : node.children)
        {
            if (expandPath(child))
            {
                node.expanded = true;
                return true;
            }
        }
        return false;
    };

    expandPath(root);
}

// ============================================================================
// Log View
// ============================================================================

void logView(StringView id, std::span<const LogEntry> entries, const LogViewConfig& config, LogViewState& state)
{
    auto* ctx = getContext();
    if (!ctx)
        return;

    WidgetId logId = ctx->getId(id);
    (void)logId;

    auto* theme = ctx->getTheme();

    Vec2 pos = ctx->getCursorScreenPos();
    f32 width = ctx->getContentWidth();
    f32 height = 200.0f;
    f32 lineHeight = 16.0f;

    Rect bounds{pos, pos + Vec2{width, height}};
    ctx->getDrawList().addRectFilled(bounds, theme->frameBg);
    ctx->getDrawList().pushClipRect(bounds);

    f32 y = pos.y - state.scroll.y;
    i32 visibleLines = 0;

    for (const auto& entry : entries)
    {
        // Filter by level
        if (entry.level < state.minLevel)
            continue;

        // Filter by text
        if (!state.filterText.empty())
        {
            // Simple substring search (case-insensitive would be better)
            if (entry.message.find(state.filterText) == StringView::npos)
                continue;
        }

        if (y + lineHeight < pos.y || y > bounds.max.y)
        {
            y += lineHeight;
            continue;
        }

        f32 x = pos.x + 4;

        // Timestamp
        if (config.showTimestamp)
        {
            char timeBuf[16];
            std::snprintf(timeBuf, sizeof(timeBuf), "[%.2f]", entry.timestamp);
            ctx->getDrawList().addText({x, y}, timeBuf, theme->textDisabled);
            x += 80;
        }

        // Level
        if (config.showLevel)
        {
            StringView levelStr;
            Color levelColor;

            switch (entry.level)
            {
                case LogLevel::Trace:
                    levelStr = "[TRC]";
                    levelColor = config.traceColor;
                    break;
                case LogLevel::Debug:
                    levelStr = "[DBG]";
                    levelColor = config.debugColor;
                    break;
                case LogLevel::Info:
                    levelStr = "[INF]";
                    levelColor = config.infoColor;
                    break;
                case LogLevel::Warning:
                    levelStr = "[WRN]";
                    levelColor = config.warningColor;
                    break;
                case LogLevel::Error:
                    levelStr = "[ERR]";
                    levelColor = config.errorColor;
                    break;
                case LogLevel::Fatal:
                    levelStr = "[FTL]";
                    levelColor = config.fatalColor;
                    break;
            }

            ctx->getDrawList().addText({x, y}, levelStr, levelColor);
            x += 40;
        }

        // Source
        if (config.showSource && !entry.source.empty())
        {
            ctx->getDrawList().addText({x, y}, entry.source, theme->textDisabled);
            x += 100;
        }

        // Message
        Color msgColor;
        switch (entry.level)
        {
            case LogLevel::Trace:
                msgColor = config.traceColor;
                break;
            case LogLevel::Debug:
                msgColor = config.debugColor;
                break;
            case LogLevel::Info:
                msgColor = config.infoColor;
                break;
            case LogLevel::Warning:
                msgColor = config.warningColor;
                break;
            case LogLevel::Error:
                msgColor = config.errorColor;
                break;
            case LogLevel::Fatal:
                msgColor = config.fatalColor;
                break;
        }
        ctx->getDrawList().addText({x, y}, entry.message, msgColor);

        y += lineHeight;
        visibleLines++;
    }

    ctx->getDrawList().popClipRect();

    // Handle scroll
    if (bounds.contains(ctx->getMousePos()))
    {
        f32 wheel = ctx->getMouseWheel();
        state.scroll.y -= wheel * lineHeight * 3;
        f32 maxScroll = std::max(0.0f, static_cast<f32>(entries.size()) * lineHeight - height);
        state.scroll.y = std::clamp(state.scroll.y, 0.0f, maxScroll);
    }

    // Auto-scroll
    if (config.autoScroll && state.scrollToBottom)
    {
        state.scroll.y = std::max(0.0f, static_cast<f32>(entries.size()) * lineHeight - height);
        state.scrollToBottom = false;
    }

    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(bounds.size());
    }
}

void logViewScrollToBottom(LogViewState& state)
{
    state.scrollToBottom = true;
}

void logViewClear(LogViewState& state)
{
    state.scroll = {0, 0};
    state.filterText.clear();
}

// ============================================================================
// Code Editor (Stub)
// ============================================================================

bool codeEditor(StringView id, String& text, const CodeEditorConfig& config, CodeEditorState& state)
{
    (void)id;
    (void)config;
    (void)state;

    // For now, just render as a multiline text input
    return inputTextMultiline("##code", text, Vec2{-1, 300});
}

void codeEditorGoToLine(CodeEditorState& state, i32 line)
{
    state.cursorLine = line;
    state.cursorColumn = 0;
}

void codeEditorSelectAll(CodeEditorState& state, const String& text)
{
    state.selectionStartLine = 0;
    state.selectionStartColumn = 0;

    // Count lines
    i32 lines = 1;
    i32 lastLineLen = 0;
    for (char c : text)
    {
        if (c == '\n')
        {
            lines++;
            lastLineLen = 0;
        }
        else
        {
            lastLineLen++;
        }
    }

    state.selectionEndLine = lines - 1;
    state.selectionEndColumn = lastLineLen;
}

// ============================================================================
// File Browser (Stub)
// ============================================================================

bool fileBrowser(StringView id, const FileBrowserConfig& config, FileBrowserState& state)
{
    (void)id;
    (void)config;

    auto* ctx = getContext();
    if (!ctx)
        return false;

    // This is a stub - full implementation would need filesystem access
    text("File Browser: ");
    text(state.currentPath.empty() ? "(no path)" : StringView(state.currentPath));

    return false;
}

void fileBrowserSetPath(FileBrowserState& state, StringView path)
{
    state.currentPath = String(path);
    state.needsRefresh = true;
}

void fileBrowserRefresh(FileBrowserState& state)
{
    state.needsRefresh = true;
}

void fileBrowserGoUp(FileBrowserState& state)
{
    // Remove last path component
    if (state.currentPath.empty())
        return;

    usize lastSlash = state.currentPath.rfind('/');
    if (lastSlash == String::npos)
    {
        lastSlash = state.currentPath.rfind('\\');
    }

    if (lastSlash != String::npos && lastSlash > 0)
    {
        state.currentPath = state.currentPath.substr(0, lastSlash);
        state.needsRefresh = true;
    }
}

StringView fileBrowserGetSelectedPath(const FileBrowserState& state)
{
    return state.selectedPath;
}

// ============================================================================
// Timeline (Stub)
// ============================================================================

bool timeline(StringView id, std::vector<TimelineTrack>& tracks, std::vector<TimelineKeyframe>& keyframes,
              const TimelineConfig& config, TimelineState& state)
{
    (void)id;
    (void)keyframes;

    auto* ctx = getContext();
    if (!ctx)
        return false;

    auto* theme = ctx->getTheme();

    Vec2 pos = ctx->getCursorScreenPos();
    f32 width = ctx->getContentWidth();
    f32 height = config.trackHeaderWidth + static_cast<f32>(tracks.size()) * config.trackHeight;

    Rect bounds{pos, pos + Vec2{width, height}};
    ctx->getDrawList().addRectFilled(bounds, theme->frameBg);

    // Draw time ruler
    f64 timeRange = state.viewEnd - state.viewStart;
    f64 pixelsPerSecond = (timeRange > 0.0) ? static_cast<f64>(width - config.trackHeaderWidth) / timeRange : 100.0;

    // Draw track headers and content
    f32 y = pos.y;
    for (auto& track : tracks)
    {
        if (!track.visible)
            continue;

        // Track header
        Rect headerRect{{pos.x, y}, {pos.x + config.trackHeaderWidth, y + track.height}};
        ctx->getDrawList().addRectFilled(headerRect, theme->childBg);
        ctx->getDrawList().addText(headerRect.min + Vec2{4, 4}, track.name, theme->text);

        // Track content
        Rect trackRect{{pos.x + config.trackHeaderWidth, y}, {pos.x + width, y + track.height}};
        ctx->getDrawList().addRectFilled(trackRect, track.color);

        y += track.height;
    }

    // Draw current time marker (playhead)
    f32 playheadX =
        pos.x + config.trackHeaderWidth + static_cast<f32>((state.currentTime - state.viewStart) * pixelsPerSecond);
    Color playheadColor = Color(255, 76, 76, 255);  // Red
    ctx->getDrawList().addLine({playheadX, pos.y}, {playheadX, pos.y + height}, playheadColor, 2.0f);

    // Handle clicking to set playhead
    if (bounds.contains(ctx->getMousePos()) && ctx->isMouseDown(MouseButton::Left))
    {
        f32 mouseX = ctx->getMousePos().x;
        if (mouseX > pos.x + config.trackHeaderWidth)
        {
            state.currentTime =
                state.viewStart + static_cast<f64>(mouseX - pos.x - config.trackHeaderWidth) / pixelsPerSecond;
            state.currentTime = std::clamp(state.currentTime, config.startTime, config.endTime);
        }
    }

    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(bounds.size());
    }

    return false;
}

void timelineSetTime(TimelineState& state, f64 time)
{
    state.currentTime = time;
}

void timelinePlay(TimelineState& state)
{
    state.isPlaying = true;
}

void timelinePause(TimelineState& state)
{
    state.isPlaying = false;
}

void timelineStop(TimelineState& state)
{
    state.isPlaying = false;
    state.currentTime = 0.0;
}

void timelineZoomToFit(TimelineState& state, const TimelineConfig& config)
{
    state.viewStart = config.startTime;
    state.viewEnd = config.endTime;
    state.zoomLevel = 1.0;
}

// ============================================================================
// Node Graph Editor (Stub)
// ============================================================================

bool nodeGraph(StringView id, std::vector<GraphNode>& nodes, std::vector<NodeLink>& links,
               const NodeGraphConfig& config, NodeGraphState& state)
{
    (void)id;
    (void)links;

    auto* ctx = getContext();
    if (!ctx)
        return false;

    auto* theme = ctx->getTheme();

    Vec2 pos = ctx->getCursorScreenPos();
    f32 width = ctx->getContentWidth();
    f32 height = 400.0f;

    Rect bounds{pos, pos + Vec2{width, height}};
    ctx->getDrawList().addRectFilled(bounds, config.backgroundColor);
    ctx->getDrawList().pushClipRect(bounds);

    // Draw grid
    if (config.showGrid)
    {
        f32 gridSize = config.gridSize * state.zoom;
        Vec2 offset{std::fmod(state.viewOffset.x, gridSize), std::fmod(state.viewOffset.y, gridSize)};

        for (f32 x = pos.x + offset.x; x < bounds.max.x; x += gridSize)
        {
            ctx->getDrawList().addLine({x, pos.y}, {x, bounds.max.y}, config.gridColor);
        }
        for (f32 y = pos.y + offset.y; y < bounds.max.y; y += gridSize)
        {
            ctx->getDrawList().addLine({pos.x, y}, {bounds.max.x, y}, config.gridColor);
        }
    }

    // Draw links
    for (const auto& link : links)
    {
        // Find source and dest nodes by id
        GraphNode* srcNode = nullptr;
        GraphNode* dstNode = nullptr;

        for (auto& node : nodes)
        {
            if (node.id == link.sourceNodeId)
                srcNode = &node;
            if (node.id == link.destNodeId)
                dstNode = &node;
        }

        if (!srcNode || !dstNode)
            continue;

        Vec2 srcPos =
            pos + state.viewOffset + srcNode->position * state.zoom + Vec2{srcNode->size.x, 30.0f} * state.zoom;
        Vec2 dstPos = pos + state.viewOffset + dstNode->position * state.zoom + Vec2{0, 30.0f} * state.zoom;

        // Draw bezier curve
        Vec2 cp1 = srcPos + Vec2{50, 0};
        Vec2 cp2 = dstPos - Vec2{50, 0};
        ctx->getDrawList().addBezierCubic(srcPos, cp1, cp2, dstPos, link.color, config.linkThickness);
    }

    // Draw nodes
    for (auto& node : nodes)
    {
        Vec2 nodePos = pos + state.viewOffset + node.position * state.zoom;
        Vec2 nodeSize = node.size * state.zoom;

        Rect nodeRect{nodePos, nodePos + nodeSize};
        bool nodeHovered = nodeRect.contains(ctx->getMousePos());

        // Draw node background
        ctx->getDrawList().addRectFilled(nodeRect, theme->childBg, 4.0f);

        // Draw header
        Rect headerRect{nodePos, nodePos + Vec2{nodeSize.x, 24.0f * state.zoom}};
        ctx->getDrawList().addRectFilled(headerRect, node.selected ? theme->header : node.headerColor, 4.0f,
                                         Corner::Top);

        // Draw title
        ctx->getDrawList().addText(nodePos + Vec2{8, 4} * state.zoom, node.title, theme->text);

        // Draw border
        Color borderColor = node.selected ? theme->navHighlight : theme->border;
        ctx->getDrawList().addRect(nodeRect, borderColor, 4.0f);

        // Draw input pins
        f32 pinY = 30.0f * state.zoom;
        for (const auto& pin : node.inputs)
        {
            Vec2 pinPos = nodePos + Vec2{0, pinY};
            ctx->getDrawList().addCircleFilled(pinPos, config.pinRadius * state.zoom, pin.color);
            ctx->getDrawList().addText(pinPos + Vec2{8, -6} * state.zoom, pin.name, theme->text);
            pinY += 20.0f * state.zoom;
        }

        // Draw output pins
        pinY = 30.0f * state.zoom;
        for (const auto& pin : node.outputs)
        {
            Vec2 pinPos = nodePos + Vec2{nodeSize.x, pinY};
            ctx->getDrawList().addCircleFilled(pinPos, config.pinRadius * state.zoom, pin.color);
            auto* font = ctx->getFont();
            Vec2 textSize = font ? font->calcTextSize(pin.name) : Vec2{40, 14};
            ctx->getDrawList().addText(pinPos - Vec2{textSize.x + 8, 6} * state.zoom, pin.name, theme->text);
            pinY += 20.0f * state.zoom;
        }

        // Handle node selection and dragging
        if (nodeHovered && ctx->isMouseClicked(MouseButton::Left))
        {
            node.selected = true;
        }

        if (node.selected && ctx->isMouseDown(MouseButton::Left))
        {
            node.position = node.position + ctx->getMouseDelta() / state.zoom;
        }
    }

    // Handle panning
    if (bounds.contains(ctx->getMousePos()) && ctx->isMouseDown(MouseButton::Middle))
    {
        state.viewOffset = state.viewOffset + ctx->getMouseDelta();
    }

    // Handle zooming
    if (bounds.contains(ctx->getMousePos()))
    {
        f32 wheel = ctx->getMouseWheel();
        if (wheel != 0.0f)
        {
            f32 zoomDelta = wheel * 0.1f;
            state.zoom = std::clamp(state.zoom + zoomDelta, 0.25f, 4.0f);
        }
    }

    ctx->getDrawList().popClipRect();

    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(bounds.size());
    }

    return false;
}

void nodeGraphCenterOnNodes(NodeGraphState& state, std::span<const GraphNode> nodes)
{
    if (nodes.empty())
        return;

    Vec2 min{FLT_MAX, FLT_MAX};
    Vec2 max{-FLT_MAX, -FLT_MAX};

    for (const auto& node : nodes)
    {
        min.x = std::min(min.x, node.position.x);
        min.y = std::min(min.y, node.position.y);
        max.x = std::max(max.x, node.position.x + node.size.x);
        max.y = std::max(max.y, node.position.y + node.size.y);
    }

    Vec2 center = (min + max) * 0.5f;
    state.viewOffset = -center * state.zoom;
}

void nodeGraphSelectAll(std::vector<GraphNode>& nodes, NodeGraphState& state)
{
    state.selectedNodes.clear();
    for (auto& node : nodes)
    {
        node.selected = true;
        state.selectedNodes.push_back(&node);
    }
}

void nodeGraphDeselectAll(NodeGraphState& state)
{
    state.selectedNodes.clear();
}

void nodeGraphDeleteSelected(std::vector<GraphNode>& nodes, std::vector<NodeLink>& links, NodeGraphState& state)
{
    // Remove selected nodes
    nodes.erase(std::remove_if(nodes.begin(), nodes.end(), [](const GraphNode& n) { return n.selected; }), nodes.end());

    // Remove links connected to deleted nodes
    std::vector<uintptr_t> nodeIds;
    for (const auto& node : nodes)
    {
        nodeIds.push_back(node.id);
    }

    links.erase(std::remove_if(links.begin(), links.end(),
                               [&nodeIds](const NodeLink& link)
                               {
                                   return std::find(nodeIds.begin(), nodeIds.end(), link.sourceNodeId) ==
                                              nodeIds.end() ||
                                          std::find(nodeIds.begin(), nodeIds.end(), link.destNodeId) == nodeIds.end();
                               }),
                links.end());

    state.selectedNodes.clear();
}

}  // namespace dakt::gui

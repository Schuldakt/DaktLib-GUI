// ============================================================================
// DaktLib GUI Module - Container Widgets Implementation
// ============================================================================

#include <dakt/gui/Containers.hpp>
#include <dakt/gui/Context.hpp>
#include <dakt/gui/DrawList.hpp>
#include <dakt/gui/Theme.hpp>
#include <dakt/gui/Widgets.hpp>

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace dakt::gui
{

// ============================================================================
// Internal State
// ============================================================================

namespace
{

// Next window settings
struct NextWindowSettings
{
    Option<Vec2> pos;
    Option<Vec2> size;
    Option<Vec2> contentSize;
    Option<bool> collapsed;
    Option<f32> bgAlpha;
    bool focus = false;

    void clear()
    {
        pos.reset();
        size.reset();
        contentSize.reset();
        collapsed.reset();
        bgAlpha.reset();
        focus = false;
    }
};

NextWindowSettings g_nextWindowSettings;

// Panel stack for nested panels
struct PanelState
{
    WidgetId id;
    Rect bounds;
    Vec2 contentStart;
    PanelStyle style;
};

std::vector<PanelState> g_panelStack;

// Scroll area states
std::unordered_map<WidgetId, ScrollState> g_scrollStates;

// Tab bar state
struct TabBarState
{
    WidgetId id;
    i32 selectedTab = 0;
    i32 hoveredTab = -1;
    f32 scrollOffset = 0.0f;
    std::vector<StringView> tabLabels;
    Rect bounds;
};

std::unordered_map<WidgetId, TabBarState> g_tabBarStates;
TabBarState* g_currentTabBar = nullptr;

// Tree state
std::unordered_map<WidgetId, bool> g_treeNodeStates;
i32 g_treeIndentLevel = 0;

// Grid state
struct GridState
{
    WidgetId id;
    GridConfig config;
    i32 currentColumn = 0;
    i32 currentRow = 0;
    f32 rowHeight = 0.0f;
    Vec2 startPos;
    std::vector<f32> columnWidths;
};

GridState* g_currentGrid = nullptr;
std::unordered_map<WidgetId, GridState> g_gridStates;

// Table state
struct TableState
{
    WidgetId id;
    i32 columnCount = 0;
    i32 currentColumn = -1;
    i32 currentRow = -1;
    TableFlags flags = TableFlags::None;
    std::vector<StringView> columnLabels;
    std::vector<TableColumnFlags> columnFlags;
    std::vector<f32> columnWidths;
    Vec2 startPos;
    f32 rowHeight = 20.0f;
    bool headersDone = false;
};

TableState* g_currentTable = nullptr;
std::unordered_map<WidgetId, TableState> g_tableStates;

// Menu state
bool g_menuBarActive = false;
bool g_mainMenuBarActive = false;
i32 g_menuDepth = 0;

// Group state
struct GroupState
{
    Vec2 startPos;
    Vec2 minPos;
    Vec2 maxPos;
};

std::vector<GroupState> g_groupStack;

// Popup state
struct PopupState
{
    WidgetId id;
    bool open = false;
    Vec2 openPos;
};

std::unordered_map<WidgetId, PopupState> g_popupStates;
std::vector<WidgetId> g_openPopups;

}  // anonymous namespace

// ============================================================================
// Window Functions
// ============================================================================

bool begin(StringView name, bool* open, WindowFlags flags)
{
    (void)flags;  // TODO: Use flags for window behavior

    auto* ctx = getContext();
    if (!ctx)
        return false;

    // Apply next window settings
    if (g_nextWindowSettings.pos)
    {
        setWindowPos(name, *g_nextWindowSettings.pos);
    }
    if (g_nextWindowSettings.size)
    {
        setWindowSize(name, *g_nextWindowSettings.size);
    }

    g_nextWindowSettings.clear();

    // Begin window in context
    bool visible = ctx->beginWindow(name, open);

    return visible;
}

void end()
{
    auto* ctx = getContext();
    if (ctx)
    {
        ctx->endWindow();
    }
}

Vec2 getWindowPos()
{
    auto* ctx = getContext();
    auto* window = ctx ? ctx->getCurrentWindow() : nullptr;
    return window ? window->rect.min : Vec2{0, 0};
}

Vec2 getWindowSize()
{
    auto* ctx = getContext();
    auto* window = ctx ? ctx->getCurrentWindow() : nullptr;
    return window ? window->rect.size() : Vec2{0, 0};
}

f32 getWindowWidth()
{
    return getWindowSize().x;
}

f32 getWindowHeight()
{
    return getWindowSize().y;
}

Vec2 getWindowContentRegionMin()
{
    auto* ctx = getContext();
    auto* window = ctx ? ctx->getCurrentWindow() : nullptr;
    return window ? window->contentRect.min : Vec2{0, 0};
}

Vec2 getWindowContentRegionMax()
{
    auto* ctx = getContext();
    auto* window = ctx ? ctx->getCurrentWindow() : nullptr;
    return window ? window->contentRect.max : Vec2{0, 0};
}

f32 getWindowContentRegionWidth()
{
    Vec2 min = getWindowContentRegionMin();
    Vec2 max = getWindowContentRegionMax();
    return max.x - min.x;
}

void setWindowPos(Vec2 pos)
{
    auto* ctx = getContext();
    auto* window = ctx ? ctx->getCurrentWindow() : nullptr;
    if (window)
    {
        Vec2 size = window->rect.size();
        window->rect = Rect{pos, pos + size};
    }
}

void setWindowSize(Vec2 size)
{
    auto* ctx = getContext();
    auto* window = ctx ? ctx->getCurrentWindow() : nullptr;
    if (window)
    {
        window->rect.max = window->rect.min + size;
    }
}

void setWindowCollapsed(bool collapsed)
{
    auto* ctx = getContext();
    auto* window = ctx ? ctx->getCurrentWindow() : nullptr;
    if (window)
    {
        window->collapsed = collapsed;
    }
}

void setWindowFocus()
{
    // TODO: Implement window focus ordering
}

void setWindowPos(StringView name, Vec2 pos)
{
    auto* ctx = getContext();
    if (ctx)
    {
        auto* window = ctx->findWindow(name);
        if (window)
        {
            Vec2 size = window->rect.size();
            window->rect = Rect{pos, pos + size};
        }
    }
}

void setWindowSize(StringView name, Vec2 size)
{
    auto* ctx = getContext();
    if (ctx)
    {
        auto* window = ctx->findWindow(name);
        if (window)
        {
            window->rect.max = window->rect.min + size;
        }
    }
}

void setWindowCollapsed(StringView name, bool collapsed)
{
    auto* ctx = getContext();
    if (ctx)
    {
        auto* window = ctx->findWindow(name);
        if (window)
        {
            window->collapsed = collapsed;
        }
    }
}

void setWindowFocus(StringView name)
{
    (void)name;
    // TODO: Implement window focus by name
}

void setNextWindowPos(Vec2 pos)
{
    g_nextWindowSettings.pos = pos;
}

void setNextWindowSize(Vec2 size)
{
    g_nextWindowSettings.size = size;
}

void setNextWindowContentSize(Vec2 size)
{
    g_nextWindowSettings.contentSize = size;
}

void setNextWindowCollapsed(bool collapsed)
{
    g_nextWindowSettings.collapsed = collapsed;
}

void setNextWindowFocus()
{
    g_nextWindowSettings.focus = true;
}

void setNextWindowBgAlpha(f32 alpha)
{
    g_nextWindowSettings.bgAlpha = alpha;
}

// ============================================================================
// Child Windows
// ============================================================================

bool beginChild(StringView id, Vec2 size, bool border, WindowFlags flags)
{
    auto* ctx = getContext();
    if (!ctx)
        return false;

    WidgetId childId = ctx->getId(id);
    (void)childId;  // TODO: Use for child window state
    auto* theme = ctx->getTheme();

    Vec2 pos = ctx->getCursorScreenPos();
    Vec2 contentSize = ctx->getCurrentWindow()->contentRect.size();

    Vec2 childSize{size.x > 0 ? size.x : contentSize.x, size.y > 0 ? size.y : contentSize.y};

    Rect childBounds{pos, pos + childSize};

    // Draw child background
    if (!hasFlag(flags, WindowFlags::NoBackground))
    {
        ctx->getDrawList().addRectFilled(childBounds, theme->childBg);
    }

    if (border)
    {
        ctx->getDrawList().addRect(childBounds, theme->border);
    }

    // Push clip rect
    ctx->getDrawList().pushClipRect(childBounds);

    // Update cursor position
    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.cursor = pos + Vec2{4, 4};
    }

    return true;
}

void endChild()
{
    auto* ctx = getContext();
    if (ctx)
    {
        ctx->getDrawList().popClipRect();
    }
}

// ============================================================================
// Panel Functions
// ============================================================================

bool beginPanel(StringView id, Vec2 size, const PanelStyle& style)
{
    auto* ctx = getContext();
    if (!ctx)
        return false;

    WidgetId panelId = ctx->getId(id);

    Vec2 pos = ctx->getCursorScreenPos();
    Vec2 contentWidth = Vec2{ctx->getContentWidth(), ctx->getContentWidth()};

    Vec2 panelSize{size.x > 0 ? size.x : contentWidth.x, size.y > 0 ? size.y : 100.0f};

    Rect bounds{pos, pos + panelSize};

    // Draw panel background
    if (style.drawBackground)
    {
        ctx->getDrawList().addRectFilled(bounds, style.background, style.rounding);
    }

    if (style.drawBorder)
    {
        ctx->getDrawList().addRect(bounds, style.border, style.rounding, Corner::All, style.borderWidth);
    }

    // Push panel state
    PanelState state;
    state.id = panelId;
    state.bounds = bounds;
    state.contentStart = pos + style.padding;
    state.style = style;
    g_panelStack.push_back(state);

    // Set cursor to content area
    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.cursor = state.contentStart;
    }

    // Push clip rect
    Rect contentRect{pos + style.padding, pos + panelSize - style.padding};
    ctx->getDrawList().pushClipRect(contentRect);

    return true;
}

void endPanel()
{
    auto* ctx = getContext();
    if (!ctx || g_panelStack.empty())
        return;

    PanelState& state = g_panelStack.back();

    // Pop clip rect
    ctx->getDrawList().popClipRect();

    // Advance cursor past panel
    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(state.bounds.size());
    }

    g_panelStack.pop_back();
}

// ============================================================================
// Scroll Area Functions
// ============================================================================

bool beginScrollArea(StringView id, Vec2 size, bool horizontalScroll)
{
    auto* ctx = getContext();
    if (!ctx)
        return false;

    WidgetId scrollId = ctx->getId(id);
    auto* theme = ctx->getTheme();

    // Get or create scroll state
    ScrollState& state = g_scrollStates[scrollId];

    Vec2 pos = ctx->getCursorScreenPos();
    Vec2 areaSize{size.x > 0 ? size.x : ctx->getContentWidth(), size.y > 0 ? size.y : 200.0f};

    Rect bounds{pos, pos + areaSize};

    // Determine scrollbar visibility
    f32 scrollbarWidth = 12.0f;
    state.showScrollbarY = state.contentSize.y > areaSize.y;
    state.showScrollbarX = horizontalScroll && state.contentSize.x > areaSize.x;

    // Adjust content area for scrollbars
    Rect contentBounds = bounds;
    if (state.showScrollbarY)
        contentBounds.max.x -= scrollbarWidth;
    if (state.showScrollbarX)
        contentBounds.max.y -= scrollbarWidth;

    // Draw background
    ctx->getDrawList().addRectFilled(bounds, theme->childBg);

    // Handle scroll input
    if (bounds.contains(ctx->getMousePos()))
    {
        f32 wheelY = ctx->getMouseWheel();
        f32 wheelX = ctx->getMouseWheelH();
        state.scroll.y -= wheelY * 20.0f;
        if (horizontalScroll)
        {
            state.scroll.x -= wheelX * 20.0f;
        }
    }

    // Clamp scroll
    state.scrollMax.x = std::max(0.0f, state.contentSize.x - contentBounds.width());
    state.scrollMax.y = std::max(0.0f, state.contentSize.y - contentBounds.height());
    state.scroll.x = std::clamp(state.scroll.x, 0.0f, state.scrollMax.x);
    state.scroll.y = std::clamp(state.scroll.y, 0.0f, state.scrollMax.y);

    // Draw scrollbars
    if (state.showScrollbarY)
    {
        Rect scrollbarBounds{{bounds.max.x - scrollbarWidth, bounds.min.y},
                             {bounds.max.x, bounds.max.y - (state.showScrollbarX ? scrollbarWidth : 0)}};
        ctx->getDrawList().addRectFilled(scrollbarBounds, theme->scrollbarBg);

        f32 scrollRatio = state.scroll.y / state.scrollMax.y;
        f32 thumbHeight = std::max(20.0f, scrollbarBounds.height() * (contentBounds.height() / state.contentSize.y));
        f32 thumbY = scrollbarBounds.min.y + scrollRatio * (scrollbarBounds.height() - thumbHeight);

        Rect thumbRect{{scrollbarBounds.min.x + 2, thumbY}, {scrollbarBounds.max.x - 2, thumbY + thumbHeight}};
        ctx->getDrawList().addRectFilled(thumbRect, theme->scrollbarGrab, 4.0f);
    }

    if (state.showScrollbarX)
    {
        Rect scrollbarBounds{{bounds.min.x, bounds.max.y - scrollbarWidth},
                             {bounds.max.x - (state.showScrollbarY ? scrollbarWidth : 0), bounds.max.y}};
        ctx->getDrawList().addRectFilled(scrollbarBounds, theme->scrollbarBg);

        f32 scrollRatio = state.scroll.x / state.scrollMax.x;
        f32 thumbWidth = std::max(20.0f, scrollbarBounds.width() * (contentBounds.width() / state.contentSize.x));
        f32 thumbX = scrollbarBounds.min.x + scrollRatio * (scrollbarBounds.width() - thumbWidth);

        Rect thumbRect{{thumbX, scrollbarBounds.min.y + 2}, {thumbX + thumbWidth, scrollbarBounds.max.y - 2}};
        ctx->getDrawList().addRectFilled(thumbRect, theme->scrollbarGrab, 4.0f);
    }

    // Push clip rect and adjust cursor
    ctx->getDrawList().pushClipRect(contentBounds);

    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.cursor = contentBounds.min - state.scroll;
    }

    // Reset content size tracking
    state.contentSize = Vec2{0, 0};

    return true;
}

void endScrollArea()
{
    auto* ctx = getContext();
    if (ctx)
    {
        ctx->getDrawList().popClipRect();
    }
}

Vec2 getScrollPos()
{
    // Return current scroll area's position
    return Vec2{0, 0};  // TODO: Track current scroll area
}

void setScrollPos(Vec2 pos)
{
    (void)pos;
    // TODO: Set scroll position
}

void setScrollHereX(f32 centerRatio)
{
    (void)centerRatio;
}
void setScrollHereY(f32 centerRatio)
{
    (void)centerRatio;
}
void setScrollFromPosX(f32 localX, f32 centerRatio)
{
    (void)localX;
    (void)centerRatio;
}
void setScrollFromPosY(f32 localY, f32 centerRatio)
{
    (void)localY;
    (void)centerRatio;
}
f32 getScrollX()
{
    return 0.0f;
}
f32 getScrollY()
{
    return 0.0f;
}
f32 getScrollMaxX()
{
    return 0.0f;
}
f32 getScrollMaxY()
{
    return 0.0f;
}

// ============================================================================
// Tab Bar Functions
// ============================================================================

bool beginTabBar(StringView id, TabBarFlags flags)
{
    (void)flags;

    auto* ctx = getContext();
    if (!ctx)
        return false;

    WidgetId tabBarId = ctx->getId(id);
    auto* theme = ctx->getTheme();

    TabBarState& state = g_tabBarStates[tabBarId];
    state.id = tabBarId;
    state.tabLabels.clear();

    Vec2 pos = ctx->getCursorScreenPos();
    f32 tabBarHeight = 24.0f;

    state.bounds = Rect{pos, pos + Vec2{ctx->getContentWidth(), tabBarHeight}};

    // Draw tab bar background
    ctx->getDrawList().addRectFilled(state.bounds, theme->frameBg, 0, Corner::Top);

    g_currentTabBar = &state;

    return true;
}

void endTabBar()
{
    auto* ctx = getContext();
    if (!ctx || !g_currentTabBar)
        return;

    // Advance cursor past tab bar
    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(Vec2{g_currentTabBar->bounds.width(), g_currentTabBar->bounds.height()});
    }

    g_currentTabBar = nullptr;
}

bool beginTabItem(StringView label, bool* open, TabItemFlags flags)
{
    (void)flags;

    auto* ctx = getContext();
    if (!ctx || !g_currentTabBar)
        return false;

    auto* theme = ctx->getTheme();
    i32 tabIndex = static_cast<i32>(g_currentTabBar->tabLabels.size());
    g_currentTabBar->tabLabels.push_back(label);

    // Calculate tab position
    f32 tabWidth = 100.0f;
    f32 tabX = g_currentTabBar->bounds.min.x + tabIndex * tabWidth;

    Rect tabRect{{tabX, g_currentTabBar->bounds.min.y}, {tabX + tabWidth, g_currentTabBar->bounds.max.y}};

    bool selected = (tabIndex == g_currentTabBar->selectedTab);
    bool hovered = tabRect.contains(ctx->getMousePos());

    // Handle click
    if (hovered && ctx->isMouseClicked(MouseButton::Left))
    {
        g_currentTabBar->selectedTab = tabIndex;
        selected = true;
    }

    // Handle close button
    if (open && hovered && ctx->isMouseClicked(MouseButton::Middle))
    {
        *open = false;
    }

    // Draw tab
    Color tabColor = selected ? theme->tab : (hovered ? theme->tabHovered : theme->tabUnfocused);
    ctx->getDrawList().addRectFilled(tabRect, tabColor, 4.0f, Corner::Top);

    // Draw label
    auto* font = ctx->getFont();
    Vec2 textSize = font ? font->calcTextSize(label) : Vec2{60, 14};
    Vec2 textPos = tabRect.center() - textSize * 0.5f;
    ctx->getDrawList().addText(textPos, label, selected ? theme->text : theme->textDisabled);

    // Draw close button if closeable
    if (open)
    {
        Vec2 closePos = tabRect.max - Vec2{16, tabRect.height() * 0.5f + 4};
        Rect closeRect{closePos, closePos + Vec2{8, 8}};
        if (closeRect.contains(ctx->getMousePos()))
        {
            ctx->getDrawList().addRectFilled(closeRect, theme->buttonHovered);
        }
        ctx->getDrawList().addLine(closeRect.min, closeRect.max, theme->text);
        ctx->getDrawList().addLine({closeRect.min.x, closeRect.max.y}, {closeRect.max.x, closeRect.min.y}, theme->text);
    }

    return selected;
}

void endTabItem()
{
    // No-op for now
}

void setTabItemClosed(StringView tabOrDockedWindowLabel)
{
    (void)tabOrDockedWindowLabel;
    // TODO: Implement
}

// ============================================================================
// Tree Functions
// ============================================================================

bool treeNode(StringView label, TreeNodeFlags flags)
{
    auto* ctx = getContext();
    if (!ctx)
        return false;

    WidgetId nodeId = ctx->getId(label);
    auto* theme = ctx->getTheme();

    // Get or create state
    bool& isOpen = g_treeNodeStates[nodeId];
    if (hasFlag(flags, TreeNodeFlags::DefaultOpen) && g_treeNodeStates.find(nodeId) == g_treeNodeStates.end())
    {
        isOpen = true;
    }

    bool isLeaf = hasFlag(flags, TreeNodeFlags::Leaf);
    bool isSelected = hasFlag(flags, TreeNodeFlags::Selected);

    Vec2 pos = ctx->getCursorScreenPos();
    f32 indentWidth = theme->indentSpacing * g_treeIndentLevel;
    pos.x += indentWidth;

    f32 arrowSize = 12.0f;
    f32 height = 20.0f;

    auto* font = ctx->getFont();
    Vec2 textSize = font ? font->calcTextSize(label) : Vec2{80, 14};

    Rect nodeRect{pos, pos + Vec2{textSize.x + arrowSize + 8, height}};
    bool hovered = nodeRect.contains(ctx->getMousePos());

    // Handle click
    if (hovered && ctx->isMouseClicked(MouseButton::Left))
    {
        if (!isLeaf)
        {
            isOpen = !isOpen;
        }
    }

    // Draw background if selected or hovered
    if (isSelected || hovered)
    {
        Color bgColor = isSelected ? theme->header : theme->headerHovered;
        ctx->getDrawList().addRectFilled(nodeRect, bgColor);
    }

    // Draw arrow
    if (!isLeaf)
    {
        Vec2 arrowCenter = pos + Vec2{arrowSize * 0.5f, height * 0.5f};
        if (isOpen)
        {
            // Down arrow
            ctx->getDrawList().addTriangleFilled(arrowCenter + Vec2{0, 3}, arrowCenter + Vec2{-4, -2},
                                                 arrowCenter + Vec2{4, -2}, theme->text);
        }
        else
        {
            // Right arrow
            ctx->getDrawList().addTriangleFilled(arrowCenter + Vec2{3, 0}, arrowCenter + Vec2{-2, -4},
                                                 arrowCenter + Vec2{-2, 4}, theme->text);
        }
    }

    // Draw label
    Vec2 textPos = pos + Vec2{arrowSize + 4, (height - textSize.y) * 0.5f};
    ctx->getDrawList().addText(textPos, label, theme->text);

    // Advance cursor
    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(Vec2{ctx->getContentWidth() - indentWidth, height});
    }

    if (isOpen && !isLeaf)
    {
        g_treeIndentLevel++;
    }

    return isOpen;
}

bool treeNode(StringView strId, StringView label, TreeNodeFlags flags)
{
    auto* ctx = getContext();
    if (!ctx)
        return false;

    ctx->pushId(strId);
    bool result = treeNode(label, flags);
    ctx->popId();
    return result;
}

bool treeNodeEx(StringView label, TreeNodeFlags flags)
{
    return treeNode(label, flags);
}

void treePush(StringView strId)
{
    auto* ctx = getContext();
    if (ctx)
    {
        ctx->pushId(strId);
        g_treeIndentLevel++;
    }
}

void treePop()
{
    auto* ctx = getContext();
    if (ctx)
    {
        ctx->popId();
        if (g_treeIndentLevel > 0)
        {
            g_treeIndentLevel--;
        }
    }
}

f32 getTreeNodeToLabelSpacing()
{
    return 12.0f + 4.0f;  // Arrow size + padding
}

bool collapsingHeader(StringView label, TreeNodeFlags flags)
{
    return treeNode(label, flags | TreeNodeFlags::CollapsingHeader);
}

bool collapsingHeader(StringView label, bool* open, TreeNodeFlags flags)
{
    if (open && !*open)
    {
        return false;
    }
    bool result = collapsingHeader(label, flags);
    if (open)
    {
        *open = result;
    }
    return result;
}

// ============================================================================
// Group Functions
// ============================================================================

void beginGroup()
{
    auto* ctx = getContext();
    if (!ctx)
        return;

    GroupState state;
    state.startPos = ctx->getCursorScreenPos();
    state.minPos = state.startPos;
    state.maxPos = state.startPos;
    g_groupStack.push_back(state);
}

void endGroup()
{
    auto* ctx = getContext();
    if (!ctx || g_groupStack.empty())
        return;

    g_groupStack.pop_back();
}

void dummy(Vec2 size)
{
    auto* ctx = getContext();
    if (!ctx)
        return;

    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(size);
    }
}

void separator()
{
    auto* ctx = getContext();
    if (!ctx)
        return;

    auto* theme = ctx->getTheme();
    Vec2 pos = ctx->getCursorScreenPos();
    f32 width = ctx->getContentWidth();

    ctx->getDrawList().addLine(pos, pos + Vec2{width, 0}, theme->separator);

    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(Vec2{width, theme->itemSpacing.y});
    }
}

void spacing()
{
    auto* ctx = getContext();
    if (!ctx)
        return;

    auto* theme = ctx->getTheme();
    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(Vec2{0, theme->itemSpacing.y});
    }
}

void sameLine(f32 offsetFromStartX, f32 spacing)
{
    auto* ctx = getContext();
    if (!ctx)
        return;

    ctx->sameLine(offsetFromStartX, spacing);
}

void newLine()
{
    auto* ctx = getContext();
    if (!ctx)
        return;

    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.cursor.x = window->contentRect.min.x;
        window->layout.cursor.y += window->layout.currentLineHeight + ctx->getTheme()->itemSpacing.y;
        window->layout.currentLineHeight = 0;
    }
}

void indent(f32 indentWidth)
{
    auto* ctx = getContext();
    if (!ctx)
        return;

    f32 amount = indentWidth > 0 ? indentWidth : ctx->getTheme()->indentSpacing;
    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.indentLevel += amount;
        window->layout.cursor.x += amount;
    }
}

void unindent(f32 indentWidth)
{
    auto* ctx = getContext();
    if (!ctx)
        return;

    f32 amount = indentWidth > 0 ? indentWidth : ctx->getTheme()->indentSpacing;
    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.indentLevel -= amount;
        window->layout.cursor.x -= amount;
    }
}

// ============================================================================
// Menu Functions
// ============================================================================

bool beginMainMenuBar()
{
    auto* ctx = getContext();
    if (!ctx)
        return false;

    auto* theme = ctx->getTheme();
    f32 menuBarHeight = 20.0f;

    Rect menuBarRect{{0, 0}, {static_cast<f32>(ctx->getDisplaySize().x), menuBarHeight}};
    ctx->getDrawList().addRectFilled(menuBarRect, theme->menuBarBg);

    g_mainMenuBarActive = true;

    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.cursor = Vec2{4, 2};
    }

    return true;
}

void endMainMenuBar()
{
    g_mainMenuBarActive = false;
}

bool beginMenuBar()
{
    auto* ctx = getContext();
    if (!ctx)
        return false;

    g_menuBarActive = true;
    return true;
}

void endMenuBar()
{
    g_menuBarActive = false;
}

bool beginMenu(StringView label, bool enabled)
{
    auto* ctx = getContext();
    if (!ctx)
        return false;

    auto* theme = ctx->getTheme();
    auto* font = ctx->getFont();

    Vec2 pos = ctx->getCursorScreenPos();
    Vec2 textSize = font ? font->calcTextSize(label) : Vec2{60, 14};
    Vec2 menuSize = textSize + Vec2{16, 4};

    Rect menuRect{pos, pos + menuSize};
    bool hovered = menuRect.contains(ctx->getMousePos());

    Color bgColor = hovered ? theme->headerHovered : Color{0, 0, 0, 0};
    if (hovered)
    {
        ctx->getDrawList().addRectFilled(menuRect, bgColor);
    }

    Color textColor = enabled ? theme->text : theme->textDisabled;
    Vec2 textPos = pos + Vec2{8, 2};
    ctx->getDrawList().addText(textPos, label, textColor);

    ctx->sameLine();

    bool clicked = hovered && ctx->isMouseClicked(MouseButton::Left) && enabled;

    if (clicked)
    {
        g_menuDepth++;
    }

    return clicked;
}

void endMenu()
{
    if (g_menuDepth > 0)
    {
        g_menuDepth--;
    }
}

bool menuItem(StringView label, StringView shortcut, bool selected, bool enabled)
{
    auto* ctx = getContext();
    if (!ctx)
        return false;

    auto* theme = ctx->getTheme();
    auto* font = ctx->getFont();

    Vec2 pos = ctx->getCursorScreenPos();
    Vec2 textSize = font ? font->calcTextSize(label) : Vec2{80, 14};
    f32 itemHeight = 20.0f;
    f32 itemWidth = ctx->getContentWidth();

    Rect itemRect{pos, pos + Vec2{itemWidth, itemHeight}};
    bool hovered = itemRect.contains(ctx->getMousePos());

    // Draw background
    if (hovered)
    {
        ctx->getDrawList().addRectFilled(itemRect, theme->headerHovered);
    }

    // Draw checkmark if selected
    if (selected)
    {
        Vec2 checkPos = pos + Vec2{4, (itemHeight - 14) * 0.5f};
        ctx->getDrawList().addText(checkPos, "✓", theme->text);
    }

    // Draw label
    Color textColor = enabled ? theme->text : theme->textDisabled;
    Vec2 textPos = pos + Vec2{selected ? 20.0f : 4.0f, (itemHeight - textSize.y) * 0.5f};
    ctx->getDrawList().addText(textPos, label, textColor);

    // Draw shortcut
    if (!shortcut.empty())
    {
        Vec2 shortcutSize = font ? font->calcTextSize(shortcut) : Vec2{40, 14};
        Vec2 shortcutPos = pos + Vec2{itemWidth - shortcutSize.x - 4, (itemHeight - shortcutSize.y) * 0.5f};
        ctx->getDrawList().addText(shortcutPos, shortcut, theme->textDisabled);
    }

    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(Vec2{itemWidth, itemHeight});
    }

    return hovered && ctx->isMouseClicked(MouseButton::Left) && enabled;
}

bool menuItem(StringView label, StringView shortcut, bool* selected, bool enabled)
{
    bool pressed = menuItem(label, shortcut, selected ? *selected : false, enabled);
    if (pressed && selected)
    {
        *selected = !*selected;
    }
    return pressed;
}

// ============================================================================
// Popup Functions
// ============================================================================

void openPopup(StringView strId)
{
    auto* ctx = getContext();
    if (!ctx)
        return;

    WidgetId id = ctx->getId(strId);
    PopupState& state = g_popupStates[id];
    state.id = id;
    state.open = true;
    state.openPos = ctx->getMousePos();
    g_openPopups.push_back(id);
}

bool beginPopup(StringView strId, WindowFlags flags)
{
    auto* ctx = getContext();
    if (!ctx)
        return false;

    WidgetId id = ctx->getId(strId);
    auto it = g_popupStates.find(id);
    if (it == g_popupStates.end() || !it->second.open)
    {
        return false;
    }

    setNextWindowPos(it->second.openPos);
    return begin(strId, nullptr, flags | WindowFlags::NoTitleBar | WindowFlags::NoResize);
}

bool beginPopupModal(StringView name, bool* open, WindowFlags flags)
{
    auto* ctx = getContext();
    if (!ctx)
        return false;

    // Draw dimmed background
    ctx->getDrawList().addRectFilled(
        Rect{{0, 0}, Vec2{static_cast<f32>(ctx->getDisplaySize().x), static_cast<f32>(ctx->getDisplaySize().y)}},
        Color::fromFloat(0.0f, 0.0f, 0.0f, 0.5f));

    setNextWindowPos(Vec2{static_cast<f32>(ctx->getDisplaySize().x) * 0.5f - 150,
                          static_cast<f32>(ctx->getDisplaySize().y) * 0.5f - 100});

    return begin(name, open, flags);
}

bool beginPopupContextItem(StringView strId, i32 mouseButton)
{
    auto* ctx = getContext();
    if (!ctx)
        return false;

    if (ctx->isMouseClicked(static_cast<MouseButton>(mouseButton)))
    {
        openPopup(strId.empty() ? "context" : strId);
    }

    return beginPopup(strId.empty() ? "context" : strId);
}

bool beginPopupContextWindow(StringView strId, i32 mouseButton)
{
    return beginPopupContextItem(strId, mouseButton);
}

bool beginPopupContextVoid(StringView strId, i32 mouseButton)
{
    return beginPopupContextItem(strId, mouseButton);
}

void endPopup()
{
    end();
}

void closeCurrentPopup()
{
    if (!g_openPopups.empty())
    {
        WidgetId id = g_openPopups.back();
        g_popupStates[id].open = false;
        g_openPopups.pop_back();
    }
}

bool isPopupOpen(StringView strId)
{
    auto* ctx = getContext();
    if (!ctx)
        return false;

    WidgetId id = ctx->getId(strId);
    auto it = g_popupStates.find(id);
    return it != g_popupStates.end() && it->second.open;
}

// ============================================================================
// Grid Layout
// ============================================================================

bool beginGrid(StringView id, const GridConfig& config)
{
    auto* ctx = getContext();
    if (!ctx)
        return false;

    WidgetId gridId = ctx->getId(id);
    GridState& state = g_gridStates[gridId];

    state.id = gridId;
    state.config = config;
    state.currentColumn = 0;
    state.currentRow = 0;
    state.rowHeight = 0;
    state.startPos = ctx->getCursorScreenPos();

    // Calculate column widths
    state.columnWidths.resize(config.columns);
    f32 totalWidth = ctx->getContentWidth();
    f32 columnWidth = config.columnWidth > 0 ? config.columnWidth : totalWidth / config.columns;

    for (i32 i = 0; i < config.columns; i++)
    {
        state.columnWidths[i] = columnWidth;
    }

    g_currentGrid = &state;

    return true;
}

void endGrid()
{
    auto* ctx = getContext();
    if (!ctx || !g_currentGrid)
        return;

    // Calculate total height
    f32 totalHeight =
        g_currentGrid->startPos.y +
        (g_currentGrid->currentRow + 1) *
            (g_currentGrid->config.rowHeight > 0 ? g_currentGrid->config.rowHeight : g_currentGrid->rowHeight) -
        g_currentGrid->startPos.y;

    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(Vec2{ctx->getContentWidth(), totalHeight});
    }

    g_currentGrid = nullptr;
}

void nextCell()
{
    if (!g_currentGrid)
        return;

    g_currentGrid->currentColumn++;
    if (g_currentGrid->currentColumn >= g_currentGrid->config.columns)
    {
        g_currentGrid->currentColumn = 0;
        g_currentGrid->currentRow++;
    }

    // Update cursor position
    auto* ctx = getContext();
    if (ctx)
    {
        f32 x = g_currentGrid->startPos.x;
        for (i32 i = 0; i < g_currentGrid->currentColumn; i++)
        {
            x += g_currentGrid->columnWidths[i] + g_currentGrid->config.cellPadding.x * 2;
        }

        f32 rowHeight = g_currentGrid->config.rowHeight > 0 ? g_currentGrid->config.rowHeight : 20.0f;
        f32 y = g_currentGrid->startPos.y +
                g_currentGrid->currentRow * (rowHeight + g_currentGrid->config.cellPadding.y * 2);

        auto* window = ctx->getCurrentWindow();
        if (window)
        {
            window->layout.cursor =
                Vec2{x + g_currentGrid->config.cellPadding.x, y + g_currentGrid->config.cellPadding.y};
        }
    }
}

void setColumnWidth(i32 column, f32 width)
{
    if (g_currentGrid && column >= 0 && column < static_cast<i32>(g_currentGrid->columnWidths.size()))
    {
        g_currentGrid->columnWidths[column] = width;
    }
}

// ============================================================================
// Table Functions
// ============================================================================

bool beginTable(StringView strId, i32 columns, TableFlags flags, Vec2 outerSize, f32 innerWidth)
{
    (void)outerSize;
    (void)innerWidth;

    auto* ctx = getContext();
    if (!ctx)
        return false;

    WidgetId tableId = ctx->getId(strId);
    TableState& state = g_tableStates[tableId];

    state.id = tableId;
    state.columnCount = columns;
    state.currentColumn = -1;
    state.currentRow = -1;
    state.flags = flags;
    state.columnLabels.clear();
    state.columnLabels.resize(columns);
    state.columnFlags.resize(columns, TableColumnFlags::None);
    state.columnWidths.resize(columns, ctx->getContentWidth() / columns);
    state.startPos = ctx->getCursorScreenPos();
    state.headersDone = false;

    g_currentTable = &state;

    return true;
}

void endTable()
{
    auto* ctx = getContext();
    if (!ctx || !g_currentTable)
        return;

    f32 totalHeight = (g_currentTable->currentRow + 1) * g_currentTable->rowHeight;

    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.advanceCursor(Vec2{ctx->getContentWidth(), totalHeight});
    }

    g_currentTable = nullptr;
}

void tableSetupColumn(StringView label, TableColumnFlags flags, f32 initWidthOrWeight, u32 userId)
{
    (void)userId;

    if (!g_currentTable)
        return;

    i32 columnIndex = static_cast<i32>(g_currentTable->columnLabels.size());
    if (columnIndex < g_currentTable->columnCount)
    {
        g_currentTable->columnLabels[columnIndex] = label;
        g_currentTable->columnFlags[columnIndex] = flags;
        if (initWidthOrWeight > 0)
        {
            g_currentTable->columnWidths[columnIndex] = initWidthOrWeight;
        }
    }
}

void tableSetupScrollFreeze(i32 cols, i32 rows)
{
    (void)cols;
    (void)rows;
    // TODO: Implement scroll freeze
}

void tableHeadersRow()
{
    if (!g_currentTable)
        return;

    auto* ctx = getContext();
    if (!ctx)
        return;

    auto* theme = ctx->getTheme();

    g_currentTable->currentRow = 0;

    f32 x = g_currentTable->startPos.x;
    f32 y = g_currentTable->startPos.y;

    // Draw header background
    Rect headerRect{{x, y}, {x + ctx->getContentWidth(), y + g_currentTable->rowHeight}};
    ctx->getDrawList().addRectFilled(headerRect, theme->tableHeaderBg);

    // Draw column headers
    for (i32 i = 0; i < g_currentTable->columnCount; i++)
    {
        Rect cellRect{{x, y}, {x + g_currentTable->columnWidths[i], y + g_currentTable->rowHeight}};

        Vec2 textPos = cellRect.min + Vec2{4, (g_currentTable->rowHeight - 14) * 0.5f};
        ctx->getDrawList().addText(textPos, g_currentTable->columnLabels[i], theme->text);

        x += g_currentTable->columnWidths[i];
    }

    g_currentTable->headersDone = true;
}

void tableNextRow(TableRowFlags flags, f32 minRowHeight)
{
    (void)flags;

    if (!g_currentTable)
        return;

    g_currentTable->currentRow++;
    g_currentTable->currentColumn = -1;

    if (minRowHeight > g_currentTable->rowHeight)
    {
        g_currentTable->rowHeight = minRowHeight;
    }
}

bool tableNextColumn()
{
    if (!g_currentTable)
        return false;

    auto* ctx = getContext();
    if (!ctx)
        return false;

    g_currentTable->currentColumn++;

    if (g_currentTable->currentColumn >= g_currentTable->columnCount)
    {
        return false;
    }

    // Calculate cell position
    f32 x = g_currentTable->startPos.x;
    for (i32 i = 0; i < g_currentTable->currentColumn; i++)
    {
        x += g_currentTable->columnWidths[i];
    }

    f32 y = g_currentTable->startPos.y + g_currentTable->currentRow * g_currentTable->rowHeight;

    auto* window = ctx->getCurrentWindow();
    if (window)
    {
        window->layout.cursor = Vec2{x + 4, y + 2};
    }

    return true;
}

bool tableSetColumnIndex(i32 columnIndex)
{
    if (!g_currentTable || columnIndex < 0 || columnIndex >= g_currentTable->columnCount)
    {
        return false;
    }

    g_currentTable->currentColumn = columnIndex - 1;
    return tableNextColumn();
}

i32 tableGetColumnCount()
{
    return g_currentTable ? g_currentTable->columnCount : 0;
}

i32 tableGetColumnIndex()
{
    return g_currentTable ? g_currentTable->currentColumn : -1;
}

i32 tableGetRowIndex()
{
    return g_currentTable ? g_currentTable->currentRow : -1;
}

StringView tableGetColumnName(i32 columnIndex)
{
    if (!g_currentTable)
        return "";
    i32 idx = columnIndex >= 0 ? columnIndex : g_currentTable->currentColumn;
    if (idx >= 0 && idx < static_cast<i32>(g_currentTable->columnLabels.size()))
    {
        return g_currentTable->columnLabels[idx];
    }
    return "";
}

TableColumnFlags tableGetColumnFlags(i32 columnIndex)
{
    if (!g_currentTable)
        return TableColumnFlags::None;
    i32 idx = columnIndex >= 0 ? columnIndex : g_currentTable->currentColumn;
    if (idx >= 0 && idx < static_cast<i32>(g_currentTable->columnFlags.size()))
    {
        return g_currentTable->columnFlags[idx];
    }
    return TableColumnFlags::None;
}

void tableSetColumnEnabled(i32 columnIndex, bool enabled)
{
    (void)columnIndex;
    (void)enabled;
    // TODO: Implement
}

void tableSetBgColor(i32 target, Color color, i32 columnIndex)
{
    (void)target;
    (void)color;
    (void)columnIndex;
    // TODO: Implement
}

}  // namespace dakt::gui

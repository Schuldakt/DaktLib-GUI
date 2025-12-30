// ============================================================================
// DaktLib GUI Module - Container Widgets
// ============================================================================
// Provides container widgets: Window, Panel, ScrollArea, TabBar, TreeNode,
// Child regions, Groups, Collapsing headers, Menus
// ============================================================================

#pragma once

#include <dakt/core/Types.hpp>
#include <dakt/gui/core/Types.hpp>

#include <functional>

namespace dakt::gui
{

using namespace dakt::core;

// ============================================================================
// Window Flags
// ============================================================================

enum class WindowFlags : u32
{
    None = 0,
    NoTitleBar = 1 << 0,
    NoResize = 1 << 1,
    NoMove = 1 << 2,
    NoScrollbar = 1 << 3,
    NoScrollWithMouse = 1 << 4,
    NoCollapse = 1 << 5,
    AlwaysAutoResize = 1 << 6,
    NoBackground = 1 << 7,
    NoSavedSettings = 1 << 8,
    NoMouseInputs = 1 << 9,
    MenuBar = 1 << 10,
    HorizontalScrollbar = 1 << 11,
    NoFocusOnAppearing = 1 << 12,
    NoBringToFrontOnFocus = 1 << 13,
    AlwaysVerticalScrollbar = 1 << 14,
    AlwaysHorizontalScrollbar = 1 << 15,
    AlwaysUseWindowPadding = 1 << 16,
    NoNavInputs = 1 << 17,
    NoNavFocus = 1 << 18,
    UnsavedDocument = 1 << 19,
    NoNav = NoNavInputs | NoNavFocus,
    NoDecoration = NoTitleBar | NoResize | NoScrollbar | NoCollapse,
    NoInputs = NoMouseInputs | NoNavInputs | NoNavFocus,
};

inline WindowFlags operator|(WindowFlags a, WindowFlags b)
{
    return static_cast<WindowFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

inline WindowFlags operator&(WindowFlags a, WindowFlags b)
{
    return static_cast<WindowFlags>(static_cast<u32>(a) & static_cast<u32>(b));
}

inline bool hasFlag(WindowFlags flags, WindowFlags flag)
{
    return (static_cast<u32>(flags) & static_cast<u32>(flag)) != 0;
}

// ============================================================================
// Tab Bar Flags
// ============================================================================

enum class TabBarFlags : u32
{
    None = 0,
    Reorderable = 1 << 0,
    AutoSelectNewTabs = 1 << 1,
    TabListPopupButton = 1 << 2,
    NoCloseWithMiddleMouseButton = 1 << 3,
    NoTabListScrollingButtons = 1 << 4,
    NoTooltip = 1 << 5,
    FittingPolicyResizeDown = 1 << 6,
    FittingPolicyScroll = 1 << 7,
};

inline TabBarFlags operator|(TabBarFlags a, TabBarFlags b)
{
    return static_cast<TabBarFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

// ============================================================================
// Tab Item Flags
// ============================================================================

enum class TabItemFlags : u32
{
    None = 0,
    UnsavedDocument = 1 << 0,
    SetSelected = 1 << 1,
    NoCloseWithMiddleMouseButton = 1 << 2,
    NoPushId = 1 << 3,
    NoTooltip = 1 << 4,
    NoReorder = 1 << 5,
    Leading = 1 << 6,
    Trailing = 1 << 7,
};

inline TabItemFlags operator|(TabItemFlags a, TabItemFlags b)
{
    return static_cast<TabItemFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

// ============================================================================
// Tree Node Flags
// ============================================================================

enum class TreeNodeFlags : u32
{
    None = 0,
    Selected = 1 << 0,
    Framed = 1 << 1,
    AllowItemOverlap = 1 << 2,
    NoTreePushOnOpen = 1 << 3,
    NoAutoOpenOnLog = 1 << 4,
    DefaultOpen = 1 << 5,
    OpenOnDoubleClick = 1 << 6,
    OpenOnArrow = 1 << 7,
    Leaf = 1 << 8,
    Bullet = 1 << 9,
    FramePadding = 1 << 10,
    SpanAvailWidth = 1 << 11,
    SpanFullWidth = 1 << 12,
    NavLeftJumpsBackHere = 1 << 13,
    CollapsingHeader = Framed | NoTreePushOnOpen | NoAutoOpenOnLog,
};

inline TreeNodeFlags operator|(TreeNodeFlags a, TreeNodeFlags b)
{
    return static_cast<TreeNodeFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

inline bool hasFlag(TreeNodeFlags flags, TreeNodeFlags flag)
{
    return (static_cast<u32>(flags) & static_cast<u32>(flag)) != 0;
}

// ============================================================================
// Panel Style
// ============================================================================

struct PanelStyle
{
    Color background = Color::fromFloat(0.15f, 0.15f, 0.15f, 1.0f);
    Color border = Color::fromFloat(0.4f, 0.4f, 0.4f, 1.0f);
    f32 borderWidth = 1.0f;
    f32 rounding = 4.0f;
    Vec2 padding = {8.0f, 8.0f};
    bool drawBorder = true;
    bool drawBackground = true;
};

// ============================================================================
// Scroll Area State
// ============================================================================

struct ScrollState
{
    Vec2 scroll = {0.0f, 0.0f};
    Vec2 scrollMax = {0.0f, 0.0f};
    Vec2 contentSize = {0.0f, 0.0f};
    bool scrollingX = false;
    bool scrollingY = false;
    bool showScrollbarX = false;
    bool showScrollbarY = false;
};

// ============================================================================
// Window Functions
// ============================================================================

// Begin a new window
// Returns false if the window is collapsed or clipped
bool begin(StringView name, bool* open = nullptr, WindowFlags flags = WindowFlags::None);

// End the current window
void end();

// Get window properties
Vec2 getWindowPos();
Vec2 getWindowSize();
f32 getWindowWidth();
f32 getWindowHeight();
Vec2 getWindowContentRegionMin();
Vec2 getWindowContentRegionMax();
f32 getWindowContentRegionWidth();

// Set window properties (only call between begin/end)
void setWindowPos(Vec2 pos);
void setWindowSize(Vec2 size);
void setWindowCollapsed(bool collapsed);
void setWindowFocus();

// Set window properties by name (can call anytime)
void setWindowPos(StringView name, Vec2 pos);
void setWindowSize(StringView name, Vec2 size);
void setWindowCollapsed(StringView name, bool collapsed);
void setWindowFocus(StringView name);

// Set next window properties (must call before begin())
void setNextWindowPos(Vec2 pos);
void setNextWindowSize(Vec2 size);
void setNextWindowContentSize(Vec2 size);
void setNextWindowCollapsed(bool collapsed);
void setNextWindowFocus();
void setNextWindowBgAlpha(f32 alpha);

// ============================================================================
// Child Windows
// ============================================================================

// Begin a child region (scrollable subarea)
bool beginChild(StringView id, Vec2 size = {0, 0}, bool border = false, WindowFlags flags = WindowFlags::None);
void endChild();

// ============================================================================
// Panel Functions
// ============================================================================

// Begin a simple panel (styled container without window functionality)
bool beginPanel(StringView id, Vec2 size = {0, 0}, const PanelStyle& style = {});
void endPanel();

// ============================================================================
// Scroll Area Functions
// ============================================================================

// Begin a scroll area with automatic scrollbars
bool beginScrollArea(StringView id, Vec2 size, bool horizontalScroll = false);
void endScrollArea();

// Get/Set scroll position
Vec2 getScrollPos();
void setScrollPos(Vec2 pos);
void setScrollHereX(f32 centerRatio = 0.5f);
void setScrollHereY(f32 centerRatio = 0.5f);
void setScrollFromPosX(f32 localX, f32 centerRatio = 0.5f);
void setScrollFromPosY(f32 localY, f32 centerRatio = 0.5f);
f32 getScrollX();
f32 getScrollY();
f32 getScrollMaxX();
f32 getScrollMaxY();

// ============================================================================
// Tab Bar Functions
// ============================================================================

// Begin a tab bar
bool beginTabBar(StringView id, TabBarFlags flags = TabBarFlags::None);
void endTabBar();

// Begin a tab item (returns true if the tab is selected)
bool beginTabItem(StringView label, bool* open = nullptr, TabItemFlags flags = TabItemFlags::None);
void endTabItem();

// Set a tab as selected
void setTabItemClosed(StringView tabOrDockedWindowLabel);

// ============================================================================
// Tree Functions
// ============================================================================

// Tree nodes (expandable/collapsible)
bool treeNode(StringView label, TreeNodeFlags flags = TreeNodeFlags::None);
bool treeNode(StringView strId, StringView label, TreeNodeFlags flags = TreeNodeFlags::None);
bool treeNodeEx(StringView label, TreeNodeFlags flags = TreeNodeFlags::None);
void treePush(StringView strId);
void treePop();
f32 getTreeNodeToLabelSpacing();

// Collapsing headers (tree node without indentation)
bool collapsingHeader(StringView label, TreeNodeFlags flags = TreeNodeFlags::None);
bool collapsingHeader(StringView label, bool* open, TreeNodeFlags flags = TreeNodeFlags::None);

// ============================================================================
// Group Functions
// ============================================================================

// Group widgets together (for layout purposes)
void beginGroup();
void endGroup();

// Dummy space (for alignment)
void dummy(Vec2 size);

// Separator line
void separator();

// Spacing (small empty space)
void spacing();

// Same line (continue on same line as previous widget)
void sameLine(f32 offsetFromStartX = 0.0f, f32 spacing = -1.0f);

// New line (force a new line)
void newLine();

// Indentation
void indent(f32 indentWidth = 0.0f);
void unindent(f32 indentWidth = 0.0f);

// ============================================================================
// Menu Functions
// ============================================================================

// Main menu bar (at top of screen/window)
bool beginMainMenuBar();
void endMainMenuBar();

// Menu bar (in a window)
bool beginMenuBar();
void endMenuBar();

// Menu dropdown
bool beginMenu(StringView label, bool enabled = true);
void endMenu();

// Menu items
bool menuItem(StringView label, StringView shortcut = "", bool selected = false, bool enabled = true);
bool menuItem(StringView label, StringView shortcut, bool* selected, bool enabled = true);

// ============================================================================
// Popup Functions
// ============================================================================

// Popups (modal/non-modal)
void openPopup(StringView strId);
bool beginPopup(StringView strId, WindowFlags flags = WindowFlags::None);
bool beginPopupModal(StringView name, bool* open = nullptr, WindowFlags flags = WindowFlags::None);
bool beginPopupContextItem(StringView strId = "", i32 mouseButton = 1);
bool beginPopupContextWindow(StringView strId = "", i32 mouseButton = 1);
bool beginPopupContextVoid(StringView strId = "", i32 mouseButton = 1);
void endPopup();
void closeCurrentPopup();
bool isPopupOpen(StringView strId);

// ============================================================================
// Grid Layout
// ============================================================================

struct GridConfig
{
    i32 columns = 1;
    f32 columnWidth = -1.0f;  // -1 = auto
    f32 rowHeight = -1.0f;    // -1 = auto
    Vec2 cellPadding = {4.0f, 4.0f};
    bool drawBorders = false;
    bool stretchColumns = true;
};

// Begin a grid layout
bool beginGrid(StringView id, const GridConfig& config);
void endGrid();

// Move to next cell in grid
void nextCell();

// Set column width
void setColumnWidth(i32 column, f32 width);

// ============================================================================
// Table Functions (Advanced Data Tables)
// ============================================================================

enum class TableFlags : u32
{
    None = 0,
    Resizable = 1 << 0,
    Reorderable = 1 << 1,
    Hideable = 1 << 2,
    Sortable = 1 << 3,
    NoSavedSettings = 1 << 4,
    ContextMenuInBody = 1 << 5,
    RowBg = 1 << 6,
    BordersInnerH = 1 << 7,
    BordersOuterH = 1 << 8,
    BordersInnerV = 1 << 9,
    BordersOuterV = 1 << 10,
    BordersH = BordersInnerH | BordersOuterH,
    BordersV = BordersInnerV | BordersOuterV,
    BordersInner = BordersInnerV | BordersInnerH,
    BordersOuter = BordersOuterV | BordersOuterH,
    Borders = BordersInner | BordersOuter,
    NoBordersInBody = 1 << 11,
    NoBordersInBodyUntilResize = 1 << 12,
    SizingFixedFit = 1 << 13,
    SizingFixedSame = 1 << 14,
    SizingStretchProp = 1 << 15,
    SizingStretchSame = 1 << 16,
    NoHostExtendX = 1 << 17,
    NoHostExtendY = 1 << 18,
    NoKeepColumnsVisible = 1 << 19,
    PreciseWidths = 1 << 20,
    NoClip = 1 << 21,
    PadOuterX = 1 << 22,
    NoPadOuterX = 1 << 23,
    NoPadInnerX = 1 << 24,
    ScrollX = 1 << 25,
    ScrollY = 1 << 26,
    SortMulti = 1 << 27,
    SortTristate = 1 << 28,
};

inline TableFlags operator|(TableFlags a, TableFlags b)
{
    return static_cast<TableFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

enum class TableColumnFlags : u32
{
    None = 0,
    Disabled = 1 << 0,
    DefaultHide = 1 << 1,
    DefaultSort = 1 << 2,
    WidthStretch = 1 << 3,
    WidthFixed = 1 << 4,
    NoResize = 1 << 5,
    NoReorder = 1 << 6,
    NoHide = 1 << 7,
    NoClip = 1 << 8,
    NoSort = 1 << 9,
    NoSortAscending = 1 << 10,
    NoSortDescending = 1 << 11,
    NoHeaderLabel = 1 << 12,
    NoHeaderWidth = 1 << 13,
    PreferSortAscending = 1 << 14,
    PreferSortDescending = 1 << 15,
    IndentEnable = 1 << 16,
    IndentDisable = 1 << 17,
    IsEnabled = 1 << 24,
    IsVisible = 1 << 25,
    IsSorted = 1 << 26,
    IsHovered = 1 << 27,
};

inline TableColumnFlags operator|(TableColumnFlags a, TableColumnFlags b)
{
    return static_cast<TableColumnFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

enum class TableRowFlags : u32
{
    None = 0,
    Headers = 1 << 0,
};

inline TableRowFlags operator|(TableRowFlags a, TableRowFlags b)
{
    return static_cast<TableRowFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

// Table functions
bool beginTable(StringView strId, i32 columns, TableFlags flags = TableFlags::None, Vec2 outerSize = {0, 0},
                f32 innerWidth = 0.0f);
void endTable();

// Table setup (call after beginTable, before first row)
void tableSetupColumn(StringView label, TableColumnFlags flags = TableColumnFlags::None, f32 initWidthOrWeight = 0.0f,
                      u32 userId = 0);
void tableSetupScrollFreeze(i32 cols, i32 rows);
void tableHeadersRow();

// Table content
void tableNextRow(TableRowFlags flags = TableRowFlags::None, f32 minRowHeight = 0.0f);
bool tableNextColumn();
bool tableSetColumnIndex(i32 columnIndex);

// Table info
i32 tableGetColumnCount();
i32 tableGetColumnIndex();
i32 tableGetRowIndex();
StringView tableGetColumnName(i32 columnIndex = -1);
TableColumnFlags tableGetColumnFlags(i32 columnIndex = -1);
void tableSetColumnEnabled(i32 columnIndex, bool enabled);
void tableSetBgColor(i32 target, Color color, i32 columnIndex = -1);

// ============================================================================
// Docking (Optional - stub for future expansion)
// ============================================================================

// Placeholder for potential docking support
// bool beginDockSpace(StringView id);
// void endDockSpace();

}  // namespace dakt::gui

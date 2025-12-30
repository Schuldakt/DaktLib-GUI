#pragma once

// ============================================================================
// DaktLib GUI Module - Data Widgets
// Specialized widgets for data visualization and editing
// ============================================================================

#include "Context.hpp"
#include "DrawList.hpp"
#include "Font.hpp"
#include "Theme.hpp"
#include "Types.hpp"
#include "Widgets.hpp"

#include <dakt/core/Types.hpp>

#include <functional>
#include <span>
#include <variant>
#include <vector>

namespace dakt::gui
{

using namespace dakt::core;

// ============================================================================
// Property Grid
// ============================================================================

// Property value types
using PropertyValue = std::variant<bool, i32, f32, String, Vec2, Vec4, Color>;

enum class PropertyType
{
    Bool,
    Int,
    Float,
    String,
    Vec2,
    Vec4,
    Color,
    Enum,
    Flags,
    Button,
    Custom
};

struct PropertyGridConfig
{
    f32 nameColumnWidth = 150.0f;
    f32 valueColumnWidth = -1.0f;  // -1 = fill remaining
    bool showCategories = true;
    bool sortAlphabetically = false;
    Color categoryBg = Color(51, 51, 51, 255);   // 0.2, 0.2, 0.2, 1
    Color alternateBg = Color(38, 38, 38, 128);  // 0.15, 0.15, 0.15, 0.5
};

// Begin a property grid
bool beginPropertyGrid(StringView id, const PropertyGridConfig& config = {});
void endPropertyGrid();

// Property editing functions - return true if value changed
bool propertyBool(StringView name, bool* value, StringView tooltip = "");
bool propertyInt(StringView name, i32* value, i32 min = 0, i32 max = 100, StringView tooltip = "");
bool propertyFloat(StringView name, f32* value, f32 min = 0.0f, f32 max = 1.0f, StringView tooltip = "");
bool propertyString(StringView name, String& value, StringView tooltip = "");
bool propertyString(StringView name, char* buf, usize bufSize, StringView tooltip = "");
bool propertyVec2(StringView name, Vec2* value, StringView tooltip = "");
bool propertyVec4(StringView name, Vec4* value, StringView tooltip = "");
bool propertyColor(StringView name, Color* value, StringView tooltip = "");
bool propertyColor3(StringView name, f32 col[3], StringView tooltip = "");
bool propertyColor4(StringView name, f32 col[4], StringView tooltip = "");
bool propertyEnum(StringView name, i32* value, std::span<const StringView> options, StringView tooltip = "");
bool propertyFlags(StringView name, u32* value, std::span<const StringView> options, StringView tooltip = "");
bool propertyButton(StringView name, StringView buttonLabel);

// Category management
void propertyGridBeginCategory(StringView name);
void propertyGridEndCategory();

// ============================================================================
// Data Table
// ============================================================================

enum class SortDirection
{
    None,
    Ascending,
    Descending
};

// Column configuration
struct DataTableColumn
{
    StringView header;
    f32 width = -1.0f;  // -1 = auto
    bool sortable = true;
    bool resizable = true;
    bool visible = true;
    TextAlign alignment = TextAlign::topLeft();
};

// Row selection mode
enum class SelectionMode
{
    None,
    Single,
    Multiple
};

struct DataTableConfig
{
    std::vector<DataTableColumn> columns;
    SelectionMode selectionMode = SelectionMode::Single;
    bool showRowNumbers = false;
    bool showHeaders = true;
    bool alternateRowColors = true;
    bool showBorders = true;
    bool enableFilter = false;
    f32 rowHeight = 20.0f;
    Color headerBg = Color(51, 51, 64, 255);      // 0.2, 0.2, 0.25, 1
    Color rowBg = Color(38, 38, 38, 255);         // 0.15, 0.15, 0.15, 1
    Color rowBgAlt = Color(46, 46, 46, 255);      // 0.18, 0.18, 0.18, 1
    Color selectedBg = Color(77, 102, 153, 255);  // 0.3, 0.4, 0.6, 1
    Color hoverBg = Color(64, 89, 140, 128);      // 0.25, 0.35, 0.55, 0.5
};

// Table state
struct DataTableState
{
    i32 sortColumn = -1;
    SortDirection sortDirection = SortDirection::None;
    std::vector<i32> selectedRows;
    i32 hoveredRow = -1;
    String filterText;
    Vec2 scroll = {0, 0};
};

// Begin a data table
bool beginDataTable(StringView id, const DataTableConfig& config, DataTableState& state);
void endDataTable();

// Add rows to the table
void dataTableBeginRow(i32 rowIndex);
void dataTableEndRow();
void dataTableCell(StringView content);
void dataTableCell(i32 value);
void dataTableCell(f32 value, i32 precision = 2);

// Row selection helpers
bool isRowSelected(const DataTableState& state, i32 row);
void selectRow(DataTableState& state, i32 row, bool addToSelection = false);
void deselectRow(DataTableState& state, i32 row);
void clearSelection(DataTableState& state);

// Callbacks for custom cell rendering
using CellRenderCallback = std::function<void(i32 row, i32 col, Rect cellRect)>;
void dataTableSetCellRenderer(CellRenderCallback callback);

// ============================================================================
// Hex View
// ============================================================================

struct HexViewConfig
{
    i32 bytesPerRow = 16;
    bool showAscii = true;
    bool showAddresses = true;
    bool editable = false;
    bool showHeader = true;
    Color addressColor = Color(153, 153, 204, 255);   // 0.6, 0.6, 0.8, 1
    Color hexColor = Color(230, 230, 230, 255);       // 0.9, 0.9, 0.9, 1
    Color asciiColor = Color(179, 230, 179, 255);     // 0.7, 0.9, 0.7, 1
    Color modifiedColor = Color(255, 128, 128, 255);  // 1.0, 0.5, 0.5, 1
    Color selectionBg = Color(77, 102, 153, 128);     // 0.3, 0.4, 0.6, 0.5
    Color zeroColor = Color(128, 128, 128, 255);      // 0.5, 0.5, 0.5, 1
};

struct HexViewState
{
    usize selectionStart = 0;
    usize selectionEnd = 0;
    usize cursorPos = 0;
    bool inAsciiView = false;
    Vec2 scroll = {0, 0};
    std::vector<usize> modifiedBytes;  // Track indices of modified bytes
};

// Render a hex view
// Returns true if data was modified (when editable)
bool hexView(StringView id, std::span<byte> data, const HexViewConfig& config, HexViewState& state);
bool hexView(StringView id, std::span<const byte> data, const HexViewConfig& config, HexViewState& state);

// Hex view utilities
void hexViewGoToAddress(HexViewState& state, usize address);
void hexViewSelectRange(HexViewState& state, usize start, usize end);
std::span<const byte> hexViewGetSelection(std::span<const byte> data, const HexViewState& state);

// ============================================================================
// Tree List (Hierarchical List with Columns)
// ============================================================================

// Tree node for TreeList
struct TreeListNode
{
    StringView label;
    std::vector<StringView> columnValues;
    std::vector<TreeListNode> children;
    bool expanded = false;
    bool selected = false;
    void* userData = nullptr;
    Color iconColor = Color::white();
    uintptr_t iconTexture = 0;
};

struct TreeListConfig
{
    std::vector<DataTableColumn> columns;
    bool showRoot = true;
    bool multiSelect = false;
    bool showLines = true;
    bool showIcons = false;
    f32 indentWidth = 20.0f;
    f32 rowHeight = 20.0f;
    Color lineColor = Color(77, 77, 77, 255);  // 0.3, 0.3, 0.3, 1
};

struct TreeListState
{
    std::vector<TreeListNode*> selectedNodes;
    TreeListNode* hoveredNode = nullptr;
    Vec2 scroll = {0, 0};
};

// Render a tree list
bool beginTreeList(StringView id, const TreeListConfig& config, TreeListState& state);
void endTreeList();

// Render tree nodes
bool treeListNode(TreeListNode& node, i32 depth = 0);
void treeListLeaf(StringView label, std::span<const StringView> columnValues);

// Selection
void treeListSelectNode(TreeListState& state, TreeListNode* node, bool addToSelection = false);
void treeListClearSelection(TreeListState& state);
std::span<TreeListNode* const> treeListGetSelectedNodes(const TreeListState& state);

// Expand/collapse
void treeListExpandAll(TreeListNode& root);
void treeListCollapseAll(TreeListNode& root);
void treeListExpandToNode(TreeListNode& root, TreeListNode* target);

// ============================================================================
// Log/Console View
// ============================================================================

enum class LogLevel
{
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

struct LogEntry
{
    StringView message;
    StringView source;
    LogLevel level = LogLevel::Info;
    f64 timestamp = 0.0;
};

struct LogViewConfig
{
    bool showTimestamp = true;
    bool showSource = true;
    bool showLevel = true;
    bool autoScroll = true;
    bool wordWrap = false;
    i32 maxLines = 1000;

    Color traceColor = Color(128, 128, 128, 255);   // 0.5, 0.5, 0.5, 1
    Color debugColor = Color(179, 179, 179, 255);   // 0.7, 0.7, 0.7, 1
    Color infoColor = Color::white();               // 1.0, 1.0, 1.0, 1
    Color warningColor = Color(255, 204, 77, 255);  // 1.0, 0.8, 0.3, 1
    Color errorColor = Color(255, 102, 102, 255);   // 1.0, 0.4, 0.4, 1
    Color fatalColor = Color(255, 51, 51, 255);     // 1.0, 0.2, 0.2, 1
};

struct LogViewState
{
    String filterText;
    LogLevel minLevel = LogLevel::Trace;
    Vec2 scroll = {0, 0};
    bool scrollToBottom = true;
};

// Render log view
void logView(StringView id, std::span<const LogEntry> entries, const LogViewConfig& config, LogViewState& state);

// Log view utilities
void logViewScrollToBottom(LogViewState& state);
void logViewClear(LogViewState& state);

// ============================================================================
// Code Editor View (Simple)
// ============================================================================

struct CodeEditorConfig
{
    bool showLineNumbers = true;
    bool showCurrentLine = true;
    bool enableSyntaxHighlight = true;
    bool readOnly = false;
    f32 tabSize = 4.0f;
    StringView language = "text";  // "cpp", "json", "xml", etc.

    Color lineNumberColor = Color(128, 128, 128, 255);  // 0.5, 0.5, 0.5, 1
    Color currentLineBg = Color(51, 51, 64, 255);       // 0.2, 0.2, 0.25, 1
    Color selectionBg = Color(77, 102, 153, 128);       // 0.3, 0.4, 0.6, 0.5

    // Syntax colors
    Color keywordColor = Color(128, 179, 255, 255);   // 0.5, 0.7, 1.0, 1
    Color stringColor = Color(204, 153, 102, 255);    // 0.8, 0.6, 0.4, 1
    Color commentColor = Color(102, 153, 102, 255);   // 0.4, 0.6, 0.4, 1
    Color numberColor = Color(204, 204, 128, 255);    // 0.8, 0.8, 0.5, 1
    Color typeColor = Color(128, 204, 204, 255);      // 0.5, 0.8, 0.8, 1
    Color functionColor = Color(220, 220, 170, 255);  // 0.86, 0.86, 0.67, 1
};

struct CodeEditorState
{
    i32 cursorLine = 0;
    i32 cursorColumn = 0;
    i32 selectionStartLine = -1;
    i32 selectionStartColumn = -1;
    i32 selectionEndLine = -1;
    i32 selectionEndColumn = -1;
    Vec2 scroll = {0, 0};
    bool modified = false;
};

// Simple code editor
bool codeEditor(StringView id, String& text, const CodeEditorConfig& config, CodeEditorState& state);

// Editor utilities
void codeEditorGoToLine(CodeEditorState& state, i32 line);
void codeEditorSelectAll(CodeEditorState& state);
bool codeEditorHasSelection(const CodeEditorState& state);
StringView codeEditorGetSelectedText(StringView text, const CodeEditorState& state);

// ============================================================================
// File Browser
// ============================================================================

struct FileBrowserEntry
{
    String name;
    String path;
    bool isDirectory = false;
    u64 size = 0;
    i64 modifiedTime = 0;
    bool selected = false;
};

struct FileBrowserConfig
{
    bool showHiddenFiles = false;
    bool allowMultiSelect = false;
    bool showPathBar = true;
    bool showFilter = true;
    StringView filterExtensions = "";  // e.g., ".txt;.md;.cpp"
    bool showFileDetails = true;
    f32 iconSize = 16.0f;
};

struct FileBrowserState
{
    String currentPath;
    std::vector<FileBrowserEntry> entries;
    std::vector<String> selectedPaths;
    String filterText;
    String selectedPath;  // Currently selected path (single selection mode)
    i32 sortColumn = 0;
    SortDirection sortDirection = SortDirection::Ascending;
    Vec2 scroll = {0, 0};
    bool needsRefresh = true;  // Flag to trigger directory refresh
};

// Render file browser
// Returns true if selection changed or double-click occurred
bool fileBrowser(StringView id, const FileBrowserConfig& config, FileBrowserState& state);

// File browser utilities
void fileBrowserNavigateTo(FileBrowserState& state, StringView path);
void fileBrowserRefresh(FileBrowserState& state);
void fileBrowserGoUp(FileBrowserState& state);
std::span<const String> fileBrowserGetSelectedPaths(const FileBrowserState& state);

// ============================================================================
// Timeline / Sequence Editor
// ============================================================================

struct TimelineTrack
{
    StringView name;
    Color color = Color(100, 149, 237, 255);  // Cornflower blue
    bool muted = false;
    bool locked = false;
    bool visible = true;
    f32 height = 30.0f;
};

struct TimelineKeyframe
{
    f64 time = 0.0;
    PropertyValue value;
    i32 trackIndex = 0;
    bool selected = false;
};

struct TimelineConfig
{
    f64 startTime = 0.0;
    f64 endTime = 10.0;
    f64 frameRate = 30.0;
    bool showFrameNumbers = true;
    bool snapToFrames = true;
    bool showMarkers = true;
    f32 trackHeaderWidth = 150.0f;
    f32 headerWidth = 150.0f;  // Alias for trackHeaderWidth (used in timeline rendering)
    f32 trackHeight = 30.0f;   // Default height for tracks
    f32 pixelsPerSecond = 100.0f;
};

struct TimelineState
{
    f64 currentTime = 0.0;
    f64 zoomLevel = 1.0;
    f64 scrollX = 0.0;
    f32 scrollY = 0.0f;
    f64 viewStart = 0.0;  // Visible time range start
    f64 viewEnd = 10.0;   // Visible time range end
    std::vector<TimelineKeyframe*> selectedKeyframes;
    bool isPlaying = false;
    bool isDragging = false;
};

// Render timeline
bool timeline(StringView id, std::vector<TimelineTrack>& tracks, std::vector<TimelineKeyframe>& keyframes,
              const TimelineConfig& config, TimelineState& state);

// Timeline utilities
void timelineSetTime(TimelineState& state, f64 time);
void timelinePlay(TimelineState& state);
void timelinePause(TimelineState& state);
void timelineStop(TimelineState& state);
void timelineZoomToFit(TimelineState& state, const TimelineConfig& config);

// ============================================================================
// Node Graph Editor
// ============================================================================

struct NodePin
{
    StringView name;
    Color color = Color::white();
    bool isInput = true;
    bool isConnected = false;
    uintptr_t id = 0;
};

struct GraphNode
{
    StringView title;
    Vec2 position;
    Vec2 size;
    Color headerColor = Color(60, 60, 60, 255);
    std::vector<NodePin> inputs;
    std::vector<NodePin> outputs;
    bool selected = false;
    bool collapsed = false;
    uintptr_t id = 0;
    void* userData = nullptr;
};

struct NodeLink
{
    uintptr_t sourceNodeId = 0;
    uintptr_t sourcePinId = 0;
    uintptr_t destNodeId = 0;
    uintptr_t destPinId = 0;
    Color color = Color(200, 200, 200, 255);
};

struct NodeGraphConfig
{
    bool showGrid = true;
    bool showMinimap = true;
    f32 gridSize = 32.0f;
    f32 linkThickness = 2.0f;
    f32 pinRadius = 5.0f;
    bool allowNodeDeletion = true;
    bool allowLinkDeletion = true;
    Color gridColor = Color(40, 40, 40, 255);
    Color backgroundColor = Color(30, 30, 30, 255);
};

struct NodeGraphState
{
    Vec2 viewOffset = {0, 0};
    f32 zoom = 1.0f;
    std::vector<GraphNode*> selectedNodes;
    NodeLink* pendingLink = nullptr;
    bool isDragging = false;
    bool isLinking = false;
};

// Render node graph
bool nodeGraph(StringView id, std::vector<GraphNode>& nodes, std::vector<NodeLink>& links,
               const NodeGraphConfig& config, NodeGraphState& state);

// Node graph utilities
void nodeGraphCenterOnNodes(NodeGraphState& state, std::span<const GraphNode> nodes);
void nodeGraphDeleteSelected(std::vector<GraphNode>& nodes, std::vector<NodeLink>& links, NodeGraphState& state);
void nodeGraphSelectAll(std::vector<GraphNode>& nodes, NodeGraphState& state);
void nodeGraphDeselectAll(NodeGraphState& state);

// Callbacks
using NodeCreatedCallback = std::function<void(GraphNode& node)>;
using LinkCreatedCallback = std::function<bool(NodeLink& link)>;  // Return false to reject
using NodeDeletedCallback = std::function<void(GraphNode& node)>;
using LinkDeletedCallback = std::function<void(NodeLink& link)>;

void nodeGraphSetCallbacks(NodeCreatedCallback onCreate, LinkCreatedCallback onLink, NodeDeletedCallback onDeleteNode,
                           LinkDeletedCallback onDeleteLink);

}  // namespace dakt::gui

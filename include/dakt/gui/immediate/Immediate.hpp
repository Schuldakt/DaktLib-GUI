#ifndef DAKT_GUI_IMMEDIATE_HPP
#define DAKT_GUI_IMMEDIATE_HPP

#include "../core/Types.hpp"
#include <string>
#include <string_view>

namespace dakt::gui {

class Context;
class DrawList;
struct Theme;
class Widget;

// ============================================================================
// Frame Management
// ============================================================================

void beginFrame(Context* ctx, float deltaTime);
void endFrame(Context* ctx);

// ============================================================================
// ID Stack
// ============================================================================

void pushID(const char* strId);
void pushID(int intId);
void pushID(const void* ptrId);
void popID();
ID getID(const char* strId);
ID getID(int intId);

// ============================================================================
// Windows
// ============================================================================

enum class WindowFlags : uint32_t {
    None = 0,
    NoTitleBar = 1 << 0,
    NoResize = 1 << 1,
    NoMove = 1 << 2,
    NoScrollbar = 1 << 3,
    NoCollapse = 1 << 4,
    AlwaysAutoResize = 1 << 5,
    NoBackground = 1 << 6,
    NoBringToFrontOnFocus = 1 << 7,
    NoNavFocus = 1 << 8,
};

inline WindowFlags operator|(WindowFlags a, WindowFlags b) { return static_cast<WindowFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }
inline WindowFlags operator&(WindowFlags a, WindowFlags b) { return static_cast<WindowFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b)); }
inline bool hasFlag(WindowFlags flags, WindowFlags flag) { return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(flag)) != 0; }

bool beginWindow(const char* name, bool* open = nullptr, WindowFlags flags = WindowFlags::None);
void endWindow();

void setNextWindowPos(Vec2 pos);
void setNextWindowSize(Vec2 size);
void setNextWindowContentSize(Vec2 size);

Vec2 getWindowPos();
Vec2 getWindowSize();
Vec2 getContentRegionAvail();
Vec2 getCursorPos();
void setCursorPos(Vec2 pos);

// ============================================================================
// Layout
// ============================================================================

void sameLine(float offsetX = 0.0f, float spacing = -1.0f);
void newLine();
void spacing();
void separator();
void indent(float width = 0.0f);
void unindent(float width = 0.0f);

void beginGroup();
void endGroup();

// ============================================================================
// Basic Widgets
// ============================================================================

// Text
void text(const char* fmt, ...);
void textColored(Color color, const char* fmt, ...);
void textDisabled(const char* fmt, ...);
void textWrapped(const char* fmt, ...);
void labelText(const char* label, const char* fmt, ...);

// Buttons
bool button(const char* label, Vec2 size = Vec2(0, 0));
bool smallButton(const char* label);
bool invisibleButton(const char* strId, Vec2 size);
bool colorButton(const char* descId, Color color, Vec2 size = Vec2(0, 0));

// Checkboxes & Radio
bool checkbox(const char* label, bool* value);
bool radioButton(const char* label, bool active);
bool radioButton(const char* label, int* value, int buttonValue);

// Progress & Loading
void progressBar(float fraction, Vec2 size = Vec2(-1, 0), const char* overlay = nullptr);

// ============================================================================
// Input Widgets
// ============================================================================

enum class InputTextFlags : uint32_t {
    None = 0,
    CharsDecimal = 1 << 0,
    CharsHexadecimal = 1 << 1,
    CharsUppercase = 1 << 2,
    CharsNoBlank = 1 << 3,
    AutoSelectAll = 1 << 4,
    EnterReturnsTrue = 1 << 5,
    Password = 1 << 6,
    ReadOnly = 1 << 7,
    Multiline = 1 << 8,
};

inline InputTextFlags operator|(InputTextFlags a, InputTextFlags b) { return static_cast<InputTextFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }

bool inputText(const char* label, char* buf, size_t bufSize, InputTextFlags flags = InputTextFlags::None);
bool inputTextMultiline(const char* label, char* buf, size_t bufSize, Vec2 size = Vec2(0, 0), InputTextFlags flags = InputTextFlags::None);
bool inputInt(const char* label, int* value, int step = 1, int stepFast = 100);
bool inputFloat(const char* label, float* value, float step = 0.0f, float stepFast = 0.0f, const char* format = "%.3f");

// ============================================================================
// Sliders & Drags
// ============================================================================

bool sliderInt(const char* label, int* value, int min, int max, const char* format = "%d");
bool sliderFloat(const char* label, float* value, float min, float max, const char* format = "%.3f");
bool sliderVec2(const char* label, Vec2* value, float min, float max, const char* format = "%.3f");

bool dragInt(const char* label, int* value, float speed = 1.0f, int min = 0, int max = 0);
bool dragFloat(const char* label, float* value, float speed = 1.0f, float min = 0.0f, float max = 0.0f, const char* format = "%.3f");

// ============================================================================
// Color Editors
// ============================================================================

bool colorEdit3(const char* label, Color* color);
bool colorEdit4(const char* label, Color* color);
bool colorPicker3(const char* label, Color* color);
bool colorPicker4(const char* label, Color* color);

// ============================================================================
// Trees & Lists
// ============================================================================

enum class TreeNodeFlags : uint32_t {
    None = 0,
    Selected = 1 << 0,
    Framed = 1 << 1,
    OpenOnArrow = 1 << 2,
    OpenOnDoubleClick = 1 << 3,
    Leaf = 1 << 4,
    DefaultOpen = 1 << 5,
    SpanAllColumns = 1 << 6,
};

inline TreeNodeFlags operator|(TreeNodeFlags a, TreeNodeFlags b) { return static_cast<TreeNodeFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }

bool treeNode(const char* label, TreeNodeFlags flags = TreeNodeFlags::None);
bool treeNodeEx(const char* strId, TreeNodeFlags flags, const char* fmt, ...);
void treePop();

bool collapsingHeader(const char* label, TreeNodeFlags flags = TreeNodeFlags::None);

bool selectable(const char* label, bool selected = false, Vec2 size = Vec2(0, 0));
bool selectable(const char* label, bool* selected, Vec2 size = Vec2(0, 0));

bool listBox(const char* label, int* currentItem, const char* const items[], int itemCount, int heightInItems = -1);

// ============================================================================
// Menus
// ============================================================================

bool beginMainMenuBar();
void endMainMenuBar();
bool beginMenuBar();
void endMenuBar();
bool beginMenu(const char* label, bool enabled = true);
void endMenu();
bool menuItem(const char* label, const char* shortcut = nullptr, bool selected = false, bool enabled = true);
bool menuItem(const char* label, const char* shortcut, bool* selected, bool enabled = true);

// ============================================================================
// Popups & Modals
// ============================================================================

void openPopup(const char* strId);
bool beginPopup(const char* strId);
bool beginPopupModal(const char* name, bool* open = nullptr);
void endPopup();
void closeCurrentPopup();

bool beginPopupContextItem(const char* strId = nullptr);
bool beginPopupContextWindow(const char* strId = nullptr);

// ============================================================================
// Tooltips
// ============================================================================

void beginTooltip();
void endTooltip();
void setTooltip(const char* fmt, ...);

// ============================================================================
// Tables (Simplified)
// ============================================================================

enum class TableFlags : uint32_t {
    None = 0,
    Resizable = 1 << 0,
    Reorderable = 1 << 1,
    Hideable = 1 << 2,
    Sortable = 1 << 3,
    RowBg = 1 << 4,
    Borders = 1 << 5,
    ScrollX = 1 << 6,
    ScrollY = 1 << 7,
};

inline TableFlags operator|(TableFlags a, TableFlags b) { return static_cast<TableFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }

bool beginTable(const char* strId, int columns, TableFlags flags = TableFlags::None, Vec2 outerSize = Vec2(0, 0));
void endTable();
void tableNextRow();
bool tableNextColumn();
bool tableSetColumnIndex(int columnIndex);
void tableSetupColumn(const char* label, float initWidth = 0.0f);
void tableHeadersRow();

// ============================================================================
// Utilities
// ============================================================================

bool isItemHovered();
bool isItemActive();
bool isItemFocused();
bool isItemClicked(MouseButton button = MouseButton::Left);
bool isItemEdited();
bool isItemActivated();
bool isItemDeactivated();

Vec2 getItemRectMin();
Vec2 getItemRectMax();
Vec2 getItemRectSize();

void setItemDefaultFocus();
void setKeyboardFocusHere(int offset = 0);

// Mouse utilities
bool isMouseHoveringRect(Vec2 min, Vec2 max);
bool isMouseClicked(MouseButton button);
bool isMouseDoubleClicked(MouseButton button);
bool isMouseDown(MouseButton button);
bool isMouseReleased(MouseButton button);
Vec2 getMousePos();
Vec2 getMouseDragDelta(MouseButton button = MouseButton::Left);

// ============================================================================
// Drawing Helpers (Direct DrawList Access)
// ============================================================================

DrawList* getWindowDrawList();
DrawList* getForegroundDrawList();
DrawList* getBackgroundDrawList();

// ============================================================================
// Retained embedding
// ============================================================================
// Embed a retained-mode widget tree inside the current immediate layout.
// `id` is used to generate stable IDs for interaction. If `size` is zero,
// the widget's `measureContent()` will be used to size it.
void embedRetained(const char* id, dakt::gui::Widget* widget, Vec2 size = Vec2(0, 0));
} // namespace dakt::gui

#endif

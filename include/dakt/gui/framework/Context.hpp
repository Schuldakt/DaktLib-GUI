// ============================================================================
// DaktLib GUI Module - Context
// ============================================================================
// Main GUI context managing state, input, and rendering.
// ============================================================================

#pragma once

#include <dakt/gui/core/DrawList.hpp>
#include <dakt/gui/core/Font.hpp>
#include <dakt/gui/core/Types.hpp>

#include <functional>
#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>

namespace dakt::gui
{

// Forward declarations
class Theme;
class Widget;

// ============================================================================
// Widget ID
// ============================================================================

using WidgetId = u64;

constexpr WidgetId InvalidWidgetId = 0;

// ============================================================================
// Input State
// ============================================================================

struct InputState
{
    Vec2 mousePos;
    Vec2 mouseDelta;
    Vec2 mouseWheel;

    std::array<bool, static_cast<usize>(MouseButton::Count)> mouseDown{};
    std::array<bool, static_cast<usize>(MouseButton::Count)> mouseClicked{};
    std::array<bool, static_cast<usize>(MouseButton::Count)> mouseDoubleClicked{};
    std::array<bool, static_cast<usize>(MouseButton::Count)> mouseReleased{};
    std::array<f32, static_cast<usize>(MouseButton::Count)> mouseDownDuration{};

    std::array<bool, static_cast<usize>(Key::Count)> keyDown{};
    std::array<bool, static_cast<usize>(Key::Count)> keyPressed{};
    std::array<bool, static_cast<usize>(Key::Count)> keyReleased{};
    std::array<f32, static_cast<usize>(Key::Count)> keyDownDuration{};

    KeyMods mods;

    // Text input
    std::vector<u32> textInput;  // UTF-32 codepoints

    // Focus
    bool wantCaptureMouse = false;
    bool wantCaptureKeyboard = false;

    void clear();
};

// ============================================================================
// Layout Context
// ============================================================================

struct LayoutState
{
    Vec2 cursor;            // Current drawing position
    Vec2 contentMin;        // Content bounding box min
    Vec2 contentMax;        // Content bounding box max
    f32 currentLineHeight;  // Height of current line
    f32 itemSpacing;        // Space between items
    f32 indentLevel;        // Current indent
    bool sameLine;          // Next item on same line

    void reset(Vec2 startPos);
    void newLine(f32 height);
    void advanceCursor(Vec2 size);
    [[nodiscard]] Vec2 calcItemPos(Vec2 size) const;
};

// ============================================================================
// Window State
// ============================================================================

struct WindowState
{
    WidgetId id = InvalidWidgetId;
    String name;
    Rect rect;
    Rect contentRect;
    Vec2 scroll;
    Vec2 scrollMax;
    bool collapsed = false;
    bool resizing = false;
    bool moving = false;
    bool focused = false;
    bool visible = true;
    i32 zOrder = 0;

    LayoutState layout;
    DrawList drawList;
};

// ============================================================================
// Context Options
// ============================================================================

struct ContextOptions
{
    f32 doubleClickTime = 0.3f;
    f32 doubleClickMaxDist = 6.0f;
    f32 dragThreshold = 5.0f;
    f32 keyRepeatDelay = 0.25f;
    f32 keyRepeatRate = 0.05f;
    bool antiAliasedLines = true;
    bool antiAliasedFill = true;
};

// ============================================================================
// GUI Context
// ============================================================================

class Context
{
public:
    Context();
    ~Context();

    // Frame management
    void beginFrame(f32 deltaTime);
    void endFrame();

    // Input handling
    void setMousePos(Vec2 pos);
    void setMouseButton(MouseButton button, bool down);
    void setMouseWheel(Vec2 wheel);
    void setKeyState(Key key, bool down);
    void setKeyMods(KeyMods mods);
    void addTextInput(u32 codepoint);
    void clearTextInput();

    // Input queries
    [[nodiscard]] bool isMouseDown(MouseButton button) const;
    [[nodiscard]] bool isMouseClicked(MouseButton button, bool repeat = false) const;
    [[nodiscard]] bool isMouseDoubleClicked(MouseButton button) const;
    [[nodiscard]] bool isMouseReleased(MouseButton button) const;
    [[nodiscard]] bool isMouseDragging(MouseButton button, f32 threshold = -1.0f) const;
    [[nodiscard]] Vec2 getMousePos() const;
    [[nodiscard]] Vec2 getMouseDelta() const;
    [[nodiscard]] Vec2 getMouseDragDelta(MouseButton button) const;
    [[nodiscard]] f32 getMouseWheel() const;
    [[nodiscard]] f32 getMouseWheelH() const;

    [[nodiscard]] bool isKeyDown(Key key) const;
    [[nodiscard]] bool isKeyPressed(Key key, bool repeat = false) const;
    [[nodiscard]] bool isKeyReleased(Key key) const;
    [[nodiscard]] const KeyMods& getKeyMods() const;

    // ID management
    void pushId(WidgetId id);
    void pushId(StringView str);
    void pushId(i32 intId);
    void popId();
    [[nodiscard]] WidgetId getId(StringView str) const;
    [[nodiscard]] WidgetId getId(i32 intId) const;
    [[nodiscard]] WidgetId getCurrentId() const;

    // Active/Hot widget management
    void setActiveWidget(WidgetId id);
    void setHotWidget(WidgetId id);
    void clearActiveWidget();
    [[nodiscard]] WidgetId getActiveWidget() const;
    [[nodiscard]] WidgetId getHotWidget() const;
    [[nodiscard]] bool isWidgetActive(WidgetId id) const;
    [[nodiscard]] bool isWidgetHot(WidgetId id) const;

    // Focus management
    void setFocusedWidget(WidgetId id);
    [[nodiscard]] WidgetId getFocusedWidget() const;
    [[nodiscard]] bool isWidgetFocused(WidgetId id) const;

    // Window management
    bool beginWindow(StringView name, bool* open = nullptr, Rect initialRect = {100, 100, 400, 300});
    void endWindow();
    [[nodiscard]] WindowState* getCurrentWindow() const;
    [[nodiscard]] WindowState* findWindow(StringView name);
    void setNextWindowPos(Vec2 pos);
    void setNextWindowSize(Vec2 size);
    void setNextWindowCollapsed(bool collapsed);
    void setNextWindowFocus();

    // Draw list access
    [[nodiscard]] DrawList& getDrawList();
    [[nodiscard]] DrawList& getOverlayDrawList();
    [[nodiscard]] DrawList& getForegroundDrawList();

    // Rendering
    [[nodiscard]] const std::vector<DrawList*>& getRenderDrawLists() const;

    // Layout
    void sameLine(f32 offsetX = 0.0f, f32 spacing = -1.0f);
    void newLine();
    void separator();
    void spacing();
    void indent(f32 amount = 0.0f);
    void unindent(f32 amount = 0.0f);
    void beginGroup();
    void endGroup();
    void dummy(Vec2 size);

    [[nodiscard]] Vec2 getCursorPos() const;
    void setCursorPos(Vec2 pos);
    [[nodiscard]] Vec2 getCursorScreenPos() const;
    void setCursorScreenPos(Vec2 pos);
    [[nodiscard]] f32 getContentWidth() const;
    [[nodiscard]] f32 getContentHeight() const;

    // Item queries
    [[nodiscard]] bool isItemHovered() const;
    [[nodiscard]] bool isItemActive() const;
    [[nodiscard]] bool isItemFocused() const;
    [[nodiscard]] bool isItemClicked(MouseButton button = MouseButton::Left) const;
    [[nodiscard]] bool isItemEdited() const;
    [[nodiscard]] bool isItemActivated() const;
    [[nodiscard]] bool isItemDeactivated() const;
    [[nodiscard]] Rect getItemRect() const;
    [[nodiscard]] Vec2 getItemSize() const;

    // Style/Theme
    void pushStyleColor(i32 idx, Color color);
    void popStyleColor(i32 count = 1);
    void pushStyleVar(i32 idx, f32 value);
    void pushStyleVar(i32 idx, Vec2 value);
    void popStyleVar(i32 count = 1);
    [[nodiscard]] Theme* getTheme() const;
    void setTheme(std::unique_ptr<Theme> theme);

    // Fonts
    [[nodiscard]] FontManager& getFontManager();
    [[nodiscard]] Font* getFont() const;
    void setFont(Font* font);
    void pushFont(Font* font);
    void popFont();

    // Clipboard
    [[nodiscard]] String getClipboardText() const;
    void setClipboardText(StringView text);

    // Cursor
    void setMouseCursor(Cursor cursor);
    [[nodiscard]] Cursor getMouseCursor() const;

    // Options
    [[nodiscard]] ContextOptions& getOptions();
    [[nodiscard]] const ContextOptions& getOptions() const;

    // Display info
    void setDisplaySize(Vec2 size);
    [[nodiscard]] Vec2 getDisplaySize() const;
    void setDisplayFramebufferScale(Vec2 scale);
    [[nodiscard]] Vec2 getDisplayFramebufferScale() const;

    // Time
    [[nodiscard]] f32 getDeltaTime() const;
    [[nodiscard]] f64 getTime() const;
    [[nodiscard]] i64 getFrameCount() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    // Hash helpers
    [[nodiscard]] static u64 hashString(StringView str);
    [[nodiscard]] static u64 hashCombine(u64 seed, u64 value);
};

// ============================================================================
// Global Context Access
// ============================================================================

Context* getContext();
void setContext(Context* ctx);

}  // namespace dakt::gui

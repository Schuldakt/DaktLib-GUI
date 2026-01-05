#include "dakt/gui/immediate/Immediate.hpp"
#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/draw/DrawList.hpp"
#include "dakt/gui/input/Input.hpp"
#include "dakt/gui/style/Style.hpp"
#include <cstdarg>
#include <stack>
#include <vector>

namespace dakt::gui {

// ============================================================================
// Global State
// ============================================================================

struct WindowState {
    ID id = 0;
    const char* name = nullptr;
    Vec2 pos;
    Vec2 size;
    Vec2 contentSize;
    Vec2 cursorPos;
    Vec2 cursorStartPos;
    WindowFlags flags = WindowFlags::None;
    bool collapsed = false;
    bool skipItems = false;
};

struct ImmediateState {
    Context* ctx = nullptr;
    float deltaTime = 0.0f;

    // ID stack
    std::vector<ID> idStack;
    ID currentId = 0;

    // Window state
    std::stack<WindowState> windowStack;
    WindowState* currentWindow = nullptr;

    // Next window hints
    bool nextWindowPosSet = false;
    Vec2 nextWindowPos;
    bool nextWindowSizeSet = false;
    Vec2 nextWindowSize;

    // Item state
    ID lastItemId = 0;
    Rect lastItemRect;
    bool lastItemHovered = false;
    bool lastItemActive = false;
    bool lastItemEdited = false;
    bool lastItemActivated = false;
    bool lastItemDeactivated = false;

    // Hot/Active tracking
    ID hotId = 0;        // Hovered item
    ID activeId = 0;     // Currently interacting item
    ID prevActiveId = 0; // Active item from previous frame

    // Focus management
    ID focusRequestId = 0;      // Widget requesting focus
    int focusRequestOffset = 0; // Offset for setKeyboardFocusHere
    ID keyboardFocusId = 0;     // Currently focused widget for keyboard input
    ID defaultFocusId = 0;      // Default focus for current scope

    // Double-click tracking
    float lastClickTime[5] = {};                     // Time of last click per button
    Vec2 lastClickPos[5] = {};                       // Position of last click per button
    ID lastClickedId[5] = {};                        // Widget ID of last click per button
    static constexpr float DOUBLE_CLICK_TIME = 0.3f; // Max time between clicks
    static constexpr float DOUBLE_CLICK_DIST = 6.0f; // Max distance between clicks

    // Input
    InputSystem* input = nullptr;

    // Accumulated time for double-click detection
    float totalTime = 0.0f;
};

ImmediateState g_state;

// ============================================================================
// ID Management
// ============================================================================

static ID hashCombine(ID seed, ID value) {
    // FNV-1a inspired hash combine
    return seed ^ (value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
}

void pushID(const char* strId) {
    ID id = 0;
    while (*strId) {
        id = hashCombine(id, static_cast<ID>(*strId++));
    }
    if (!g_state.idStack.empty()) {
        id = hashCombine(g_state.idStack.back(), id);
    }
    g_state.idStack.push_back(id);
}

void pushID(int intId) {
    ID id = static_cast<ID>(intId);
    if (!g_state.idStack.empty()) {
        id = hashCombine(g_state.idStack.back(), id);
    }
    g_state.idStack.push_back(id);
}

void pushID(const void* ptrId) {
    ID id = reinterpret_cast<ID>(ptrId);
    if (!g_state.idStack.empty()) {
        id = hashCombine(g_state.idStack.back(), id);
    }
    g_state.idStack.push_back(id);
}

void popID() {
    if (!g_state.idStack.empty()) {
        g_state.idStack.pop_back();
    }
}

ID getID(const char* strId) {
    ID id = 0;
    while (*strId) {
        id = hashCombine(id, static_cast<ID>(*strId++));
    }
    if (!g_state.idStack.empty()) {
        id = hashCombine(g_state.idStack.back(), id);
    }
    return id;
}

ID getID(int intId) {
    ID id = static_cast<ID>(intId);
    if (!g_state.idStack.empty()) {
        id = hashCombine(g_state.idStack.back(), id);
    }
    return id;
}

// ============================================================================
// Frame Management
// ============================================================================

void beginFrame(Context* ctx, float deltaTime) {
    g_state.ctx = ctx;
    g_state.deltaTime = deltaTime;
    g_state.totalTime += deltaTime;
    g_state.idStack.clear();
    g_state.nextWindowPosSet = false;
    g_state.nextWindowSizeSet = false;

    // Track previous frame's active ID for activation/deactivation detection
    g_state.prevActiveId = g_state.activeId;

    // Reset per-frame item state
    g_state.lastItemEdited = false;
    g_state.lastItemActivated = false;
    g_state.lastItemDeactivated = false;

    // Process focus requests from previous frame
    if (g_state.focusRequestId != 0) {
        g_state.keyboardFocusId = g_state.focusRequestId;
        g_state.focusRequestId = 0;
    }

    if (ctx) {
        ctx->newFrame(deltaTime);
    }
}

void endFrame(Context* ctx) {
    if (ctx) {
        ctx->endFrame();
    }
    g_state.ctx = nullptr;
}

// ============================================================================
// Window Management
// ============================================================================

void setNextWindowPos(Vec2 pos) {
    g_state.nextWindowPosSet = true;
    g_state.nextWindowPos = pos;
}

void setNextWindowSize(Vec2 size) {
    g_state.nextWindowSizeSet = true;
    g_state.nextWindowSize = size;
}

void setNextWindowContentSize(Vec2 size) {
    // Store for next window
    (void)size;
}

bool beginWindow(const char* name, bool* open, WindowFlags flags) {
    ID id = getID(name);

    WindowState window;
    window.id = id;
    window.name = name;
    window.flags = flags;

    // Apply next window hints
    if (g_state.nextWindowPosSet) {
        window.pos = g_state.nextWindowPos;
        g_state.nextWindowPosSet = false;
    } else {
        window.pos = Vec2(100, 100); // Default
    }

    if (g_state.nextWindowSizeSet) {
        window.size = g_state.nextWindowSize;
        g_state.nextWindowSizeSet = false;
    } else {
        window.size = Vec2(400, 300); // Default
    }

    // Calculate content area
    float titleBarHeight = hasFlag(flags, WindowFlags::NoTitleBar) ? 0.0f : 24.0f;
    window.cursorStartPos = Vec2(window.pos.x + 8, window.pos.y + titleBarHeight + 8);
    window.cursorPos = window.cursorStartPos;

    g_state.windowStack.push(window);
    g_state.currentWindow = &g_state.windowStack.top();

    // Push window ID
    pushID(name);

    // Draw window background
    if (!hasFlag(flags, WindowFlags::NoBackground) && g_state.ctx) {
        DrawList& dl = g_state.ctx->getDrawList();
        const Theme& theme = g_state.ctx->getTheme();
        const ColorScheme& colors = theme.colors();

        // Background
        dl.drawRectFilledRounded(Rect(window.pos.x, window.pos.y, window.size.x, window.size.y), colors.surface, 4.0f);

        // Title bar
        if (!hasFlag(flags, WindowFlags::NoTitleBar)) {
            Vec2 titleMin = window.pos;
            Vec2 titleMax = Vec2(window.pos.x + window.size.x, window.pos.y + titleBarHeight);
            dl.drawRectFilledRounded(Rect(titleMin.x, titleMin.y, titleMax.x - titleMin.x, titleMax.y - titleMin.y), colors.surfaceVariant, 4.0f);
            dl.drawText(Vec2(titleMin.x + 8, titleMin.y + 4), name, colors.textPrimary);

            // Close button
            if (open != nullptr) {
                Vec2 closePos = Vec2(titleMax.x - 20, titleMin.y + 4);
                dl.drawText(closePos, "X", colors.textSecondary);
            }
        }

        // Border
        dl.drawRectRounded(Rect(window.pos.x, window.pos.y, window.size.x, window.size.y), colors.border, 4.0f);
    }

    return !window.collapsed;
}

void endWindow() {
    if (!g_state.windowStack.empty()) {
        popID();
        g_state.windowStack.pop();
        g_state.currentWindow = g_state.windowStack.empty() ? nullptr : &g_state.windowStack.top();
    }
}

Vec2 getWindowPos() {
    if (g_state.currentWindow)
        return g_state.currentWindow->pos;
    return Vec2(0, 0);
}

Vec2 getWindowSize() {
    if (g_state.currentWindow)
        return g_state.currentWindow->size;
    return Vec2(0, 0);
}

Vec2 getContentRegionAvail() {
    if (g_state.currentWindow) {
        Vec2 max = g_state.currentWindow->pos + g_state.currentWindow->size - Vec2(8, 8);
        return max - g_state.currentWindow->cursorPos;
    }
    return Vec2(0, 0);
}

Vec2 getCursorPos() {
    if (g_state.currentWindow)
        return g_state.currentWindow->cursorPos - g_state.currentWindow->pos;
    return Vec2(0, 0);
}

void setCursorPos(Vec2 pos) {
    if (g_state.currentWindow) {
        g_state.currentWindow->cursorPos = g_state.currentWindow->pos + pos;
    }
}

// ============================================================================
// Layout Helpers
// ============================================================================

void sameLine(float offsetX, float spacing) {
    if (g_state.currentWindow) {
        float sp = (spacing < 0) ? 8.0f : spacing;
        g_state.currentWindow->cursorPos.x = g_state.lastItemRect.right() + sp + offsetX;
        g_state.currentWindow->cursorPos.y = g_state.lastItemRect.y;
    }
}

void newLine() {
    if (g_state.currentWindow) {
        g_state.currentWindow->cursorPos.x = g_state.currentWindow->cursorStartPos.x;
        g_state.currentWindow->cursorPos.y += 20.0f;
    }
}

void spacing() {
    if (g_state.currentWindow) {
        g_state.currentWindow->cursorPos.y += 8.0f;
    }
}

void separator() {
    if (g_state.currentWindow && g_state.ctx) {
        DrawList& dl = g_state.ctx->getDrawList();
        const Theme& theme = g_state.ctx->getTheme();

        Vec2 p1 = g_state.currentWindow->cursorPos;
        Vec2 p2 = Vec2(g_state.currentWindow->pos.x + g_state.currentWindow->size.x - 8, p1.y);
        dl.drawLine(p1, p2, theme.colors().border, 1.0f);

        g_state.currentWindow->cursorPos.y += 8.0f;
    }
}

void indent(float width) {
    if (g_state.currentWindow) {
        float w = (width == 0.0f) ? 20.0f : width;
        g_state.currentWindow->cursorPos.x += w;
        g_state.currentWindow->cursorStartPos.x += w;
    }
}

void unindent(float width) {
    if (g_state.currentWindow) {
        float w = (width == 0.0f) ? 20.0f : width;
        g_state.currentWindow->cursorPos.x -= w;
        g_state.currentWindow->cursorStartPos.x -= w;
    }
}

void beginGroup() {
    // Push group state - simplified for now
}

void endGroup() {
    // Pop group state
}

// ============================================================================
// Item State
// ============================================================================

bool isItemHovered() { return g_state.lastItemHovered; }

bool isItemActive() { return g_state.lastItemActive; }

bool isItemFocused() { return g_state.activeId == g_state.lastItemId; }

bool isItemClicked(MouseButton button) {
    // Check if last item was clicked
    return g_state.lastItemHovered && g_state.input && g_state.input->isMouseButtonPressed(button);
}

bool isItemEdited() { return g_state.lastItemEdited; }

bool isItemActivated() { return g_state.lastItemActivated; }

bool isItemDeactivated() { return g_state.lastItemDeactivated; }

Vec2 getItemRectMin() { return g_state.lastItemRect.topLeft(); }

Vec2 getItemRectMax() { return g_state.lastItemRect.bottomRight(); }

Vec2 getItemRectSize() { return g_state.lastItemRect.size(); }

void setItemDefaultFocus() {
    // Mark the last item as the default focus for this scope
    g_state.defaultFocusId = g_state.lastItemId;
    // If no widget currently has focus, give it to this one
    if (g_state.keyboardFocusId == 0) {
        g_state.keyboardFocusId = g_state.lastItemId;
    }
}

void setKeyboardFocusHere(int offset) {
    // Request focus for a widget at the given offset
    // offset=0 means the next widget, offset=-1 means the previous widget
    g_state.focusRequestOffset = offset;
    // The actual focus will be applied when the target widget is processed
    // For now, we mark that a focus request is pending
    g_state.focusRequestId = g_state.lastItemId; // Will be updated by next widget if offset > 0
}

// ============================================================================
// Mouse Utilities
// ============================================================================

bool isMouseHoveringRect(Vec2 min, Vec2 max) {
    if (!g_state.input)
        return false;
    Vec2 mouse = g_state.input->getMousePosition();
    return mouse.x >= min.x && mouse.x <= max.x && mouse.y >= min.y && mouse.y <= max.y;
}

bool isMouseClicked(MouseButton button) { return g_state.input && g_state.input->isMouseButtonPressed(button); }

bool isMouseDoubleClicked(MouseButton button) {
    if (!g_state.input)
        return false;

    int btnIdx = static_cast<int>(button);
    if (btnIdx < 0 || btnIdx >= 5)
        return false;

    // Check if button was just pressed
    if (!g_state.input->isMouseButtonPressed(button))
        return false;

    Vec2 mousePos = g_state.input->getMousePosition();
    float currentTime = g_state.totalTime;

    // Check if this click qualifies as a double-click
    float timeSinceLastClick = currentTime - g_state.lastClickTime[btnIdx];
    Vec2 posDiff = mousePos - g_state.lastClickPos[btnIdx];
    float distSinceLastClick = posDiff.length();

    bool isDoubleClick = (timeSinceLastClick < ImmediateState::DOUBLE_CLICK_TIME) && (distSinceLastClick < ImmediateState::DOUBLE_CLICK_DIST);

    // Update last click tracking
    g_state.lastClickTime[btnIdx] = currentTime;
    g_state.lastClickPos[btnIdx] = mousePos;
    g_state.lastClickedId[btnIdx] = g_state.lastItemId;

    return isDoubleClick;
}

bool isMouseDown(MouseButton button) { return g_state.input && g_state.input->isMouseButtonDown(button); }

bool isMouseReleased(MouseButton button) { return g_state.input && g_state.input->isMouseButtonReleased(button); }

Vec2 getMousePos() { return g_state.input ? g_state.input->getMousePosition() : Vec2(0, 0); }

Vec2 getMouseDragDelta(MouseButton button) {
    (void)button;
    return g_state.input ? g_state.input->getMouseDelta() : Vec2(0, 0);
}

// ============================================================================
// DrawList Access
// ============================================================================

DrawList* getWindowDrawList() { return g_state.ctx ? &g_state.ctx->getDrawList() : nullptr; }

DrawList* getForegroundDrawList() { return g_state.ctx ? &g_state.ctx->getDrawList() : nullptr; }

DrawList* getBackgroundDrawList() { return g_state.ctx ? &g_state.ctx->getDrawList() : nullptr; }

} // namespace dakt::gui

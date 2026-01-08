#include "dakt/gui/immediate/Immediate.hpp"
#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"
#include "dakt/gui/subsystems/input/Input.hpp"
#include "dakt/gui/subsystems/style/Style.hpp"

#include <cstdarg>

namespace dakt::gui {

// ============================================================================
// Helper Functions
// ============================================================================

static bool hasFlag(WindowFlags flags, WindowFlags flag) {
    return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(flag)) != 0;
}

static ID hashCombine(ID seed, ID value) {
    // FNV-1a inspired hash combine
    return seed ^ (value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
}

// ============================================================================
// ID Management
// ============================================================================

void pushID(const char* strId) {
    ImmediateState& state = getState();
    ID id = 0;
    while (*strId) {
        id = hashCombine(id, static_cast<ID>(*strId++));
    }
    if (!state.idStack.empty()) {
        id = hashCombine(state.idStack.back(), id);
    }
    state.idStack.push_back(id);
}

void pushID(int intId) {
    ImmediateState& state = getState();
    ID id = static_cast<ID>(intId);
    if (!state.idStack.empty()) {
        id = hashCombine(state.idStack.back(), id);
    }
    state.idStack.push_back(id);
}

void pushID(const void* ptrId) {
    ImmediateState& state = getState();
    ID id = reinterpret_cast<ID>(ptrId);
    if (!state.idStack.empty()) {
        id = hashCombine(state.idStack.back(), id);
    }
    state.idStack.push_back(id);
}

void popID() {
    ImmediateState& state = getState();
    if (!state.idStack.empty()) {
        state.idStack.pop_back();
    }
}

ID getID(const char* strId) {
    ImmediateState& state = getState();
    ID id = 0;
    while (*strId) {
        id = hashCombine(id, static_cast<ID>(*strId++));
    }
    if (!state.idStack.empty()) {
        id = hashCombine(state.idStack.back(), id);
    }
    return id;
}

ID getID(int intId) {
    ImmediateState& state = getState();
    ID id = static_cast<ID>(intId);
    if (!state.idStack.empty()) {
        id = hashCombine(state.idStack.back(), id);
    }
    return id;
}

ID getID(ID existingId) {
    ImmediateState& state = getState();
    if (!state.idStack.empty()) {
        return hashCombine(state.idStack.back(), existingId);
    }
    return existingId;
}

void pushID(ID existingId) {
    ImmediateState& state = getState();
    ID id = existingId;
    if (!state.idStack.empty()) {
        id = hashCombine(state.idStack.back(), id);
    }
    state.idStack.push_back(id);
}

std::uint64_t getFrameIndex() {
    Context* ctx = getCurrentContext();
    return ctx ? ctx->getFrameCount() : 0;
}

// ============================================================================
// Frame Management
// ============================================================================

void beginFrame(Context& ctx, float deltaTime) {
    // Set the current context for this thread
    setCurrentContext(&ctx);

    ImmediateState& state = ctx.getImmediateState();
    state.deltaTime = deltaTime;
    state.totalTime += deltaTime;
    state.idStack.clear();
    state.nextWindowPosSet = false;
    state.nextWindowSizeSet = false;

    // Track previous frame's active ID for activation/deactivation detection
    state.prevActiveId = state.activeId;

    // Reset per-frame item state
    state.lastItemEdited = false;
    state.lastItemActivated = false;
    state.lastItemDeactivated = false;

    // Process focus requests from previous frame
    if (state.focusRequestId != 0) {
        state.keyboardFocusId = state.focusRequestId;
        state.focusRequestId = 0;
    }

    ctx.newFrame(deltaTime);
}

void endFrame(Context& ctx) {
    ctx.endFrame();
    setCurrentContext(nullptr);
}

// ============================================================================
// Window Management
// ============================================================================

void setNextWindowPos(Vec2 pos) {
    ImmediateState& state = getState();
    state.nextWindowPosSet = true;
    state.nextWindowPos = pos;
}

void setNextWindowSize(Vec2 size) {
    ImmediateState& state = getState();
    state.nextWindowSizeSet = true;
    state.nextWindowSize = size;
}

void setNextWindowCollapsed(bool collapsed) {
    // Store for next window (not yet implemented)
    (void)collapsed;
}

bool beginWindow(const char* name, bool* open, WindowFlags flags) {
    Context* ctx = getCurrentContext();
    if (!ctx) return false;

    ImmediateState& state = ctx->getImmediateState();
    ID id = getID(name);

    WindowState window;
    window.id = id;
    window.name = name;
    window.flags = flags;

    // Apply next window hints
    if (state.nextWindowPosSet) {
        window.pos = state.nextWindowPos;
        state.nextWindowPosSet = false;
    } else {
        window.pos = Vec2(100, 100); // Default
    }

    if (state.nextWindowSizeSet) {
        window.size = state.nextWindowSize;
        state.nextWindowSizeSet = false;
    } else {
        window.size = Vec2(400, 300); // Default
    }

    // Calculate content area
    float titleBarHeight = hasFlag(flags, WindowFlags::NoTitleBar) ? 0.0f : 24.0f;
    window.cursorStartPos = Vec2(window.pos.x + 8, window.pos.y + titleBarHeight + 8);
    window.cursorPos = window.cursorStartPos;

    state.windowStack.push(window);
    state.currentWindow = &state.windowStack.top();

    // Push window ID
    pushID(name);

    // Draw window background
    DrawList& dl = ctx->getDrawList();
    const Theme& theme = ctx->getTheme();
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

    return !window.collapsed;
}

void endWindow() {
    ImmediateState& state = getState();
    if (!state.windowStack.empty()) {
        popID();
        state.windowStack.pop();
        state.currentWindow = state.windowStack.empty() ? nullptr : &state.windowStack.top();
    }
}

Vec2 getWindowPos() {
    ImmediateState& state = getState();
    if (state.currentWindow)
        return state.currentWindow->pos;
    return Vec2(0, 0);
}

Vec2 getWindowSize() {
    ImmediateState& state = getState();
    if (state.currentWindow)
        return state.currentWindow->size;
    return Vec2(0, 0);
}

Vec2 getContentRegionAvail() {
    ImmediateState& state = getState();
    if (state.currentWindow) {
        Vec2 max = state.currentWindow->pos + state.currentWindow->size - Vec2(8, 8);
        return max - state.currentWindow->cursorPos;
    }
    return Vec2(0, 0);
}

Vec2 getCursorPos() {
    ImmediateState& state = getState();
    if (state.currentWindow)
        return state.currentWindow->cursorPos - state.currentWindow->pos;
    return Vec2(0, 0);
}

void setCursorPos(Vec2 pos) {
    ImmediateState& state = getState();
    if (state.currentWindow) {
        state.currentWindow->cursorPos = state.currentWindow->pos + pos;
    }
}

// ============================================================================
// Layout Helpers
// ============================================================================

void sameLine(float offsetX, float spacing) {
    ImmediateState& state = getState();
    if (state.currentWindow) {
        float sp = (spacing < 0) ? 8.0f : spacing;
        state.currentWindow->cursorPos.x = state.lastItemRect.right() + sp + offsetX;
        state.currentWindow->cursorPos.y = state.lastItemRect.y;
    }
}

void newLine() {
    ImmediateState& state = getState();
    if (state.currentWindow) {
        state.currentWindow->cursorPos.x = state.currentWindow->cursorStartPos.x;
        state.currentWindow->cursorPos.y += 20.0f;
    }
}

void spacing() {
    ImmediateState& state = getState();
    if (state.currentWindow) {
        state.currentWindow->cursorPos.y += 8.0f;
    }
}

void separator() {
    Context* ctx = getCurrentContext();
    if (!ctx) return;

    ImmediateState& state = ctx->getImmediateState();
    if (state.currentWindow) {
        DrawList& dl = ctx->getDrawList();
        const Theme& theme = ctx->getTheme();

        Vec2 p1 = state.currentWindow->cursorPos;
        Vec2 p2 = Vec2(state.currentWindow->pos.x + state.currentWindow->size.x - 8, p1.y);
        dl.drawLine(p1, p2, theme.colors().border, 1.0f);

        state.currentWindow->cursorPos.y += 8.0f;
    }
}

void indent(float width) {
    ImmediateState& state = getState();
    if (state.currentWindow) {
        float w = (width == 0.0f) ? 20.0f : width;
        state.currentWindow->cursorPos.x += w;
        state.currentWindow->cursorStartPos.x += w;
    }
}

void unindent(float width) {
    ImmediateState& state = getState();
    if (state.currentWindow) {
        float w = (width == 0.0f) ? 20.0f : width;
        state.currentWindow->cursorPos.x -= w;
        state.currentWindow->cursorStartPos.x -= w;
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

bool isItemHovered() {
    return getState().lastItemHovered;
}

bool isItemActive() {
    return getState().lastItemActive;
}

bool isItemFocused() {
    ImmediateState& state = getState();
    return state.activeId == state.lastItemId;
}

bool isItemClicked(MouseButton button) {
    ImmediateState& state = getState();
    return state.lastItemHovered && state.input && state.input->isMouseButtonPressed(button);
}

bool isItemEdited() {
    return getState().lastItemEdited;
}

bool isItemActivated() {
    return getState().lastItemActivated;
}

bool isItemDeactivated() {
    return getState().lastItemDeactivated;
}

Vec2 getItemRectMin() {
    return getState().lastItemRect.topLeft();
}

Vec2 getItemRectMax() {
    return getState().lastItemRect.bottomRight();
}

Vec2 getItemRectSize() {
    return getState().lastItemRect.size();
}

void setItemDefaultFocus() {
    ImmediateState& state = getState();
    state.defaultFocusId = state.lastItemId;
    if (state.keyboardFocusId == 0) {
        state.keyboardFocusId = state.lastItemId;
    }
}

void setKeyboardFocusHere(int offset) {
    ImmediateState& state = getState();
    state.focusRequestOffset = offset;
    state.focusRequestId = state.lastItemId;
}

// ============================================================================
// Mouse Utilities
// ============================================================================

bool isMouseHoveringRect(Vec2 min, Vec2 max) {
    ImmediateState& state = getState();
    if (!state.input)
        return false;
    Vec2 mouse = state.input->getMousePosition();
    return mouse.x >= min.x && mouse.x <= max.x && mouse.y >= min.y && mouse.y <= max.y;
}

bool isMouseClicked(MouseButton button) {
    ImmediateState& state = getState();
    return state.input && state.input->isMouseButtonPressed(button);
}

bool isMouseDoubleClicked(MouseButton button) {
    ImmediateState& state = getState();
    if (!state.input)
        return false;

    int btnIdx = static_cast<int>(button);
    if (btnIdx < 0 || btnIdx >= 5)
        return false;

    if (!state.input->isMouseButtonPressed(button))
        return false;

    Vec2 mousePos = state.input->getMousePosition();
    float currentTime = state.totalTime;

    float timeSinceLastClick = currentTime - state.lastClickTime[btnIdx];
    Vec2 posDiff = mousePos - state.lastClickPos[btnIdx];
    float distSinceLastClick = posDiff.length();

    bool isDoubleClick = (timeSinceLastClick < ImmediateState::DOUBLE_CLICK_TIME) &&
                         (distSinceLastClick < ImmediateState::DOUBLE_CLICK_DIST);

    state.lastClickTime[btnIdx] = currentTime;
    state.lastClickPos[btnIdx] = mousePos;
    state.lastClicked[btnIdx] = state.lastItemId;

    return isDoubleClick;
}

bool isMouseDown(MouseButton button) {
    ImmediateState& state = getState();
    return state.input && state.input->isMouseButtonDown(button);
}

bool isMouseReleased(MouseButton button) {
    ImmediateState& state = getState();
    return state.input && state.input->isMouseButtonReleased(button);
}

Vec2 getMousePos() {
    ImmediateState& state = getState();
    return state.input ? state.input->getMousePosition() : Vec2(0, 0);
}

Vec2 getMouseDragDelta(MouseButton button) {
    ImmediateState& state = getState();
    (void)button;
    return state.input ? state.input->getMouseDelta() : Vec2(0, 0);
}

// ============================================================================
// DrawList Access
// ============================================================================

DrawList* getWindowDrawList() {
    Context* ctx = getCurrentContext();
    return ctx ? &ctx->getDrawList() : nullptr;
}

DrawList* getForegroundDrawList() {
    Context* ctx = getCurrentContext();
    return ctx ? &ctx->getDrawList() : nullptr;
}

DrawList* getBackgroundDrawList() {
    Context* ctx = getCurrentContext();
    return ctx ? &ctx->getDrawList() : nullptr;
}

} // namespace dakt::gui

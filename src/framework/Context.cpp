// ============================================================================
// DaktLib GUI Module - Context Implementation
// ============================================================================

#include <dakt/gui/Context.hpp>
#include <dakt/gui/Theme.hpp>

#include <algorithm>
#include <cstring>

namespace dakt::gui
{

// ============================================================================
// Global Context
// ============================================================================

static Context* g_currentContext = nullptr;

Context* getContext()
{
    return g_currentContext;
}

void setContext(Context* ctx)
{
    g_currentContext = ctx;
}

// ============================================================================
// InputState
// ============================================================================

void InputState::clear()
{
    mouseDelta = {};
    mouseWheel = {};
    std::fill(mouseClicked.begin(), mouseClicked.end(), false);
    std::fill(mouseDoubleClicked.begin(), mouseDoubleClicked.end(), false);
    std::fill(mouseReleased.begin(), mouseReleased.end(), false);
    std::fill(keyPressed.begin(), keyPressed.end(), false);
    std::fill(keyReleased.begin(), keyReleased.end(), false);
    textInput.clear();
}

// ============================================================================
// LayoutState
// ============================================================================

void LayoutState::reset(Vec2 startPos)
{
    cursor = startPos;
    contentMin = startPos;
    contentMax = startPos;
    currentLineHeight = 0.0f;
    itemSpacing = 4.0f;
    indentLevel = 0.0f;
    sameLine = false;
}

void LayoutState::newLine(f32 height)
{
    cursor.x = contentMin.x + indentLevel;
    cursor.y += currentLineHeight + itemSpacing;
    currentLineHeight = height;
    sameLine = false;
}

void LayoutState::advanceCursor(Vec2 size)
{
    if (sameLine)
    {
        cursor.x += size.x + itemSpacing;
    }
    else
    {
        cursor.y += currentLineHeight + itemSpacing;
        cursor.x = contentMin.x + indentLevel;
    }

    currentLineHeight = std::max(currentLineHeight, size.y);
    contentMax = Vec2::max(contentMax, cursor + size);
    sameLine = false;
}

Vec2 LayoutState::calcItemPos(Vec2 size) const
{
    (void)size;
    Vec2 pos = cursor;
    if (!sameLine)
    {
        pos.x = contentMin.x + indentLevel;
    }
    return pos;
}

// ============================================================================
// Context Implementation
// ============================================================================

struct Context::Impl
{
    // Input
    InputState input;
    InputState prevInput;
    Vec2 mouseClickPos[static_cast<usize>(MouseButton::Count)];
    f64 mouseClickTime[static_cast<usize>(MouseButton::Count)]{};

    // Widgets
    WidgetId activeWidget = InvalidWidgetId;
    WidgetId hotWidget = InvalidWidgetId;
    WidgetId focusedWidget = InvalidWidgetId;
    WidgetId lastItemId = InvalidWidgetId;
    Rect lastItemRect;
    bool lastItemHovered = false;

    // ID stack
    std::vector<WidgetId> idStack;

    // Windows
    std::vector<std::unique_ptr<WindowState>> windows;
    std::vector<WindowState*> windowStack;
    WindowState* currentWindow = nullptr;

    // Next window settings
    Option<Vec2> nextWindowPos;
    Option<Vec2> nextWindowSize;
    Option<bool> nextWindowCollapsed;
    bool nextWindowFocus = false;

    // Draw lists
    DrawList backgroundDrawList;
    DrawList overlayDrawList;
    DrawList foregroundDrawList;
    std::vector<DrawList*> renderDrawLists;

    // Fonts
    FontManager fontManager;
    std::vector<Font*> fontStack;
    Font* currentFont = nullptr;

    // Theme
    std::unique_ptr<Theme> theme;
    std::vector<std::pair<i32, Color>> colorStack;
    std::vector<std::pair<i32, f32>> varStackFloat;
    std::vector<std::pair<i32, Vec2>> varStackVec2;

    // Display
    Vec2 displaySize = {1280, 720};
    Vec2 framebufferScale = {1, 1};

    // Time
    f32 deltaTime = 1.0f / 60.0f;
    f64 time = 0.0;
    i64 frameCount = 0;

    // Options
    ContextOptions options;

    // Cursor
    Cursor mouseCursor = Cursor::Arrow;

    // Clipboard (callback-based)
    std::function<String()> getClipboardFn;
    std::function<void(StringView)> setClipboardFn;
};

Context::Context() : m_impl(std::make_unique<Impl>())
{
    // Initialize with default theme
    m_impl->theme = std::make_unique<Theme>();

    // Add default font
    m_impl->currentFont = m_impl->fontManager.addFontDefault();

    // Set as current context
    if (!g_currentContext)
    {
        g_currentContext = this;
    }
}

Context::~Context()
{
    if (g_currentContext == this)
    {
        g_currentContext = nullptr;
    }
}

// ============================================================================
// Frame Management
// ============================================================================

void Context::beginFrame(f32 deltaTime)
{
    m_impl->deltaTime = deltaTime;
    m_impl->time += deltaTime;
    m_impl->frameCount++;

    // Update input state
    m_impl->prevInput = m_impl->input;

    // Process mouse buttons
    for (usize i = 0; i < static_cast<usize>(MouseButton::Count); ++i)
    {
        bool wasDown = m_impl->prevInput.mouseDown[i];
        bool isDown = m_impl->input.mouseDown[i];

        m_impl->input.mouseClicked[i] = !wasDown && isDown;
        m_impl->input.mouseReleased[i] = wasDown && !isDown;

        if (m_impl->input.mouseClicked[i])
        {
            // Check for double click
            f64 clickTime = m_impl->time;
            f64 lastClickTime = m_impl->mouseClickTime[i];
            Vec2 clickPos = m_impl->input.mousePos;
            Vec2 lastClickPos = m_impl->mouseClickPos[i];

            f64 timeDiff = clickTime - lastClickTime;
            f32 dist = (clickPos - lastClickPos).length();

            m_impl->input.mouseDoubleClicked[i] =
                timeDiff < m_impl->options.doubleClickTime && dist < m_impl->options.doubleClickMaxDist;

            m_impl->mouseClickTime[i] = clickTime;
            m_impl->mouseClickPos[i] = clickPos;
        }

        // Update down duration
        if (isDown)
        {
            m_impl->input.mouseDownDuration[i] += deltaTime;
        }
        else
        {
            m_impl->input.mouseDownDuration[i] = -1.0f;
        }
    }

    // Process keys
    for (usize i = 0; i < static_cast<usize>(Key::Count); ++i)
    {
        bool wasDown = m_impl->prevInput.keyDown[i];
        bool isDown = m_impl->input.keyDown[i];

        m_impl->input.keyPressed[i] = !wasDown && isDown;
        m_impl->input.keyReleased[i] = wasDown && !isDown;

        if (isDown)
        {
            m_impl->input.keyDownDuration[i] += deltaTime;
        }
        else
        {
            m_impl->input.keyDownDuration[i] = -1.0f;
        }
    }

    // Clear hot widget
    m_impl->hotWidget = InvalidWidgetId;

    // Clear draw lists
    m_impl->backgroundDrawList.clear();
    m_impl->overlayDrawList.clear();
    m_impl->foregroundDrawList.clear();

    // Reset render list
    m_impl->renderDrawLists.clear();
    m_impl->renderDrawLists.push_back(&m_impl->backgroundDrawList);
}

void Context::endFrame()
{
    // Close any unclosed windows
    while (!m_impl->windowStack.empty())
    {
        endWindow();
    }

    // Build render list
    // Sort windows by z-order and add their draw lists
    std::vector<WindowState*> sortedWindows;
    for (auto& w : m_impl->windows)
    {
        if (w->visible)
        {
            sortedWindows.push_back(w.get());
        }
    }
    std::sort(sortedWindows.begin(), sortedWindows.end(),
              [](WindowState* a, WindowState* b) { return a->zOrder < b->zOrder; });

    for (auto* w : sortedWindows)
    {
        m_impl->renderDrawLists.push_back(&w->drawList);
    }

    // Add overlay lists
    m_impl->renderDrawLists.push_back(&m_impl->overlayDrawList);
    m_impl->renderDrawLists.push_back(&m_impl->foregroundDrawList);

    // Clear per-frame input
    m_impl->input.mouseDelta = {};
    m_impl->input.mouseWheel = {};
    m_impl->input.textInput.clear();
}

// ============================================================================
// Input Handling
// ============================================================================

void Context::setMousePos(Vec2 pos)
{
    m_impl->input.mouseDelta = pos - m_impl->input.mousePos;
    m_impl->input.mousePos = pos;
}

void Context::setMouseButton(MouseButton button, bool down)
{
    m_impl->input.mouseDown[static_cast<usize>(button)] = down;
}

void Context::setMouseWheel(Vec2 wheel)
{
    m_impl->input.mouseWheel = wheel;
}

void Context::setKeyState(Key key, bool down)
{
    m_impl->input.keyDown[static_cast<usize>(key)] = down;
}

void Context::setKeyMods(KeyMods mods)
{
    m_impl->input.mods = mods;
}

void Context::addTextInput(u32 codepoint)
{
    m_impl->input.textInput.push_back(codepoint);
}

void Context::clearTextInput()
{
    m_impl->input.textInput.clear();
}

// ============================================================================
// Input Queries
// ============================================================================

bool Context::isMouseDown(MouseButton button) const
{
    return m_impl->input.mouseDown[static_cast<usize>(button)];
}

bool Context::isMouseClicked(MouseButton button, bool repeat) const
{
    usize idx = static_cast<usize>(button);
    if (m_impl->input.mouseClicked[idx])
        return true;

    if (repeat && m_impl->input.mouseDown[idx])
    {
        f32 duration = m_impl->input.mouseDownDuration[idx];
        if (duration > m_impl->options.keyRepeatDelay)
        {
            f32 repeatTime = duration - m_impl->options.keyRepeatDelay;
            return std::fmod(repeatTime, m_impl->options.keyRepeatRate) < m_impl->deltaTime;
        }
    }

    return false;
}

bool Context::isMouseDoubleClicked(MouseButton button) const
{
    return m_impl->input.mouseDoubleClicked[static_cast<usize>(button)];
}

bool Context::isMouseReleased(MouseButton button) const
{
    return m_impl->input.mouseReleased[static_cast<usize>(button)];
}

bool Context::isMouseDragging(MouseButton button, f32 threshold) const
{
    if (!isMouseDown(button))
        return false;

    if (threshold < 0.0f)
        threshold = m_impl->options.dragThreshold;

    Vec2 delta = getMouseDragDelta(button);
    return delta.lengthSq() >= threshold * threshold;
}

Vec2 Context::getMousePos() const
{
    return m_impl->input.mousePos;
}

Vec2 Context::getMouseDelta() const
{
    return m_impl->input.mouseDelta;
}

Vec2 Context::getMouseDragDelta(MouseButton button) const
{
    usize idx = static_cast<usize>(button);
    if (m_impl->input.mouseDown[idx])
    {
        return m_impl->input.mousePos - m_impl->mouseClickPos[idx];
    }
    return {};
}

f32 Context::getMouseWheel() const
{
    return m_impl->input.mouseWheel.y;
}

f32 Context::getMouseWheelH() const
{
    return m_impl->input.mouseWheel.x;
}

bool Context::isKeyDown(Key key) const
{
    return m_impl->input.keyDown[static_cast<usize>(key)];
}

bool Context::isKeyPressed(Key key, bool repeat) const
{
    usize idx = static_cast<usize>(key);
    if (m_impl->input.keyPressed[idx])
        return true;

    if (repeat && m_impl->input.keyDown[idx])
    {
        f32 duration = m_impl->input.keyDownDuration[idx];
        if (duration > m_impl->options.keyRepeatDelay)
        {
            f32 repeatTime = duration - m_impl->options.keyRepeatDelay;
            return std::fmod(repeatTime, m_impl->options.keyRepeatRate) < m_impl->deltaTime;
        }
    }

    return false;
}

bool Context::isKeyReleased(Key key) const
{
    return m_impl->input.keyReleased[static_cast<usize>(key)];
}

const KeyMods& Context::getKeyMods() const
{
    return m_impl->input.mods;
}

// ============================================================================
// ID Management
// ============================================================================

u64 Context::hashString(StringView str)
{
    // FNV-1a hash
    u64 hash = 14695981039346656037ULL;
    for (char c : str)
    {
        hash ^= static_cast<u64>(c);
        hash *= 1099511628211ULL;
    }
    return hash;
}

u64 Context::hashCombine(u64 seed, u64 value)
{
    return seed ^ (value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
}

void Context::pushId(WidgetId id)
{
    m_impl->idStack.push_back(id);
}

void Context::pushId(StringView str)
{
    pushId(getId(str));
}

void Context::pushId(i32 intId)
{
    pushId(getId(intId));
}

void Context::popId()
{
    if (!m_impl->idStack.empty())
    {
        m_impl->idStack.pop_back();
    }
}

WidgetId Context::getId(StringView str) const
{
    u64 hash = hashString(str);
    if (!m_impl->idStack.empty())
    {
        hash = hashCombine(m_impl->idStack.back(), hash);
    }
    return hash;
}

WidgetId Context::getId(i32 intId) const
{
    u64 hash = static_cast<u64>(intId);
    if (!m_impl->idStack.empty())
    {
        hash = hashCombine(m_impl->idStack.back(), hash);
    }
    return hash;
}

WidgetId Context::getCurrentId() const
{
    return m_impl->idStack.empty() ? InvalidWidgetId : m_impl->idStack.back();
}

// ============================================================================
// Widget State Management
// ============================================================================

void Context::setActiveWidget(WidgetId id)
{
    m_impl->activeWidget = id;
}

void Context::setHotWidget(WidgetId id)
{
    m_impl->hotWidget = id;
}

void Context::clearActiveWidget()
{
    m_impl->activeWidget = InvalidWidgetId;
}

WidgetId Context::getActiveWidget() const
{
    return m_impl->activeWidget;
}

WidgetId Context::getHotWidget() const
{
    return m_impl->hotWidget;
}

bool Context::isWidgetActive(WidgetId id) const
{
    return m_impl->activeWidget == id;
}

bool Context::isWidgetHot(WidgetId id) const
{
    return m_impl->hotWidget == id;
}

void Context::setFocusedWidget(WidgetId id)
{
    m_impl->focusedWidget = id;
}

WidgetId Context::getFocusedWidget() const
{
    return m_impl->focusedWidget;
}

bool Context::isWidgetFocused(WidgetId id) const
{
    return m_impl->focusedWidget == id;
}

// ============================================================================
// Window Management
// ============================================================================

bool Context::beginWindow(StringView name, bool* open, Rect initialRect)
{
    WidgetId id = getId(name);

    // Find or create window
    WindowState* window = findWindow(name);
    if (!window)
    {
        auto newWindow = std::make_unique<WindowState>();
        newWindow->id = id;
        newWindow->name = String(name);
        newWindow->rect = initialRect;
        window = newWindow.get();
        m_impl->windows.push_back(std::move(newWindow));
    }

    // Apply next window settings
    if (m_impl->nextWindowPos)
    {
        window->rect.min = *m_impl->nextWindowPos;
        window->rect.max = window->rect.min + window->rect.size();
        m_impl->nextWindowPos.reset();
    }
    if (m_impl->nextWindowSize)
    {
        window->rect.max = window->rect.min + *m_impl->nextWindowSize;
        m_impl->nextWindowSize.reset();
    }
    if (m_impl->nextWindowCollapsed)
    {
        window->collapsed = *m_impl->nextWindowCollapsed;
        m_impl->nextWindowCollapsed.reset();
    }
    if (m_impl->nextWindowFocus)
    {
        window->focused = true;
        m_impl->nextWindowFocus = false;
    }

    // Handle close button
    if (open && !*open)
    {
        window->visible = false;
        return false;
    }
    window->visible = true;

    // Push window onto stack
    m_impl->windowStack.push_back(window);
    m_impl->currentWindow = window;

    // Initialize layout
    f32 titleBarHeight = 24.0f;
    window->contentRect = Rect{window->rect.min + Vec2{4, titleBarHeight + 4}, window->rect.max - Vec2{4, 4}};
    window->layout.reset(window->contentRect.min);

    // Clear and set up draw list
    window->drawList.clear();
    window->drawList.pushClipRect(window->rect);

    // Draw window background
    window->drawList.addRectFilled(window->rect, m_impl->theme->windowBg, 4.0f);

    // Draw title bar
    Rect titleRect{window->rect.min, {window->rect.max.x, window->rect.min.y + titleBarHeight}};
    window->drawList.addRectFilled(titleRect, m_impl->theme->titleBg, 4.0f, Corner::Top);

    // Draw title text
    if (m_impl->currentFont)
    {
        window->drawList.addText(window->rect.min + Vec2{8, 4}, name, m_impl->theme->text, 14.0f);
    }

    // Draw border
    window->drawList.addRect(window->rect, m_impl->theme->border, 4.0f);

    // Set content clip rect
    window->drawList.pushClipRect(window->contentRect);

    pushId(id);

    return !window->collapsed;
}

void Context::endWindow()
{
    if (m_impl->windowStack.empty())
        return;

    WindowState* window = m_impl->windowStack.back();

    // Pop content clip rect
    window->drawList.popClipRect();

    // Pop window clip rect
    window->drawList.popClipRect();

    popId();

    m_impl->windowStack.pop_back();
    m_impl->currentWindow = m_impl->windowStack.empty() ? nullptr : m_impl->windowStack.back();
}

WindowState* Context::getCurrentWindow() const
{
    return m_impl->currentWindow;
}

WindowState* Context::findWindow(StringView name)
{
    for (auto& w : m_impl->windows)
    {
        if (w->name == name)
            return w.get();
    }
    return nullptr;
}

void Context::setNextWindowPos(Vec2 pos)
{
    m_impl->nextWindowPos = pos;
}

void Context::setNextWindowSize(Vec2 size)
{
    m_impl->nextWindowSize = size;
}

void Context::setNextWindowCollapsed(bool collapsed)
{
    m_impl->nextWindowCollapsed = collapsed;
}

void Context::setNextWindowFocus()
{
    m_impl->nextWindowFocus = true;
}

// ============================================================================
// Draw List Access
// ============================================================================

DrawList& Context::getDrawList()
{
    if (m_impl->currentWindow)
    {
        return m_impl->currentWindow->drawList;
    }
    return m_impl->backgroundDrawList;
}

DrawList& Context::getOverlayDrawList()
{
    return m_impl->overlayDrawList;
}

DrawList& Context::getForegroundDrawList()
{
    return m_impl->foregroundDrawList;
}

const std::vector<DrawList*>& Context::getRenderDrawLists() const
{
    return m_impl->renderDrawLists;
}

// ============================================================================
// Layout
// ============================================================================

void Context::sameLine(f32 offsetX, f32 spacing)
{
    if (!m_impl->currentWindow)
        return;

    auto& layout = m_impl->currentWindow->layout;
    layout.sameLine = true;

    if (offsetX > 0.0f)
    {
        layout.cursor.x = layout.contentMin.x + offsetX;
    }

    if (spacing >= 0.0f)
    {
        layout.itemSpacing = spacing;
    }
}

void Context::newLine()
{
    if (!m_impl->currentWindow)
        return;
    m_impl->currentWindow->layout.newLine(0.0f);
}

void Context::separator()
{
    if (!m_impl->currentWindow)
        return;

    auto& layout = m_impl->currentWindow->layout;
    f32 x1 = layout.contentMin.x;
    f32 x2 = m_impl->currentWindow->contentRect.max.x;
    f32 y = layout.cursor.y + layout.currentLineHeight + layout.itemSpacing;

    getDrawList().addLine({x1, y}, {x2, y}, m_impl->theme->separator);

    layout.cursor.y = y + 1.0f;
    layout.currentLineHeight = 0.0f;
}

void Context::spacing()
{
    if (!m_impl->currentWindow)
        return;
    m_impl->currentWindow->layout.cursor.y += m_impl->currentWindow->layout.itemSpacing;
}

void Context::indent(f32 amount)
{
    if (!m_impl->currentWindow)
        return;
    if (amount == 0.0f)
        amount = 16.0f;
    m_impl->currentWindow->layout.indentLevel += amount;
}

void Context::unindent(f32 amount)
{
    if (!m_impl->currentWindow)
        return;
    if (amount == 0.0f)
        amount = 16.0f;
    m_impl->currentWindow->layout.indentLevel -= amount;
    m_impl->currentWindow->layout.indentLevel = std::max(0.0f, m_impl->currentWindow->layout.indentLevel);
}

void Context::beginGroup()
{
    // TODO: Implement groups
}

void Context::endGroup()
{
    // TODO: Implement groups
}

void Context::dummy(Vec2 size)
{
    if (!m_impl->currentWindow)
        return;
    m_impl->currentWindow->layout.advanceCursor(size);
}

Vec2 Context::getCursorPos() const
{
    if (!m_impl->currentWindow)
        return {};
    return m_impl->currentWindow->layout.cursor - m_impl->currentWindow->contentRect.min;
}

void Context::setCursorPos(Vec2 pos)
{
    if (!m_impl->currentWindow)
        return;
    m_impl->currentWindow->layout.cursor = m_impl->currentWindow->contentRect.min + pos;
}

Vec2 Context::getCursorScreenPos() const
{
    if (!m_impl->currentWindow)
        return {};
    return m_impl->currentWindow->layout.cursor;
}

void Context::setCursorScreenPos(Vec2 pos)
{
    if (!m_impl->currentWindow)
        return;
    m_impl->currentWindow->layout.cursor = pos;
}

f32 Context::getContentWidth() const
{
    if (!m_impl->currentWindow)
        return 0.0f;
    return m_impl->currentWindow->contentRect.width();
}

f32 Context::getContentHeight() const
{
    if (!m_impl->currentWindow)
        return 0.0f;
    return m_impl->currentWindow->contentRect.height();
}

// ============================================================================
// Item Queries
// ============================================================================

bool Context::isItemHovered() const
{
    return m_impl->lastItemHovered;
}

bool Context::isItemActive() const
{
    return m_impl->activeWidget == m_impl->lastItemId;
}

bool Context::isItemFocused() const
{
    return m_impl->focusedWidget == m_impl->lastItemId;
}

bool Context::isItemClicked(MouseButton button) const
{
    return isItemHovered() && isMouseClicked(button);
}

bool Context::isItemEdited() const
{
    // TODO: Track edit state
    return false;
}

bool Context::isItemActivated() const
{
    // TODO: Track activation state
    return false;
}

bool Context::isItemDeactivated() const
{
    // TODO: Track deactivation state
    return false;
}

Rect Context::getItemRect() const
{
    return m_impl->lastItemRect;
}

Vec2 Context::getItemSize() const
{
    return m_impl->lastItemRect.size();
}

// ============================================================================
// Style/Theme
// ============================================================================

void Context::pushStyleColor(i32 idx, Color color)
{
    // TODO: Implement color stack
    (void)idx;
    (void)color;
}

void Context::popStyleColor(i32 count)
{
    (void)count;
}

void Context::pushStyleVar(i32 idx, f32 value)
{
    (void)idx;
    (void)value;
}

void Context::pushStyleVar(i32 idx, Vec2 value)
{
    (void)idx;
    (void)value;
}

void Context::popStyleVar(i32 count)
{
    (void)count;
}

Theme* Context::getTheme() const
{
    return m_impl->theme.get();
}

void Context::setTheme(std::unique_ptr<Theme> theme)
{
    m_impl->theme = std::move(theme);
}

// ============================================================================
// Fonts
// ============================================================================

FontManager& Context::getFontManager()
{
    return m_impl->fontManager;
}

Font* Context::getFont() const
{
    return m_impl->currentFont;
}

void Context::setFont(Font* font)
{
    m_impl->currentFont = font;
}

void Context::pushFont(Font* font)
{
    m_impl->fontStack.push_back(m_impl->currentFont);
    m_impl->currentFont = font;
}

void Context::popFont()
{
    if (!m_impl->fontStack.empty())
    {
        m_impl->currentFont = m_impl->fontStack.back();
        m_impl->fontStack.pop_back();
    }
}

// ============================================================================
// Clipboard
// ============================================================================

String Context::getClipboardText() const
{
    if (m_impl->getClipboardFn)
    {
        return m_impl->getClipboardFn();
    }
    return {};
}

void Context::setClipboardText(StringView text)
{
    if (m_impl->setClipboardFn)
    {
        m_impl->setClipboardFn(text);
    }
}

// ============================================================================
// Cursor
// ============================================================================

void Context::setMouseCursor(Cursor cursor)
{
    m_impl->mouseCursor = cursor;
}

Cursor Context::getMouseCursor() const
{
    return m_impl->mouseCursor;
}

// ============================================================================
// Options
// ============================================================================

ContextOptions& Context::getOptions()
{
    return m_impl->options;
}

const ContextOptions& Context::getOptions() const
{
    return m_impl->options;
}

// ============================================================================
// Display
// ============================================================================

void Context::setDisplaySize(Vec2 size)
{
    m_impl->displaySize = size;
}

Vec2 Context::getDisplaySize() const
{
    return m_impl->displaySize;
}

void Context::setDisplayFramebufferScale(Vec2 scale)
{
    m_impl->framebufferScale = scale;
}

Vec2 Context::getDisplayFramebufferScale() const
{
    return m_impl->framebufferScale;
}

// ============================================================================
// Time
// ============================================================================

f32 Context::getDeltaTime() const
{
    return m_impl->deltaTime;
}

f64 Context::getTime() const
{
    return m_impl->time;
}

i64 Context::getFrameCount() const
{
    return m_impl->frameCount;
}

}  // namespace dakt::gui

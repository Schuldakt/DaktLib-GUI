/**
 * @file dakt_gui.cpp
 * @brief C API implementation for DaktLib-GUI
 *
 * This file implements the C API by wrapping the C++ classes
 * and providing stable ABI-safe function exports.
 */

#include "dakt/gui/c_api/dakt_gui.h"

// C++ headers
#include "dakt/gui/backend/IRenderBackend.hpp"
#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/core/Types.hpp"
#include "dakt/gui/draw/DrawList.hpp"
#include "dakt/gui/immediate/Immediate.hpp"
#include "dakt/gui/input/Input.hpp"
#include "dakt/gui/style/Style.hpp"

// Backend headers (conditionally included)
#if defined(DAKTLIB_ENABLE_VULKAN)
#include "dakt/gui/backend/vulkan/VulkanBackend.hpp"
#endif
#if defined(DAKTLIB_ENABLE_DX11)
#include "dakt/gui/backend/dx11/DX11Backend.hpp"
#endif
#if defined(DAKTLIB_ENABLE_DX12)
#include "dakt/gui/backend/dx12/DX12Backend.hpp"
#endif
#if defined(DAKTLIB_ENABLE_OPENGL)
#include "dakt/gui/backend/opengl/OpenGLBackend.hpp"
#endif
#if defined(DAKTLIB_ENABLE_METAL)
#include "dakt/gui/backend/metal/MetalBackend.hpp"
#endif

#include <cstring>
#include <memory>
#include <mutex>
#include <string>

// =============================================================================
// Internal Types and State
// =============================================================================

namespace {

/**
 * @brief Internal context wrapper that holds all GUI state
 */
struct InternalContext {
    std::unique_ptr<dakt::gui::Context> context;
    std::unique_ptr<dakt::gui::InputSystem> input;
    std::unique_ptr<dakt::gui::IRenderBackend> backend;
    DuiBackendType backendType = DAKT_GUI_BACKEND_NONE;

    // Cached state for C API queries
    dakt::gui::MouseInput mouseInput{};
    dakt::gui::KeyboardInput keyboardInput{};
};

// Thread-local error handling
thread_local std::string g_lastError;

// Current context (for implicit context functions)
thread_local InternalContext* g_currentContext = nullptr;

// Mutex for context list (if needed for multi-context scenarios)
std::mutex g_contextMutex;

/**
 * @brief Set the last error message
 */
void setError(const char* message) { g_lastError = message ? message : "Unknown error"; }

/**
 * @brief Clear the last error
 */
void clearError() { g_lastError.clear(); }

// =============================================================================
// Type Conversion Helpers
// =============================================================================

inline dakt::gui::Vec2 toVec2(DuiVec2 v) { return dakt::gui::Vec2(v.x, v.y); }

inline DuiVec2 fromVec2(dakt::gui::Vec2 v) { return DuiVec2{v.x, v.y}; }

inline dakt::gui::Rect toRect(DuiRect r) { return dakt::gui::Rect(r.x, r.y, r.width, r.height); }

inline DuiRect fromRect(dakt::gui::Rect r) { return DuiRect{r.x, r.y, r.width, r.height}; }

inline dakt::gui::Color toColor(DuiColor c) { return dakt::gui::Color(c.r, c.g, c.b, c.a); }

inline DuiColor fromColor(dakt::gui::Color c) { return DuiColor{c.r, c.g, c.b, c.a}; }

inline dakt::gui::MouseButton toMouseButton(DuiMButton btn) {
    switch (btn) {
    case DAKT_GUI_MOUSE_LEFT:
        return dakt::gui::MouseButton::Left;
    case DAKT_GUI_MOUSE_RIGHT:
        return dakt::gui::MouseButton::Right;
    case DAKT_GUI_MOUSE_MIDDLE:
        return dakt::gui::MouseButton::Middle;
    case DAKT_GUI_MOUSE_X1:
        return dakt::gui::MouseButton::X1;
    case DAKT_GUI_MOUSE_X2:
        return dakt::gui::MouseButton::X2;
    default:
        return dakt::gui::MouseButton::Left;
    }
}

inline dakt::gui::Key toKey(DuiKey key) {
    // Direct mapping assuming enum values align
    // In production, you'd want explicit mapping for safety
    if (key >= DAKT_GUI_KEY_NONE && key < DAKT_GUI_KEY_COUNT) {
        return static_cast<dakt::gui::Key>(key);
    }
    return dakt::gui::Key::None;
}

inline dakt::gui::WindowFlags toWindowFlags(DuiWinFlags flags) {
    dakt::gui::WindowFlags result = dakt::gui::WindowFlags::None;

    if (flags & DAKT_GUI_WINDOW_NO_TITLE_BAR)
        result = result | dakt::gui::WindowFlags::NoTitleBar;
    if (flags & DAKT_GUI_WINDOW_NO_RESIZE)
        result = result | dakt::gui::WindowFlags::NoResize;
    if (flags & DAKT_GUI_WINDOW_NO_MOVE)
        result = result | dakt::gui::WindowFlags::NoMove;
    if (flags & DAKT_GUI_WINDOW_NO_SCROLLBAR)
        result = result | dakt::gui::WindowFlags::NoScrollbar;
    if (flags & DAKT_GUI_WINDOW_NO_COLLAPSE)
        result = result | dakt::gui::WindowFlags::NoCollapse;
    if (flags & DAKT_GUI_WINDOW_NO_BACKGROUND)
        result = result | dakt::gui::WindowFlags::NoBackground;
    if (flags & DAKT_GUI_WINDOW_ALWAYS_AUTO_SIZE)
        result = result | dakt::gui::WindowFlags::AlwaysAutoResize;

    return result;
}

inline dakt::gui::InputTextFlags toInputTextFlags(DuiITFlags flags) {
    dakt::gui::InputTextFlags result = dakt::gui::InputTextFlags::None;

    if (flags & DAKT_GUI_INPUT_TEXT_CHARS_DECIMAL)
        result = result | dakt::gui::InputTextFlags::CharsDecimal;
    if (flags & DAKT_GUI_INPUT_TEXT_CHARS_HEXADECIMAL)
        result = result | dakt::gui::InputTextFlags::CharsHexadecimal;
    if (flags & DAKT_GUI_INPUT_TEXT_CHARS_UPPERCASE)
        result = result | dakt::gui::InputTextFlags::CharsUppercase;
    if (flags & DAKT_GUI_INPUT_TEXT_CHARS_NO_BLANK)
        result = result | dakt::gui::InputTextFlags::CharsNoBlank;
    if (flags & DAKT_GUI_INPUT_TEXT_AUTO_SELECT_ALL)
        result = result | dakt::gui::InputTextFlags::AutoSelectAll;
    if (flags & DAKT_GUI_INPUT_TEXT_ENTER_RETURNS_TRUE)
        result = result | dakt::gui::InputTextFlags::EnterReturnsTrue;
    if (flags & DAKT_GUI_INPUT_TEXT_PASSWORD)
        result = result | dakt::gui::InputTextFlags::Password;
    if (flags & DAKT_GUI_INPUT_TEXT_READ_ONLY)
        result = result | dakt::gui::InputTextFlags::ReadOnly;

    return result;
}

inline dakt::gui::TreeNodeFlags toTreeNodeFlags(DuiTreeNodeFlags flags) {
    dakt::gui::TreeNodeFlags result = dakt::gui::TreeNodeFlags::None;

    if (flags & DAKT_GUI_TREE_NODE_SELECTED)
        result = result | dakt::gui::TreeNodeFlags::Selected;
    if (flags & DAKT_GUI_TREE_NODE_FRAMED)
        result = result | dakt::gui::TreeNodeFlags::Framed;
    if (flags & DAKT_GUI_TREE_NODE_OPEN_ON_ARROW)
        result = result | dakt::gui::TreeNodeFlags::OpenOnArrow;
    if (flags & DAKT_GUI_TREE_NODE_OPEN_ON_DOUBLE_CLICK)
        result = result | dakt::gui::TreeNodeFlags::OpenOnDoubleClick;
    if (flags & DAKT_GUI_TREE_NODE_LEAF)
        result = result | dakt::gui::TreeNodeFlags::Leaf;
    if (flags & DAKT_GUI_TREE_NODE_DEFAULT_OPEN)
        result = result | dakt::gui::TreeNodeFlags::DefaultOpen;

    return result;
}

inline dakt::gui::SelectableFlags toSelectableFlags(DuiSelectableFlags flags) {
    dakt::gui::SelectableFlags result = dakt::gui::SelectableFlags::None;

    if (flags & DAKT_GUI_SELECTABLE_DONT_CLOSE_POPUPS)
        result = result | dakt::gui::SelectableFlags::DontClosePopups;
    if (flags & DAKT_GUI_SELECTABLE_SPAN_ALL_COLUMNS)
        result = result | dakt::gui::SelectableFlags::SpanAllColumns;
    if (flags & DAKT_GUI_SELECTABLE_ALLOW_DOUBLE_CLICK)
        result = result | dakt::gui::SelectableFlags::AllowDoubleClick;
    if (flags & DAKT_GUI_SELECTABLE_DISABLED)
        result = result | dakt::gui::SelectableFlags::Disabled;

    return result;
}

inline dakt::gui::TableFlags toTableFlags(DuiTableFlags flags) {
    dakt::gui::TableFlags result = dakt::gui::TableFlags::None;

    if (flags & DAKT_GUI_TABLE_RESIZABLE)
        result = result | dakt::gui::TableFlags::Resizable;
    if (flags & DAKT_GUI_TABLE_REORDERABLE)
        result = result | dakt::gui::TableFlags::Reorderable;
    if (flags & DAKT_GUI_TABLE_SORTABLE)
        result = result | dakt::gui::TableFlags::Sortable;
    if (flags & DAKT_GUI_TABLE_ROW_BACKGROUND)
        result = result | dakt::gui::TableFlags::RowBg;
    if (flags & DAKT_GUI_TABLE_BORDERS_INNER_H)
        result = result | dakt::gui::TableFlags::BordersInnerH;
    if (flags & DAKT_GUI_TABLE_BORDERS_INNER_V)
        result = result | dakt::gui::TableFlags::BordersInnerV;
    if (flags & DAKT_GUI_TABLE_BORDERS_OUTER_H)
        result = result | dakt::gui::TableFlags::BordersOuterH;
    if (flags & DAKT_GUI_TABLE_BORDERS_OUTER_V)
        result = result | dakt::gui::TableFlags::BordersOuterV;
    if (flags & DAKT_GUI_TABLE_SCROLL_X)
        result = result | dakt::gui::TableFlags::ScrollX;
    if (flags & DAKT_GUI_TABLE_SCROLL_Y)
        result = result | dakt::gui::TableFlags::ScrollY;

    return result;
}

/**
 * @brief Create a render backend based on type
 */
std::unique_ptr<dakt::gui::IRenderBackend> createBackend(DuiBackendType type) {
    switch (type) {
#if defined(DAKTLIB_ENABLE_VULKAN)
    case DAKT_GUI_BACKEND_VULKAN:
        return dakt::gui::createVulkanBackend();
#endif
#if defined(DAKTLIB_ENABLE_DX11)
    case DAKT_GUI_BACKEND_DX11:
        return dakt::gui::createDX11Backend();
#endif
#if defined(DAKTLIB_ENABLE_DX12)
    case DAKT_GUI_BACKEND_DX12:
        return dakt::gui::createDX12Backend();
#endif
#if defined(DAKTLIB_ENABLE_OPENGL)
    case DAKT_GUI_BACKEND_OPENGL:
        return dakt::gui::createOpenGLBackend();
#endif
#if defined(DAKTLIB_ENABLE_METAL)
    case DAKT_GUI_BACKEND_METAL:
        return dakt::gui::createMetalBackend();
#endif
    default:
        return nullptr;
    }
}

/**
 * @brief Get InternalContext from handle with validation
 */
InternalContext* getContext(DuiCtx ctx) {
    if (!ctx) {
        setError("Invalid context handle (NULL)");
        return nullptr;
    }
    return reinterpret_cast<InternalContext*>(ctx);
}

} // anonymous namespace

// =============================================================================
// Context Lifecycle
// =============================================================================

extern "C" {

DAKTLIB_GUI_API DuiCtx Create(const DuiConfig* config) {
    clearError();

    try {
        auto* internal = new InternalContext();

        // Create input system
        internal->input = std::make_unique<dakt::gui::InputSystem>();

        // Create context
        internal->context = std::make_unique<dakt::gui::Context>();

        // Create backend if specified
        if (config && config->backend != DAKT_GUI_BACKEND_NONE) {
            internal->backend = createBackend(config->backend);

            if (!internal->backend) {
                setError("Failed to create render backend");
                delete internal;
                return nullptr;
            }

            internal->backendType = config->backend;

            // Initialize backend
            uint32_t width = config->width > 0 ? config->width : 1280;
            uint32_t height = config->height > 0 ? config->height : 720;

            if (!internal->backend->initialize(config->windowHandle, width, height)) {
                setError("Failed to initialize render backend");
                delete internal;
                return nullptr;
            }
        }

        // Set as current context
        g_currentContext = internal;

        return reinterpret_cast<DuiCtx>(internal);

    } catch (const std::exception& e) {
        setError(e.what());
        return nullptr;
    }
}

DAKTLIB_GUI_API void Destroy(DuiCtx ctx) {
    clearError();

    InternalContext* internal = getContext(ctx);
    if (!internal) {
        return;
    }

    // Shutdown backend
    if (internal->backend) {
        internal->backend->shutdown();
    }

    // Clear current context if this was it
    if (g_currentContext == internal) {
        g_currentContext = nullptr;
    }

    delete internal;
}

DAKTLIB_GUI_API DuiCtx GetContext(void) { return reinterpret_cast<DuiCtx>(g_currentContext); }

DAKTLIB_GUI_API void SetContext(DuiCtx ctx) { g_currentContext = reinterpret_cast<InternalContext*>(ctx); }

// =============================================================================
// Frame Management
// =============================================================================

DAKTLIB_GUI_API void NewFrame(DuiCtx ctx, float deltaTime) {
    clearError();

    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }

    // Begin backend frame
    if (internal->backend) {
        internal->backend->beginFrame();
    }

    // Reset input frame state
    if (internal->input) {
        internal->input->resetFrameState();
    }

    // Begin immediate mode frame
    dakt::gui::beginFrame(internal->context.get(), deltaTime);
}

DAKTLIB_GUI_API void EndFrame(DuiCtx ctx) {
    clearError();

    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }

    // End immediate mode frame
    dakt::gui::endFrame();
}

DAKTLIB_GUI_API void Render(DuiCtx ctx) {
    clearError();

    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !internal->backend) {
        return;
    }

    // Submit draw list to backend
    const dakt::gui::DrawList& drawList = internal->context->getDrawList();
    internal->backend->submit(drawList);

    // End backend frame
    internal->backend->endFrame();
}

DAKTLIB_GUI_API void Present(DuiCtx ctx) {
    clearError();

    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->backend) {
        return;
    }

    internal->backend->present();
}

// =============================================================================
// Input Handling
// =============================================================================

DAKTLIB_GUI_API void ProcMouse(DuiCtx ctx, const DuiMInput* input) {
    clearError();

    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->input || !input) {
        return;
    }

    dakt::gui::MouseInput mouseIn;
    mouseIn.position = dakt::gui::Vec2(input->x, input->y);
    mouseIn.wheelDelta = input->wheelY;

    // Decode button bitmask
    mouseIn.buttons[0] = (input->buttons & (1 << 0)) != 0; // Left
    mouseIn.buttons[1] = (input->buttons & (1 << 1)) != 0; // Right
    mouseIn.buttons[2] = (input->buttons & (1 << 2)) != 0; // Middle
    mouseIn.buttons[3] = (input->buttons & (1 << 3)) != 0; // X1
    mouseIn.buttons[4] = (input->buttons & (1 << 4)) != 0; // X2

    internal->input->processMouse(mouseIn);
    internal->mouseInput = mouseIn;
}

DAKTLIB_GUI_API void ProcKey(DuiCtx ctx, const DuiKeyInput* input) {
    clearError();

    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->input || !input) {
        return;
    }

    dakt::gui::KeyboardInput kbIn;

    // Copy key states
    for (int i = 0; i < DAKT_GUI_KEY_COUNT && i < 256; ++i) {
        kbIn.keyPressed[i] = input->keysDown[i] != 0;
    }

    kbIn.shift = input->shift != 0;
    kbIn.ctrl = input->ctrl != 0;
    kbIn.alt = input->alt != 0;

    internal->input->processKeyboard(kbIn);
    internal->keyboardInput = kbIn;
}

DAKTLIB_GUI_API void ProcText(DuiCtx ctx, const char* text) {
    clearError();

    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->input || !text) {
        return;
    }

    dakt::gui::KeyboardInput kbIn = internal->keyboardInput;
    kbIn.textInput = text;
    internal->input->processKeyboard(kbIn);
}

DAKTLIB_GUI_API int32_t MDown(DuiCtx ctx, DuiMButton button) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->input) {
        return 0;
    }
    return internal->input->isMouseButtonDown(toMouseButton(button)) ? 1 : 0;
}

DAKTLIB_GUI_API int32_t MClicked(DuiCtx ctx, DuiMButton button) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->input) {
        return 0;
    }
    return internal->input->isMouseButtonPressed(toMouseButton(button)) ? 1 : 0;
}

DAKTLIB_GUI_API int32_t MReleased(DuiCtx ctx, DuiMButton button) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->input) {
        return 0;
    }
    return internal->input->isMouseButtonReleased(toMouseButton(button)) ? 1 : 0;
}

DAKTLIB_GUI_API DuiVec2 MPos(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->input) {
        return DuiVec2{0.0f, 0.0f};
    }
    return fromVec2(internal->input->getMousePosition());
}

DAKTLIB_GUI_API int32_t KeyDown(DuiCtx ctx, DuiKey key) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->input) {
        return 0;
    }
    return internal->input->isKeyDown(toKey(key)) ? 1 : 0;
}

DAKTLIB_GUI_API int32_t KeyPressed(DuiCtx ctx, DuiKey key) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->input) {
        return 0;
    }
    return internal->input->isKeyPressed(toKey(key)) ? 1 : 0;
}

// =============================================================================
// Window Management
// =============================================================================

DAKTLIB_GUI_API int32_t BeginWin(DuiCtx ctx, const char* name, int32_t* open, DuiWinFlags flags) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !name) {
        return 0;
    }

    bool openBool = open ? (*open != 0) : true;
    bool* openPtr = open ? &openBool : nullptr;

    bool result = dakt::gui::beginWindow(name, openPtr, toWindowFlags(flags));

    if (open && openPtr) {
        *open = openBool ? 1 : 0;
    }

    return result ? 1 : 0;
}

DAKTLIB_GUI_API void EndWin(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::endWindow();
}

DAKTLIB_GUI_API void SetWinPos(DuiCtx ctx, DuiVec2 pos) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::setNextWindowPos(toVec2(pos));
}

DAKTLIB_GUI_API void SetNextWinSize(DuiCtx ctx, DuiVec2 size) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::setNextWindowSize(toVec2(size));
}

DAKTLIB_GUI_API int32_t BeginChild(DuiCtx ctx, const char* id, DuiVec2 size, int32_t border) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !id) {
        return 0;
    }
    return dakt::gui::beginChild(id, toVec2(size), border != 0) ? 1 : 0;
}

DAKTLIB_GUI_API void EndChild(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::endChild();
}

// =============================================================================
// Basic Widgets
// =============================================================================

DAKTLIB_GUI_API void DisplayText(DuiCtx ctx, const char* text) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !text) {
        return;
    }
    dakt::gui::text(text);
}

DAKTLIB_GUI_API void ColoredText(DuiCtx ctx, DuiColor color, const char* text) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !text) {
        return;
    }
    dakt::gui::textColored(toColor(color), text);
}

DAKTLIB_GUI_API void DisabledText(DuiCtx ctx, const char* text) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !text) {
        return;
    }
    dakt::gui::textDisabled(text);
}

DAKTLIB_GUI_API void WrapText(DuiCtx ctx, const char* text) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !text) {
        return;
    }
    dakt::gui::textWrapped(text);
}

DAKTLIB_GUI_API void TextLabel(DuiCtx ctx, const char* label, const char* text) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label || !text) {
        return;
    }
    dakt::gui::labelText(label, text);
}

DAKTLIB_GUI_API void Bullet(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::bullet();
}

DAKTLIB_GUI_API int32_t Button(DuiCtx ctx, const char* label) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label) {
        return 0;
    }
    return dakt::gui::button(label) ? 1 : 0;
}

DAKTLIB_GUI_API int32_t SizedButton(DuiCtx ctx, const char* label, DuiVec2 size) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label) {
        return 0;
    }
    return dakt::gui::button(label, toVec2(size)) ? 1 : 0;
}

DAKTLIB_GUI_API int32_t SmallButton(DuiCtx ctx, const char* label) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label) {
        return 0;
    }
    return dakt::gui::smallButton(label) ? 1 : 0;
}

DAKTLIB_GUI_API int32_t Checkbox(DuiCtx ctx, const char* label, int32_t* value) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label || !value) {
        return 0;
    }

    bool boolValue = *value != 0;
    bool changed = dakt::gui::checkbox(label, &boolValue);
    *value = boolValue ? 1 : 0;

    return changed ? 1 : 0;
}

DAKTLIB_GUI_API int32_t RadButton(DuiCtx ctx, const char* label, int32_t active) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label) {
        return 0;
    }
    return dakt::gui::radioButton(label, active != 0) ? 1 : 0;
}

DAKTLIB_GUI_API void ProgBar(DuiCtx ctx, float fraction, DuiVec2 size, const char* overlay) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::progressBar(fraction, toVec2(size), overlay);
}

// =============================================================================
// Input Widgets
// =============================================================================

DAKTLIB_GUI_API int32_t InputTextField(DuiCtx ctx, const char* label, char* buffer, size_t bufferSize, DuiITFlags flags) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label || !buffer || bufferSize == 0) {
        return 0;
    }
    return dakt::gui::inputText(label, buffer, bufferSize, toInputTextFlags(flags)) ? 1 : 0;
}

DAKTLIB_GUI_API int32_t MultiTextField(DuiCtx ctx, const char* label, char* buffer, size_t bufferSize, DuiVec2 size, DuiITFlags flags) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label || !buffer || bufferSize == 0) {
        return 0;
    }
    return dakt::gui::inputTextMultiline(label, buffer, bufferSize, toVec2(size), toInputTextFlags(flags)) ? 1 : 0;
}

DAKTLIB_GUI_API int32_t IntField(DuiCtx ctx, const char* label, int32_t* value, int32_t step, int32_t stepFast) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label || !value) {
        return 0;
    }

    int intValue = static_cast<int>(*value);
    bool changed = dakt::gui::inputInt(label, &intValue, step, stepFast);
    *value = static_cast<int32_t>(intValue);

    return changed ? 1 : 0;
}

DAKTLIB_GUI_API int32_t FloatField(DuiCtx ctx, const char* label, float* value, float step, float stepFast, const char* format) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label || !value) {
        return 0;
    }
    return dakt::gui::inputFloat(label, value, step, stepFast, format ? format : "%.3f") ? 1 : 0;
}

// =============================================================================
// Sliders
// =============================================================================

DAKTLIB_GUI_API int32_t IntSlider(DuiCtx ctx, const char* label, int32_t* value, int32_t min, int32_t max, const char* format) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label || !value) {
        return 0;
    }

    int intValue = static_cast<int>(*value);
    bool changed = dakt::gui::sliderInt(label, &intValue, min, max, format ? format : "%d");
    *value = static_cast<int32_t>(intValue);

    return changed ? 1 : 0;
}

DAKTLIB_GUI_API int32_t FloatSlider(DuiCtx ctx, const char* label, float* value, float min, float max, const char* format) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label || !value) {
        return 0;
    }
    return dakt::gui::sliderFloat(label, value, min, max, format ? format : "%.3f") ? 1 : 0;
}

DAKTLIB_GUI_API int32_t Vec2Slider(DuiCtx ctx, const char* label, DuiVec2* value, float min, float max, const char* format) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label || !value) {
        return 0;
    }

    dakt::gui::Vec2 vec = toVec2(*value);
    bool changed = dakt::gui::sliderVec2(label, &vec, min, max, format ? format : "%.3f");
    *value = fromVec2(vec);

    return changed ? 1 : 0;
}

// =============================================================================
// Drag Inputs
// =============================================================================

DAKTLIB_GUI_API int32_t DragInt(DuiCtx ctx, const char* label, int32_t* value, float speed, int32_t min, int32_t max) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label || !value) {
        return 0;
    }

    int intValue = static_cast<int>(*value);
    bool changed = dakt::gui::dragInt(label, &intValue, speed, min, max);
    *value = static_cast<int32_t>(intValue);

    return changed ? 1 : 0;
}

DAKTLIB_GUI_API int32_t DragFloat(DuiCtx ctx, const char* label, float* value, float speed, float min, float max, const char* format) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label || !value) {
        return 0;
    }
    return dakt::gui::dragFloat(label, value, speed, min, max, format ? format : "%.3f") ? 1 : 0;
}

// =============================================================================
// Color Editors
// =============================================================================

DAKTLIB_GUI_API int32_t ColorEdit3(DuiCtx ctx, const char* label, DuiColor* color) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label || !color) {
        return 0;
    }

    dakt::gui::Color col = toColor(*color);
    bool changed = dakt::gui::colorEdit3(label, &col);
    *color = fromColor(col);

    return changed ? 1 : 0;
}

DAKTLIB_GUI_API int32_t ColorEdit4(DuiCtx ctx, const char* label, DuiColor* color) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label || !color) {
        return 0;
    }

    dakt::gui::Color col = toColor(*color);
    bool changed = dakt::gui::colorEdit4(label, &col);
    *color = fromColor(col);

    return changed ? 1 : 0;
}

DAKTLIB_GUI_API int32_t ColorPicker3(DuiCtx ctx, const char* label, DuiColor* color) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label || !color) {
        return 0;
    }

    dakt::gui::Color col = toColor(*color);
    bool changed = dakt::gui::colorPicker3(label, &col);
    *color = fromColor(col);

    return changed ? 1 : 0;
}

DAKTLIB_GUI_API int32_t ColorPicker4(DuiCtx ctx, const char* label, DuiColor* color) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label || !color) {
        return 0;
    }

    dakt::gui::Color col = toColor(*color);
    bool changed = dakt::gui::colorPicker4(label, &col);
    *color = fromColor(col);

    return changed ? 1 : 0;
}

// =============================================================================
// Combo Box / Dropdown
// =============================================================================

DAKTLIB_GUI_API int32_t BeginCombo(DuiCtx ctx, const char* label, const char* previewValue) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label) {
        return 0;
    }
    return dakt::gui::beginCombo(label, previewValue ? previewValue : "") ? 1 : 0;
}

DAKTLIB_GUI_API void EndCombo(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::endCombo();
}

DAKTLIB_GUI_API int32_t SimpCombo(DuiCtx ctx, const char* label, int32_t* currentItem, const char* const* items, int32_t itemCount) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label || !currentItem || !items || itemCount <= 0) {
        return 0;
    }

    int index = static_cast<int>(*currentItem);
    bool changed = dakt::gui::combo(label, &index, items, itemCount);
    *currentItem = static_cast<int32_t>(index);

    return changed ? 1 : 0;
}

// =============================================================================
// Trees
// =============================================================================

DAKTLIB_GUI_API int32_t TreeNode(DuiCtx ctx, const char* label, DuiTreeNodeFlags flags) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label) {
        return 0;
    }
    return dakt::gui::treeNode(label, toTreeNodeFlags(flags)) ? 1 : 0;
}

DAKTLIB_GUI_API void TreePop(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::treePop();
}

DAKTLIB_GUI_API int32_t CollapHead(DuiCtx ctx, const char* label, DuiTreeNodeFlags flags) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label) {
        return 0;
    }
    return dakt::gui::collapsingHeader(label, toTreeNodeFlags(flags)) ? 1 : 0;
}

// =============================================================================
// Selectables & Lists
// =============================================================================

DAKTLIB_GUI_API int32_t SelectableItem(DuiCtx ctx, const char* label, int32_t* selected, DuiSelectableFlags flags) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label) {
        return 0;
    }

    bool sel = selected ? (*selected != 0) : false;
    bool clicked = dakt::gui::selectable(label, &sel, toSelectableFlags(flags));

    if (selected) {
        *selected = sel ? 1 : 0;
    }

    return clicked ? 1 : 0;
}

DAKTLIB_GUI_API int32_t BeginListBox(DuiCtx ctx, const char* label, DuiVec2 size) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label) {
        return 0;
    }
    return dakt::gui::beginListBox(label, toVec2(size)) ? 1 : 0;
}

DAKTLIB_GUI_API void EndListBox(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::endListBox();
}

// =============================================================================
// Menus
// =============================================================================

DAKTLIB_GUI_API int32_t BeginMainMenuBar(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return 0;
    }
    return dakt::gui::beginMainMenuBar() ? 1 : 0;
}

DAKTLIB_GUI_API void EndMainMenuBar(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::endMainMenuBar();
}

DAKTLIB_GUI_API int32_t BeginMenuBar(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return 0;
    }
    return dakt::gui::beginMenuBar() ? 1 : 0;
}

DAKTLIB_GUI_API void EndMenuBar(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::endMenuBar();
}

DAKTLIB_GUI_API int32_t BeginMenu(DuiCtx ctx, const char* label) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label) {
        return 0;
    }
    return dakt::gui::beginMenu(label) ? 1 : 0;
}

DAKTLIB_GUI_API void EndMenu(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::endMenu();
}

DAKTLIB_GUI_API int32_t MenuItem(DuiCtx ctx, const char* label, const char* shortcut, int32_t* selected, int32_t enabled) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label) {
        return 0;
    }

    bool sel = selected ? (*selected != 0) : false;
    bool* selPtr = selected ? &sel : nullptr;

    bool clicked = dakt::gui::menuItem(label, shortcut, selPtr, enabled != 0);

    if (selected) {
        *selected = sel ? 1 : 0;
    }

    return clicked ? 1 : 0;
}

// =============================================================================
// Popups & Modals
// =============================================================================

DAKTLIB_GUI_API void OpenPop(DuiCtx ctx, const char* id) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !id) {
        return;
    }
    dakt::gui::openPopup(id);
}

DAKTLIB_GUI_API int32_t BeginPop(DuiCtx ctx, const char* id) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !id) {
        return 0;
    }
    return dakt::gui::beginPopup(id) ? 1 : 0;
}

DAKTLIB_GUI_API int32_t BeginModal(DuiCtx ctx, const char* name, int32_t* open) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !name) {
        return 0;
    }

    bool openBool = open ? (*open != 0) : true;
    bool* openPtr = open ? &openBool : nullptr;

    bool result = dakt::gui::beginPopupModal(name, openPtr);

    if (open) {
        *open = openBool ? 1 : 0;
    }

    return result ? 1 : 0;
}

DAKTLIB_GUI_API void EndPop(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::endPopup();
}

DAKTLIB_GUI_API void CloseCurrentPop(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::closeCurrentPopup();
}

// =============================================================================
// Tables
// =============================================================================

DAKTLIB_GUI_API int32_t BeginTable(DuiCtx ctx, const char* id, int32_t columnCount, DuiTableFlags flags, DuiVec2 outerSize) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !id || columnCount <= 0) {
        return 0;
    }
    return dakt::gui::beginTable(id, columnCount, toTableFlags(flags), toVec2(outerSize)) ? 1 : 0;
}

DAKTLIB_GUI_API void EndTable(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::endTable();
}

DAKTLIB_GUI_API void TableColumn(DuiCtx ctx, const char* label, float initWidthOrWeight) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::tableSetupColumn(label, initWidthOrWeight);
}

DAKTLIB_GUI_API void TableHeadersRow(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::tableHeadersRow();
}

DAKTLIB_GUI_API void TableNextRow(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::tableNextRow();
}

DAKTLIB_GUI_API int32_t TableNextColumn(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return 0;
    }
    return dakt::gui::tableNextColumn() ? 1 : 0;
}

DAKTLIB_GUI_API int32_t TableSetColumnIdx(DuiCtx ctx, int32_t columnIndex) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return 0;
    }
    return dakt::gui::tableSetColumnIndex(columnIndex) ? 1 : 0;
}

// =============================================================================
// Tabs
// =============================================================================

DAKTLIB_GUI_API int32_t BeginTabBar(DuiCtx ctx, const char* id) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !id) {
        return 0;
    }
    return dakt::gui::beginTabBar(id) ? 1 : 0;
}

DAKTLIB_GUI_API void EndTabBar(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::endTabBar();
}

DAKTLIB_GUI_API int32_t BeginTabItem(DuiCtx ctx, const char* label, int32_t* open) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !label) {
        return 0;
    }

    bool openBool = open ? (*open != 0) : true;
    bool* openPtr = open ? &openBool : nullptr;

    bool result = dakt::gui::beginTabItem(label, openPtr);

    if (open) {
        *open = openBool ? 1 : 0;
    }

    return result ? 1 : 0;
}

DAKTLIB_GUI_API void EndTabItem(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::endTabItem();
}

// =============================================================================
// Layout Utilities
// =============================================================================

DAKTLIB_GUI_API void HSpacing(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::sameLine();
}

DAKTLIB_GUI_API void Separator(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::separator();
}

DAKTLIB_GUI_API void VSpacing(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::spacing();
}

DAKTLIB_GUI_API void Dummy(DuiCtx ctx, DuiVec2 size) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::dummy(toVec2(size));
}

DAKTLIB_GUI_API void Indent(DuiCtx ctx, float width) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::indent(width);
}

DAKTLIB_GUI_API void Unindent(DuiCtx ctx, float width) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::unindent(width);
}

DAKTLIB_GUI_API void BeginGroup(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::beginGroup();
}

DAKTLIB_GUI_API void EndGroup(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::endGroup();
}

DAKTLIB_GUI_API void SetNextItemWidth(DuiCtx ctx, float width) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::setNextItemWidth(width);
}

DAKTLIB_GUI_API void PushItemWidth(DuiCtx ctx, float width) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::pushItemWidth(width);
}

DAKTLIB_GUI_API void PopItemWidth(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::popItemWidth();
}

// =============================================================================
// ID Stack
// =============================================================================

DAKTLIB_GUI_API void PushIdStr(DuiCtx ctx, const char* id) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !id) {
        return;
    }
    dakt::gui::pushID(id);
}

DAKTLIB_GUI_API void PushIdInt(DuiCtx ctx, int32_t id) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::pushID(id);
}

DAKTLIB_GUI_API void PushIdPtr(DuiCtx ctx, const void* id) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !id) {
        return;
    }
    dakt::gui::pushID(id);
}

DAKTLIB_GUI_API void PopId(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::popID();
}

// =============================================================================
// Style
// =============================================================================

DAKTLIB_GUI_API void GetStyleColors(DuiCtx ctx, DuiStyleColors* colors) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !colors) {
        return;
    }

    const dakt::gui::ColorScheme& scheme = internal->context->getTheme().colors();

    colors->text = fromColor(scheme.text);
    colors->textDisabled = fromColor(scheme.textDisabled);
    colors->windowBg = fromColor(scheme.background);
    colors->childBg = fromColor(scheme.surface);
    colors->popupBg = fromColor(scheme.surface);
    colors->border = fromColor(scheme.border);
    colors->frameBg = fromColor(scheme.surface);
    colors->frameBgHovered = fromColor(scheme.surfaceHover);
    colors->frameBgActive = fromColor(scheme.surfaceActive);
    colors->titleBg = fromColor(scheme.primary);
    colors->titleBgActive = fromColor(scheme.primaryHover);
    colors->button = fromColor(scheme.primary);
    colors->buttonHovered = fromColor(scheme.primaryHover);
    colors->buttonActive = fromColor(scheme.primaryActive);
    colors->header = fromColor(scheme.primary);
    colors->headerHovered = fromColor(scheme.primaryHover);
    colors->headerActive = fromColor(scheme.primaryActive);
    colors->separator = fromColor(scheme.border);
    colors->checkMark = fromColor(scheme.success);
    colors->sliderGrab = fromColor(scheme.primary);
    colors->sliderGrabActive = fromColor(scheme.primaryActive);
}

DAKTLIB_GUI_API void SetStyleColors(DuiCtx ctx, const DuiStyleColors* colors) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !colors) {
        return;
    }

    // Note: This would require modifying the Theme's ColorScheme
    // For now, this is a stub that would need full implementation
    // based on how the C++ Theme class is designed
}

DAKTLIB_GUI_API void StyleColorsDark(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    internal->context->getTheme().setDarkMode(true);
}

DAKTLIB_GUI_API void StyleColorsLight(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    internal->context->getTheme().setDarkMode(false);
}

// =============================================================================
// Tooltips
// =============================================================================

DAKTLIB_GUI_API void BeginTooltip(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::beginTooltip();
}

DAKTLIB_GUI_API void EndTooltip(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::endTooltip();
}

DAKTLIB_GUI_API void SetTooltip(DuiCtx ctx, const char* text) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context || !text) {
        return;
    }
    dakt::gui::setTooltip(text);
}

// =============================================================================
// Item State Queries
// =============================================================================

DAKTLIB_GUI_API int32_t ItemHovered(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return 0;
    }
    return dakt::gui::isItemHovered() ? 1 : 0;
}

DAKTLIB_GUI_API int32_t ItemActive(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return 0;
    }
    return dakt::gui::isItemActive() ? 1 : 0;
}

DAKTLIB_GUI_API int32_t ItemFocused(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return 0;
    }
    return dakt::gui::isItemFocused() ? 1 : 0;
}

DAKTLIB_GUI_API int32_t ItemClicked(DuiCtx ctx, DuiMButton button) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return 0;
    }
    return dakt::gui::isItemClicked(toMouseButton(button)) ? 1 : 0;
}

DAKTLIB_GUI_API int32_t ItemEdited(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return 0;
    }
    return dakt::gui::isItemEdited() ? 1 : 0;
}

DAKTLIB_GUI_API int32_t ItemActivated(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return 0;
    }
    return dakt::gui::isItemActivated() ? 1 : 0;
}

DAKTLIB_GUI_API int32_t ItemDeactivated(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return 0;
    }
    return dakt::gui::isItemDeactivated() ? 1 : 0;
}

DAKTLIB_GUI_API DuiRect GetRect(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return DuiRect{0, 0, 0, 0};
    }
    return fromRect(dakt::gui::getItemRect());
}

// =============================================================================
// Utilities
// =============================================================================

DAKTLIB_GUI_API DuiVec2 GetRegionSize(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return DuiVec2{0, 0};
    }
    return fromVec2(dakt::gui::getContentRegionAvail());
}

DAKTLIB_GUI_API DuiVec2 GetCursorPos(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return DuiVec2{0, 0};
    }
    return fromVec2(dakt::gui::getCursorPos());
}

DAKTLIB_GUI_API void SetCursorPos(DuiCtx ctx, DuiVec2 pos) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return;
    }
    dakt::gui::setCursorPos(toVec2(pos));
}

DAKTLIB_GUI_API DuiVec2 GetWinSize(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return DuiVec2{0, 0};
    }
    return fromVec2(dakt::gui::getWindowSize());
}

DAKTLIB_GUI_API DuiVec2 GetWinPos(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->context) {
        return DuiVec2{0, 0};
    }
    return fromVec2(dakt::gui::getWindowPos());
}

// =============================================================================
// Backend Information
// =============================================================================

DAKTLIB_GUI_API DuiBackendType GetBackendType(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal) {
        return DAKT_GUI_BACKEND_NONE;
    }
    return internal->backendType;
}

DAKTLIB_GUI_API const char* GetBackendName(DuiCtx ctx) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->backend) {
        return "None";
    }
    return internal->backend->getName();
}

DAKTLIB_GUI_API void Resize(DuiContextHandle ctx, uint32_t width, uint32_t height) {
    InternalContext* internal = getContext(ctx);
    if (!internal || !internal->backend) {
        return;
    }
    internal->backend->resize(width, height);
}

// =============================================================================
// Error Handling
// =============================================================================

DAKTLIB_GUI_API const char* GetLastErr(void) {
    if (g_lastError.empty()) {
        return nullptr;
    }
    return g_lastError.c_str();
}

DAKTLIB_GUI_API void ClearErr(void) { clearError(); }

// =============================================================================
// Version Information
// =============================================================================

DAKTLIB_GUI_API const char* GetVersion(void) { return "0.1.0"; }

DAKTLIB_GUI_API uint32_t GetVersionNum(void) {
    // MAJOR * 10000 + MINOR * 100 + PATCH
    return 0 * 10000 + 1 * 100 + 0;
}

} // extern "C"
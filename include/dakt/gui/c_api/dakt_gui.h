/**
 * @file dakt_gui.h
 * @brief C API for DaktLib-GUI - ClangSharp compatible
 *
 * This header provides a stable C ABI for the GUI library,
 * suitable for P/Invoke bindings and FFI from other languages.
 */

#ifndef DAKTLIB_GUI_C_API_H
#define DAKTLIB_GUI_C_API_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =============================================================================
 * Export Macros
 * ============================================================================= */

#if defined(DAKTLIB_GUI_STATIC)
#define DAKTLIB_GUI_API
#elif defined(_WIN32)
#ifdef DAKTLIB_GUI_EXPORTS
#define DAKTLIB_GUI_API __declspec(dllexport)
#else
#define DAKTLIB_GUI_API __declspec(dllimport)
#endif
#else
#define DAKTLIB_GUI_API __attribute__((visibility("default")))
#endif

/* =============================================================================
 * Opaque Handle Types
 * ============================================================================= */

typedef struct DuiContext* DuiCtx;
typedef struct DuiBackend* DuiBackendHandle;
typedef struct DuiFont* DuiFontHandle;
typedef struct DuiDrawList* DuiDrawListHandle;

/* =============================================================================
 * Basic Types
 * ============================================================================= */

typedef struct DuiVec2 {
    float x;
    float y;
} DuiVec2;

typedef struct DuiVec4 {
    float x;
    float y;
    float z;
    float w;
} DuiVec4;

typedef struct DuiRect {
    float x;
    float y;
    float width;
    float height;
} DuiRect;

typedef struct DuiColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} DuiColor;

typedef struct DuiEdgeInsets {
    float top;
    float right;
    float bottom;
    float left;
} DuiEdgeInsets;

/* =============================================================================
 * Enumerations
 * ============================================================================= */

typedef enum DuiBackendType { DAKT_GUI_BACKEND_NONE = 0, DAKT_GUI_BACKEND_VULKAN, DAKT_GUI_BACKEND_DX11, DAKT_GUI_BACKEND_DX12, DAKT_GUI_BACKEND_OPENGL, DAKT_GUI_BACKEND_METAL } DuiBackendType;

typedef enum DuiMButton { DAKT_GUI_MOUSE_LEFT = 0, DAKT_GUI_MOUSE_RIGHT = 1, DAKT_GUI_MOUSE_MIDDLE = 2, DAKT_GUI_MOUSE_X1 = 3, DAKT_GUI_MOUSE_X2 = 4 } DuiMButton;

typedef enum DuiKey {
    DAKT_GUI_KEY_NONE = 0,
    DAKT_GUI_KEY_TAB,
    DAKT_GUI_KEY_LEFT,
    DAKT_GUI_KEY_RIGHT,
    DAKT_GUI_KEY_UP,
    DAKT_GUI_KEY_DOWN,
    DAKT_GUI_KEY_PAGE_UP,
    DAKT_GUI_KEY_PAGE_DOWN,
    DAKT_GUI_KEY_HOME,
    DAKT_GUI_KEY_END,
    DAKT_GUI_KEY_INSERT,
    DAKT_GUI_KEY_DELETE,
    DAKT_GUI_KEY_BACKSPACE,
    DAKT_GUI_KEY_SPACE,
    DAKT_GUI_KEY_ENTER,
    DAKT_GUI_KEY_ESCAPE,
    DAKT_GUI_KEY_A,
    DAKT_GUI_KEY_B,
    DAKT_GUI_KEY_C,
    DAKT_GUI_KEY_D,
    DAKT_GUI_KEY_E,
    DAKT_GUI_KEY_F,
    DAKT_GUI_KEY_G,
    DAKT_GUI_KEY_H,
    DAKT_GUI_KEY_I,
    DAKT_GUI_KEY_J,
    DAKT_GUI_KEY_K,
    DAKT_GUI_KEY_L,
    DAKT_GUI_KEY_M,
    DAKT_GUI_KEY_N,
    DAKT_GUI_KEY_O,
    DAKT_GUI_KEY_P,
    DAKT_GUI_KEY_Q,
    DAKT_GUI_KEY_R,
    DAKT_GUI_KEY_S,
    DAKT_GUI_KEY_T,
    DAKT_GUI_KEY_U,
    DAKT_GUI_KEY_V,
    DAKT_GUI_KEY_W,
    DAKT_GUI_KEY_X,
    DAKT_GUI_KEY_Y,
    DAKT_GUI_KEY_Z,
    DAKT_GUI_KEY_0,
    DAKT_GUI_KEY_1,
    DAKT_GUI_KEY_2,
    DAKT_GUI_KEY_3,
    DAKT_GUI_KEY_4,
    DAKT_GUI_KEY_5,
    DAKT_GUI_KEY_6,
    DAKT_GUI_KEY_7,
    DAKT_GUI_KEY_8,
    DAKT_GUI_KEY_9,
    DAKT_GUI_KEY_F1,
    DAKT_GUI_KEY_F2,
    DAKT_GUI_KEY_F3,
    DAKT_GUI_KEY_F4,
    DAKT_GUI_KEY_F5,
    DAKT_GUI_KEY_F6,
    DAKT_GUI_KEY_F7,
    DAKT_GUI_KEY_F8,
    DAKT_GUI_KEY_F9,
    DAKT_GUI_KEY_F10,
    DAKT_GUI_KEY_F11,
    DAKT_GUI_KEY_F12,
    DAKT_GUI_KEY_COUNT
} DuiKey;

typedef enum DuiWinFlags {
    DAKT_GUI_WINDOW_NONE = 0,
    DAKT_GUI_WINDOW_NO_TITLE_BAR = 1 << 0,
    DAKT_GUI_WINDOW_NO_RESIZE = 1 << 1,
    DAKT_GUI_WINDOW_NO_MOVE = 1 << 2,
    DAKT_GUI_WINDOW_NO_SCROLLBAR = 1 << 3,
    DAKT_GUI_WINDOW_NO_COLLAPSE = 1 << 4,
    DAKT_GUI_WINDOW_NO_BACKGROUND = 1 << 5,
    DAKT_GUI_WINDOW_NO_SAVED_SETTINGS = 1 << 6,
    DAKT_GUI_WINDOW_ALWAYS_AUTO_SIZE = 1 << 7,
    DAKT_GUI_WINDOW_NO_NAV = 1 << 8
} DuiWinFlags;

typedef enum DuiITFlags {
    DAKT_GUI_INPUT_TEXT_NONE = 0,
    DAKT_GUI_INPUT_TEXT_CHARS_DECIMAL = 1 << 0,
    DAKT_GUI_INPUT_TEXT_CHARS_HEXADECIMAL = 1 << 1,
    DAKT_GUI_INPUT_TEXT_CHARS_UPPERCASE = 1 << 2,
    DAKT_GUI_INPUT_TEXT_CHARS_NO_BLANK = 1 << 3,
    DAKT_GUI_INPUT_TEXT_AUTO_SELECT_ALL = 1 << 4,
    DAKT_GUI_INPUT_TEXT_ENTER_RETURNS_TRUE = 1 << 5,
    DAKT_GUI_INPUT_TEXT_PASSWORD = 1 << 6,
    DAKT_GUI_INPUT_TEXT_READ_ONLY = 1 << 7,
    DAKT_GUI_INPUT_TEXT_MULTILINE = 1 << 8
} DuiITFlags;

typedef enum DuiTreeNodeFlags {
    DAKT_GUI_TREE_NODE_NONE = 0,
    DAKT_GUI_TREE_NODE_SELECTED = 1 << 0,
    DAKT_GUI_TREE_NODE_FRAMED = 1 << 1,
    DAKT_GUI_TREE_NODE_OPEN_ON_ARROW = 1 << 2,
    DAKT_GUI_TREE_NODE_OPEN_ON_DOUBLE_CLICK = 1 << 3,
    DAKT_GUI_TREE_NODE_LEAF = 1 << 4,
    DAKT_GUI_TREE_NODE_DEFAULT_OPEN = 1 << 5
} DuiTreeNodeFlags;

typedef enum DuiSelectableFlags {
    DAKT_GUI_SELECTABLE_NONE = 0,
    DAKT_GUI_SELECTABLE_DONT_CLOSE_POPUPS = 1 << 0,
    DAKT_GUI_SELECTABLE_SPAN_ALL_COLUMNS = 1 << 1,
    DAKT_GUI_SELECTABLE_ALLOW_DOUBLE_CLICK = 1 << 2,
    DAKT_GUI_SELECTABLE_DISABLED = 1 << 3
} DuiSelectableFlags;

typedef enum DuiTableFlags {
    DAKT_GUI_TABLE_NONE = 0,
    DAKT_GUI_TABLE_RESIZABLE = 1 << 0,
    DAKT_GUI_TABLE_REORDERABLE = 1 << 1,
    DAKT_GUI_TABLE_HIDEABLE = 1 << 2,
    DAKT_GUI_TABLE_SORTABLE = 1 << 3,
    DAKT_GUI_TABLE_ROW_BACKGROUND = 1 << 4,
    DAKT_GUI_TABLE_BORDERS_INNER_H = 1 << 5,
    DAKT_GUI_TABLE_BORDERS_INNER_V = 1 << 6,
    DAKT_GUI_TABLE_BORDERS_OUTER_H = 1 << 7,
    DAKT_GUI_TABLE_BORDERS_OUTER_V = 1 << 8,
    DAKT_GUI_TABLE_SCROLL_X = 1 << 9,
    DAKT_GUI_TABLE_SCROLL_Y = 1 << 10
} DuiTableFlags;

/* =============================================================================
 * Configuration Structures
 * ============================================================================= */

typedef struct DuiConfig {
    DuiBackendType backend;
    void* windowHandle;
    uint32_t width;
    uint32_t height;
    int32_t enableVSync;
    int32_t enableMSAA;
    uint32_t msaaSamples;
} DuiConfig;

typedef struct DuiMInput {
    float x;
    float y;
    float wheelX;
    float wheelY;
    uint32_t buttons; /* Bitmask: bit 0 = left, bit 1 = right, bit 2 = middle, etc. */
} DuiMInput;

typedef struct DuiKeyInput {
    uint8_t keysDown[DAKT_GUI_KEY_COUNT];
    int32_t shift;
    int32_t ctrl;
    int32_t alt;
    int32_t super;
} DuiKeyInput;

typedef struct DuiStyleColors {
    DuiColor text;
    DuiColor textDisabled;
    DuiColor windowBg;
    DuiColor childBg;
    DuiColor popupBg;
    DuiColor border;
    DuiColor borderShadow;
    DuiColor frameBg;
    DuiColor frameBgHovered;
    DuiColor frameBgActive;
    DuiColor titleBg;
    DuiColor titleBgActive;
    DuiColor titleBgCollapsed;
    DuiColor menuBarBg;
    DuiColor scrollbarBg;
    DuiColor scrollbarGrab;
    DuiColor scrollbarGrabHovered;
    DuiColor scrollbarGrabActive;
    DuiColor checkMark;
    DuiColor sliderGrab;
    DuiColor sliderGrabActive;
    DuiColor button;
    DuiColor buttonHovered;
    DuiColor buttonActive;
    DuiColor header;
    DuiColor headerHovered;
    DuiColor headerActive;
    DuiColor separator;
    DuiColor separatorHovered;
    DuiColor separatorActive;
    DuiColor tab;
    DuiColor tabHovered;
    DuiColor tabActive;
} DuiStyleColors;

/* =============================================================================
 * Context Lifecycle
 * ============================================================================= */

/**
 * @brief Create a new GUI context
 * @param config Configuration parameters (can be NULL for defaults)
 * @return Handle to the new context, or NULL on failure
 */
DAKTLIB_GUI_API DuiCtx Create(const DuiConfig* config);

/**
 * @brief Destroy a GUI context and free all resources
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void Destroy(DuiCtx ctx);

/**
 * @brief Get the currently active context
 * @return Current context handle, or NULL if none
 */
DAKTLIB_GUI_API DuiCtx GetContext(void);

/**
 * @brief Set the currently active context
 * @param ctx Context to make active
 */
DAKTLIB_GUI_API void SetContext(DuiCtx ctx);

/* =============================================================================
 * Frame Management
 * ============================================================================= */

/**
 * @brief Begin a new frame
 * @param ctx Context handle
 * @param deltaTime Time since last frame in seconds
 */
DAKTLIB_GUI_API void NewFrame(DuiCtx ctx, float deltaTime);

/**
 * @brief End the current frame and finalize draw data
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void EndFrame(DuiCtx ctx);

/**
 * @brief Render the GUI to the backend
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void Render(DuiCtx ctx);

/**
 * @brief Present the rendered frame to the screen
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void Present(DuiCtx ctx);

/* =============================================================================
 * Input Handling
 * ============================================================================= */

/**
 * @brief Process mouse input
 * @param ctx Context handle
 * @param input Mouse input state
 */
DAKTLIB_GUI_API void ProcMouse(DuiCtx ctx, const DuiMInput* input);

/**
 * @brief Process keyboard input
 * @param ctx Context handle
 * @param input Keyboard input state
 */
DAKTLIB_GUI_API void ProcKey(DuiCtx ctx, const DuiKeyInput* input);

/**
 * @brief Process text input (UTF-8)
 * @param ctx Context handle
 * @param text UTF-8 encoded text string
 */
DAKTLIB_GUI_API void ProcText(DuiCtx ctx, const char* text);

/**
 * @brief Check if a mouse button is currently down
 * @param ctx Context handle
 * @param button Mouse button to check
 * @return Non-zero if button is down
 */
DAKTLIB_GUI_API int32_t MDown(DuiCtx ctx, DuiMButton button);

/**
 * @brief Check if a mouse button was just pressed this frame
 * @param ctx Context handle
 * @param button Mouse button to check
 * @return Non-zero if button was just pressed
 */
DAKTLIB_GUI_API int32_t MClicked(DuiCtx ctx, DuiMButton button);

/**
 * @brief Check if a mouse button was just released this frame
 * @param ctx Context handle
 * @param button Mouse button to check
 * @return Non-zero if button was just released
 */
DAKTLIB_GUI_API int32_t MReleased(DuiCtx ctx, DuiMButton button);

/**
 * @brief Get current mouse position
 * @param ctx Context handle
 * @return Mouse position in screen coordinates
 */
DAKTLIB_GUI_API DuiVec2 MPos(DuiCtx ctx);

/**
 * @brief Check if a key is currently down
 * @param ctx Context handle
 * @param key Key to check
 * @return Non-zero if key is down
 */
DAKTLIB_GUI_API int32_t KeyDown(DuiCtx ctx, DuiKey key);

/**
 * @brief Check if a key was just pressed this frame
 * @param ctx Context handle
 * @param key Key to check
 * @return Non-zero if key was just pressed
 */
DAKTLIB_GUI_API int32_t KeyPressed(DuiCtx ctx, DuiKey key);

/* =============================================================================
 * Window Management
 * ============================================================================= */

/**
 * @brief Begin a new window
 * @param ctx Context handle
 * @param name Window title/ID
 * @param open Optional pointer to bool for close button (can be NULL)
 * @param flags Window behavior flags
 * @return Non-zero if window is visible and should be rendered
 */
DAKTLIB_GUI_API int32_t BeginWin(DuiCtx ctx, const char* name, int32_t* open, DuiWinFlags flags);

/**
 * @brief End the current window
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void EndWin(DuiCtx ctx);

/**
 * @brief Set position of the next window
 * @param ctx Context handle
 * @param pos Window position
 */
DAKTLIB_GUI_API void SetWinPos(DuiCtx ctx, DuiVec2 pos);

/**
 * @brief Set size of the next window
 * @param ctx Context handle
 * @param size Window size
 */
DAKTLIB_GUI_API void SetNextWinSize(DuiCtx ctx, DuiVec2 size);

/**
 * @brief Begin a child region
 * @param ctx Context handle
 * @param id Child region ID
 * @param size Size (0,0 for auto)
 * @param border Show border
 * @return Non-zero if visible
 */
DAKTLIB_GUI_API int32_t BeginChild(DuiCtx ctx, const char* id, DuiVec2 size, int32_t border);

/**
 * @brief End a child region
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void EndChild(DuiCtx ctx);

/* =============================================================================
 * Basic Widgets
 * ============================================================================= */

/**
 * @brief Display text
 * @param ctx Context handle
 * @param text Text to display
 */
DAKTLIB_GUI_API void DisplayText(DuiCtx ctx, const char* text);

/**
 * @brief Display colored text
 * @param ctx Context handle
 * @param color Text color
 * @param text Text to display
 */
DAKTLIB_GUI_API void ColoredText(DuiCtx ctx, DuiColor color, const char* text);

/**
 * @brief Display disabled (grayed out) text
 * @param ctx Context handle
 * @param text Text to display
 */
DAKTLIB_GUI_API void DisabledText(DuiCtx ctx, const char* text);

/**
 * @brief Display wrapped text
 * @param ctx Context handle
 * @param text Text to display
 */
DAKTLIB_GUI_API void WrapText(DuiCtx ctx, const char* text);

/**
 * @brief Display a label with text on the left
 * @param ctx Context handle
 * @param label Label prefix
 * @param text Text to display
 */
DAKTLIB_GUI_API void TextLabel(DuiCtx ctx, const char* label, const char* text);

/**
 * @brief Display a bullet point
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void Bullet(DuiCtx ctx);

/**
 * @brief Create a button
 * @param ctx Context handle
 * @param label Button label
 * @return Non-zero if clicked
 */
DAKTLIB_GUI_API int32_t Button(DuiCtx ctx, const char* label);

/**
 * @brief Create a button with custom size
 * @param ctx Context handle
 * @param label Button label
 * @param size Button size
 * @return Non-zero if clicked
 */
DAKTLIB_GUI_API int32_t SizedButton(DuiCtx ctx, const char* label, DuiVec2 size);

/**
 * @brief Create a small button (no frame padding)
 * @param ctx Context handle
 * @param label Button label
 * @return Non-zero if clicked
 */
DAKTLIB_GUI_API int32_t SmallButton(DuiCtx ctx, const char* label);

/**
 * @brief Create a checkbox
 * @param ctx Context handle
 * @param label Checkbox label
 * @param value Pointer to bool value
 * @return Non-zero if value changed
 */
DAKTLIB_GUI_API int32_t Checkbox(DuiCtx ctx, const char* label, int32_t* value);

/**
 * @brief Create a radio button
 * @param ctx Context handle
 * @param label Radio button label
 * @param active Non-zero if this option is selected
 * @return Non-zero if clicked
 */
DAKTLIB_GUI_API int32_t RadButton(DuiCtx ctx, const char* label, int32_t active);

/**
 * @brief Create a progress bar
 * @param ctx Context handle
 * @param fraction Progress value (0.0 to 1.0)
 * @param size Bar size (use -1 for default width)
 * @param overlay Optional overlay text (can be NULL)
 */
DAKTLIB_GUI_API void ProgBar(DuiCtx ctx, float fraction, DuiVec2 size, const char* overlay);

/* =============================================================================
 * Input Widgets
 * ============================================================================= */

/**
 * @brief Create a text input field
 * @param ctx Context handle
 * @param label Field label
 * @param buffer Text buffer
 * @param bufferSize Buffer capacity
 * @param flags Input behavior flags
 * @return Non-zero if value changed
 */
DAKTLIB_GUI_API int32_t InputTextField(DuiCtx ctx, const char* label, char* buffer, size_t bufferSize, DuiITFlags flags);

/**
 * @brief Create a multiline text input field
 * @param ctx Context handle
 * @param label Field label
 * @param buffer Text buffer
 * @param bufferSize Buffer capacity
 * @param size Widget size
 * @param flags Input behavior flags
 * @return Non-zero if value changed
 */
DAKTLIB_GUI_API int32_t MultiTextField(DuiCtx ctx, const char* label, char* buffer, size_t bufferSize, DuiVec2 size, DuiITFlags flags);

/**
 * @brief Create an integer input field
 * @param ctx Context handle
 * @param label Field label
 * @param value Pointer to int value
 * @param step Step increment
 * @param stepFast Fast step increment
 * @return Non-zero if value changed
 */
DAKTLIB_GUI_API int32_t IntField(DuiCtx ctx, const char* label, int32_t* value, int32_t step, int32_t stepFast);

/**
 * @brief Create a float input field
 * @param ctx Context handle
 * @param label Field label
 * @param value Pointer to float value
 * @param step Step increment (0 for none)
 * @param stepFast Fast step increment
 * @param format Printf format string
 * @return Non-zero if value changed
 */
DAKTLIB_GUI_API int32_t FloatField(DuiCtx ctx, const char* label, float* value, float step, float stepFast, const char* format);

/* =============================================================================
 * Sliders
 * ============================================================================= */

/**
 * @brief Create an integer slider
 * @param ctx Context handle
 * @param label Slider label
 * @param value Pointer to int value
 * @param min Minimum value
 * @param max Maximum value
 * @param format Printf format string
 * @return Non-zero if value changed
 */
DAKTLIB_GUI_API int32_t IntSlider(DuiCtx ctx, const char* label, int32_t* value, int32_t min, int32_t max, const char* format);

/**
 * @brief Create a float slider
 * @param ctx Context handle
 * @param label Slider label
 * @param value Pointer to float value
 * @param min Minimum value
 * @param max Maximum value
 * @param format Printf format string
 * @return Non-zero if value changed
 */
DAKTLIB_GUI_API int32_t FloatSlider(DuiCtx ctx, const char* label, float* value, float min, float max, const char* format);

/**
 * @brief Create a 2D float slider
 * @param ctx Context handle
 * @param label Slider label
 * @param value Pointer to Vec2 value
 * @param min Minimum value (both axes)
 * @param max Maximum value (both axes)
 * @param format Printf format string
 * @return Non-zero if value changed
 */
DAKTLIB_GUI_API int32_t Vec2Slider(DuiCtx ctx, const char* label, DuiVec2* value, float min, float max, const char* format);

/* =============================================================================
 * Drag Inputs
 * ============================================================================= */

/**
 * @brief Create an integer drag input
 * @param ctx Context handle
 * @param label Input label
 * @param value Pointer to int value
 * @param speed Drag speed
 * @param min Minimum value (0 for no limit)
 * @param max Maximum value (0 for no limit)
 * @return Non-zero if value changed
 */
DAKTLIB_GUI_API int32_t DragInt(DuiCtx ctx, const char* label, int32_t* value, float speed, int32_t min, int32_t max);

/**
 * @brief Create a float drag input
 * @param ctx Context handle
 * @param label Input label
 * @param value Pointer to float value
 * @param speed Drag speed
 * @param min Minimum value
 * @param max Maximum value
 * @param format Printf format string
 * @return Non-zero if value changed
 */
DAKTLIB_GUI_API int32_t DragFloat(DuiCtx ctx, const char* label, float* value, float speed, float min, float max, const char* format);

/* =============================================================================
 * Color Editors
 * ============================================================================= */

/**
 * @brief Create an RGB color editor
 * @param ctx Context handle
 * @param label Editor label
 * @param color Pointer to color value
 * @return Non-zero if color changed
 */
DAKTLIB_GUI_API int32_t ColorEdit3(DuiCtx ctx, const char* label, DuiColor* color);

/**
 * @brief Create an RGBA color editor
 * @param ctx Context handle
 * @param label Editor label
 * @param color Pointer to color value
 * @return Non-zero if color changed
 */
DAKTLIB_GUI_API int32_t ColorEdit4(DuiCtx ctx, const char* label, DuiColor* color);

/**
 * @brief Create an RGB color picker
 * @param ctx Context handle
 * @param label Picker label
 * @param color Pointer to color value
 * @return Non-zero if color changed
 */
DAKTLIB_GUI_API int32_t ColorPicker3(DuiCtx ctx, const char* label, DuiColor* color);

/**
 * @brief Create an RGBA color picker
 * @param ctx Context handle
 * @param label Picker label
 * @param color Pointer to color value
 * @return Non-zero if color changed
 */
DAKTLIB_GUI_API int32_t ColorPicker4(DuiCtx ctx, const char* label, DuiColor* color);

/* =============================================================================
 * Combo Box / Dropdown
 * ============================================================================= */

/**
 * @brief Begin a combo box
 * @param ctx Context handle
 * @param label Combo label
 * @param previewValue Currently selected item text
 * @return Non-zero if combo is open
 */
DAKTLIB_GUI_API int32_t BeginCombo(DuiCtx ctx, const char* label, const char* previewValue);

/**
 * @brief End a combo box
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void EndCombo(DuiCtx ctx);

/**
 * @brief Create a simple combo box from array of strings
 * @param ctx Context handle
 * @param label Combo label
 * @param currentItem Pointer to current selection index
 * @param items Array of item strings
 * @param itemCount Number of items
 * @return Non-zero if selection changed
 */
DAKTLIB_GUI_API int32_t SimpCombo(DuiCtx ctx, const char* label, int32_t* currentItem, const char* const* items, int32_t itemCount);

/* =============================================================================
 * Trees
 * ============================================================================= */

/**
 * @brief Create a tree node
 * @param ctx Context handle
 * @param label Node label
 * @param flags Tree node flags
 * @return Non-zero if node is open
 */
DAKTLIB_GUI_API int32_t TreeNode(DuiCtx ctx, const char* label, DuiTreeNodeFlags flags);

/**
 * @brief Pop tree node (call after tree_node returns true)
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void TreePop(DuiCtx ctx);

/**
 * @brief Create a collapsing header
 * @param ctx Context handle
 * @param label Header label
 * @param flags Tree node flags
 * @return Non-zero if header is open
 */
DAKTLIB_GUI_API int32_t CollapHead(DuiCtx ctx, const char* label, DuiTreeNodeFlags flags);

/* =============================================================================
 * Selectables & Lists
 * ============================================================================= */

/**
 * @brief Create a selectable item
 * @param ctx Context handle
 * @param label Item label
 * @param selected Pointer to selection state
 * @param flags Selectable flags
 * @return Non-zero if clicked
 */
DAKTLIB_GUI_API int32_t SelectableItem(DuiCtx ctx, const char* label, int32_t* selected, DuiSelectableFlags flags);

/**
 * @brief Begin a list box
 * @param ctx Context handle
 * @param label List label
 * @param size List size
 * @return Non-zero if list is visible
 */
DAKTLIB_GUI_API int32_t BeginListBox(DuiCtx ctx, const char* label, DuiVec2 size);

/**
 * @brief End a list box
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void EndListBox(DuiCtx ctx);

/* =============================================================================
 * Menus
 * ============================================================================= */

/**
 * @brief Begin main menu bar
 * @param ctx Context handle
 * @return Non-zero if menu bar is visible
 */
DAKTLIB_GUI_API int32_t BeginMainMenuBar(DuiCtx ctx);

/**
 * @brief End main menu bar
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void EndMainMenuBar(DuiCtx ctx);

/**
 * @brief Begin a menu bar
 * @param ctx Context handle
 * @return Non-zero if menu bar is visible
 */
DAKTLIB_GUI_API int32_t BeginMenuBar(DuiCtx ctx);

/**
 * @brief End a menu bar
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void EndMenuBar(DuiCtx ctx);

/**
 * @brief Begin a menu
 * @param ctx Context handle
 * @param label Menu label
 * @return Non-zero if menu is open
 */
DAKTLIB_GUI_API int32_t BeginMenu(DuiCtx ctx, const char* label);

/**
 * @brief End a menu
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void EndMenu(DuiCtx ctx);

/**
 * @brief Create a menu item
 * @param ctx Context handle
 * @param label Item label
 * @param shortcut Optional keyboard shortcut text (can be NULL)
 * @param selected Optional pointer to selection state (can be NULL)
 * @param enabled Item enabled state
 * @return Non-zero if clicked
 */
DAKTLIB_GUI_API int32_t MenuItem(DuiCtx ctx, const char* label, const char* shortcut, int32_t* selected, int32_t enabled);

/* =============================================================================
 * Popups & Modals
 * ============================================================================= */

/**
 * @brief Open a popup by ID
 * @param ctx Context handle
 * @param id Popup ID
 */
DAKTLIB_GUI_API void OpenPop(DuiCtx ctx, const char* id);

/**
 * @brief Begin a popup
 * @param ctx Context handle
 * @param id Popup ID
 * @return Non-zero if popup is open
 */
DAKTLIB_GUI_API int32_t BeginPop(DuiCtx ctx, const char* id);

/**
 * @brief Begin a modal popup
 * @param ctx Context handle
 * @param name Modal name/title
 * @param open Optional pointer to open state (can be NULL)
 * @return Non-zero if modal is open
 */
DAKTLIB_GUI_API int32_t BeginModal(DuiCtx ctx, const char* name, int32_t* open);

/**
 * @brief End a popup
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void EndPop(DuiCtx ctx);

/**
 * @brief Close the current popup
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void CloseCurrentPop(DuiCtx ctx);

/* =============================================================================
 * Tables
 * ============================================================================= */

/**
 * @brief Begin a table
 * @param ctx Context handle
 * @param id Table ID
 * @param columnCount Number of columns
 * @param flags Table flags
 * @param outerSize Outer size (0,0 for auto)
 * @return Non-zero if table is visible
 */
DAKTLIB_GUI_API int32_t BeginTable(DuiCtx ctx, const char* id, int32_t columnCount, DuiTableFlags flags, DuiVec2 outerSize);

/**
 * @brief End a table
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void EndTable(DuiCtx ctx);

/**
 * @brief Setup a table column
 * @param ctx Context handle
 * @param label Column header label
 * @param initWidthOrWeight Initial width or weight
 */
DAKTLIB_GUI_API void TableColumn(DuiCtx ctx, const char* label, float initWidthOrWeight);

/**
 * @brief Submit headers row
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void TableHeadersRow(DuiCtx ctx);

/**
 * @brief Move to next table row
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void TableNextRow(DuiCtx ctx);

/**
 * @brief Move to next table column
 * @param ctx Context handle
 * @return Non-zero if column is visible
 */
DAKTLIB_GUI_API int32_t TableNextColumn(DuiCtx ctx);

/**
 * @brief Set current column index
 * @param ctx Context handle
 * @param columnIndex Column index
 * @return Non-zero if column is visible
 */
DAKTLIB_GUI_API int32_t TableSetColumnIdx(DuiCtx ctx, int32_t columnIndex);

/* =============================================================================
 * Tabs
 * ============================================================================= */

/**
 * @brief Begin a tab bar
 * @param ctx Context handle
 * @param id Tab bar ID
 * @return Non-zero if tab bar is visible
 */
DAKTLIB_GUI_API int32_t BeginTabBar(DuiCtx ctx, const char* id);

/**
 * @brief End a tab bar
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void EndTabBar(DuiCtx ctx);

/**
 * @brief Begin a tab item
 * @param ctx Context handle
 * @param label Tab label
 * @param open Optional pointer to open state for close button (can be NULL)
 * @return Non-zero if tab is selected
 */
DAKTLIB_GUI_API int32_t BeginTabItem(DuiCtx ctx, const char* label, int32_t* open);

/**
 * @brief End a tab item
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void EndTabItem(DuiCtx ctx);

/* =============================================================================
 * Layout Utilities
 * ============================================================================= */

/**
 * @brief Add horizontal spacing
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void HSpacing(DuiCtx ctx);

/**
 * @brief Add a separator line
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void Separator(DuiCtx ctx);

/**
 * @brief Add vertical spacing
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void VSpacing(DuiCtx ctx);

/**
 * @brief Add a dummy item of specified size
 * @param ctx Context handle
 * @param size Dummy size
 */
DAKTLIB_GUI_API void Dummy(DuiCtx ctx, DuiVec2 size);

/**
 * @brief Indent content
 * @param ctx Context handle
 * @param width Indent width (0 for default)
 */
DAKTLIB_GUI_API void Indent(DuiCtx ctx, float width);

/**
 * @brief Unindent content
 * @param ctx Context handle
 * @param width Unindent width (0 for default)
 */
DAKTLIB_GUI_API void Unindent(DuiCtx ctx, float width);

/**
 * @brief Begin a group (for layout purposes)
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void BeginGroup(DuiCtx ctx);

/**
 * @brief End a group
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void EndGroup(DuiCtx ctx);

/**
 * @brief Set next item width
 * @param ctx Context handle
 * @param width Item width
 */
DAKTLIB_GUI_API void SetNextItemWidth(DuiCtx ctx, float width);

/**
 * @brief Push item width for subsequent items
 * @param ctx Context handle
 * @param width Item width
 */
DAKTLIB_GUI_API void PushItemWidth(DuiCtx ctx, float width);

/**
 * @brief Pop item width
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void PopItemWidth(DuiCtx ctx);

/* =============================================================================
 * ID Stack
 * ============================================================================= */

/**
 * @brief Push string ID onto stack
 * @param ctx Context handle
 * @param id String ID
 */
DAKTLIB_GUI_API void PushIdStr(DuiCtx ctx, const char* id);

/**
 * @brief Push integer ID onto stack
 * @param ctx Context handle
 * @param id Integer ID
 */
DAKTLIB_GUI_API void PushIdInt(DuiCtx ctx, int32_t id);

/**
 * @brief Push pointer ID onto stack
 * @param ctx Context handle
 * @param id Pointer ID
 */
DAKTLIB_GUI_API void PushIdPtr(DuiCtx ctx, const void* id);

/**
 * @brief Pop ID from stack
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void PopId(DuiCtx ctx);

/* =============================================================================
 * Style
 * ============================================================================= */

/**
 * @brief Get current style colors
 * @param ctx Context handle
 * @param colors Output colors structure
 */
DAKTLIB_GUI_API void GetStyleColors(DuiCtx ctx, DuiStyleColors* colors);

/**
 * @brief Set style colors
 * @param ctx Context handle
 * @param colors Input colors structure
 */
DAKTLIB_GUI_API void SetStyleColors(DuiCtx ctx, const DuiStyleColors* colors);

/**
 * @brief Apply dark theme
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void StyleColorsDark(DuiCtx ctx);

/**
 * @brief Apply light theme
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void StyleColorsLight(DuiCtx ctx);

/* =============================================================================
 * Tooltips
 * ============================================================================= */

/**
 * @brief Begin a tooltip
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void BeginTooltip(DuiCtx ctx);

/**
 * @brief End a tooltip
 * @param ctx Context handle
 */
DAKTLIB_GUI_API void EndTooltip(DuiCtx ctx);

/**
 * @brief Set tooltip text (shortcut for begin_tooltip + text + end_tooltip)
 * @param ctx Context handle
 * @param text Tooltip text
 */
DAKTLIB_GUI_API void SetTooltip(DuiCtx ctx, const char* text);

/* =============================================================================
 * Item State Queries
 * ============================================================================= */

/**
 * @brief Check if last item is hovered
 * @param ctx Context handle
 * @return Non-zero if hovered
 */
DAKTLIB_GUI_API int32_t ItemHovered(DuiCtx ctx);

/**
 * @brief Check if last item is active
 * @param ctx Context handle
 * @return Non-zero if active
 */
DAKTLIB_GUI_API int32_t ItemActive(DuiCtx ctx);

/**
 * @brief Check if last item is focused
 * @param ctx Context handle
 * @return Non-zero if focused
 */
DAKTLIB_GUI_API int32_t ItemFocused(DuiCtx ctx);

/**
 * @brief Check if last item was clicked
 * @param ctx Context handle
 * @param button Mouse button
 * @return Non-zero if clicked
 */
DAKTLIB_GUI_API int32_t ItemClicked(DuiCtx ctx, DuiMButton button);

/**
 * @brief Check if last item was edited
 * @param ctx Context handle
 * @return Non-zero if edited
 */
DAKTLIB_GUI_API int32_t ItemEdited(DuiCtx ctx);

/**
 * @brief Check if last item was just activated
 * @param ctx Context handle
 * @return Non-zero if activated
 */
DAKTLIB_GUI_API int32_t ItemActivated(DuiCtx ctx);

/**
 * @brief Check if last item was just deactivated
 * @param ctx Context handle
 * @return Non-zero if deactivated
 */
DAKTLIB_GUI_API int32_t ItemDeactivated(DuiCtx ctx);

/**
 * @brief Get last item bounding rectangle
 * @param ctx Context handle
 * @return Item rectangle
 */
DAKTLIB_GUI_API DuiRect GetRect(DuiCtx ctx);

/* =============================================================================
 * Utilities
 * ============================================================================= */

/**
 * @brief Get content region available size
 * @param ctx Context handle
 * @return Available size
 */
DAKTLIB_GUI_API DuiVec2 GetRegionSize(DuiCtx ctx);

/**
 * @brief Get current cursor position
 * @param ctx Context handle
 * @return Cursor position
 */
DAKTLIB_GUI_API DuiVec2 GetCursorPos(DuiCtx ctx);

/**
 * @brief Set cursor position
 * @param ctx Context handle
 * @param pos New cursor position
 */
DAKTLIB_GUI_API void SetCursorPos(DuiCtx ctx, DuiVec2 pos);

/**
 * @brief Get window size
 * @param ctx Context handle
 * @return Window size
 */
DAKTLIB_GUI_API DuiVec2 GetWinSize(DuiCtx ctx);

/**
 * @brief Get window position
 * @param ctx Context handle
 * @return Window position
 */
DAKTLIB_GUI_API DuiVec2 GetWinPos(DuiCtx ctx);

/* =============================================================================
 * Backend Information
 * ============================================================================= */

/**
 * @brief Get the active backend type
 * @param ctx Context handle
 * @return Backend type enum value
 */
DAKTLIB_GUI_API DuiBackendType GetBackendType(DuiCtx ctx);

/**
 * @brief Get the backend name string
 * @param ctx Context handle
 * @return Backend name (e.g., "Vulkan", "DirectX 11")
 */
DAKTLIB_GUI_API const char* GetBackendName(DuiCtx ctx);

/**
 * @brief Resize the rendering surface
 * @param ctx Context handle
 * @param width New width
 * @param height New height
 */
DAKTLIB_GUI_API void Resize(DuiCtx ctx, uint32_t width, uint32_t height);

/* =============================================================================
 * Error Handling
 * ============================================================================= */

/**
 * @brief Get the last error message (thread-local)
 * @return Error message string, or NULL if no error
 */
DAKTLIB_GUI_API const char* GetLastErr(void);

/**
 * @brief Clear the last error
 */
DAKTLIB_GUI_API void ClearErr(void);

/* =============================================================================
 * Version Information
 * ============================================================================= */

/**
 * @brief Get library version string
 * @return Version string (e.g., "1.0.0")
 */
DAKTLIB_GUI_API const char* GetVersion(void);

/**
 * @brief Get library version number
 * @return Version as MAJOR*10000 + MINOR*100 + PATCH
 */
DAKTLIB_GUI_API uint32_t GetVersionNum(void);

#ifdef __cplusplus
}
#endif

#endif /* DAKTLIB_GUI_C_API_H */
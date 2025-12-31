// ============================================================================
// DaktLib GUI Module - Window Container
// ============================================================================

#pragma once

#include <dakt/core/Types.hpp>
#include <dakt/gui/core/Types.hpp>

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

}  // namespace dakt::gui

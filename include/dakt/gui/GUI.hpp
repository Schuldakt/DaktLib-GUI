// ============================================================================
// DaktLib GUI Module
// ============================================================================
// A high-performance, immediate-mode GUI library for Star Citizen tooling.
// Features custom widgets, DirectX 11 rendering backend, and extensive
// theming support.
//
// Architecture:
// - Immediate-mode design (no retained widget state)
// - Custom draw list for efficient batching
// - D3D11 rendering backend with shader-based drawing
// - Extensible theme system
// - Rich set of widgets including geometric shapes
//
// Usage:
//   #include <dakt/gui/GUI.hpp>
//
//   // Initialize
//   dakt::gui::Context ctx;
//   ctx.initialize(options);
//
//   // In render loop:
//   ctx.beginFrame(deltaTime);
//
//   if (dakt::gui::begin("Window")) {
//       dakt::gui::text("Hello World");
//       if (dakt::gui::button("Click Me")) { ... }
//   }
//   dakt::gui::end();
//
//   ctx.endFrame();
//   ctx.render();
//
// ============================================================================

#pragma once

// Core types and utilities
#include <dakt/gui/Types.hpp>

// Draw list for rendering
#include <dakt/gui/DrawList.hpp>

// Font loading and text rendering
#include <dakt/gui/Font.hpp>

// Main context and state management
#include <dakt/gui/Context.hpp>

// Theming and styling
#include <dakt/gui/Theme.hpp>

// Basic widgets (text, buttons, sliders, etc.)
#include <dakt/gui/Widgets.hpp>

// Container widgets (windows, panels, tabs, etc.)
#include <dakt/gui/Containers.hpp>

// Advanced data widgets (property grids, tables, hex view, etc.)
#include <dakt/gui/DataWidgets.hpp>

// DirectX 11 rendering backend
#include <dakt/gui/D3D11Backend.hpp>

namespace dakt::gui
{

// ============================================================================
// Version Information
// ============================================================================

constexpr i32 VERSION_MAJOR = 1;
constexpr i32 VERSION_MINOR = 0;
constexpr i32 VERSION_PATCH = 0;
constexpr const char* VERSION_STRING = "1.0.0";

// ============================================================================
// Quick Start Functions
// ============================================================================

// Initialize the GUI system with default settings
// Returns true on success
inline bool initializeDefault(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
    // Create and initialize the D3D11 backend
    static D3D11Backend backend;

    D3D11BackendConfig config;
    config.device = device;
    config.deviceContext = deviceContext;

    if (!backend.initialize(config))
    {
        return false;
    }

    setD3D11Backend(&backend);

    // Create and initialize the context
    static Context ctx;

    ContextOptions options;
    options.displayWidth = 1920;
    options.displayHeight = 1080;

    if (!ctx.initialize(options))
    {
        return false;
    }

    setContext(&ctx);

    return true;
}

// Shutdown the GUI system
inline void shutdownDefault()
{
    if (auto* ctx = getContext())
    {
        ctx->shutdown();
    }

    if (auto* backend = getD3D11Backend())
    {
        backend->shutdown();
    }

    setContext(nullptr);
    setD3D11Backend(nullptr);
}

// ============================================================================
// Frame Helpers
// ============================================================================

// Begin a new frame
inline void newFrame(f32 deltaTime, i32 displayWidth, i32 displayHeight)
{
    auto* ctx = getContext();
    auto* backend = getD3D11Backend();

    if (backend)
    {
        backend->beginFrame(displayWidth, displayHeight);
    }

    if (ctx)
    {
        ctx->setDisplaySize(displayWidth, displayHeight);
        ctx->beginFrame(deltaTime);
    }
}

// End the current frame
inline void endFrame()
{
    auto* ctx = getContext();
    auto* backend = getD3D11Backend();

    if (ctx)
    {
        ctx->endFrame();
    }

    if (backend)
    {
        backend->endFrame();
    }
}

// Render the GUI
inline void renderFrame()
{
    auto* ctx = getContext();
    auto* backend = getD3D11Backend();

    if (ctx && backend)
    {
        backend->render(ctx->getDrawList());
    }
}

// ============================================================================
// Input Forwarding
// ============================================================================

// Forward mouse position
inline void setMousePos(f32 x, f32 y)
{
    if (auto* ctx = getContext())
    {
        ctx->setMousePos(Vec2{x, y});
    }
}

// Forward mouse button state
inline void setMouseButton(MouseButton button, bool down)
{
    if (auto* ctx = getContext())
    {
        ctx->setMouseButton(button, down);
    }
}

// Forward mouse wheel
inline void setMouseWheel(f32 x, f32 y)
{
    if (auto* ctx = getContext())
    {
        ctx->setMouseWheel(Vec2{x, y});
    }
}

// Forward key state
inline void setKeyState(Key key, bool down)
{
    if (auto* ctx = getContext())
    {
        ctx->setKeyState(key, down);
    }
}

// Forward character input
inline void addInputCharacter(char32_t c)
{
    if (auto* ctx = getContext())
    {
        ctx->addInputCharacter(c);
    }
}

}  // namespace dakt::gui

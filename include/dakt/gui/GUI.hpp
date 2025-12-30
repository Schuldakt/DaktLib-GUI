// ============================================================================
// DaktLib GUI Module
// ============================================================================
// Custom immediate-mode GUI system with multiple rendering backend support.
//
// Organization:
//      core/           - Fundamental types, draw list, font rendering
//      framework/      - GUI context, layout system, input handling, theming
//      widgets/        - Basic UI widgets (buttons, sliders, inputs, etc.)
//      containers/     - Layout containers (windows, panels, tabs, menus)
//      data/           - Advanced data widgets (property grid, hex view, etc.)
//      backends/       - Rendering backends (D3D11, D3D12, OpenGL, Vulkan, Metal)
//
// Usage:
//      #include <dakt/gui/GUI.hpp>     // Include everything
//      Or include specific headers for faster compile times
// ============================================================================

#pragma once

// ============================================================================
// Core Components
// ============================================================================

#include <dakt/gui/core/DrawList.hpp>
#include <dakt/gui/core/Font.hpp>
#include <dakt/gui/core/Types.hpp>

// ============================================================================
// Framework Components
// ============================================================================

#include <dakt/gui/framework/Context.hpp>
#include <dakt/gui/framework/Input.hpp>
#include <dakt/gui/framework/Layout.hpp>
#include <dakt/gui/framework/Theme.hpp>

// ============================================================================
// Basic Widgets
// ============================================================================

#include <dakt/gui/widgets/Widgets.hpp>

// ============================================================================
// Container Widgets
// ============================================================================

#include <dakt/gui/widgets/Containers.hpp>

// ============================================================================
// Data/Advanced Widgets
// ============================================================================

#include <dakt/gui/widgets/DataWidgets.hpp>

// ============================================================================
// Rendering Backends
// ============================================================================

#include <dakt/gui/backends/Backends.hpp>

// ============================================================================
// Convenience Namespace
// ============================================================================

namespace dakt
{
/// GUI module namespace alias
namespace ui = gui;
}  // namespace dakt
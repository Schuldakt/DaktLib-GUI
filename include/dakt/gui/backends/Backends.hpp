// ============================================================================
// DaktLib GUI Module - Backends Header
// ============================================================================
// Includes all available rendering backends. Use this header for convenience
// or include individual backend headers for reduced compile times.
// ============================================================================

#pragma once

#include <dakt/gui/backends/RenderBackend.hpp>

// Platform-specific backends
#if defined(_WIN32)
    #include <dakt/gui/backends/D3D11Backend.hpp>
    #include <dakt/gui/backends/D3D12Backend.hpp>
#elif defined(__APPLE__)
    #include <dakt/gui/backends/MetalBackend.hpp>
#endif

// Cross-platform backends
#include <dakt/gui/backends/OpenGLBackend.hpp>
#include <dakt/gui/backends/VulkanBackend.hpp>
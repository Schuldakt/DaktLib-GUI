// ============================================================================
// DaktLib GUI Module - Render Backend Factory Implementation
// ============================================================================

#include <dakt/gui/backends/RenderBackend.hpp>

#if defined(_WIN32)
    #include <dakt/gui/backends/D3D11Backend.hpp>
    #include <dakt/gui/backends/D3D12Backend.hpp>
#elif defined(__APPLE__)
    #include <dakt/gui/backends/MetalBackend.hpp>
#endif

#include <dakt/gui/backends/OpenGLBackend.hpp>
#include <dakt/gui/backends/VulkanBackend.hpp>

namespace dakt::gui
{

std::unique_ptr<IRenderBackend> createRenderBackend(BackendType type)
{
    switch (type)
    {
#if defined(_WIN32)
        case BackendType::D3D11:
            return std::make_unique<D3D11Backend>();

        case BackendType::D3D12:
            return std::make_unique<D3D12Backend>();
#endif

#if defined(__APPLE__)
        case BackendType::Metal:
            return std::make_unique<MetalBackend>();
#endif

        case BackendType::OpenGL:
            return std::make_unique<OpenGLBackend>();

        case BackendType::Vulkan:
            return std::make_unique<VulkanBackend>();

        default:
            return nullptr;
    }
}

BackendType getPreferredBackendType()
{
#if defined(_WIN32)
    // On Windows, prefer D3D11 for broad compatibility
    // D3D12 requires Windows 10+ and more complex setup
    return BackendType::D3D11;
#elif defined(__APPLE__)
    // On macOS/iOS, Metal is the native and preferred graphics API
    // OpenGL is deprecated on Apple platforms
    return BackendType::Metal;
#elif defined(__linux__)
    // On Linux, prefer Vulkan if available, fall back to OpenGL
    // Could add runtime detection here
    return BackendType::Vulkan;
#else
    return BackendType::OpenGL;
#endif
}

bool isBackendAvailable(BackendType type)
{
    switch (type)
    {
#if defined(_WIN32)
        case BackendType::D3D11:
            // D3D11 is available on Windows Vista SP2+ with Platform Update
            // or Windows 7+
            return true;

        case BackendType::D3D12:
            // D3D12 requires Windows 10+
            // TODO: Runtime check for Windows version
            return true;
#endif

#if defined(__APPLE__)
        case BackendType::Metal:
            // Metal is available on macOS 10.11+ and iOS 8+
            // All modern Apple devices support Metal
            return true;
#endif

        case BackendType::OpenGL:
            // OpenGL is generally available everywhere
            // Note: Deprecated on macOS but still functional
            return true;

        case BackendType::Vulkan:
            // Vulkan availability depends on drivers
            // Not natively available on macOS (requires MoltenVK)
#if defined(__APPLE__)
            return false;  // Would need MoltenVK
#else
            return true;
#endif

        default:
            return false;
    }
}

}  // namespace dakt::gui
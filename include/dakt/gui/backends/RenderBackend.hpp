// ============================================================================
// DaktLib GUI Module - Render Backend Interface
// ============================================================================
// Abstract base class for all rendering backends. Defines the interface that
// D3D11, D3D12, OpenGL, and Vulkan backends must implement.
// ============================================================================

#pragma once

#include <dakt/core/Types.hpp>
#include <dakt/gui/core/DrawList.hpp>
#include <dakt/gui/core/Types.hpp>

#include <memory>
#include <span>

namespace dakt::gui
{

using namespace dakt::core;

// ============================================================================
// Backend Type Enumeration
// ============================================================================

enum class BackendType : u8
{
    None = 0,
    D3D11,
    D3D12,
    OpenGL,
    Vulkan,
    Metal
};

constexpr StringView backendTypeToString(BackendType type)
{
    switch (type)
    {
        case BackendType::D3D11:
            return "Direct3D 11";
        case BackendType::D3D12:
            return "Direct3D 12";
        case BackendType::OpenGL:
            return "OpenGL";
        case BackendType::Vulkan:
            return "Vulkan";
        case BackendType::Metal:
            return "Metal";
        default:
            return "None";
    }
}

// ============================================================================
// Backend Capabilities
// ============================================================================

struct BackendCapabilities
{
    bool supportsMultiViewport = false;
    bool supportsTextureArrays = false;
    bool supportsMSAA = false;
    bool supportsCompute = false;
    bool supportsBindless = false;
    u32 maxTextureSize = 4096;
    u32 maxTextures = 16;
    u32 maxVerticesPerBatch = 65536;
    u32 maxIndicesPerBatch = 65536 * 3;
};

// ============================================================================
// Texture Handle
// ============================================================================

struct TextureHandle
{
    uintptr_t id = 0;
    u32 width = 0;
    u32 height = 0;
    u32 format = 0;

    [[nodiscard]] bool isValid() const { return id != 0; }
    explicit operator bool() const { return isValid(); }
};

// ============================================================================
// Texture Creation Info
// ============================================================================

enum class TextureFormat : u8
{
    RGBA8,
    BGRA8,
    R8,
    RG8,
    RGBA16F,
    RGBA32F,
    BC1,  // DXT1
    BC3,  // DXT5
    BC7
};

struct TextureCreateInfo
{
    u32 width = 0;
    u32 height = 0;
    TextureFormat format = TextureFormat::RGBA8;
    bool generateMips = false;
    bool renderTarget = false;
    const void* initialData = nullptr;
    usize initialDataSize = 0;
};

// ============================================================================
// Render Backend Interface
// ============================================================================

class IRenderBackend
{
public:
    virtual ~IRenderBackend() = default;

    // ========================================================================
    // Lifecycle
    // ========================================================================

    /// Initialize the backend
    /// @return true on success
    [[nodiscard]] virtual bool initialize() = 0;

    /// Shutdown and release all resources
    virtual void shutdown() = 0;

    /// Check if the backend is initialized
    [[nodiscard]] virtual bool isInitialized() const = 0;

    /// Get the backend type
    [[nodiscard]] virtual BackendType getType() const = 0;

    /// Get backend capabilities
    [[nodiscard]] virtual const BackendCapabilities& getCapabilities() const = 0;

    // ========================================================================
    // Frame Management
    // ========================================================================

    /// Begin a new frame
    /// @param displayWidth Framebuffer width in pixels
    /// @param displayHeight Framebuffer height in pixels
    virtual void beginFrame(i32 displayWidth, i32 displayHeight) = 0;

    /// End the current frame and submit draw commands
    virtual void endFrame() = 0;

    // ========================================================================
    // Draw List Rendering
    // ========================================================================

    /// Render a draw list
    /// @param drawList The draw list to render
    virtual void renderDrawList(const DrawList& drawList) = 0;

    /// Render multiple draw lists
    /// @param drawLists Span of draw lists to render
    virtual void renderDrawLists(std::span<const DrawList*> drawLists) = 0;

    // ========================================================================
    // Texture Management
    // ========================================================================

    /// Create a texture
    /// @param info Texture creation parameters
    /// @return Handle to the created texture, or invalid handle on failure
    [[nodiscard]] virtual TextureHandle createTexture(const TextureCreateInfo& info) = 0;

    /// Update texture data
    /// @param texture Texture to update
    /// @param data Pointer to new pixel data
    /// @param dataSize Size of the pixel data in bytes
    /// @param x X offset for partial update (0 for full update)
    /// @param y Y offset for partial update (0 for full update)
    /// @param width Width of update region (0 for full width)
    /// @param height Height of update region (0 for full height)
    virtual void updateTexture(TextureHandle texture, const void* data, usize dataSize, u32 x = 0, u32 y = 0,
                               u32 width = 0, u32 height = 0) = 0;

    /// Destroy a texture
    /// @param texture Texture to destroy
    virtual void destroyTexture(TextureHandle texture) = 0;

    // ========================================================================
    // Viewport & Scissor
    // ========================================================================

    /// Set the viewport
    /// @param x X position
    /// @param y Y position
    /// @param width Viewport width
    /// @param height Viewport height
    virtual void setViewport(i32 x, i32 y, i32 width, i32 height) = 0;

    /// Set the scissor rectangle
    /// @param x X position
    /// @param y Y position
    /// @param width Scissor width
    /// @param height Scissor height
    virtual void setScissorRect(i32 x, i32 y, i32 width, i32 height) = 0;

    // ========================================================================
    // State Management
    // ========================================================================

    /// Invalidate cached device state (call after external rendering)
    virtual void invalidateDeviceState() = 0;

    /// Get the native device handle (ID3D11Device*, VkDevice, etc.)
    [[nodiscard]] virtual void* getNativeDevice() const = 0;

    /// Get the native device context (ID3D11DeviceContext*, VkCommandBuffer, etc.)
    [[nodiscard]] virtual void* getNativeContext() const = 0;

protected:
    IRenderBackend() = default;

    // Non-copyable
    IRenderBackend(const IRenderBackend&) = delete;
    IRenderBackend& operator=(const IRenderBackend&) = delete;
};

// ============================================================================
// Backend Factory
// ============================================================================

/// Create a render backend of the specified type
/// @param type The backend type to create
/// @return Unique pointer to the backend, or nullptr on failure
[[nodiscard]] std::unique_ptr<IRenderBackend> createRenderBackend(BackendType type);

/// Get the preferred backend type for the current platform
[[nodiscard]] BackendType getPreferredBackendType();

/// Check if a backend type is available on the current system
[[nodiscard]] bool isBackendAvailable(BackendType type);

}  // namespace dakt::gui
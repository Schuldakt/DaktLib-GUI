// ============================================================================
// DaktLib GUI Module - Metal Backend
// ============================================================================
// Apple Metal rendering backend for the GUI system.
// Provides native, high-performance rendering on macOS, iOS, and tvOS
// using Apple's modern graphics API.
// ============================================================================

#pragma once

#include <dakt/gui/backends/RenderBackend.hpp>

#include <vector>

// Forward declarations for Metal types (Objective-C objects)
#ifdef __OBJC__
@protocol MTLDevice;
@protocol MTLCommandQueue;
@protocol MTLCommandBuffer;
@protocol MTLRenderCommandEncoder;
@protocol MTLBuffer;
@protocol MTLTexture;
@protocol MTLRenderPipelineState;
@protocol MTLDepthStencilState;
@protocol MTLSamplerState;
@class MTLRenderPassDescriptor;
#else
typedef void* id;
#endif

namespace dakt::gui
{

// ============================================================================
// Metal Backend Configuration
// ============================================================================

struct MetalBackendConfig
{
    // Metal device (id<MTLDevice>)
    // If null, the default device will be used
    void* device = nullptr;

    // Command queue (id<MTLCommandQueue>)
    // If null, one will be created
    void* commandQueue = nullptr;

    // Pixel format for the render target (MTLPixelFormat)
    // Default: MTLPixelFormatBGRA8Unorm (80)
    u32 colorPixelFormat = 80;

    // Depth pixel format (MTLPixelFormat)
    // Default: MTLPixelFormatInvalid (0) - no depth buffer
    u32 depthPixelFormat = 0;

    // Stencil pixel format (MTLPixelFormat)
    // Default: MTLPixelFormatInvalid (0) - no stencil buffer
    u32 stencilPixelFormat = 0;

    // Sample count for MSAA
    u32 sampleCount = 1;

    // Number of frames in flight
    u32 numFramesInFlight = 3;

    // Initial buffer sizes
    u32 initialVertexBufferSize = 5000;
    u32 initialIndexBufferSize = 10000;

    // Enable debug features
    bool enableDebug = false;
};

// ============================================================================
// Metal Frame Resources
// ============================================================================

struct MetalFrameResources
{
    void* vertexBuffer = nullptr;  // id<MTLBuffer>
    void* indexBuffer = nullptr;   // id<MTLBuffer>
    u32 vertexBufferSize = 0;
    u32 indexBufferSize = 0;
};

// ============================================================================
// Metal Texture Data
// ============================================================================

struct MetalTextureData
{
    void* texture = nullptr;  // id<MTLTexture>
    u32 width = 0;
    u32 height = 0;
    u32 format = 0;
};

// ============================================================================
// Metal Backend Implementation
// ============================================================================

class MetalBackend final : public IRenderBackend
{
public:
    MetalBackend();
    ~MetalBackend() override;

    // Non-copyable
    MetalBackend(const MetalBackend&) = delete;
    MetalBackend& operator=(const MetalBackend&) = delete;

    // ========================================================================
    // Configuration
    // ========================================================================

    /// Configure the backend before initialization
    /// @param config Configuration options
    void configure(const MetalBackendConfig& config);

    // ========================================================================
    // IRenderBackend Implementation
    // ========================================================================

    [[nodiscard]] bool initialize() override;
    void shutdown() override;
    [[nodiscard]] bool isInitialized() const override { return m_initialized; }
    [[nodiscard]] BackendType getType() const override { return BackendType::Metal; }
    [[nodiscard]] const BackendCapabilities& getCapabilities() const override { return m_capabilities; }

    void beginFrame(i32 displayWidth, i32 displayHeight) override;
    void endFrame() override;

    void renderDrawList(const DrawList& drawList) override;
    void renderDrawLists(std::span<const DrawList*> drawLists) override;

    [[nodiscard]] TextureHandle createTexture(const TextureCreateInfo& info) override;
    void updateTexture(TextureHandle texture, const void* data, usize dataSize, u32 x = 0, u32 y = 0, u32 width = 0,
                       u32 height = 0) override;
    void destroyTexture(TextureHandle texture) override;

    void setViewport(i32 x, i32 y, i32 width, i32 height) override;
    void setScissorRect(i32 x, i32 y, i32 width, i32 height) override;

    void invalidateDeviceState() override;

    [[nodiscard]] void* getNativeDevice() const override { return m_device; }
    [[nodiscard]] void* getNativeContext() const override { return m_currentEncoder; }

    // ========================================================================
    // Metal-Specific Methods
    // ========================================================================

    /// Get the Metal device (id<MTLDevice>)
    [[nodiscard]] void* getDevice() const { return m_device; }

    /// Get the command queue (id<MTLCommandQueue>)
    [[nodiscard]] void* getCommandQueue() const { return m_commandQueue; }

    /// Get the current render command encoder (id<MTLRenderCommandEncoder>)
    [[nodiscard]] void* getCurrentEncoder() const { return m_currentEncoder; }

    /// Set the render pass descriptor for the current frame
    /// This must be called before beginFrame() each frame
    /// @param descriptor MTLRenderPassDescriptor*
    void setRenderPassDescriptor(void* descriptor);

    /// Set the current drawable for presentation
    /// @param drawable id<CAMetalDrawable> or similar
    void setDrawable(void* drawable);

    /// Create a font texture from atlas data
    /// @param width Atlas width
    /// @param height Atlas height
    /// @param pixels Grayscale pixel data (R8)
    /// @return Handle to the created texture
    [[nodiscard]] TextureHandle createFontTexture(u32 width, u32 height, const u8* pixels);

    /// Get the pipeline state (id<MTLRenderPipelineState>)
    [[nodiscard]] void* getPipelineState() const { return m_pipelineState; }

private:
    // Initialization helpers
    bool createDevice();
    bool createCommandQueue();
    bool createPipelineState();
    bool createDepthStencilState();
    bool createSamplerState();
    bool createBuffers();
    void destroyDeviceObjects();

    // Buffer management
    void ensureVertexBufferSize(u32 requiredSize);
    void ensureIndexBufferSize(u32 requiredSize);

    // Rendering helpers
    void setupRenderState();
    void renderDrawCommands(const DrawList& drawList);

    // Frame management
    MetalFrameResources& getCurrentFrameResources();
    void waitForFrame(u32 frameIndex);

private:
    // Configuration
    MetalBackendConfig m_config{};
    BackendCapabilities m_capabilities{};
    bool m_initialized = false;
    bool m_ownsDevice = false;
    bool m_ownsCommandQueue = false;

    // Metal objects (stored as void* for C++ compatibility)
    void* m_device = nullptr;             // id<MTLDevice>
    void* m_commandQueue = nullptr;       // id<MTLCommandQueue>
    void* m_pipelineState = nullptr;      // id<MTLRenderPipelineState>
    void* m_depthStencilState = nullptr;  // id<MTLDepthStencilState>
    void* m_samplerState = nullptr;       // id<MTLSamplerState>

    // Current frame state
    void* m_currentCommandBuffer = nullptr;  // id<MTLCommandBuffer>
    void* m_currentEncoder = nullptr;        // id<MTLRenderCommandEncoder>
    void* m_renderPassDescriptor = nullptr;  // MTLRenderPassDescriptor*
    void* m_currentDrawable = nullptr;       // id<CAMetalDrawable>

    // Frame resources (triple buffering)
    std::vector<MetalFrameResources> m_frameResources;
    u32 m_frameIndex = 0;

    // Synchronization
    void* m_frameSemaphore = nullptr;  // dispatch_semaphore_t

    // Font texture
    MetalTextureData m_fontTexture{};

    // Texture tracking
    std::vector<MetalTextureData> m_textures;
    uintptr_t m_nextTextureId = 1;

    // Display dimensions
    i32 m_displayWidth = 0;
    i32 m_displayHeight = 0;

    // Current viewport/scissor
    f32 m_viewportX = 0;
    f32 m_viewportY = 0;
    f32 m_viewportWidth = 0;
    f32 m_viewportHeight = 0;
};

}  // namespace dakt::gui
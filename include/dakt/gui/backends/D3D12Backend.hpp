// ============================================================================
// DaktLib GUI Module - D3D12 Backend
// ============================================================================
// DirectX 12 rendering backend for the GUI system.
// Provides lower-level GPU control with explicit resource manage,
// command lists, and descriptor heaps for maximum performance.
// ============================================================================

#pragma once

#include <dakt/gui/backends/RenderBackend.hpp>

#include <array>
#include <vector>

// Forward declarations for D3D12 types
struct ID3D12Device;
struct ID3D12CommandQueue;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12RootSignature;
struct ID3D12PipelineState;
struct ID3D12DescriptorHeap;
struct ID3D12Resource;
struct ID3D12Fence;
struct D3D12_CPU_DESCRIPTOR_HANDLE;
struct D3D12_GPU_DESCRIPTOR_HANDLE;

namespace dakt::gui
{

// ============================================================================
// D3D12 Backend Configuration
// ============================================================================

struct D3D12BackendConfig
{
    ID3D12Device* device = nullptr;
    ID3D12CommandQueue* commandQueue = nullptr;

    // Number of frames in flight (typically 2-3)
    u32 numFramesInFlight = 2;

    // Initial buffer sizes (will grow as needed)
    u32 initialVertexBufferSize = 5000;
    u32 initialIndexBufferSize = 10000;

    // Descriptor heap sizes
    u32 srvHeapSize = 256;

    // Enable debug features
    bool enableDebug = false;
};

// ============================================================================
// D3D12 Frame Resources
// ============================================================================

struct D3D12FrameResources
{
    ID3D12CommandAllocator* commandAllocator = nullptr;
    ID3D12Resource* vertexBuffer = nullptr;
    ID3D12Resource* indexBuffer = nullptr;
    void* vertexBufferCpuAddr = nullptr;
    void* indexBufferCpuAddr = nullptr;
    u32 vertexBufferSize = 0;
    u32 indexBufferSize = 0;
    u64 fenceValue = 0;
};

// ============================================================================
// D3D12 Backend Implementation
// ============================================================================

class D3D12Backend final : public IRenderBackend
{
public:
    D3D12Backend();
    ~D3D12Backend() override;

    // Non-copyable
    D3D12Backend(const D3D12Backend&) = delete;
    D3D12Backend& operator=(const D3D12Backend&) = delete;

    // ========================================================================
    // Configuration
    // ========================================================================

    /// Configure the backend before initialization
    /// @param config Configuration options
    void configure(const D3D12BackendConfig& config);

    // ========================================================================
    // IRenderBackend Implementation
    // ========================================================================

    [[nodiscard]] bool initialize() override;
    void shutdown() override;
    [[nodiscard]] bool isInitialized() const override { return m_initialized; }
    [[nodiscard]] BackendType getType() const override { return BackendType::D3D12; }
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
    [[nodiscard]] void* getNativeContext() const override { return m_commandList; }

    // ========================================================================
    // D3D12-Specific Methods
    // ========================================================================

    /// Get the D3D12 device
    [[nodiscard]] ID3D12Device* getDevice() const { return m_device; }

    /// Get the current command list
    [[nodiscard]] ID3D12GraphicsCommandList* getCommandList() const { return m_commandList; }

    /// Get the command queue
    [[nodiscard]] ID3D12CommandQueue* getCommandQueue() const { return m_commandQueue; }

    /// Set the render target for the current frame
    /// @param rtv CPU descriptor handle for the render target
    /// @param dsv CPU descriptor handle for depth stencil (optional)
    void setRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE rtv, D3D12_CPU_DESCRIPTOR_HANDLE* dsv = nullptr);

    /// Create a font texture from atlas data
    /// @param width Atlas width
    /// @param height Atlas height
    /// @param pixels Grayscale pixel data (R8)
    /// @return Handle to the created texture
    [[nodiscard]] TextureHandle createFontTexture(u32 width, u32 height, const u8* pixels);

    /// Wait for the GPU to finish all pending work
    void waitForGpu();

private:
    // Initialization helpers
    bool createDeviceObjects();
    void destroyDeviceObjects();
    bool createRootSignature();
    bool createPipelineState();
    bool createDescriptorHeaps();
    bool createFrameResources();

    // Frame management
    void waitForFrame(u32 frameIndex);
    D3D12FrameResources& getCurrentFrameResources();

    // Buffer management
    void ensureVertexBufferSize(D3D12FrameResources& frame, u32 requiredSize);
    void ensureIndexBufferSize(D3D12FrameResources& frame, u32 requiredSize);

    // Rending helpers
    void setupRenderState(i32 displayWidth, i32 displayHeight);
    void renderDrawCommands(const DrawList& drawList);

    // Descriptor heap helpers
    D3D12_GPU_DESCRIPTOR_HANDLE getTextureGpuHandle(TextureHandle texture);

private:
    // Configuration
    D3D12BackendConfig m_config{};
    BackendCapabilities m_capabilities{};
    bool m_initialized = false;

    // D3D12 Device & Queue
    ID3D12Device* m_device = nullptr;
    ID3D12CommandQueue* m_commandQueue = nullptr;
    ID3D12GraphicsCommandList* m_commandList = nullptr;

    // Pipeline state
    ID3D12RootSignature* m_rootSignature = nullptr;
    ID3D12PipelineState* m_pipelineState = nullptr;

    // Descriptor heaps
    ID3D12DescriptorHeap* m_srvHeap = nullptr;
    u32 m_srvHeapIncrementSize = 0;
    u32 m_nextSrvIndex = 0;

    // Font texture
    ID3D12Resource* m_fontTexture = nullptr;
    u32 m_fontTextureIndex = 0;

    // Frame resources (double/triple buffering)
    std::vector<D3D12FrameResources> m_frameResources;
    u32 m_frameIndex = 0;

    // Synchronization
    ID3D12Fence* m_fence = nullptr;
    void* m_fenceEvent = nullptr;
    u64 m_fenceValue = 0;

    // Frame state
    i32 m_displayWidth = 0;
    i32 m_displayHeight = 0;

    // Current render target
    D3D12_CPU_DESCRIPTOR_HANDLE* m_currentRtv = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE* m_currentDsv = nullptr;
};

}  // namespace dakt::gui
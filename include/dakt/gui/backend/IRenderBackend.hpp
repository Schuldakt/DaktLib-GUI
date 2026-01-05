#ifndef DAKT_GUI_IRENDER_BACKEND_HPP
#define DAKT_GUI_IRENDER_BACKEND_HPP

#include "../core/Types.hpp"
#include <cstdint>
#include <memory>
#include <string>

namespace dakt::gui {

class DrawList;

// ============================================================================
// Opaque Resource Handles (ABI-stable)
// ============================================================================

using BufferHandle = uint64_t;
using TextureHandle = uint64_t;
using ShaderHandle = uint64_t;
using PipelineHandle = uint64_t;

constexpr BufferHandle InvalidBuffer = 0;
constexpr TextureHandle InvalidTexture = 0;
constexpr ShaderHandle InvalidShader = 0;
constexpr PipelineHandle InvalidPipeline = 0;

// ============================================================================
// Resource Descriptors
// ============================================================================

enum class BufferUsage : uint32_t { Vertex = 1 << 0, Index = 1 << 1, Uniform = 1 << 2, Storage = 1 << 3, Staging = 1 << 4 };

inline BufferUsage operator|(BufferUsage a, BufferUsage b) { return static_cast<BufferUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }

inline bool operator&(BufferUsage a, BufferUsage b) { return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0; }

enum class TextureFormat : uint32_t { R8, RG8, RGBA8, BGRA8, R16F, RGBA16F, R32F, RGBA32F, Depth24Stencil8, Depth32F };

enum class TextureUsage : uint32_t { Sampled = 1 << 0, Storage = 1 << 1, RenderTarget = 1 << 2, DepthStencil = 1 << 3 };

inline TextureUsage operator|(TextureUsage a, TextureUsage b) { return static_cast<TextureUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }

struct BufferDesc {
    uint64_t size = 0;
    BufferUsage usage = BufferUsage::Vertex;
    bool hostVisible = false; // CPU-mappable
    const void* initialData = nullptr;
};

struct TextureDesc {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 1;
    uint32_t mipLevels = 1;
    TextureFormat format = TextureFormat::RGBA8;
    TextureUsage usage = TextureUsage::Sampled;
    const void* initialData = nullptr;
};

// ============================================================================
// Render Backend Capabilities
// ============================================================================

struct BackendCapabilities {
    uint32_t maxTextureSize = 4096;
    uint32_t maxUniformBufferSize = 65536;
    uint32_t maxVertexAttributes = 16;
    bool supportsCompute = false;
    bool supportsGeometryShaders = false;
    bool supportsTessellation = false;
    bool supportsMSAA = true;
    uint32_t maxMSAASamples = 8;
    std::string deviceName;
    std::string apiVersion;
};

// ============================================================================
// Render Backend Interface
// ============================================================================

class DAKT_GUI_API IRenderBackend {
  public:
    virtual ~IRenderBackend() = default;

    // Lifecycle
    virtual bool initialize(void* windowHandle, uint32_t width, uint32_t height) = 0;
    virtual void shutdown() = 0;

    // Frame management
    virtual bool beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void present() = 0;

    // Draw submission
    virtual void submit(const DrawList& drawList) = 0;

    // Resize handling
    virtual void resize(uint32_t width, uint32_t height) = 0;

    // Resource creation
    virtual BufferHandle createBuffer(const BufferDesc& desc) = 0;
    virtual void destroyBuffer(BufferHandle handle) = 0;
    virtual void* mapBuffer(BufferHandle handle) = 0;
    virtual void unmapBuffer(BufferHandle handle) = 0;
    virtual void updateBuffer(BufferHandle handle, const void* data, uint64_t size, uint64_t offset = 0) = 0;

    virtual TextureHandle createTexture(const TextureDesc& desc) = 0;
    virtual void destroyTexture(TextureHandle handle) = 0;
    virtual void updateTexture(TextureHandle handle, const void* data, uint32_t width, uint32_t height) = 0;

    // Capabilities
    virtual const BackendCapabilities& getCapabilities() const = 0;
    virtual const char* getName() const = 0;

    // Debug naming
    enum class ResourceType { Buffer, Texture };
    virtual void setDebugName(ResourceType type, uint64_t handle, const char* name) = 0;
};

// Factory function (implemented per backend)
std::unique_ptr<IRenderBackend> createVulkanBackend();
std::unique_ptr<IRenderBackend> createMetalBackend();
std::unique_ptr<IRenderBackend> createDX11Backend();
std::unique_ptr<IRenderBackend> createDX12Backend();
std::unique_ptr<IRenderBackend> createOpenGLBackend();

} // namespace dakt::gui

#endif // DAKT_GUI_IRENDER_BACKEND_HPP

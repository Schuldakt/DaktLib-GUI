// ============================================================================
// DaktLib GUI Module - Vulkan Backend
// ============================================================================
// Vulkan rendering backend for the GUI system.
// Provides low-level, high-performance rendering with explicit control over
// GPU resources, synchronization, and command buffers.
// ============================================================================

#pragma once

#include <dakt/gui/backends/RenderBackend.hpp>

#include <vector>

// Forward declarations for Vulkan types
// Using opaque handles to avoid including vulkan.h in the header
typedef struct VkInstance_T* VkInstance;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkDevice_T* VkDevice;
typedef struct VkQueue_T* VkQueue;
typedef struct VkCommandPool_T* VkCommandPool;
typedef struct VkCommandBuffer_T* VkCommandBuffer;
typedef struct VkRenderPass_T* VkRenderPass;
typedef struct VkFramebuffer_T* VkFramebuffer;
typedef struct VkPipelineLayout_T* VkPipelineLayout;
typedef struct VkPipeline_T* VkPipeline;
typedef struct VkDescriptorSetLayout_T* VkDescriptorSetLayout;
typedef struct VkDescriptorPool_T* VkDescriptorPool;
typedef struct VkDescriptorSet_T* VkDescriptorSet;
typedef struct VkBuffer_T* VkBuffer;
typedef struct VkDeviceMemory_T* VkDeviceMemory;
typedef struct VkImage_T* VkImage;
typedef struct VkImageView_T* VkImageView;
typedef struct VkSampler_T* VkSampler;
typedef struct VkShaderModule_T* VkShaderModule;
typedef struct VkFence_T* VkFence;
typedef struct VkSemaphore_T* VkSemaphore;
typedef uint64_t VkDeviceSize;
typedef uint32_t VkFormat;

namespace dakt::gui
{

// ============================================================================
// Vulkan Backend Configuration
// ============================================================================

struct VulkanBackendConfig
{
    VkInstance instance = nullptr;
    VkPhysicalDevice physicalDevice = nullptr;
    VkDevice device = nullptr;
    VkQueue graphicsQueue = nullptr;
    u32 graphicsQueueFamily = 0;

    // Render pass configuration
    VkRenderPass renderPass = nullptr;  // External render pass to use
    u32 subpass = 0;

    // Number of frames in flight
    u32 numFramesInFlight = 2;

    // Initial buffer sizes (will grow as needed)
    u32 initialVertexBufferSize = 5000;
    u32 initialIndexBufferSize = 10000;

    // Descriptor pool sizes
    u32 maxDescriptorSets = 1000;
    u32 maxSampledImages = 1000;

    // Sample count for MSAA (VK_SAMPLE_COUNT_1_BIT = no MSAA)
    u32 msaaSamples = 1;

    // Enable validation layers in debug
    bool enableValidation = false;
};

// ============================================================================
// Vulkan Frame Resources
// ============================================================================

struct VulkanFrameResources
{
    VkCommandPool commandPool = nullptr;
    VkCommandBuffer commandBuffer = nullptr;
    VkFence fence = nullptr;

    // Dynamic buffers
    VkBuffer vertexBuffer = nullptr;
    VkDeviceMemory vertexBufferMemory = nullptr;
    void* vertexBufferMapped = nullptr;
    VkDeviceSize vertexBufferSize = 0;

    VkBuffer indexBuffer = nullptr;
    VkDeviceMemory indexBufferMemory = nullptr;
    void* indexBufferMapped = nullptr;
    VkDeviceSize indexBufferSize = 0;
};

// ============================================================================
// Vulkan Texture Data
// ============================================================================

struct VulkanTextureData
{
    VkImage image = nullptr;
    VkDeviceMemory memory = nullptr;
    VkImageView view = nullptr;
    VkDescriptorSet descriptorSet = nullptr;
    u32 width = 0;
    u32 height = 0;
    VkFormat format = 0;
};

// ============================================================================
// Vulkan Backend Implementation
// ============================================================================

class VulkanBackend final : public IRenderBackend
{
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // Non-copyable
    VulkanBackend(const VulkanBackend&) = delete;
    VulkanBackend& operator=(const VulkanBackend&) = delete;

    // ========================================================================
    // Configuration
    // ========================================================================

    /// Configure the backend before initialization
    /// @param config Configuration options
    void configure(const VulkanBackendConfig& config);

    // ========================================================================
    // IRenderBackend Implementation
    // ========================================================================

    [[nodiscard]] bool initialize() override;
    void shutdown() override;
    [[nodiscard]] bool isInitialized() const override { return m_initialized; }
    [[nodiscard]] BackendType getType() const override { return BackendType::Vulkan; }
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
    [[nodiscard]] void* getNativeContext() const override { return getCurrentCommandBuffer(); }

    // ========================================================================
    // Vulkan-Specific Methods
    // ========================================================================

    /// Get the Vulkan device
    [[nodiscard]] VkDevice getDevice() const { return m_device; }

    /// Get the Vulkan physical device
    [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }

    /// Get the current command buffer
    [[nodiscard]] VkCommandBuffer getCurrentCommandBuffer() const;

    /// Get the graphics queue
    [[nodiscard]] VkQueue getGraphicsQueue() const { return m_graphicsQueue; }

    /// Get the pipeline
    [[nodiscard]] VkPipeline getPipeline() const { return m_pipeline; }

    /// Get the pipeline layout
    [[nodiscard]] VkPipelineLayout getPipelineLayout() const { return m_pipelineLayout; }

    /// Set the current framebuffer for rendering
    /// @param framebuffer The framebuffer to render to
    /// @param width Framebuffer width
    /// @param height Framebuffer height
    void setFramebuffer(VkFramebuffer framebuffer, u32 width, u32 height);

    /// Create a font texture from atlas data
    /// @param width Atlas width
    /// @param height Atlas height
    /// @param pixels Grayscale pixel data (R8)
    /// @return Handle to the created texture
    [[nodiscard]] TextureHandle createFontTexture(u32 width, u32 height, const u8* pixels);

    /// Get descriptor set for a texture (for external rendering)
    [[nodiscard]] VkDescriptorSet getTextureDescriptorSet(TextureHandle texture) const;

    /// Wait for device to be idle
    void waitIdle();

private:
    // Initialization helpers
    bool createDeviceObjects();
    void destroyDeviceObjects();
    bool createDescriptorSetLayout();
    bool createPipelineLayout();
    bool createPipeline();
    bool createDescriptorPool();
    bool createSampler();
    bool createFrameResources();

    // Shader loading
    VkShaderModule createShaderModule(const u32* code, usize codeSize);

    // Buffer management
    bool createBuffer(VkDeviceSize size, u32 usage, u32 properties, VkBuffer& buffer, VkDeviceMemory& memory);
    void ensureVertexBufferSize(VulkanFrameResources& frame, VkDeviceSize requiredSize);
    void ensureIndexBufferSize(VulkanFrameResources& frame, VkDeviceSize requiredSize);

    // Texture helpers
    bool createImage(u32 width, u32 height, VkFormat format, u32 usage, VkImage& image, VkDeviceMemory& memory);
    bool createImageView(VkImage image, VkFormat format, VkImageView& view);
    void transitionImageLayout(VkImage image, VkFormat format, u32 oldLayout, u32 newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, u32 width, u32 height);

    // Memory helpers
    u32 findMemoryType(u32 typeFilter, u32 properties);

    // Command buffer helpers
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    // Frame management
    VulkanFrameResources& getCurrentFrameResources();
    void waitForFrame(u32 frameIndex);

    // Rendering helpers
    void setupRenderState(i32 displayWidth, i32 displayHeight);
    void renderDrawCommands(const DrawList& drawList);

private:
    // Configuration
    VulkanBackendConfig m_config{};
    BackendCapabilities m_capabilities{};
    bool m_initialized = false;

    // Vulkan objects
    VkInstance m_instance = nullptr;
    VkPhysicalDevice m_physicalDevice = nullptr;
    VkDevice m_device = nullptr;
    VkQueue m_graphicsQueue = nullptr;
    u32 m_graphicsQueueFamily = 0;

    // Render pass (external or internal)
    VkRenderPass m_renderPass = nullptr;
    bool m_ownsRenderPass = false;

    // Pipeline
    VkDescriptorSetLayout m_descriptorSetLayout = nullptr;
    VkPipelineLayout m_pipelineLayout = nullptr;
    VkPipeline m_pipeline = nullptr;

    // Descriptors
    VkDescriptorPool m_descriptorPool = nullptr;
    VkSampler m_sampler = nullptr;

    // Shaders
    VkShaderModule m_vertexShader = nullptr;
    VkShaderModule m_fragmentShader = nullptr;

    // Frame resources
    std::vector<VulkanFrameResources> m_frameResources;
    u32 m_frameIndex = 0;

    // Font texture
    VulkanTextureData m_fontTexture{};

    // Texture tracking
    std::vector<VulkanTextureData> m_textures;
    uintptr_t m_nextTextureId = 1;

    // Current frame state
    VkFramebuffer m_currentFramebuffer = nullptr;
    i32 m_displayWidth = 0;
    i32 m_displayHeight = 0;
};

}  // namespace dakt::gui
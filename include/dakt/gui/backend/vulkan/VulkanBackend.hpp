#ifndef DAKTLIB_GUI_VULKAN_BACKEND_HPP
#define DAKTLIB_GUI_VULKAN_BACKEND_HPP

#include "../IRenderBackend.hpp"

// Only compile Vulkan backend when explicitly enabled
#if defined(DAKTLIB_ENABLE_VULKAN)

#include <array>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

// =============================================================================
// Vulkan Forward Declarations
// =============================================================================
// Forward declare Vulkan types to avoid including vulkan.h in the header
// These match the Vulkan dispatchable/non-dispatchable handle types

typedef struct VkInstance_T* VkInstance;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkDevice_T* VkDevice;
typedef struct VkQueue_T* VkQueue;
typedef struct VkSurfaceKHR_T* VkSurfaceKHR;
typedef struct VkSwapchainKHR_T* VkSwapchainKHR;
typedef struct VkCommandPool_T* VkCommandPool;
typedef struct VkCommandBuffer_T* VkCommandBuffer;
typedef struct VkRenderPass_T* VkRenderPass;
typedef struct VkFramebuffer_T* VkFramebuffer;
typedef struct VkPipeline_T* VkPipeline;
typedef struct VkPipelineLayout_T* VkPipelineLayout;
typedef struct VkDescriptorPool_T* VkDescriptorPool;
typedef struct VkDescriptorSetLayout_T* VkDescriptorSetLayout;
typedef struct VkDescriptorSet_T* VkDescriptorSet;
typedef struct VkSemaphore_T* VkSemaphore;
typedef struct VkFence_T* VkFence;
typedef struct VkBuffer_T* VkBuffer;
typedef struct VkImage_T* VkImage;
typedef struct VkImageView_T* VkImageView;
typedef struct VkSampler_T* VkSampler;
typedef struct VkShaderModule_T* VkShaderModule;
typedef struct VkDeviceMemory_T* VkDeviceMemory;

namespace dakt::gui {

// =============================================================================
// Vulkan Resource Wrappers
// =============================================================================

struct VulkanBuffer {
    VkBuffer buffer = nullptr;
    VkDeviceMemory memory = nullptr;
    uint64_t size = 0;
    void* mappedPtr = nullptr;
    BufferUsage usage = BufferUsage::Vertex;
};

struct VulkanTexture {
    VkImage image = nullptr;
    VkImageView view = nullptr;
    VkDeviceMemory memory = nullptr;
    VkSampler sampler = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    TextureFormat format = TextureFormat::RGBA8;
};

// =============================================================================
// Frame Resources (per-frame-in-flight)
// =============================================================================

struct FrameResources {
    VkCommandBuffer commandBuffer = nullptr;
    VkSemaphore imageAvailable = nullptr;
    VkSemaphore renderFinished = nullptr;
    VkFence inFlightFence = nullptr;

    // Dynamic buffers (ring buffer per frame)
    VulkanBuffer vertexBuffer{};
    VulkanBuffer indexBuffer{};
    VulkanBuffer uniformBuffer{};

    uint64_t vertexBufferOffset = 0;
    uint64_t indexBufferOffset = 0;
    uint64_t uniformBufferOffset = 0;
};

// =============================================================================
// Vulkan Backend Implementation
// =============================================================================

class DAKTLIB_GUI_API VulkanBackend : public IRenderBackend {
  public:
    VulkanBackend();
    ~VulkanBackend() override;

    // Non-copyable, non-movable
    VulkanBackend(const VulkanBackend&) = delete;
    VulkanBackend& operator=(const VulkanBackend&) = delete;
    VulkanBackend(VulkanBackend&&) = delete;
    VulkanBackend& operator=(VulkanBackend&&) = delete;

    // IRenderBackend interface
    bool initialize(void* windowHandle, uint32_t width, uint32_t height) override;
    void shutdown() override;

    bool beginFrame() override;
    void endFrame() override;
    void present() override;

    void submit(const DrawList& drawList) override;
    void resize(uint32_t width, uint32_t height) override;

    BufferHandle createBuffer(const BufferDesc& desc) override;
    void destroyBuffer(BufferHandle handle) override;
    void* mapBuffer(BufferHandle handle) override;
    void unmapBuffer(BufferHandle handle) override;
    void updateBuffer(BufferHandle handle, const void* data, uint64_t size, uint64_t offset) override;

    TextureHandle createTexture(const TextureDesc& desc) override;
    void destroyTexture(TextureHandle handle) override;
    void updateTexture(TextureHandle handle, const void* data, uint32_t width, uint32_t height) override;

    [[nodiscard]] const BackendCapabilities& getCapabilities() const override { return capabilities_; }
    [[nodiscard]] const char* getName() const override { return "Vulkan"; }

    void setDebugName(ResourceType type, uint64_t handle, const char* name) override;

  private:
    // Initialization helpers
    bool createInstance();
    bool selectPhysicalDevice();
    bool createLogicalDevice();
    bool createSurface(void* windowHandle);
    bool createSwapchain();
    bool createRenderPass();
    bool createFramebuffers();
    bool createCommandPool();
    bool createSyncObjects();
    bool createDescriptorPool();
    bool createPipelines();
    bool createDefaultResources();

    // Cleanup helpers
    void destroySwapchain();
    void cleanupResources();

    // Utility helpers
    uint32_t findMemoryType(uint32_t typeFilter, uint32_t properties);
    VkShaderModule createShaderModule(const uint32_t* code, size_t size);

    // Rendering helpers
    void recordCommandBuffer(const DrawList& drawList);
    void bindPipeline(bool textured);
    void updateUniformBuffer();

  private:
    // Vulkan core handles
    VkInstance instance_ = nullptr;
    VkPhysicalDevice physicalDevice_ = nullptr;
    VkDevice device_ = nullptr;
    VkQueue graphicsQueue_ = nullptr;
    VkQueue presentQueue_ = nullptr;
    VkSurfaceKHR surface_ = nullptr;
    VkSwapchainKHR swapchain_ = nullptr;
    VkRenderPass renderPass_ = nullptr;
    VkCommandPool commandPool_ = nullptr;
    VkDescriptorPool descriptorPool_ = nullptr;
    VkDescriptorSetLayout descriptorSetLayout_ = nullptr;

    // Pipelines
    VkPipelineLayout pipelineLayout_ = nullptr;
    VkPipeline uiPipeline_ = nullptr;   // Colored geometry
    VkPipeline textPipeline_ = nullptr; // SDF text

    // Swapchain resources
    std::vector<VkImage> swapchainImages_;
    std::vector<VkImageView> swapchainImageViews_;
    std::vector<VkFramebuffer> framebuffers_;
    uint32_t swapchainFormat_ = 0;
    uint32_t swapchainWidth_ = 0;
    uint32_t swapchainHeight_ = 0;

    // Frame management
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    std::array<FrameResources, MAX_FRAMES_IN_FLIGHT> frameResources_{};
    uint32_t currentFrame_ = 0;
    uint32_t imageIndex_ = 0;

    // Queue family indices
    uint32_t graphicsFamily_ = UINT32_MAX;
    uint32_t presentFamily_ = UINT32_MAX;

    // Resource management
    std::unordered_map<BufferHandle, VulkanBuffer> buffers_;
    std::unordered_map<TextureHandle, VulkanTexture> textures_;
    uint64_t nextBufferHandle_ = 1;
    uint64_t nextTextureHandle_ = 1;

    // Default resources
    TextureHandle whiteTexture_ = InvalidTexture;
    VkDescriptorSet currentDescriptorSet_ = nullptr;

    // Capabilities
    BackendCapabilities capabilities_{};

    // State
    bool initialized_ = false;
    bool frameInProgress_ = false;
    uint32_t windowWidth_ = 0;
    uint32_t windowHeight_ = 0;
};

// Factory function declaration
[[nodiscard]] DAKTLIB_GUI_API std::unique_ptr<IRenderBackend> createVulkanBackend();

} // namespace dakt::gui

#else // !DAKTLIB_ENABLE_VULKAN

// =============================================================================
// Stub when Vulkan is not enabled
// =============================================================================

#include <memory>

namespace dakt::gui {

// Factory returns nullptr when Vulkan is disabled
[[nodiscard]] inline std::unique_ptr<IRenderBackend> createVulkanBackend() { return nullptr; }

} // namespace dakt::gui

#endif // DAKTLIB_ENABLE_VULKAN

#endif // DAKT_GUI_VULKAN_BACKEND_HPP
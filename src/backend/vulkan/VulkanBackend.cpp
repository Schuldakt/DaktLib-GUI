#include "dakt/gui/backend/vulkan/VulkanBackend.hpp"

// Only compile when Vulkan is enabled
#if defined(DAKTLIB_ENABLE_VULKAN)

#include "dakt/gui/draw/DrawList.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <vector>

// =============================================================================
// Platform-specific Vulkan surface extensions
// =============================================================================

#if defined(DAKTLIB_PLATFORM_WINDOWS) || defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(DAKTLIB_PLATFORM_LINUX) || defined(__linux__)
// Support both X11 and Wayland - X11/XCB is more common for now
#define VK_USE_PLATFORM_XCB_KHR
// Uncomment for Wayland: #define VK_USE_PLATFORM_WAYLAND_KHR
#elif defined(DAKTLIB_PLATFORM_MACOS) || defined(__APPLE__)
#define VK_USE_PLATFORM_METAL_EXT
#endif

#include <vulkan/vulkan.h>

namespace dakt::gui {

// =============================================================================
// Embedded Shader SPIR-V
// =============================================================================
// These are placeholder SPIR-V modules - real shaders should be compiled
// from GLSL and embedded via CMake

static const uint32_t UI_VERT_SPV[] = {
    // Minimal valid SPIR-V header (placeholder)
    0x07230203, 0x00010000, 0x000d000b, 0x00000030, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
};

static const uint32_t UI_FRAG_SPV[] = {
    0x07230203, 0x00010000, 0x000d000b, 0x00000018, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
};

// =============================================================================
// VulkanBackend Implementation
// =============================================================================

VulkanBackend::VulkanBackend() = default;

VulkanBackend::~VulkanBackend() {
    if (initialized_) {
        shutdown();
    }
}

bool VulkanBackend::initialize(void* windowHandle, uint32_t width, uint32_t height) {
    windowWidth_ = width;
    windowHeight_ = height;

    if (!createInstance()) {
        return false;
    }

    if (!createSurface(windowHandle)) {
        return false;
    }

    if (!selectPhysicalDevice()) {
        return false;
    }

    if (!createLogicalDevice()) {
        return false;
    }

    if (!createSwapchain()) {
        return false;
    }

    if (!createRenderPass()) {
        return false;
    }

    if (!createFramebuffers()) {
        return false;
    }

    if (!createCommandPool()) {
        return false;
    }

    if (!createSyncObjects()) {
        return false;
    }

    if (!createDescriptorPool()) {
        return false;
    }

    if (!createPipelines()) {
        return false;
    }

    if (!createDefaultResources()) {
        return false;
    }

    // Set capabilities
    initialized_ = true;
    return true;
}

void VulkanBackend::shutdown() {
    if (!initialized_) {
        return;
    }

    // Wait for device to be idle
    if (device_) {
        vkDeviceWaitIdle(device_);
    }

    cleanupResources();
    destroySwapchain();

    // Destroy sync objects
    for (auto& frame : frameResources_) {
        if (frame.imageAvailable) {
            vkDestroySemaphore(device_, frame.imageAvailable, nullptr);
            frame.imageAvailable = nullptr;
        }
        if (frame.renderFinished) {
            vkDestroySemaphore(device_, frame.renderFinished, nullptr);
            frame.renderFinished = nullptr;
        }
        if (frame.inFlightFence) {
            vkDestroyFence(device_, frame.inFlightFence, nullptr);
            frame.inFlightFence = nullptr;
        }
    }

    if (commandPool_) {
        vkDestroyCommandPool(device_, commandPool_, nullptr);
        commandPool_ = nullptr;
    }

    if (descriptorPool_) {
        vkDestroyDescriptorPool(device_, descriptorPool_, nullptr);
        descriptorPool_ = nullptr;
    }

    if (descriptorSetLayout_) {
        vkDestroyDescriptorSetLayout(device_, descriptorSetLayout_, nullptr);
        descriptorSetLayout_ = nullptr;
    }

    if (uiPipeline_) {
        vkDestroyPipeline(device_, uiPipeline_, nullptr);
        uiPipeline_ = nullptr;
    }

    if (textPipeline_) {
        vkDestroyPipeline(device_, textPipeline_, nullptr);
        textPipeline_ = nullptr;
    }

    if (pipelineLayout_) {
        vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
        pipelineLayout_ = nullptr;
    }

    if (renderPass_) {
        vkDestroyRenderPass(device_, renderPass_, nullptr);
        renderPass_ = nullptr;
    }

    if (device_) {
        vkDestroyDevice(device_, nullptr);
        device_ = nullptr;
    }

    if (surface_) {
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        surface_ = nullptr;
    }

    if (instance_) {
        vkDestroyInstance(instance_, nullptr);
        instance_ = nullptr;
    }

    initialized_ = false;
}

// =============================================================================
// Instance Creation
// =============================================================================

bool VulkanBackend::createInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "DaktLib-GUI";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "DaktLib";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // Required extensions based on platform
    std::vector<const char*> extensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(DAKTLIB_PLATFORM_WINDOWS) || defined(_WIN32)
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(DAKTLIB_PLATFORM_LINUX) || defined(__linux__)
        VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#elif defined(DAKTLIB_PLATFORM_MACOS) || defined(__APPLE__)
        VK_EXT_METAL_SURFACE_EXTENSION_NAME,
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
#endif
    };

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

#if defined(DAKTLIB_PLATFORM_MACOS) || defined(__APPLE__)
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

#if !defined(NDEBUG)
    // Enable validation layers in debug builds
    const char* validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

    // Check if validation layers are available
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    bool validationAvailable = false;
    for (const auto& layer : availableLayers) {
        if (strcmp(layer.layerName, validationLayers[0]) == 0) {
            validationAvailable = true;
            break;
        }
    }

    if (validationAvailable) {
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = validationLayers;
    }
#endif

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance_);
    return result == VK_SUCCESS;
}

// =============================================================================
// Physical Device Selection
// =============================================================================

bool VulkanBackend::selectPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);

    if (deviceCount == 0) {
        return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

    // Select first suitable device (prefer discrete GPU)
    VkPhysicalDevice selectedDevice = nullptr;
    int bestScore = -1;

    for (const auto& device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        capabilities_.maxTextureSize = props.limits.maxImageDimension2D;
        capabilities_.maxUniformBufferSize = static_cast<uint32_t>(props.limits.maxUniformBufferRange);
        capabilities_.maxVertexAttributes = props.limits.maxVertexInputAttributes;
        capabilities_.supportsCompute = true;         // Vulkan always supports compute
        capabilities_.supportsGeometryShaders = true; // Query actual feature if needed
        capabilities_.supportsTessellation = true;    // Query actual feature if needed
        capabilities_.supportsMSAA = true;
        capabilities_.maxMSAASamples = 8; // Could query VkSampleCountFlags
        capabilities_.deviceName = props.deviceName;
        capabilities_.apiVersion = std::to_string(VK_VERSION_MAJOR(props.apiVersion)) + "." + std::to_string(VK_VERSION_MINOR(props.apiVersion)) + "." + std::to_string(VK_VERSION_PATCH(props.apiVersion));

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);

        // Find queue families
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        // Check for graphics queue
        bool hasGraphicsQueue = false;
        uint32_t graphicsIdx = UINT32_MAX;

        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                hasGraphicsQueue = true;
                graphicsIdx = i;
                break;
            }
        }

        if (!hasGraphicsQueue) {
            continue;
        }

        // Score the device
        int score = 0;
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        } else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            score += 100;
        }

        if (score > bestScore) {
            bestScore = score;
            selectedDevice = device;
            graphicsFamily_ = graphicsIdx;
            presentFamily_ = graphicsIdx; // Assume same for now
        }
    }

    if (!selectedDevice) {
        return false;
    }

    physicalDevice_ = selectedDevice;
    return true;
}

// =============================================================================
// Logical Device Creation
// =============================================================================

bool VulkanBackend::createLogicalDevice() {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsFamily_;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);

    // Add separate present queue if different
    if (presentFamily_ != graphicsFamily_) {
        VkDeviceQueueCreateInfo presentQueueInfo{};
        presentQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        presentQueueInfo.queueFamilyIndex = presentFamily_;
        presentQueueInfo.queueCount = 1;
        presentQueueInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(presentQueueInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#if defined(DAKTLIB_PLATFORM_MACOS) || defined(__APPLE__)
        "VK_KHR_portability_subset",
#endif
    };

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    VkResult result = vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_);
    if (result != VK_SUCCESS) {
        return false;
    }

    vkGetDeviceQueue(device_, graphicsFamily_, 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, presentFamily_, 0, &presentQueue_);

    return true;
}

// =============================================================================
// Surface Creation
// =============================================================================

bool VulkanBackend::createSurface(void* windowHandle) {
    if (!windowHandle) {
        // Allow headless mode (no surface)
        return true;
    }

#if defined(DAKTLIB_PLATFORM_WINDOWS) || defined(_WIN32)
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = static_cast<HWND>(windowHandle);
    createInfo.hinstance = GetModuleHandle(nullptr);

    VkResult result = vkCreateWin32SurfaceKHR(instance_, &createInfo, nullptr, &surface_);
    if (result != VK_SUCCESS) {
        return false;
    }

    // Verify present support
    VkBool32 presentSupport = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice_, presentFamily_, surface_, &presentSupport);
    return presentSupport == VK_TRUE;

#elif defined(DAKTLIB_PLATFORM_LINUX) || defined(__linux__)
    // XCB surface creation would go here
    // Requires xcb_connection_t* and xcb_window_t
    (void)windowHandle;
    return true; // Placeholder

#elif defined(DAKTLIB_PLATFORM_MACOS) || defined(__APPLE__)
    // Metal surface creation would go here
    // Requires CAMetalLayer*
    (void)windowHandle;
    return true; // Placeholder

#else
    (void)windowHandle;
    return false;
#endif
}

// =============================================================================
// Swapchain Creation
// =============================================================================

bool VulkanBackend::createSwapchain() {
    if (!surface_) {
        return true; // Headless mode
    }

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_, surface_, &capabilities);

    // Choose surface format
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface_, &formatCount, nullptr);

    if (formatCount == 0) {
        return false;
    }

    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface_, &formatCount, formats.data());

    VkSurfaceFormatKHR surfaceFormat = formats[0];
    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = format;
            break;
        }
    }

    // Choose present mode (prefer mailbox, fallback to FIFO)
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_, surface_, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_, surface_, &presentModeCount, presentModes.data());

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& mode : presentModes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = mode;
            break;
        }
    }

    // Choose extent
    VkExtent2D extent;
    if (capabilities.currentExtent.width != UINT32_MAX) {
        extent = capabilities.currentExtent;
    } else {
        extent.width = std::clamp(windowWidth_, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(windowHeight_, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface_;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapchain_) != VK_SUCCESS) {
        return false;
    }

    swapchainFormat_ = surfaceFormat.format;
    swapchainWidth_ = extent.width;
    swapchainHeight_ = extent.height;

    // Get swapchain images
    vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, nullptr);
    swapchainImages_.resize(imageCount);
    vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, swapchainImages_.data());

    // Create image views
    swapchainImageViews_.resize(imageCount);
    for (size_t i = 0; i < imageCount; ++i) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = swapchainImages_[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = static_cast<VkFormat>(swapchainFormat_);
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device_, &viewInfo, nullptr, &swapchainImageViews_[i]) != VK_SUCCESS) {
            return false;
        }
    }

    return true;
}

void VulkanBackend::destroySwapchain() {
    for (auto framebuffer : framebuffers_) {
        if (framebuffer) {
            vkDestroyFramebuffer(device_, framebuffer, nullptr);
        }
    }
    framebuffers_.clear();

    for (auto imageView : swapchainImageViews_) {
        if (imageView) {
            vkDestroyImageView(device_, imageView, nullptr);
        }
    }
    swapchainImageViews_.clear();
    swapchainImages_.clear();

    if (swapchain_) {
        vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        swapchain_ = nullptr;
    }
}

// =============================================================================
// Render Pass
// =============================================================================

bool VulkanBackend::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = static_cast<VkFormat>(swapchainFormat_);
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    return vkCreateRenderPass(device_, &renderPassInfo, nullptr, &renderPass_) == VK_SUCCESS;
}

// =============================================================================
// Framebuffers
// =============================================================================

bool VulkanBackend::createFramebuffers() {
    framebuffers_.resize(swapchainImageViews_.size());

    for (size_t i = 0; i < swapchainImageViews_.size(); ++i) {
        VkImageView attachments[] = {swapchainImageViews_[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass_;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchainWidth_;
        framebufferInfo.height = swapchainHeight_;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device_, &framebufferInfo, nullptr, &framebuffers_[i]) != VK_SUCCESS) {
            return false;
        }
    }

    return true;
}

// =============================================================================
// Command Pool
// =============================================================================

bool VulkanBackend::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = graphicsFamily_;

    if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_) != VK_SUCCESS) {
        return false;
    }

    // Allocate command buffers for each frame
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool_;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    for (auto& frame : frameResources_) {
        if (vkAllocateCommandBuffers(device_, &allocInfo, &frame.commandBuffer) != VK_SUCCESS) {
            return false;
        }
    }

    return true;
}

// =============================================================================
// Synchronization
// =============================================================================

bool VulkanBackend::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (auto& frame : frameResources_) {
        if (vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &frame.imageAvailable) != VK_SUCCESS || vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &frame.renderFinished) != VK_SUCCESS ||
            vkCreateFence(device_, &fenceInfo, nullptr, &frame.inFlightFence) != VK_SUCCESS) {
            return false;
        }
    }

    return true;
}

// =============================================================================
// Descriptor Pool
// =============================================================================

bool VulkanBackend::createDescriptorPool() {
    VkDescriptorPoolSize poolSizes[] = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100},
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = 100;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(device_, &poolInfo, nullptr, &descriptorPool_) != VK_SUCCESS) {
        return false;
    }

    // Create descriptor set layout
    VkDescriptorSetLayoutBinding bindings[] = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = bindings;

    return vkCreateDescriptorSetLayout(device_, &layoutInfo, nullptr, &descriptorSetLayout_) == VK_SUCCESS;
}

// =============================================================================
// Pipelines
// =============================================================================

bool VulkanBackend::createPipelines() {
    // Pipeline layout
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &descriptorSetLayout_;

    if (vkCreatePipelineLayout(device_, &layoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS) {
        return false;
    }

    // Note: Actual pipeline creation requires valid SPIR-V shaders
    // For now, we skip pipeline creation since we have placeholder shaders
    // Real implementation would create vertex/fragment shader modules and graphics pipeline

    return true;
}

// =============================================================================
// Default Resources
// =============================================================================

bool VulkanBackend::createDefaultResources() {
    // Create a 1x1 white texture as default
    uint32_t whitePixel = 0xFFFFFFFF;
    TextureDesc desc{};
    desc.width = 1;
    desc.height = 1;
    desc.format = TextureFormat::RGBA8;
    desc.usage = TextureUsage::Sampled;
    desc.initialData = &whitePixel;

    whiteTexture_ = createTexture(desc);
    return whiteTexture_ != InvalidTexture;
}

// =============================================================================
// Resize
// =============================================================================

void VulkanBackend::resize(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) {
        return;
    }

    windowWidth_ = width;
    windowHeight_ = height;

    if (device_) {
        vkDeviceWaitIdle(device_);
    }

    destroySwapchain();
    createSwapchain();
    createFramebuffers();
}

// =============================================================================
// Factory Function
// =============================================================================

std::unique_ptr<IRenderBackend> createVulkanBackend() { return std::make_unique<VulkanBackend>(); }

} // namespace dakt::gui

#endif // DAKT_ENABLE_VULKAN
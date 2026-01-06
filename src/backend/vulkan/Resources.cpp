/**
 * @file Resources.cpp
 * @brief Vulkan resource management (buffers, textures, memory)
 */

#if defined(DAKTLIB_ENABLE_VULKAN) || defined(__linux__) || defined(_WIN32)

#include "dakt/gui/backend/vulkan/VulkanBackend.hpp"
#include <cstring>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XCB_KHR
#elif defined(__APPLE__)
#define VK_USE_PLATFORM_METAL_EXT
#endif

#include <vulkan/vulkan.h>

namespace dakt::gui {

// ============================================================================
// Memory Management
// ============================================================================

uint32_t VulkanBackend::findMemoryType(uint32_t typeFilter, uint32_t properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    return UINT32_MAX;
}

// ============================================================================
// Buffer Management
// ============================================================================

BufferHandle VulkanBackend::createBuffer(const BufferDesc& desc) {
    VulkanBuffer vkBuffer{};
    vkBuffer.size = desc.size;
    vkBuffer.usage = desc.usage;

    // Map usage flags
    VkBufferUsageFlags usageFlags = 0;
    if (desc.usage & BufferUsage::Vertex) {
        usageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (desc.usage & BufferUsage::Index) {
        usageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (desc.usage & BufferUsage::Uniform) {
        usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (desc.usage & BufferUsage::Storage) {
        usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = desc.size;
    bufferInfo.usage = usageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device_, &bufferInfo, nullptr, &vkBuffer.buffer) != VK_SUCCESS) {
        return InvalidBuffer;
    }

    // Get memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device_, vkBuffer.buffer, &memRequirements);

    // Allocate memory
    VkMemoryPropertyFlags memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (desc.hostVisible) {
        memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, memProperties);

    if (allocInfo.memoryTypeIndex == UINT32_MAX) {
        vkDestroyBuffer(device_, vkBuffer.buffer, nullptr);
        return InvalidBuffer;
    }

    if (vkAllocateMemory(device_, &allocInfo, nullptr, &vkBuffer.memory) != VK_SUCCESS) {
        vkDestroyBuffer(device_, vkBuffer.buffer, nullptr);
        return InvalidBuffer;
    }

    vkBindBufferMemory(device_, vkBuffer.buffer, vkBuffer.memory, 0);

    // Upload initial data if provided
    if (desc.initialData && desc.hostVisible) {
        void* mapped;
        vkMapMemory(device_, vkBuffer.memory, 0, desc.size, 0, &mapped);
        memcpy(mapped, desc.initialData, desc.size);
        vkUnmapMemory(device_, vkBuffer.memory);
    }

    BufferHandle handle = nextBufferHandle_++;
    buffers_[handle] = vkBuffer;
    return handle;
}

void VulkanBackend::destroyBuffer(BufferHandle handle) {
    auto it = buffers_.find(handle);
    if (it == buffers_.end())
        return;

    VulkanBuffer& buffer = it->second;

    if (buffer.mappedPtr) {
        vkUnmapMemory(device_, buffer.memory);
    }

    vkDestroyBuffer(device_, buffer.buffer, nullptr);
    vkFreeMemory(device_, buffer.memory, nullptr);

    buffers_.erase(it);
}

void* VulkanBackend::mapBuffer(BufferHandle handle) {
    auto it = buffers_.find(handle);
    if (it == buffers_.end())
        return nullptr;

    VulkanBuffer& buffer = it->second;
    if (buffer.mappedPtr)
        return buffer.mappedPtr;

    void* mapped;
    if (vkMapMemory(device_, buffer.memory, 0, buffer.size, 0, &mapped) != VK_SUCCESS) {
        return nullptr;
    }

    buffer.mappedPtr = mapped;
    return mapped;
}

void VulkanBackend::unmapBuffer(BufferHandle handle) {
    auto it = buffers_.find(handle);
    if (it == buffers_.end())
        return;

    VulkanBuffer& buffer = it->second;
    if (buffer.mappedPtr) {
        vkUnmapMemory(device_, buffer.memory);
        buffer.mappedPtr = nullptr;
    }
}

void VulkanBackend::updateBuffer(BufferHandle handle, const void* data, uint64_t size, uint64_t offset) {
    auto it = buffers_.find(handle);
    if (it == buffers_.end())
        return;

    VulkanBuffer& buffer = it->second;

    void* mapped = mapBuffer(handle);
    if (mapped) {
        memcpy(static_cast<uint8_t*>(mapped) + offset, data, size);
        unmapBuffer(handle);
    }
}

// ============================================================================
// Texture Management
// ============================================================================

TextureHandle VulkanBackend::createTexture(const TextureDesc& desc) {
    VulkanTexture vkTexture{};
    vkTexture.width = desc.width;
    vkTexture.height = desc.height;
    vkTexture.format = desc.format;

    // Map format
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    switch (desc.format) {
    case TextureFormat::R8:
        format = VK_FORMAT_R8_UNORM;
        break;
    case TextureFormat::RG8:
        format = VK_FORMAT_R8G8_UNORM;
        break;
    case TextureFormat::RGBA8:
        format = VK_FORMAT_R8G8B8A8_UNORM;
        break;
    case TextureFormat::BGRA8:
        format = VK_FORMAT_B8G8R8A8_UNORM;
        break;
    case TextureFormat::R16F:
        format = VK_FORMAT_R16_SFLOAT;
        break;
    case TextureFormat::RGBA16F:
        format = VK_FORMAT_R16G16B16A16_SFLOAT;
        break;
    case TextureFormat::R32F:
        format = VK_FORMAT_R32_SFLOAT;
        break;
    case TextureFormat::RGBA32F:
        format = VK_FORMAT_R32G32B32A32_SFLOAT;
        break;
    case TextureFormat::Depth24Stencil8:
        format = VK_FORMAT_D24_UNORM_S8_UINT;
        break;
    case TextureFormat::Depth32F:
        format = VK_FORMAT_D32_SFLOAT;
        break;
    }

    // Map usage
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

    if (static_cast<uint32_t>(desc.usage) & static_cast<uint32_t>(TextureUsage::Sampled)) {
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (static_cast<uint32_t>(desc.usage) & static_cast<uint32_t>(TextureUsage::Storage)) {
        usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if (static_cast<uint32_t>(desc.usage) & static_cast<uint32_t>(TextureUsage::RenderTarget)) {
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (static_cast<uint32_t>(desc.usage) & static_cast<uint32_t>(TextureUsage::DepthStencil)) {
        usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    // Create image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = desc.width;
    imageInfo.extent.height = desc.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = desc.mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if (vkCreateImage(device_, &imageInfo, nullptr, &vkTexture.image) != VK_SUCCESS) {
        return InvalidTexture;
    }

    // Allocate memory
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device_, vkTexture.image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (allocInfo.memoryTypeIndex == UINT32_MAX) {
        vkDestroyImage(device_, vkTexture.image, nullptr);
        return InvalidTexture;
    }

    if (vkAllocateMemory(device_, &allocInfo, nullptr, &vkTexture.memory) != VK_SUCCESS) {
        vkDestroyImage(device_, vkTexture.image, nullptr);
        return InvalidTexture;
    }

    vkBindImageMemory(device_, vkTexture.image, vkTexture.memory, 0);

    // Create image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = vkTexture.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = desc.mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device_, &viewInfo, nullptr, &vkTexture.view) != VK_SUCCESS) {
        vkDestroyImage(device_, vkTexture.image, nullptr);
        vkFreeMemory(device_, vkTexture.memory, nullptr);
        return InvalidTexture;
    }

    // Create sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(desc.mipLevels);

    if (vkCreateSampler(device_, &samplerInfo, nullptr, &vkTexture.sampler) != VK_SUCCESS) {
        vkDestroyImageView(device_, vkTexture.view, nullptr);
        vkDestroyImage(device_, vkTexture.image, nullptr);
        vkFreeMemory(device_, vkTexture.memory, nullptr);
        return InvalidTexture;
    }

    // Upload initial data if provided
    if (desc.initialData) {
        updateTexture(nextTextureHandle_, desc.initialData, desc.width, desc.height);
    }

    TextureHandle handle = nextTextureHandle_++;
    textures_[handle] = vkTexture;
    return handle;
}

void VulkanBackend::destroyTexture(TextureHandle handle) {
    auto it = textures_.find(handle);
    if (it == textures_.end())
        return;

    VulkanTexture& texture = it->second;

    vkDestroySampler(device_, texture.sampler, nullptr);
    vkDestroyImageView(device_, texture.view, nullptr);
    vkDestroyImage(device_, texture.image, nullptr);
    vkFreeMemory(device_, texture.memory, nullptr);

    textures_.erase(it);
}

void VulkanBackend::updateTexture(TextureHandle handle, const void* data, uint32_t width, uint32_t height) {
    auto it = textures_.find(handle);
    if (it == textures_.end())
        return;

    VulkanTexture& texture = it->second;

    // Calculate buffer size
    size_t pixelSize = 4; // Assume RGBA8
    switch (texture.format) {
    case TextureFormat::R8:
        pixelSize = 1;
        break;
    case TextureFormat::RG8:
        pixelSize = 2;
        break;
    case TextureFormat::RGBA8:
    case TextureFormat::BGRA8:
        pixelSize = 4;
        break;
    case TextureFormat::R16F:
        pixelSize = 2;
        break;
    case TextureFormat::RGBA16F:
        pixelSize = 8;
        break;
    case TextureFormat::R32F:
        pixelSize = 4;
        break;
    case TextureFormat::RGBA32F:
        pixelSize = 16;
        break;
    default:
        break;
    }

    VkDeviceSize imageSize = width * height * pixelSize;

    // Create staging buffer
    BufferDesc stagingDesc;
    stagingDesc.size = imageSize;
    stagingDesc.usage = BufferUsage::Staging;
    stagingDesc.hostVisible = true;
    stagingDesc.initialData = data;

    BufferHandle staging = createBuffer(stagingDesc);
    if (staging == InvalidBuffer)
        return;

    // Transition image layout and copy
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool_;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // Transition to transfer destination
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture.image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    // Copy buffer to image
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffers_[staging].buffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // Transition to shader read
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    vkEndCommandBuffer(commandBuffer);

    // Submit
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue_);

    vkFreeCommandBuffers(device_, commandPool_, 1, &commandBuffer);
    destroyBuffer(staging);
}

// ============================================================================
// Debug Names
// ============================================================================

void VulkanBackend::setDebugName(ResourceType type, uint64_t handle, const char* name) {
    (void)type;
    (void)handle;
    (void)name;
    // Would use VK_EXT_debug_utils if available
}

void VulkanBackend::cleanupResources() {
    // Destroy all user-created buffers
    for (auto& [handle, buffer] : buffers_) {
        if (buffer.mappedPtr) {
            vkUnmapMemory(device_, buffer.memory);
        }
        vkDestroyBuffer(device_, buffer.buffer, nullptr);
        vkFreeMemory(device_, buffer.memory, nullptr);
    }
    buffers_.clear();

    // Destroy all user-created textures
    for (auto& [handle, texture] : textures_) {
        vkDestroySampler(device_, texture.sampler, nullptr);
        vkDestroyImageView(device_, texture.view, nullptr);
        vkDestroyImage(device_, texture.image, nullptr);
        vkFreeMemory(device_, texture.memory, nullptr);
    }
    textures_.clear();
}

} // namespace dakt::gui

#endif // DAKT_ENABLE_VULKAN

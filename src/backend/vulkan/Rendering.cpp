/**
 * @file Rendering.cpp
 * @brief Vulkan rendering and draw submission
 */

#if defined(DAKT_ENABLE_VULKAN) || defined(__linux__) || defined(_WIN32)

#include "dakt/gui/backend/vulkan/VulkanBackend.hpp"
#include "dakt/gui/draw/DrawList.hpp"
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
// Uniform Buffer Data
// ============================================================================

struct UIUniforms {
    float projectionMatrix[16];
};

// ============================================================================
// Frame Management
// ============================================================================

bool VulkanBackend::beginFrame() {
    if (!initialized_)
        return false;

    FrameResources& frame = frameResources_[currentFrame_];

    // Wait for previous frame
    vkWaitForFences(device_, 1, &frame.inFlightFence, VK_TRUE, UINT64_MAX);

    // Acquire next swapchain image
    if (swapchain_) {
        VkResult result = vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX, frame.imageAvailable, VK_NULL_HANDLE, &imageIndex_);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            resize(windowWidth_, windowHeight_);
            return false;
        }
    }

    vkResetFences(device_, 1, &frame.inFlightFence);
    vkResetCommandBuffer(frame.commandBuffer, 0);

    // Reset dynamic buffer offsets
    frame.vertexBufferOffset = 0;
    frame.indexBufferOffset = 0;
    frame.uniformBufferOffset = 0;

    frameInProgress_ = true;
    return true;
}

void VulkanBackend::endFrame() {
    if (!frameInProgress_)
        return;

    FrameResources& frame = frameResources_[currentFrame_];

    // End command buffer recording (should have been done in submit)
    vkEndCommandBuffer(frame.commandBuffer);

    // Submit command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {frame.imageAvailable};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = swapchain_ ? 1 : 0;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frame.commandBuffer;

    VkSemaphore signalSemaphores[] = {frame.renderFinished};
    submitInfo.signalSemaphoreCount = swapchain_ ? 1 : 0;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkQueueSubmit(graphicsQueue_, 1, &submitInfo, frame.inFlightFence);

    frameInProgress_ = false;
}

void VulkanBackend::present() {
    if (!swapchain_)
        return;

    FrameResources& frame = frameResources_[currentFrame_];

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &frame.renderFinished;

    VkSwapchainKHR swapchains[] = {swapchain_};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex_;

    VkResult result = vkQueuePresentKHR(presentQueue_, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        resize(windowWidth_, windowHeight_);
    }

    currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

// ============================================================================
// Draw Submission
// ============================================================================

void VulkanBackend::submit(const DrawList& drawList) {
    if (!frameInProgress_)
        return;

    FrameResources& frame = frameResources_[currentFrame_];

    // Begin command buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(frame.commandBuffer, &beginInfo);

    // Begin render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass_;
    renderPassInfo.framebuffer = framebuffers_.empty() ? VK_NULL_HANDLE : framebuffers_[imageIndex_];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {swapchainWidth_, swapchainHeight_};

    VkClearValue clearColor = {{{0.1f, 0.1f, 0.1f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    if (renderPassInfo.framebuffer) {
        vkCmdBeginRenderPass(frame.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Process draw commands
    recordCommandBuffer(drawList);

    if (renderPassInfo.framebuffer) {
        vkCmdEndRenderPass(frame.commandBuffer);
    }
}

void VulkanBackend::recordCommandBuffer(const DrawList& drawList) {
    FrameResources& frame = frameResources_[currentFrame_];

    const auto& vertices = drawList.getVertices();
    const auto& indices = drawList.getIndices();
    const auto& commands = drawList.getCommands();

    if (vertices.empty() || commands.empty())
        return;

    // Upload vertex data
    size_t vertexSize = vertices.size() * sizeof(Vertex);
    size_t indexSize = indices.size() * sizeof(uint32_t);

    // Ensure we have vertex buffer for this frame
    if (frame.vertexBuffer.buffer == nullptr || frame.vertexBuffer.size < vertexSize) {
        if (frame.vertexBuffer.buffer) {
            vkDestroyBuffer(device_, frame.vertexBuffer.buffer, nullptr);
            vkFreeMemory(device_, frame.vertexBuffer.memory, nullptr);
        }

        BufferDesc desc;
        desc.size = std::max(vertexSize, static_cast<size_t>(1024 * 1024)); // 1MB min
        desc.usage = BufferUsage::Vertex;
        desc.hostVisible = true;

        BufferHandle handle = createBuffer(desc);
        if (handle != InvalidBuffer) {
            frame.vertexBuffer = buffers_[handle];
            buffers_.erase(handle);
        }
    }

    // Ensure we have index buffer
    if (frame.indexBuffer.buffer == nullptr || frame.indexBuffer.size < indexSize) {
        if (frame.indexBuffer.buffer) {
            vkDestroyBuffer(device_, frame.indexBuffer.buffer, nullptr);
            vkFreeMemory(device_, frame.indexBuffer.memory, nullptr);
        }

        BufferDesc desc;
        desc.size = std::max(indexSize, static_cast<size_t>(512 * 1024)); // 512KB min
        desc.usage = BufferUsage::Index;
        desc.hostVisible = true;

        BufferHandle handle = createBuffer(desc);
        if (handle != InvalidBuffer) {
            frame.indexBuffer = buffers_[handle];
            buffers_.erase(handle);
        }
    }

    // Upload vertex data
    if (frame.vertexBuffer.buffer) {
        void* mapped;
        vkMapMemory(device_, frame.vertexBuffer.memory, 0, vertexSize, 0, &mapped);
        memcpy(mapped, vertices.data(), vertexSize);
        vkUnmapMemory(device_, frame.vertexBuffer.memory);
    }

    // Upload index data
    if (frame.indexBuffer.buffer && !indices.empty()) {
        void* mapped;
        vkMapMemory(device_, frame.indexBuffer.memory, 0, indexSize, 0, &mapped);
        memcpy(mapped, indices.data(), indexSize);
        vkUnmapMemory(device_, frame.indexBuffer.memory);
    }

    // Bind vertex buffer
    VkBuffer vertexBuffers[] = {frame.vertexBuffer.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(frame.commandBuffer, 0, 1, vertexBuffers, offsets);

    // Bind index buffer
    if (frame.indexBuffer.buffer) {
        vkCmdBindIndexBuffer(frame.commandBuffer, frame.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    // Set viewport and scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchainWidth_);
    viewport.height = static_cast<float>(swapchainHeight_);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(frame.commandBuffer, 0, 1, &viewport);

    // Process commands
    Rect currentClip(0, 0, static_cast<float>(swapchainWidth_), static_cast<float>(swapchainHeight_));

    for (const auto& cmd : commands) {
        switch (cmd.type) {
        case DrawCommandType::SetClipRect:
            currentClip = cmd.clipRect;
            break;

        case DrawCommandType::SetTexture:
            // Bind texture descriptor set
            break;

        case DrawCommandType::DrawTriangles:
            if (cmd.indexCount > 0) {
                // Set scissor
                VkRect2D scissor{};
                scissor.offset.x = static_cast<int32_t>(std::max(0.0f, currentClip.x));
                scissor.offset.y = static_cast<int32_t>(std::max(0.0f, currentClip.y));
                scissor.extent.width = static_cast<uint32_t>(currentClip.width);
                scissor.extent.height = static_cast<uint32_t>(currentClip.height);
                vkCmdSetScissor(frame.commandBuffer, 0, 1, &scissor);

                // Bind pipeline (would switch based on texture/text)
                if (uiPipeline_) {
                    vkCmdBindPipeline(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, uiPipeline_);
                }

                // Draw
                vkCmdDrawIndexed(frame.commandBuffer, cmd.indexCount, 1, cmd.indexOffset, static_cast<int32_t>(cmd.vertexOffset), 0);
            }
            break;

        default:
            break;
        }
    }
}

void VulkanBackend::bindPipeline(bool textured) {
    FrameResources& frame = frameResources_[currentFrame_];

    VkPipeline pipeline = textured ? textPipeline_ : uiPipeline_;
    if (pipeline) {
        vkCmdBindPipeline(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }
}

void VulkanBackend::updateUniformBuffer() {
    UIUniforms uniforms{};

    // Orthographic projection matrix
    float L = 0.0f;
    float R = static_cast<float>(swapchainWidth_);
    float T = 0.0f;
    float B = static_cast<float>(swapchainHeight_);

    // Column-major order
    uniforms.projectionMatrix[0] = 2.0f / (R - L);
    uniforms.projectionMatrix[1] = 0.0f;
    uniforms.projectionMatrix[2] = 0.0f;
    uniforms.projectionMatrix[3] = 0.0f;

    uniforms.projectionMatrix[4] = 0.0f;
    uniforms.projectionMatrix[5] = 2.0f / (T - B);
    uniforms.projectionMatrix[6] = 0.0f;
    uniforms.projectionMatrix[7] = 0.0f;

    uniforms.projectionMatrix[8] = 0.0f;
    uniforms.projectionMatrix[9] = 0.0f;
    uniforms.projectionMatrix[10] = -1.0f;
    uniforms.projectionMatrix[11] = 0.0f;

    uniforms.projectionMatrix[12] = (R + L) / (L - R);
    uniforms.projectionMatrix[13] = (T + B) / (B - T);
    uniforms.projectionMatrix[14] = 0.0f;
    uniforms.projectionMatrix[15] = 1.0f;

    // Update uniform buffer (would be per-frame)
}

VkShaderModule VulkanBackend::createShaderModule(const uint32_t* code, size_t size) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = code;

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device_, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}

} // namespace dakt::gui

#endif // DAKT_ENABLE_VULKAN

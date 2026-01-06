#include "dakt/gui/backend/vulkan/VulkanBackend.hpp"

#if defined(DAKTLIB_ENABLE_VULKAN)

#include "dakt/gui/subsystems/draw/DrawList.hpp"
#include <cstring>

#if defined(DAKTLIB_PLATFORM_WINDOWS) || defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(DAKTLIB_PLATFORM_LINUX) || defined(__linux__)
#define VK_USE_PLATFORM_XCB_KHR
#elif defined(DAKTLIB_PLATFORM_MACOS) || defined(__APPLE__)
#define VK_USE_PLATFORM_METAL_EXT
#endif

#include <vulkan/vulkan.h>

namespace dakt::gui {

// =============================================================================
// Uniform Buffer Data
// =============================================================================

struct UIUniforms {
    float projectionMatrix[16];
};

// =============================================================================
// Frame Management
// =============================================================================

bool VulkanBackend::beginFrame() {
    if (!initialized_) {
        return false;
    }

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

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            return false;
        }
    }

    vkResetFences(device_, 1, &frame.inFlightFence);
    vkResetCommandBuffer(frame.commandBuffer, 0);

    // Reset dynamic buffer offsets
    frame.vertexBufferOffset = 0;
    frame.indexBufferOffset = 0;
    frame.uniformBufferOffset = 0;

    // Begin command buffer recording
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(frame.commandBuffer, &beginInfo) != VK_SUCCESS) {
        return false;
    }

    // Begin render pass
    if (!framebuffers_.empty()) {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass_;
        renderPassInfo.framebuffer = framebuffers_[imageIndex_];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = {swapchainWidth_, swapchainHeight_};

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(frame.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Set viewport and scissor
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapchainWidth_);
        viewport.height = static_cast<float>(swapchainHeight_);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(frame.commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {swapchainWidth_, swapchainHeight_};
        vkCmdSetScissor(frame.commandBuffer, 0, 1, &scissor);
    }

    frameInProgress_ = true;
    return true;
}

void VulkanBackend::endFrame() {
    if (!frameInProgress_) {
        return;
    }

    FrameResources& frame = frameResources_[currentFrame_];

    // End render pass
    if (!framebuffers_.empty()) {
        vkCmdEndRenderPass(frame.commandBuffer);
    }

    // End command buffer recording
    if (vkEndCommandBuffer(frame.commandBuffer) != VK_SUCCESS) {
        frameInProgress_ = false;
        return;
    }

    // Submit command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {frame.imageAvailable};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    if (swapchain_) {
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
    }

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frame.commandBuffer;

    VkSemaphore signalSemaphores[] = {frame.renderFinished};
    if (swapchain_) {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
    }

    vkQueueSubmit(graphicsQueue_, 1, &submitInfo, frame.inFlightFence);

    frameInProgress_ = false;
}

void VulkanBackend::present() {
    if (!swapchain_) {
        return;
    }

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

// =============================================================================
// Draw Submission
// =============================================================================

void VulkanBackend::submit(const DrawList& drawList) {
    if (!frameInProgress_) {
        return;
    }

    recordCommandBuffer(drawList);
}

void VulkanBackend::recordCommandBuffer(const DrawList& drawList) {
    FrameResources& frame = frameResources_[currentFrame_];

    // Update uniform buffer
    updateUniformBuffer();

    // Get draw commands from draw list
    const auto& commands = drawList.getCommands();

    for (const auto& cmd : commands) {
        // Bind pipeline based on command type
        bindPipeline(cmd.textureID != InvalidTexture);

        // Set scissor rect if clipping
        if (cmd.clipRect.width > 0 && cmd.clipRect.height > 0) {
            VkRect2D scissor{};
            scissor.offset.x = static_cast<int32_t>(cmd.clipRect.x);
            scissor.offset.y = static_cast<int32_t>(cmd.clipRect.y);
            scissor.extent.width = static_cast<uint32_t>(cmd.clipRect.width);
            scissor.extent.height = static_cast<uint32_t>(cmd.clipRect.height);
            vkCmdSetScissor(frame.commandBuffer, 0, 1, &scissor);
        }

        // Draw
        vkCmdDrawIndexed(frame.commandBuffer, cmd.indexCount, 1, cmd.indexOffset, static_cast<int32_t>(cmd.vertexOffset), 0);
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

    // Orthographic projection matrix (column-major)
    float L = 0.0f;
    float R = static_cast<float>(swapchainWidth_);
    float T = 0.0f;
    float B = static_cast<float>(swapchainHeight_);

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

    // Would update per-frame uniform buffer here
}

// =============================================================================
// Shader Module Creation
// =============================================================================

VkShaderModule VulkanBackend::createShaderModule(const uint32_t* code, size_t size) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = code;

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    if (vkCreateShaderModule(device_, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}

} // namespace dakt::gui

#endif // DAKTLIB_ENABLE_VULKAN
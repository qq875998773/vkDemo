#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <sstream>
#include <iomanip>

#include <vulkan\vulkan.h>
#include "vkBuffer.h"
#include "vkDevice.h"
#include "vkInitializers.hpp"

#include "imgui/imgui.h"

namespace Engine
{
    struct UIOverlayCreateInfo
    {
        Engine::VulkanDevice* device;
        VkQueue copyQueue;
        VkRenderPass renderPass;
        std::vector<VkFramebuffer> framebuffers;
        VkFormat colorformat;
        VkFormat depthformat;
        uint32_t width;
        uint32_t height;
        std::vector<VkPipelineShaderStageCreateInfo> shaders;
        VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        uint32_t subpassCount = 1;
        std::vector<VkClearValue> clearValues = {};
        uint32_t attachmentCount = 1;
    };

    class UIOverlay
    {
    private:
        Engine::VulkanBuffer vertexBuffer;
        Engine::VulkanBuffer indexBuffer;
        int32_t vertexCount = 0;
        int32_t indexCount = 0;

        VkDescriptorPool descriptorPool;
        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorSet descriptorSet;
        VkPipelineLayout pipelineLayout;
        VkPipelineCache pipelineCache;
        VkPipeline pipeline;
        VkRenderPass renderPass;
        VkCommandPool commandPool;
        VkFence fence;

        VkDeviceMemory fontMemory = VK_NULL_HANDLE;
        VkImage fontImage = VK_NULL_HANDLE;
        VkImageView fontView = VK_NULL_HANDLE;
        VkSampler sampler;

        struct PushConstBlock {
            glm::vec2 scale;
            glm::vec2 translate;
        } pushConstBlock;

        UIOverlayCreateInfo createInfo = {};

        void prepareResources();
        void preparePipeline();
        void prepareRenderPass();
        void updateCommandBuffers();
    public:
        bool visible = true;
        float scale = 1.0f;

        std::vector<VkCommandBuffer> cmdBuffers;

        UIOverlay(Engine::UIOverlayCreateInfo createInfo);
        ~UIOverlay();

        void update();
        void resize(uint32_t width, uint32_t height, std::vector<VkFramebuffer> framebuffers);

        void submit(VkQueue queue, uint32_t bufferindex, VkSubmitInfo submitInfo);

        bool header(const char* caption);
        bool checkBox(const char* caption, bool* value);
        bool checkBox(const char* caption, int32_t* value);
        bool inputFloat(const char* caption, float* value, float step, uint32_t precision);
        bool sliderFloat(const char* caption, float* value, float min, float max);
        bool sliderInt(const char* caption, int32_t* value, int32_t min, int32_t max);
        bool comboBox(const char* caption, int32_t* itemindex, std::vector<std::string> items);
        bool button(const char* caption);
        void text(const char* formatstr, ...);


        // Put an image memory barrier for setting an image layout on the sub resource into the given command buffer
        void setImageLayout(
            VkCommandBuffer cmdbuffer,
            VkImage image,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkImageSubresourceRange subresourceRange,
            VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        // Uses a fixed sub resource layout with first mip level and layer
        void setImageLayout(
            VkCommandBuffer cmdbuffer,
            VkImage image,
            VkImageAspectFlags aspectMask,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        /**
       * Create a buffer on the device
       *
       * @param usageFlags Usage flag bitmask for the buffer (i.e. index, vertex, uniform buffer)
       * @param memoryPropertyFlags Memory properties for this buffer (i.e. device local, host visible, coherent)
       * @param buffer Pointer to a vk::Vulkan buffer object
       * @param size Size of the buffer in byes
       * @param data Pointer to the data that should be copied to the buffer after creation (optional, if not set, no data is copied over)
       *
       * @return VK_SUCCESS if buffer handle and memory have been created and (optionally passed) data has been copied
       */
        VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VulkanBuffer* buffer, VkDeviceSize size/*, void* data = nullptr*/);

        /**
        * Allocate a command buffer from the command pool
        *
        * @param level Level of the new command buffer (primary or secondary)
        * @param (Optional) begin If true, recording on the new command buffer will be started (vkBeginCommandBuffer) (Defaults to false)
        *
        * @return A handle to the allocated command buffer
        */
        VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false);

        /**
        * Finish command buffer recording and submit it to a queue
        *
        * @param commandBuffer Command buffer to flush
        * @param queue Queue to submit the command buffer to
        * @param free (Optional) Free the command buffer once it has been submitted (Defaults to true)
        *
        * @note The queue that the command buffer is submitted to must be from the same family index as the pool it was allocated from
        * @note Uses a fence to ensure command buffer has finished executing
        */
        void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
    };
}
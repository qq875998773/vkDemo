#pragma once

#include <vulkan\vulkan.h>
#include <vector>
#include <array>

#include "vkUtils.hpp"
#include "vkDevice.h"

namespace Engine
{
    class VulkanBuffer
    {
    public:
        VkBuffer buffer;
        VkDeviceSize size;

        VulkanBuffer();
        ~VulkanBuffer();

        /*
         * 创建两个VkBuffers.一个是位于CPU内存上的传输缓冲区,另一个是位于GPU内存上的存储缓冲区.
         * 与update()和transferToDevice()一起使用.
         */
        void create(VulkanDevice* device, VkBufferUsageFlags usage_flags, VkDeviceSize size);

        void shutDown();

        /*
         * Helper函数,可以在单个步骤中执行更新和传输
         */
        void updateAndTransfer(void* data);

        /*
         * 更新缓冲区数据
         */
        void update(void* data);

        /*
         * 将分配在CPU内存上的缓冲区复制到分配在GPU内存上的缓冲
         */
        void transferToDevice();

    private:
        VulkanDevice* m_device;
        VkBuffer m_staging_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_staging_memory = VK_NULL_HANDLE;
        VkDeviceMemory m_buffer_memory;
        VkBufferUsageFlags m_usage_flags;

        /*
         * 使用给定的参数为数据缓冲区分配内存
         */
        void allocateMemory(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory);
    };
}
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
        VkDeviceMemory m_staging_memory = VK_NULL_HANDLE;
        VkDeviceSize alignment = 0;
        VkDescriptorBufferInfo descriptor;
        VkBufferUsageFlags m_usage_flags;
        /** @brief Memory propertys flags to be filled by external source at buffer creation (to query at some later point) */
        VkMemoryPropertyFlags memoryPropertyFlags;
        void* mapped = nullptr;

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

        /**
        * Release all Vulkan resources held by this buffer
        */
        void destroy();
        /**
        * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
        *
        * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete buffer range.
        * @param offset (Optional) Byte offset from beginning
        *
        * @return VkResult of the buffer mapping call
        */
        VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        /**
        * Unmap a mapped memory range
        *
        * @note Does not return a result as vkUnmapMemory can't fail
        */
        void unmap();
        /**
        * Flush a memory range of the buffer to make it visible to the device
        *
        * @note Only required for non-coherent memory
        *
        * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the complete buffer range.
        * @param offset (Optional) Byte offset from beginning
        *
        * @return VkResult of the flush call
        */
        VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        /**
        * Setup the default descriptor for this buffer
        *
        * @param size (Optional) Size of the memory range of the descriptor
        * @param offset (Optional) Byte offset from beginning
        *
        */
        void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        /**
        * Attach the allocated memory block to the buffer
        *
        * @param offset (Optional) Byte offset (from the beginning) for the memory region to bind
        *
        * @return VkResult of the bindBufferMemory call
        */
        VkResult bind(VkDeviceSize offset = 0);

    private:
        VkBuffer m_staging_buffer = VK_NULL_HANDLE;
        
        VkDeviceMemory m_buffer_memory;
        
        VulkanDevice* m_device;

        /*
         * 使用给定的参数为数据缓冲区分配内存
         */
        void allocateMemory(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory);
    };
}
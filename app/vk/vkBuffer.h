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
         * ��������VkBuffers.һ����λ��CPU�ڴ��ϵĴ��仺����,��һ����λ��GPU�ڴ��ϵĴ洢������.
         * ��update()��transferToDevice()һ��ʹ��.
         */
        void create(VulkanDevice* device, VkBufferUsageFlags usage_flags, VkDeviceSize size);

        void shutDown();

        /*
         * Helper����,�����ڵ���������ִ�и��ºʹ���
         */
        void updateAndTransfer(void* data);

        /*
         * ���»���������
         */
        void update(void* data);

        /*
         * ��������CPU�ڴ��ϵĻ��������Ƶ�������GPU�ڴ��ϵĻ���
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
         * ʹ�ø����Ĳ���Ϊ���ݻ����������ڴ�
         */
        void allocateMemory(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory);
    };
}
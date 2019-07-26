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

    private:
        VulkanDevice* m_device;
        VkBuffer m_staging_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_staging_memory = VK_NULL_HANDLE;
        VkDeviceMemory m_buffer_memory;
        VkBufferUsageFlags m_usage_flags;

        /*
         * ʹ�ø����Ĳ���Ϊ���ݻ����������ڴ�
         */
        void allocateMemory(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory);
    };
}
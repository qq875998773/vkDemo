
#include "vkBuffer.h"

namespace Engine
{
    VulkanBuffer::VulkanBuffer()
    {
    }

    VulkanBuffer::~VulkanBuffer()
    {
    }

    // 创建buffer
    void VulkanBuffer::create(VulkanDevice* device, VkBufferUsageFlags usage_flags, VkDeviceSize size)
    {
        VV_ASSERT(device != VK_NULL_HANDLE, "VulkanDevice not present");
        m_device = device;
        m_usage_flags = usage_flags;
        this->size = size;

        // 在CPU上创建临时传输缓冲区
        allocateMemory(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_staging_buffer, m_staging_memory);

        // 为GPU创建存储缓冲区
        allocateMemory(size, usage_flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, m_buffer_memory);
    }
    
    // 销毁资源
    void VulkanBuffer::shutDown()
    {
        if (m_staging_buffer)
            vkDestroyBuffer(m_device->logical_device, m_staging_buffer, nullptr);
        if (m_staging_memory)
            vkFreeMemory(m_device->logical_device, m_staging_memory, nullptr);

        vkDestroyBuffer(m_device->logical_device, buffer, nullptr);
        vkFreeMemory(m_device->logical_device, m_buffer_memory, nullptr);
    }

    // 更新和转移
    void VulkanBuffer::updateAndTransfer(void* data)
    {
        update(data);
        transferToDevice();
    }

    // 更新buffer
    void VulkanBuffer::update(void* data)
    {
        // 将原始数据转移到暂存的Vulkan缓冲区
        void* mapped_data;
        vkMapMemory(m_device->logical_device, m_staging_memory, 0, size, 0, &mapped_data);
        memcpy(mapped_data, data, size);
        vkUnmapMemory(m_device->logical_device, m_staging_memory);
    }

    // 转移设备
    void VulkanBuffer::transferToDevice()
    {
        VV_ASSERT(m_staging_buffer && buffer, "Buffers not allocated correctly. Perhaps create() wasn't called.");

        bool use_transfer = false;
        auto command_pool_used = m_device->command_pools["graphics"];

        // 如果可用,使用传输队列
        if (m_device->command_pools.count("transfer") > 0)
        {
            command_pool_used = m_device->command_pools["transfer"];
            use_transfer = true;
        }

        auto command_buffer = util::beginSingleUseCommand(m_device->logical_device, command_pool_used);

        VkBufferCopy buffer_copy = {};
        buffer_copy.size = size;
        vkCmdCopyBuffer(command_buffer, m_staging_buffer, buffer, 1, &buffer_copy);

        if (use_transfer)
            util::endSingleUseCommand(m_device->logical_device, command_pool_used, command_buffer, m_device->transfer_queue);
        else
            util::endSingleUseCommand(m_device->logical_device, command_pool_used, command_buffer, m_device->graphics_queue);
    }

    // 分配内存
    void VulkanBuffer::allocateMemory(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory)
    {
        // 创建顶点缓冲区初始化
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.flags = 0; // 可以用来指定这个存储稀疏数据
        buffer_create_info.size = size;
        buffer_create_info.usage = usage; // 作为顶点/索引缓冲区

        // 如果可用，使用传输队列
        if (m_device->transfer_family_index != -1)
        {
            buffer_create_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
            buffer_create_info.queueFamilyIndexCount = 2;
            std::array<uint32_t, 2> queue_family_indices = {
                static_cast<uint32_t>(m_device->graphics_family_index),
                static_cast<uint32_t>(m_device->transfer_family_index)
            };
            buffer_create_info.pQueueFamilyIndices = queue_family_indices.data();
        }
        else
        {
            buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            buffer_create_info.queueFamilyIndexCount = 1;
            uint32_t queue_index = static_cast<uint32_t>(m_device->graphics_family_index);
            buffer_create_info.pQueueFamilyIndices = &queue_index;
        }

        VV_CHECK_SUCCESS(vkCreateBuffer(m_device->logical_device, &buffer_create_info, nullptr, &buffer));

        // 如果可用，请使用传输队列来确定内存需求(它在何处分配，内存类型等)
        VkMemoryRequirements memory_requirements = {};
        vkGetBufferMemoryRequirements(m_device->logical_device, buffer, &memory_requirements);
        auto memory_type = m_device->findMemoryTypeIndex(memory_requirements.memoryTypeBits, memory_properties);

        // 分配和绑定缓冲区内存
        VkMemoryAllocateInfo memory_allocate_info = {};
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = memory_type;

        VV_CHECK_SUCCESS(vkAllocateMemory(m_device->logical_device, &memory_allocate_info, nullptr, &buffer_memory));
        vkBindBufferMemory(m_device->logical_device, buffer, buffer_memory, 0);
    }

    /**
    * Release all Vulkan resources held by this buffer
    */
    void VulkanBuffer::destroy()
    {
        if (buffer)
        {
            vkDestroyBuffer(m_device->logical_device, buffer, nullptr);
        }
        if (m_staging_memory)
        {
            vkFreeMemory(m_device->logical_device, m_staging_memory, nullptr);
        }
    }

    /**
    * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
    *
    * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete buffer range.
    * @param offset (Optional) Byte offset from beginning
    *
    * @return VkResult of the buffer mapping call
    */
    VkResult VulkanBuffer::map(VkDeviceSize size, VkDeviceSize offset)
    {
        return vkMapMemory(m_device->logical_device, m_staging_memory, offset, size, 0, &mapped);
    }

    /**
    * Unmap a mapped memory range
    *
    * @note Does not return a result as vkUnmapMemory can't fail
    */
    void VulkanBuffer::unmap()
    {
        if (mapped)
        {
            vkUnmapMemory(m_device->logical_device, m_staging_memory);
            mapped = nullptr;
        }
    }

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
    VkResult VulkanBuffer::flush(VkDeviceSize size, VkDeviceSize offset)
    {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = m_staging_memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(m_device->logical_device, 1, &mappedRange);
    }

    /**
    * Setup the default descriptor for this buffer
    *
    * @param size (Optional) Size of the memory range of the descriptor
    * @param offset (Optional) Byte offset from beginning
    *
    */
    void VulkanBuffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset)
    {
        descriptor.offset = offset;
        descriptor.buffer = buffer;
        descriptor.range = size;
    }

    /**
    * Attach the allocated memory block to the buffer
    *
    * @param offset (Optional) Byte offset (from the beginning) for the memory region to bind
    *
    * @return VkResult of the bindBufferMemory call
    */
    VkResult VulkanBuffer::bind(VkDeviceSize offset)
    {
        return vkBindBufferMemory(m_device->logical_device, buffer, m_staging_memory, offset);
    }
}
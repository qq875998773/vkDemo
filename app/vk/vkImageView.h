#pragma once

#include "vkDevice.h"
#include "vkImage.h"

namespace vv
{
    class VulkanImageView
    {
    public:
        VkImageView image_view;
        VkImageViewType type;

        VulkanImageView();
        ~VulkanImageView();

        /*
         * Creates an image view for the application to interact with.
         *
         * note: This class does not maintain ownership over VulkanImages.
         *       They must be manually deleted outside of this class.
         */
        void create(VulkanDevice* device, VulkanImage* image, VkImageViewType image_view_type, uint32_t base_mip_level);

        /*
         *
         */
        void shutDown();

    private:
        VulkanDevice* m_device;
        VulkanImage* m_image;

    };
}
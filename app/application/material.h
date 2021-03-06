#pragma once

#include <vector>
#include <utility>

#include "vkDevice.h"
#include "vkBuffer.h"
#include "vkImageView.h"
#include "vkPipeline.h"
#include "vkShaderModule.h"
#include "textureManager.h"

namespace Engine
{
    struct MaterialTemplate
    {
        std::string name;
        VkPipelineLayout pipeline_layout;
        VulkanPipeline* pipeline;
        VkDescriptorSetLayout material_descriptor_set_layout;
        bool uses_environment_lighting;
        std::vector<VulkanShaderModule> shader_modules;
    };

    struct UBOStore
    {
        VkDescriptorBufferInfo info;
        VulkanBuffer* buffer;
    };

    struct TextureStore
    {
        VkDescriptorImageInfo info;
        SampledTexture* texture;
    };

    class Material
    {
    public:
        MaterialTemplate* material_template;

        Material();
        ~Material();

        /*
         * This class serves as an instance of a specified MaterialTemplate.
         * It holds the actual data relevant to the creation of a material such as VkDescriptorSets.
         *
         * note: this should only be called from within MaterialTemplate, which manages all such material instances.
         */
        void create(VulkanDevice* device, MaterialTemplate* material_template, VkDescriptorPool descriptor_pool);

        /*
         *
         */
        void shutDown();

        /*
         * Instructs this instance to support a uniform buffer binding and maintains ownership over the data.
         */
        void addUniformBuffer(VulkanBuffer* uniform_buffer, int binding);

        /*
         * Instructs this instance to support a texture binding and maintains ownership over the data.
         */
        void addTexture(SampledTexture* texture, int binding);

        /*
         * Updates the contents of the descriptor set with the uniform + samplers provided via addUniformBuffer and addTexture.
         */
        void updateDescriptorSets() const;

        /*
         * Binds all descriptor sets this instance has ownership over. Should be called at render time.
         */
        void bindDescriptorSets(VkCommandBuffer command_buffer, VkPipelineBindPoint pipeline_bind_point) const;

    private:
        VulkanDevice* m_device;
        std::vector<VkWriteDescriptorSet> m_write_sets;
        VkDescriptorSet m_descriptor_set;

        std::vector<UBOStore*> m_uniform_buffers;
        std::vector<TextureStore*> m_textures;

    };
}
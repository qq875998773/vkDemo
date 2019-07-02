#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "vkDevice.h"
#include "vkRenderPass.h"
#include "vkSampler.h"

#include "skyBox.h"
#include "modelManager.h"
#include "textureManager.h"
#include "model.h"
#include "light.h"
#include "model.h"
#include "camera.h"

namespace Engine
{
    class Scene
    {
        friend class DeferredRenderer;

    public:
        // ����ģ�弯
        std::unordered_map<std::string, MaterialTemplate> material_templates;

        Scene() = default;
        ~Scene() = default;

        // ��������
        void create(VulkanDevice* device, VulkanRenderPass* render_pass);

        // �رճ���
        void shutDown();

        // ��ӹ�Դ
        Light* addLight(glm::vec4 irradiance, float radius);

        // ���ģ��
        Model* addModel(std::string path, std::string name, std::string material_template);

        // ��������
        Camera* addCamera(float fov_y, float near_plane, float far_plane);

        // �����պ�
        SkyBox* addSkyBox(std::string path, std::string radiance_map_name, std::string diffuse_map_name,
                          std::string specular_map_name, std::string brdf_lut_name);

        // ��ȡ��������
        Camera* getActiveCamera() const;

        // ��ȡ�����պ�
        SkyBox* getActiveSkyBox() const;

        // ���û�������
        void setActiveCamera(Camera* camera);

        // ���û����պ�
        void setActiveSkyBox(SkyBox* skybox);

        // ����ȫ�ֳ���������������
        void updateUniformData(VkExtent2D extent, float time);

        // ��Ⱦ
        // ע�⣺����һ����Ⱦ�������Զ�����.����Ҫ�ֶ�
        void render(VkCommandBuffer command_buffer);

    private:
        VulkanDevice* m_device = nullptr;                    // �豸
        VulkanRenderPass* m_render_pass = nullptr;           // ��Ⱦͨ��
        ModelManager* m_model_manager = nullptr;             // ģ�͹���
        TextureManager* m_texture_manager = nullptr;         // �������
        bool m_initialized = false;                          // �Ƿ��ѳ�ʼ��

        VulkanSampler* m_sampler = nullptr;                  // ������
        VkDescriptorPool m_descriptor_pool = VK_NULL_HANDLE; // ��������

        // ����uniform
        struct SceneUBO
        {
            glm::mat4 view_mat;
            glm::mat4 projection_mat;
            glm::vec4 camera_position;
        };

        VkDescriptorSetLayout m_scene_descriptor_set_layout;
        std::vector<VkDescriptorSet> m_scene_descriptor_sets;
        SceneUBO m_scene_ubo;
        VulkanBuffer* m_scene_uniform_buffer = nullptr;

        // ��Դdata
        struct LightData
        {
            glm::vec4 position;
            glm::vec4 irradiance;
        };

        // ��Դuniforms
        struct LightUBO
        {
            LightData lights[VV_MAX_LIGHTS];
        };

        LightUBO m_lights_ubo;
        VulkanBuffer* m_lights_uniform_buffer = nullptr;

        VkDescriptorSetLayout m_environment_descriptor_set_layout;
        VkDescriptorSet m_environment_descriptor_set = VK_NULL_HANDLE; // used for IBL calculations

        VkDescriptorSetLayout m_radiance_descriptor_set_layout;
        VkDescriptorSet m_radiance_descriptor_set = VK_NULL_HANDLE; // applied to skybox model

        // todo: think of better data structure. maybe something to help with culling
        std::vector<Light> m_lights;   // ��Դ��
        std::vector<Model> m_models;   // ģ�ͼ�
        std::vector<Camera> m_cameras; // �������
        std::vector<SkyBox> m_skyboxes;// ��պм�

        Camera* m_active_camera;         // ��������
        SkyBox* m_active_skybox;         // �����պ�
        bool m_has_active_camera = false;// �Ƿ��������
        bool m_has_active_skybox = false;// �Ƿ�����պ�

        // Reads required shaders from file and creates all possible MaterialTemplates that can be used during execution.
        // These MaterialTemplates can be referenced by the name provided in the shader info file.
        // ����ģ�弯
        void createMaterialTemplates();

        /*
         * Creates global descriptor pool from which all descriptor sets will be allocated from.
         */
        // ��������
        void createDescriptorPool();

        // �������������� ���в���+��Դ����+�������ʼ��
        void createSceneDescriptorSetLayout();

        /*
         * This dynamically allocates a number of scene related descriptor sets depending on the number of
         * models specified through the scene interface.
         */
        // ���䳡������������
        void allocateSceneDescriptorSets();

        /*
         * Creates everything necessary for scene global uniforms.
         */
        // ����ȫ������������
        void createEnvironmentUniforms();

        // ��������������
        void createVulkanDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding> bindings, VkDescriptorSetLayout& layout);

        // �������������ð�
        VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptor_type, uint32_t count, VkShaderStageFlags shader_stage) const;
    };
}
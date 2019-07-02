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
        // 材质模板集
        std::unordered_map<std::string, MaterialTemplate> material_templates;

        Scene() = default;
        ~Scene() = default;

        // 创建场景
        void create(VulkanDevice* device, VulkanRenderPass* render_pass);

        // 关闭场景
        void shutDown();

        // 添加光源
        Light* addLight(glm::vec4 irradiance, float radius);

        // 添加模型
        Model* addModel(std::string path, std::string name, std::string material_template);

        // 添加摄像机
        Camera* addCamera(float fov_y, float near_plane, float far_plane);

        // 添加天空盒
        SkyBox* addSkyBox(std::string path, std::string radiance_map_name, std::string diffuse_map_name,
                          std::string specular_map_name, std::string brdf_lut_name);

        // 获取活动的摄像机
        Camera* getActiveCamera() const;

        // 获取活动的天空盒
        SkyBox* getActiveSkyBox() const;

        // 设置活动的摄像机
        void setActiveCamera(Camera* camera);

        // 设置活动的天空盒
        void setActiveSkyBox(SkyBox* skybox);

        // 更新全局场景描述符集数据
        void updateUniformData(VkExtent2D extent, float time);

        // 渲染
        // 注意：将在一个渲染器类中自动调用.不需要手动
        void render(VkCommandBuffer command_buffer);

    private:
        VulkanDevice* m_device = nullptr;                    // 设备
        VulkanRenderPass* m_render_pass = nullptr;           // 渲染通道
        ModelManager* m_model_manager = nullptr;             // 模型管理
        TextureManager* m_texture_manager = nullptr;         // 纹理管理
        bool m_initialized = false;                          // 是否已初始化

        VulkanSampler* m_sampler = nullptr;                  // 采样器
        VkDescriptorPool m_descriptor_pool = VK_NULL_HANDLE; // 描述符池

        // 场景uniform
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

        // 光源data
        struct LightData
        {
            glm::vec4 position;
            glm::vec4 irradiance;
        };

        // 光源uniforms
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
        std::vector<Light> m_lights;   // 光源集
        std::vector<Model> m_models;   // 模型集
        std::vector<Camera> m_cameras; // 摄像机集
        std::vector<SkyBox> m_skyboxes;// 天空盒集

        Camera* m_active_camera;         // 活动的摄像机
        SkyBox* m_active_skybox;         // 活动的天空盒
        bool m_has_active_camera = false;// 是否有摄像机
        bool m_has_active_skybox = false;// 是否有天空盒

        // Reads required shaders from file and creates all possible MaterialTemplates that can be used during execution.
        // These MaterialTemplates can be referenced by the name provided in the shader info file.
        // 材质模板集
        void createMaterialTemplates();

        /*
         * Creates global descriptor pool from which all descriptor sets will be allocated from.
         */
        // 描述符池
        void createDescriptorPool();

        // 场景描述符布局 所有材质+光源数据+摄像机初始化
        void createSceneDescriptorSetLayout();

        /*
         * This dynamically allocates a number of scene related descriptor sets depending on the number of
         * models specified through the scene interface.
         */
        // 分配场景描述符设置
        void allocateSceneDescriptorSets();

        /*
         * Creates everything necessary for scene global uniforms.
         */
        // 场景全局描述符分配
        void createEnvironmentUniforms();

        // 描述符布局设置
        void createVulkanDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding> bindings, VkDescriptorSetLayout& layout);

        // 描述符布局设置绑定
        VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptor_type, uint32_t count, VkShaderStageFlags shader_stage) const;
    };
}
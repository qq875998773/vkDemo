#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "deferredRenderer.h"
#include "glfwWindow.h"
#include "scene.h"

namespace vv
{
    class Application
    {
    public:
        Application() = default;
        ~Application() = default;

        // 初始化所有部件
        void create(int argc, char** argv);

        // 清除分配的资源
        // 必须在执行结束时调用,以确保成功清除所有分配的资源
        void shutDown();

        /*
         * Returns the main scene which manages all entities with physical presence.
         * Entities that should be rendered, cameras, and lights can be added through this central object.
         */
        Scene* getScene() const;

        /*
         * Signals the Vulkan engine to construct render commands and begins all central processing.
         */
        void beginMainLoop();

    private:
        int m_argc;
        char** m_argv;

        const uint32_t m_window_width = 1080;
        const uint32_t m_window_height = 720;
        const char* m_application_name = "VkDemo";

        float m_move_speed = 4.0f;

#ifdef _DEBUG
        float m_rotate_speed = 0.2f;
#else
        float m_rotate_speed = 2.0f;
#endif

        GLFWWindow m_window;
        DeferredRenderer* m_renderer;
        Scene* m_scene;

        void handleInput(float delta_time);
        void mouseInput(float delta_time);
    };
}
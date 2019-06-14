#ifndef VIRTUALVISTA_GLFWWINDOW_H
#define VIRTUALVISTA_GLFWWINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <unordered_map>
#include <vector>

#include "inputManager.h"
#include "vkUtils.hpp"

namespace vv
{
    struct VulkanDevice;

    class GLFWWindow
    {
        // 友元类 使当前类可以访问inputManager的私有成员
        friend class InputManager;

    public:
        GLFWwindow* window;
        VkSurfaceKHR surface;
        std::unordered_map<VulkanDevice*, VulkanSurfaceDetailsHandle> surface_settings;
        uint32_t glfw_extension_count;
        const char** glfw_extensions;
        uint32_t window_width;
        uint32_t window_height;
        char* application_name;

        GLFWWindow() = default;
        ~GLFWWindow() = default;

         // todo: add resizing event handler. requires manually updating framebuffer
        // 初始化创建窗体
        void create(const int width, const int height, const char* application_name);

        // 创建vk API和窗体的连接
        void createSurface(VkInstance instance);

        // 销毁所有vk和glfw容器
        void shutDown(VkInstance instance);
        
        // 运行
        void run();

        // 当窗口需要关闭时通知glfw
        void setShouldClose(bool should_close);

        // 返回glfw是否接收到终止信号
        bool shouldClose();

    private:
         // 键盘按键
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

         // 鼠标位置
        static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

         // 鼠标按键
        static void mousebuttonCallback(GLFWwindow* window, int button, int action, int mods);

         // 鼠标滚轮
        static void scrollCallback(GLFWwindow* window, double x, double y);
    };
}
#endif
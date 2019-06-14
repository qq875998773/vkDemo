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
        // ��Ԫ�� ʹ��ǰ����Է���inputManager��˽�г�Ա
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
        // ��ʼ����������
        void create(const int width, const int height, const char* application_name);

        // ����vk API�ʹ��������
        void createSurface(VkInstance instance);

        // ��������vk��glfw����
        void shutDown(VkInstance instance);
        
        // ����
        void run();

        // ��������Ҫ�ر�ʱ֪ͨglfw
        void setShouldClose(bool should_close);

        // ����glfw�Ƿ���յ���ֹ�ź�
        bool shouldClose();

    private:
         // ���̰���
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

         // ���λ��
        static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

         // ��갴��
        static void mousebuttonCallback(GLFWwindow* window, int button, int action, int mods);

         // ������
        static void scrollCallback(GLFWwindow* window, double x, double y);
    };
}
#endif
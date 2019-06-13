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
        friend class InputManager;

    public:
        GLFWwindow* window; // GLFW typedef
        VkSurfaceKHR surface;
        std::unordered_map<VulkanDevice*, VulkanSurfaceDetailsHandle> surface_settings;
        uint32_t glfw_extension_count;
        const char** glfw_extensions;
        uint32_t window_width;
        uint32_t window_height;
        char* application_name;

        GLFWWindow() = default;
        ~GLFWWindow() = default;

        /*
         * Initializes GLFW and creates the window wrapper.
         *
         * todo: add resizing event handler. requires manually updating framebuffer
         */
        void create(const int width, const int height, const char* application_name);

        /*
         * Creates a Vulkan surface for generically communicating between Vulkan and the system window API.
         */
        void createSurface(VkInstance instance);

        /*
         * Deletes all necessary Vulkan and GLFW containers.
         */
        void shutDown(VkInstance instance);

        /*
         * Execute main GLFW polling code.
         */
        void run();

        /*
         * Tell GLFW when the window needs to close.
         */
        void setShouldClose(bool should_close);

        /*
         * Returns whether GLFW received a termination signal.
         */
        bool shouldClose();

    private:

        /*
         * 键盘按键
         */
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        /*
         * 鼠标位置
         */
        static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

        /*
         * 鼠标按键
         */
        static void mousebuttonCallback(GLFWwindow* window, int button, int action, int mods);

        /*
         * 鼠标滚轮
         */
        static void scrollCallback(GLFWwindow* window, double x, double y);
    };
}

#endif // VIRTUALVISTA_GLFWWINDOW_H
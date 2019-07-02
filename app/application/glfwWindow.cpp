#include <iostream>

#include "glfwWindow.h"
#include "inputManager.h"
#include "settings.h"
#include "vkUtils.hpp"

namespace Engine
{
    // 创建窗体
    void GLFWWindow::create(const int width, const int height, const char* application_name)
    {
        VV_ASSERT(glfwInit() != 0, "GLFW failed to init");
        window_width = width;
        window_height = height;
        application_name = application_name;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Don't use OpenGL, note:此处一行不能去掉,如果去掉N卡将有机率报错.
        //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(window_width, window_height, application_name, nullptr, nullptr);

        glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetCursorPosCallback(window, cursorPositionCallback);
        glfwSetMouseButtonCallback(window, mousebuttonCallback);
        glfwSetScrollCallback(window, scrollCallback);
    }

    // API和窗体的连接
    void GLFWWindow::createSurface(VkInstance instance)
    {
        VV_CHECK_SUCCESS(glfwCreateWindowSurface(instance, window, nullptr, &surface));
    }

    // 销毁窗体
    void GLFWWindow::shutDown(VkInstance instance)
    {
        surface_settings.clear();

        if (surface != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(instance, surface, nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    // 运行
    void GLFWWindow::run()
    {
        glfwPollEvents();
    }

    // 关闭窗体
    void GLFWWindow::setShouldClose(bool should_close)
    {
        glfwSetWindowShouldClose(window, should_close);
    }

    // 关闭窗体
    bool GLFWWindow::shouldClose()
    {
        return glfwWindowShouldClose(window) == 1;
    }

    // 键盘按键
    void GLFWWindow::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        InputManager::inst()->keyboardEventsCallback(window, key, scancode, action, mods);
    }

    // 鼠标位置
    void GLFWWindow::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
    {
        InputManager::inst()->mouseEventsCallback(window, xpos, ypos);
    }

    // 鼠标按键
    void GLFWWindow::mousebuttonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        InputManager::inst()->mouseButtonCallback(window, button, action);
    }

    // 鼠标滚轮
    void GLFWWindow::scrollCallback(GLFWwindow* window, double x, double y)
    {
        InputManager::inst()->scrollCallback(window, x, y);
    }
}
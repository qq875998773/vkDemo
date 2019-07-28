#include <iostream>

#include "glfwWindow.h"
#include "inputManager.h"
#include "settings.h"
#include "vkUtils.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace Engine
{
    // ��������
    void GLFWWindow::create(const int width, const int height, const char* application_name)
    {
        VV_ASSERT(glfwInit() != 0, "GLFW failed to init");
        window_width = width;
        window_height = height;
        application_name = application_name;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Don't use OpenGL, note:�˴�һ�в���ȥ��,���ȥ��N�����л��ʱ���.
        //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(window_width, window_height, application_name, nullptr, nullptr);

        glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetCursorPosCallback(window, cursorPositionCallback);
        glfwSetMouseButtonCallback(window, mousebuttonCallback);
        glfwSetScrollCallback(window, scrollCallback);

        /*ImGui::CreateContext();
        ImGui_ImplGlfw_InitForVulkan(window, true);*/
    }

 

    // API�ʹ��������
    void GLFWWindow::createSurface(VkInstance instance)
    {
        VV_CHECK_SUCCESS(glfwCreateWindowSurface(instance, window, nullptr, &surface));
    }

    // ���ٴ���
    void GLFWWindow::shutDown(VkInstance instance)
    {
        surface_settings.clear();

        if (surface != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(instance, surface, nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    // ����
    void GLFWWindow::run()
    {
       /* ImGui_ImplGlfw_NewFrame();

        static const ImVec2 win_size(380, 580);
        ImGui::SetNextWindowSizeConstraints(win_size, win_size);
        ImGui::Begin("Global settings");
        ImGui::End();
        ImGui::Render();*/

        glfwPollEvents();
    }

    // �رմ���
    void GLFWWindow::setShouldClose(bool should_close)
    {
        glfwSetWindowShouldClose(window, should_close);
    }

    // �رմ���
    bool GLFWWindow::shouldClose()
    {
        return glfwWindowShouldClose(window) == 1;
    }

    // ���̰���
    void GLFWWindow::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        InputManager::inst()->keyboardEventsCallback(window, key, scancode, action, mods);
    }

    // ���λ��
    void GLFWWindow::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
    {
        InputManager::inst()->mouseEventsCallback(window, xpos, ypos);
    }

    // ��갴��
    void GLFWWindow::mousebuttonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        InputManager::inst()->mouseButtonCallback(window, button, action);
    }

    // ������
    void GLFWWindow::scrollCallback(GLFWwindow* window, double x, double y)
    {
        InputManager::inst()->scrollCallback(window, x, y);
    }
}
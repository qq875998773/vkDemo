#include <stdexcept>
#include <chrono>

#include "application.h"
#include "inputManager.h"
#include "settings.h"

namespace vv
{
    // ���������ʼ��
    void Application::create(int argc, char** argv)
    {
        m_argc = argc;
        m_argv = argv;

        // ��������
        m_window.create(m_window_width, m_window_height, m_application_name);

        // todo: does this need to be malloced?
        m_renderer = new DeferredRenderer; // �½���Ⱦʵ��

        // ������Ⱦ��ʼ��
        m_renderer->create(&m_window);
        m_scene = m_renderer->getScene();

        // ��Ϣ��ʾ
        std::cout << "Initialization Completed...\n";
        std::cout << "Get Ready Renderering...\n";
    }

    // �����Դ
    void Application::shutDown()
    {
        m_renderer->shutDown();
    }

    // ��ȡ����
    Scene* Application::getScene() const
    {
        return m_scene;
    }

    // ���̲���
    void Application::handleInput(float delta_time)
    {
        float move_speed = m_move_speed * delta_time;

        Camera* camera = m_scene->getActiveCamera();
        if (InputManager::inst()->keyIsPressed(GLFW_KEY_W))
            camera->translate(move_speed * camera->getForwardDirection());

        if (InputManager::inst()->keyIsPressed(GLFW_KEY_A))
            camera->translate(-move_speed * camera->getSidewaysDirection());

        if (InputManager::inst()->keyIsPressed(GLFW_KEY_S))
            camera->translate(-move_speed * camera->getForwardDirection());

        if (InputManager::inst()->keyIsPressed(GLFW_KEY_D))
            camera->translate(move_speed * camera->getSidewaysDirection());

        if (InputManager::inst()->keyIsPressed(GLFW_KEY_Q))
            camera->translate(glm::vec3(0.f, -move_speed, 0.f));

        if (InputManager::inst()->keyIsPressed(GLFW_KEY_E))
            camera->translate(glm::vec3(0.f, move_speed, 0.f));

        if (InputManager::inst()->keyIsPressed(GLFW_KEY_ESCAPE))
            m_window.setShouldClose(true);
    }

    // ��갴��
    void Application::mouseInput(float delta_time)
    {
        float rotate_speed = m_rotate_speed * delta_time;
        Camera* camera = m_scene->getActiveCamera();
        if (InputManager::inst()->keyIsPressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            double delta_x, delta_y;
            InputManager::inst()->getCursorGradient(delta_x, delta_y);
            camera->rotate(delta_x * rotate_speed, delta_y * rotate_speed);
        }
    }

    // ������ѭ��
    void Application::beginMainLoop()
    {
        m_renderer->recordCommandBuffers();

        auto last_time = glfwGetTime();

        while (!m_renderer->shouldStop())
        {
            auto curr_time = glfwGetTime();
            float delta_time = curr_time - last_time;
            last_time = curr_time;

            m_window.run();

            handleInput(delta_time);
            mouseInput(delta_time);

            m_renderer->run(delta_time);
        }
    }
}
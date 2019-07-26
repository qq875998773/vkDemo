

#include <stdexcept>
#include <chrono>

#include "application.h"
#include "inputManager.h"
#include "settings.h"

#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include "GLFW/glfw3native.h"
#endif

#include "../include/imgui/imgui.h"

namespace Engine
{
    // 创建程序初始化
    void Application::create(int argc, char** argv)
    {
        m_argc = argc;
        m_argv = argv;

        // 创建窗体
        m_window.create(m_window_width, m_window_height, m_application_name);

        // todo: does this need to be malloced?
        m_renderer = new DeferredRenderer; // 新建渲染实例

        // 创建渲染初始化
        m_renderer->create(&m_window);
        m_scene = m_renderer->getScene();

        // 消息提示
        std::cout << "Initialization Completed...\n";
        std::cout << "Get Ready Renderering...\n";

       //  // Init ImGui
       // ImGui::CreateContext();
       // // Color scheme
       // ImGuiStyle& style = ImGui::GetStyle();
       // style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
       // style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
       // style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
       // style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
       // style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
       // style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
       // style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
       // style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
       // style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
       // style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
       // style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
       // style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
       // style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
       // style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
       // style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
       // style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);

       // // Dimensions
       // ImGuiIO& io = ImGui::GetIO();

       // io.ClipboardUserData = m_window.window;
       // io.ImeWindowHandle = glfwGetWin32Window(m_window.window);

       // io.DisplaySize = ImVec2(((float)m_window_width), ((float)m_window_height));
       // io.DeltaTime = 1.0f;
       // //io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
       // // Create font texture
       // unsigned char* fontData;
       // int texWidth, texHeight;
       //// std::string path = ROOTPROJECTDIR "/assets/Fonts/Roboto-Medium.ttf";
       // //io.Fonts->AddFontFromFileTTF(path.data(), 16.0f);
       // io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
       // io.FontGlobalScale = 1.0f;


    }

    // 清除资源
    void Application::shutDown()
    {
        m_renderer->shutDown();
    }

    // 获取场景
    Scene* Application::getScene() const
    {
        return m_scene;
    }

    // 键盘操作
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

    // 鼠标按键
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


    void Application::debugWindow()
    {
        //ImGuiIO& io = ImGui::GetIO();

        //// Setup display size (every frame to accommodate for window resizing)
        //int w, h;
        //int display_w, display_h;
        //glfwGetWindowSize(m_window.window, &w, &h);
        //glfwGetFramebufferSize(m_window.window, &display_w, &display_h);
        //io.DisplaySize = ImVec2((float)w, (float)h);
        //io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

        //// Setup time step
        //double current_time = glfwGetTime();
        //io.DeltaTime = 0.0f > 0.0 ? (float)(current_time - 0.0f) : (float)(1.0f / 60.0f);
        //g_Time = current_time;

        //// Setup inputs
        //// (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
        //if (glfwGetWindowAttrib(m_window.window, GLFW_FOCUSED))
        //{
        //    double mouse_x, mouse_y;
        //    glfwGetCursorPos(m_window.window, &mouse_x, &mouse_y);
        //    io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);   // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
        //}
        //else
        //{
        //    io.MousePos = ImVec2(-1, -1);
        //}

        ////for (int i = 0; i < 3; i++)
        ////{
        ////    io.MouseDown[i] = g_MousePressed[i] || glfwGetMouseButton(m_window.window, i) != 0;    // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
        ////    g_MousePressed[i] = false;
        ////}

        //io.MouseWheel = 0.0f;
        ////g_MouseWheel = 0.0f;

        //// Hide OS mouse cursor if ImGui is drawing it
        //glfwSetInputMode(m_window.window, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

        //// Start the frame
        //ImGui::NewFrame();

        //// ===================================== info end



        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
        //ImGui::ShowDemoWindow();

        ImGui::SetNextWindowPos(ImVec2(10, 10));
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
        ImGui::Begin("Vulkan Example", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ////ImGui::TextUnformatted(title.c_str());
        ////ImGui::TextUnformatted(deviceProperties.deviceName);
        ////ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / lastFPS), lastFPS);

        ImGui::PushItemWidth(110.0f * 1.0f);
        ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen);
        bool animate = true;
        ImGui::Checkbox("Moving attractor", &animate);
        ImGui::PopItemWidth();

        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::Render();
    }

    // 开启主循环
    void Application::beginMainLoop()
    {
        m_renderer->recordCommandBuffers();

        auto last_time = glfwGetTime();

        while (!m_renderer->shouldStop())
        {
            debugWindow();

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
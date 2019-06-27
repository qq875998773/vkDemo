/*****************************************************
 * 这是学习初期写的一个面向过程的demo代码,现已废弃
 * 这段代码是可运行无错的,可以用它绘制一个房子
 * 也可以修改initVulkan里的模型加载方法使其绘制球体
 * 如有问题欢迎加QQ群交流,群号：1017638604 
 *****************************************************/

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <array>
#include <set>
#include <unordered_map>
#include "vulkan/vulkan.hpp" // 面向对象调用方式


const int WIDTH = 1280; // 窗体宽
const int HEIGHT = 720; // 窗体高

const std::string MODEL_PATH = "../resources/models/chalet.obj";     // obj模型路径
const std::string TEXTURE_PATH = "../resources/textures/chalet.jpg"; // 纹理图片路径

#pragma region 鼠标键盘操作
bool     g_is_left_pressed = false;  // 键盘A
bool     g_is_right_pressed = false; // 键盘D
bool     g_is_fwd_pressed = false;   // 键盘W
bool     g_is_back_pressed = false;  // 键盘S
bool     g_is_mouse_tracking = false;
bool     g_is_scroll_delta = false;   // 鼠标滚动
bool     g_is_middle_pressed = false; // 鼠标中键
glm::vec2   g_mouse_pos = glm::vec2(0.f, 0.f);
glm::vec2   g_mouse_delta = glm::vec2(0.f, 0.f);  // 鼠标右键拖动
glm::vec2   g_scroll_delta = glm::vec2(0.f, 0.f); // 鼠标滚动

// 鼠标移动
void OnMouseMove(GLFWwindow* window, double x, double y)
{
    if (g_is_mouse_tracking)
    {
        g_mouse_delta = glm::vec2((float)x, (float)y) - g_mouse_pos;
        g_mouse_pos = glm::vec2((float)x, (float)y);
    }
}

// 鼠标滚轮
void OnMouseScroll(GLFWwindow* window, double x, double y)
{
    g_scroll_delta = glm::vec2((float)x, (float)y);
    g_is_scroll_delta = true;
}

// 鼠标按键
void OnMouseButton(GLFWwindow* window, int button, int action, int mods)
{
    if ((button == GLFW_MOUSE_BUTTON_LEFT) ||
        (button == GLFW_MOUSE_BUTTON_RIGHT) ||
        (button == GLFW_MOUSE_BUTTON_MIDDLE))
    {
        if (action == GLFW_PRESS)
        {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            g_mouse_pos = glm::vec2((float)x, (float)y);
            g_mouse_delta = glm::vec2(0, 0);
        }
        else if (action == GLFW_RELEASE && g_is_mouse_tracking)
        {
            g_is_mouse_tracking = false;
            g_is_middle_pressed = false;
            g_mouse_delta = glm::vec2(0, 0);
        }
    }

    if ((button == GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_PRESS))
    {
        g_is_mouse_tracking = true;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            g_mouse_pos = glm::vec2((float)x, (float)y);
        }
    }

    // 鼠标中键
    if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        if (action == GLFW_PRESS)
        {
            g_is_middle_pressed = true;
            g_is_mouse_tracking = true;
        }
    }
}

// 键盘点击
void OnKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    const bool press_or_repeat = action == GLFW_PRESS || action == GLFW_REPEAT;

    switch (key)
    {
    case GLFW_KEY_W:
        g_is_fwd_pressed = press_or_repeat;
        break;
    case GLFW_KEY_S:
        g_is_back_pressed = press_or_repeat;
        break;
    case GLFW_KEY_A:
        g_is_left_pressed = press_or_repeat;
        break;
    case GLFW_KEY_D:
        g_is_right_pressed = press_or_repeat;
    default:
        break;
    }
}
#pragma endregion

// 异常处理层
const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };

// 设备扩展. 交换链
const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
// 指定要启用的layers并开启它们
const bool enableValidationLayers = true;
#endif

// 创建调试回调
VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
    auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

// 销毁调试回调
void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr)
    {
        func(instance, callback, pAllocator);
    }
}

// 返回满足某个属性的队列簇索引.定义结构体,其中索引-1表示"未找到"
// 支持graphics命令的的队列簇和支持presentation命令的队列簇可能不是同一个簇
struct QueueFamilyIndices
{
    int graphicsFamily = -1; // 绘图簇
    int presentFamily = -1;  // 当前簇

    bool isComplete()
    {
        return graphicsFamily >= 0 && presentFamily >= 0;
    }
};

// 如果仅仅是为了测试交换链的有效性是远远不够的,因为它还不能很好的与窗体surface兼容
// 创建交换链同样也需要很多设置,所以需要了解一些有关设置的细节
// 交换链结构体详细信息
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;     // 分辨率
    std::vector<VkSurfaceFormatKHR> formats;   // 格式
    std::vector<VkPresentModeKHR> presentModes;// 当前模型
};

// 顶点
struct Vertex
{
    glm::vec3 pos;      // 顶点坐标
    glm::vec3 color;    // 颜色rgb
    glm::vec2 texCoord; // 纹理坐标UV
    glm::vec3 normal;   // 顶点法线

    // 顶点输入绑定 描述了顶点数据从内存加载的规则.它指定数据条目之间的间隔字节以及是否每个顶点之后或者每个instance之后移动到下一个条目
    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;// 移动到每个顶点后的下一个数据条目
        /*
        inputRate参数可以具备一下值之一：
            VK_VERTEX_INPUT_RATE_VERTEX:移动到每个顶点后的下一个数据条目
            VK_VERTEX_INPUT_RATE_INSTANCE:在每个instance之后移动到下一个数据条目
        */

        return bindingDescription;
    }

    // 处理顶点的输入
    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};

        // binding告诉Vulkan每个顶点数据的来源
        attributeDescriptions[0].binding = 0;
        // location引用了vertex shader作为输入的location指令.顶点着色器中,location为0代表position,它是32bit单精度数据
        attributeDescriptions[0].location = 0;
        // format描述属性的类型.该格式使用与颜色格式一样的枚举
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        // offset指定每个顶点数据读取的字节宽度偏移量,绑定一次加载一个Vertex,position属性(pos)的偏移量在字节数据中为0字节.使用offsetof macro宏自动计算
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, normal);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const
    {
        return pos == other.pos && color == other.color && texCoord == other.texCoord && normal == other.normal;
    }
};

// 扩展std标准库
namespace std
{
    template<> struct hash<Vertex>
    {
        size_t operator()(Vertex const& vertex) const
        {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ 
                   (hash<glm::vec2>()(vertex.texCoord) << 1) ^ (hash<glm::vec3>()(vertex.normal) << 1);
        }
    };
}

// 着色器数据 ubo
struct UniformBufferObject
{
    glm::mat4 model;  // 模型矩阵 模型在空间中的位置
    glm::mat4 view;   // 视口矩阵 记录摄像机位置
    glm::mat4 proj;   // 投影透视矩阵 根据物体离摄像机的远近 放缩模型
    glm::vec3 testlight; //加一个灯光
};

// 管线类
class Application
{
public:
    // 运行
    void run()
    {
        initWindow(); // 初始化窗体
        initVulkan(); // 初始化vulkan|渲染管线
        mainLoop();   // 主循环将图形绘制到屏幕
        cleanup();    // 清除资源
    }

private:
    GLFWwindow* window;  // 窗体实例
    VkInstance instance; // vk实例
    VkDebugReportCallbackEXT callback; // debug异常检测,存储回调句柄
    /*
     需要在instance创建之后立即创建窗体surface
     因为它会影响物理设备的选择
     窗体surface本身对于Vulkan也是非强制的
     Vulkan允许这样做,不需要同OpenGL一样必须要创建窗体surface
    */
    VkSurfaceKHR surface; // surface就是Vulkan与窗体系统的连接桥梁

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // 物理设备
    VkDevice device;                                  // 逻辑设备

    VkQueue graphicsQueue; // 绘图队列句柄
    VkQueue presentQueue;  // 当前队列句柄

    VkSwapchainKHR swapChain;                        // 交换链对象
    std::vector<VkImage> swapChainImages;            // 交换链图像集, 图像被交换链创建,会在交换链销毁时自动清理,不需要添加清理代码
    VkFormat swapChainImageFormat;                   // 交换链图像格式
    VkExtent2D swapChainExtent;                      // 交换链范围(分辨率)
    std::vector<VkImageView> swapChainImageViews;    // 保存图像视图的句柄集
    std::vector<VkFramebuffer> swapChainFramebuffers;// 交换链帧缓冲区集

    VkRenderPass renderPass;                   // 渲染通道
    VkDescriptorSetLayout descriptorSetLayout; // 描述符集布局
    VkPipelineLayout pipelineLayout;           // 管线布局
    VkPipeline graphicsPipeline;               // 绘制管线

    VkCommandPool commandPool;      // 命令池

    VkImage depthImage;             // 图像深度附件
    VkDeviceMemory depthImageMemory;// 图像深度附件记录
    VkImageView depthImageView;     // 图像深度视图

    VkImage textureImage;              // 纹理图片
    VkDeviceMemory textureImageMemory; // 纹理图片记录
    VkImageView textureImageView;      // 纹理图像视图
    VkSampler textureSampler;          // 纹理采样器

    std::vector<Vertex> vertices; // 顶点集合
    /*
    推荐在单个内存中分配多个资源,如缓冲区,但是实际上,应该更进一步细化.Nvidia的驱动开发者建议将多个缓冲区(顶点缓冲区、索引缓冲区)存储到单个VkBuffer中
    并在诸如vkCmdBindVertexBuffers之类的命令中使用偏移量
    优点:在这种情况下,数据会更加充分的利用缓存,因为它们排列在一块区域.甚至在同一个渲染操作中可以复用来自相同内存块的多个资源块,只要刷新数据即可
    该技巧称为aliasing,一些Vulkan函数有明确的标志指定这样做的意图
    */
    std::vector<uint32_t> indices;    // 顶点索引集
    VkBuffer vertexBuffer;            // 顶点缓冲区
    VkDeviceMemory vertexBufferMemory;// 顶点缓冲区内存
    VkBuffer indexBuffer;             // 索引缓冲区
    VkDeviceMemory indexBufferMemory; // 索引缓冲区内存

    VkBuffer uniformBuffer;            // 全局缓冲区 Uniform:一个特殊类型的GLSL变量.它是全局的(在一个着色器程序中每一个着色器都能够访问uniform变量)并且只能被设定一次
    VkDeviceMemory uniformBufferMemory;// 全局缓冲区内存

    VkDescriptorPool descriptorPool; // 描述符集
    VkDescriptorSet descriptorSet;   // 描述符设置

    std::vector<VkCommandBuffer> commandBuffers; // 命令缓冲区集

    VkSemaphore imageAvailableSemaphore; // 渲染开始信号
    VkSemaphore renderFinishedSemaphore; // 渲染结束信号

    // 初始化窗体
    void initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // 创建窗体 设置尺寸和标题
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Demo", nullptr, nullptr);

        glfwSetWindowUserPointer(window, this);
        glfwSetWindowSizeCallback(window, Application::onWindowResized);
    }

    // 初始化vulkan
    void initVulkan()
    {
        createInstance();           // 创建实例
        setupDebugCallback();       // 设置调试回调句柄
        createSurface();            // 创建窗体和vulkan实例连接
        pickPhysicalDevice();       // 连接物理设备
        createLogicalDevice();      // 创建逻辑设备
        createSwapChain();          // 创建交换链
        createImageViews();         // 创建图像视图
        createRenderPass();         // 创建渲染通道
        createDescriptorSetLayout();// 创建描述符设置布局 uniform buffer布局
        createGraphicsPipeline();   // 创建绘图管线
        createCommandPool();        // 创建命令池
        createDepthResources();     // 创建深度资源
        createFramebuffers();       // 创建帧缓冲区
        createTextureImage();       // 创建纹理图片 stb库
        createTextureImageView();   // 创建纹理图像视图
        createTextureSampler();     // 创建纹理采样器
        loadModel();                // 加载模型 tinyobjloader库
        //loadModel2();             // 画球
        createVertexBuffer();       // 创建顶点缓冲区
        createIndexBuffer();        // 创建顶点索引缓冲区
        createUniformBuffer();      // 创建全局缓冲区
        createDescriptorPool();     // 创建描述符池
        createDescriptorSet();      // 创建描述符设置. unifrom layout binding顺序(shader layout uniform 访问的顺序设置)
        createCommandBuffers();     // 创建命令缓冲区
        createSemaphores();         // 创建信号对象. 开始|终止信号
    }

    // 主循环将图形绘制到屏幕
    void mainLoop()
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            updateUniformBuffer(); // 该函数会在每次循环中创建新的变换矩阵以确保旋转、移动、放缩等变换
            drawFrame();           // 绘制帧
        }

        vkDeviceWaitIdle(device);
    }

    // 清除交换链资源
    void cleanupSwapChain()
    {
        vkDestroyImageView(device, depthImageView, nullptr);
        vkDestroyImage(device, depthImage, nullptr);
        vkFreeMemory(device, depthImageMemory, nullptr);

        // 删除帧缓冲区
        for (size_t i = 0; i < swapChainFramebuffers.size(); i++)
        {
            vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
        }

        // 清理用于传输命令的命令缓冲区
        vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

        vkDestroyPipeline(device, graphicsPipeline, nullptr);     // 清除图形管线
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr); // 销毁pipeline layout
        vkDestroyRenderPass(device, renderPass, nullptr);         // 销毁渲染通道,整个程序生命周期内都被使用,需要在退出阶段进行清理

        // 图像视图需要明确的创建过程,所以在程序退出的时候,需要添加一个循环去销毁
        for (size_t i = 0; i < swapChainImageViews.size(); i++)
        {
            vkDestroyImageView(device, swapChainImageViews[i], nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);// 在清理设备前销毁交换链
    }

    // 清除资源
    void cleanup()
    {
        cleanupSwapChain(); // 清除交换链

        vkDestroySampler(device, textureSampler, nullptr);    // 销毁纹理采样器
        vkDestroyImageView(device, textureImageView, nullptr);// 销毁纹理图像视图

        vkDestroyImage(device, textureImage, nullptr);    // 清除贴图图像
        vkFreeMemory(device, textureImageMemory, nullptr);// 清除贴图图像记录

        vkDestroyDescriptorPool(device, descriptorPool, nullptr); // 销毁描述对象池

        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        vkDestroyBuffer(device, uniformBuffer, nullptr);
        vkFreeMemory(device, uniformBufferMemory, nullptr);

        vkDestroyBuffer(device, indexBuffer, nullptr);   // 清除顶点索引缓冲区
        vkFreeMemory(device, indexBufferMemory, nullptr);// 清除顶点索引缓冲区记录

        vkDestroyBuffer(device, vertexBuffer, nullptr);   // 清除顶点缓冲区
        vkFreeMemory(device, vertexBufferMemory, nullptr);// 清除顶点缓冲区记录

        vkDestroySemaphore(device, renderFinishedSemaphore, nullptr); // 清除渲染信号
        vkDestroySemaphore(device, imageAvailableSemaphore, nullptr); // 清除图像信号

        vkDestroyCommandPool(device, commandPool, nullptr);// 销毁命令缓冲区

        vkDestroyDevice(device, nullptr);                           // 清除逻辑设备资源
        DestroyDebugReportCallbackEXT(instance, callback, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);            // GLFW没提供专用函数销毁surface,可以简单的通过Vulkan原始API
        vkDestroyInstance(instance, nullptr);                       // 确保surface的清理是在instance销毁之前完成

        glfwDestroyWindow(window); // 清除窗体资源
        glfwTerminate();
    }

    // 窗口尺寸变化
    static void onWindowResized(GLFWwindow* window, int width, int height)
    {
        if (width == 0 || height == 0) return;

        Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        app->recreateSwapChain(); // 重新创建交换链
    }

    // 重新创建交换链
    void recreateSwapChain()
    {
        vkDeviceWaitIdle(device);

        // 清除交换链资源
        cleanupSwapChain();

        createSwapChain(); // 创建交换链
        createImageViews();// 创建图像视图
        createRenderPass();// 创建渲染通道
        createGraphicsPipeline();// 创建图像管线
        createDepthResources();  // 创建信号资源
        createFramebuffers();    // 创建帧缓冲区
        createCommandBuffers();  // 创建命令缓冲区
    }

    // 创建vk实例
    void createInstance()
    {
        if (enableValidationLayers && !checkValidationLayerSupport())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        // 初始化运行时
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;    // 指定结构体类型
        appInfo.pApplicationName = "Hello Triangle";           // 应用程序名
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // 应用程序的版本号
        appInfo.pEngineName = "No Engine";                     // 引擎或者中间件(应用程序基于此构建)名字
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);      // 引擎或中间件版本号
        appInfo.apiVersion = VK_API_VERSION_1_0;               // 期望运行的vk API的版本号

        // 生成实例初始化
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo; // 把运行时赋给实例

        // 获取所需的扩展
        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (enableValidationLayers)
        {   // 想激活的实例层的个数
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            // 想激活的实例层的名字
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            // 如果不需要开启检测 则设置层数为0
            createInfo.enabledLayerCount = 0;
        }

        // 创建vk实例
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create instance!");
        }
    }

    // 设置调试回调句柄
    void setupDebugCallback()
    {
        if (!enableValidationLayers) return;

        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pfnCallback = debugCallback;

        if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up debug callback!");
        }
    }

    // GLFW没有使用结构体,而是选择非常直接的参数传递来调用函数
    // vulkan和窗体的连接
    void createSurface()
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    // 选择物理设备
    void pickPhysicalDevice()
    {
        // 获取物理设备数量
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        // 如果一个设备都没有 抛出异常
        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        // 分配数组用来记录所有设备的句柄
        std::vector<VkPhysicalDevice> devices(deviceCount);
        // 根据deviceCount设备个数给devices数组赋句柄值
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // 遍历所有的设备
        for (const auto& device : devices)
        {
            // 如果这个设备符合要求 把这个设备句柄付给前面声明的句柄变量 跳出循环
            // 这里就是找第一个符合要求的设备以这个设备为计算设备
            if (isDeviceSuitable(device))
            {
                physicalDevice = device;
                break;
            }
        }

        // 如果选择的设备为空 抛异常
        if (physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    // 创建逻辑设备
    void createLogicalDevice()
    {
        /*
        需要多个VkDeviceQueueCreateInfo结构来创建不同功能的队列.
        一个优雅的方式是针对不同功能的队列簇创建一个set集合确保队列簇的唯一性
        */

        // 查找列队簇
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        // 创建列队集初始化
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

        // 遍历设备簇
        float queuePriority = 1.0f;// 使用0.0到1.0之间的浮点数分配队列优先级来影响命令缓冲区执行的调用.即使只有一个队列也必须赋值
        for (int queueFamily : uniqueQueueFamilies)
        {
            // 创建设备列队初始化
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // 明确物理设备要使用的功能特性信息
        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;// 采样器各向异性
        /*
        如果不是强制使用各向异性滤波器,也可以简单的通过条件设定来不使用它：
            samplerInfo.anisotropyEnable = VK_FALSE;
            samplerInfo.maxAnisotropy = 1;
        */

        // 创建逻辑设备初始化
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        // 把初始化的队列数组赋给逻辑设备
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        // 把设备功能特性赋值给逻辑设备
        createInfo.pEnabledFeatures = &deviceFeatures;

        // 需要激活的扩展个数
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data(); // 需要激活的扩展名字. 这里指交换链

        // 是否开启日志检测层
        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()); // 需要激活的实例层个数
            createInfo.ppEnabledLayerNames = validationLayers.data();                      // 需要激活的实例层名字
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        // 创建逻辑设备
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }

        // 检测每个队列簇中队列的句柄.参数是逻辑设备,队列簇,队列索引和存储获取队列变量句柄的指针
        // 因为只是从这个队列簇创建一个队列,所以需要使用索引0
        vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
        // 获取当前设备列队
        vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);
    }

    // 创建交换链
    void createSwapChain()
    {
        // 获取交换链结构体详细信息
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        // 每个VkSurfaceFormatKHR结构都包含一个format和一个colorSpace成员
        // format成员变量指定色彩通道和类型
        // 比如,VK_FORMAT_B8G8R8A8_UNORM代表了使用B,G,R和alpha次序的通道,且每一个通道为无符号8bit整数,每个像素总计32bits
        // colorSpace成员描述SRGB颜色空间是否通过VK_COLOR_SPACE_SRGB_NONLINEAR_KHR标志支持
        // 需要注意的是在较早版本的规范中,这个标志名为VK_COLORSPACE_SRGB_NONLINEAR_KHR
        // 尽可能使用SRGB(彩色语言协议),因为它会得到更容易感知的、精确的色彩
        // 直接与SRGB颜色打交道是比较麻烦的,使用标准的RGB作为颜色格式,VK_FORMAT_B8G8R8A8_UNORM是通常使用的格式

        // 设置surface格式.传递formats作为函数的参数,类型为SwapChainSupportDetails
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        // presentation模式对于交换链非常重要,它代表了在屏幕呈现图像的条件
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        // 交换链范围(分辨率)
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        // 交换链中的图像数量,可以理解为队列的长度.它指定运行时图像的最小数量,尝试大于1的图像数量,以实现三重缓冲
        // maxImageCount数值为0代表除了内存之外没有限制,这就是为什么需要检查
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        // 创建交换链初始化
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        // 交换链绑定到具体的surface之后,需要指定交换链图像有关的详细信息
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent; // 
        createInfo.imageArrayLayers = 1; // imageArrayLayers指定每个图像组成的层数.除非开发3D应用程序,否则始终为1
        // imageUsage指定在交换链中对图像进行的具体操作
        // 直接对它们进行渲染,这意味着它们作为颜色附件.也可以首先将图像渲染为单独的图像,进行后处理操作
        // 在这种情况下可以使用像VK_IMAGE_USAGE_TRANSFER_DST_BIT这样的值,并使用内存操作将渲染的图像传输到交换链图像队列
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        // 队列簇索引
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily }; // 绘图簇,当前簇

        // 如果绘图簇不等于当前簇
        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // 图像可以被多个队列簇访问,不需要明确所有权从属关系
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            // 同一时间图像只能被一个队列簇占用,如果其他队列簇需要其所有权需要明确指定.这种方式提供了最好的性能
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        // 如果交换链支持(supportedTransforms in capabilities),可以为交换链图像指定某些转换逻辑
        // 比如90度顺时针旋转或者水平反转.如果不需要任何transoform操作,可以简单的设置为currentTransoform

        // 当前变换赋值给preTransform
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        // 混合Alpha字段指定alpha通道是否应用与与其他的窗体系统进行混合操作.如果忽略该功能,赋VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        // presentMode指向自己.如果clipped成员设置为VK_TRUE,意味着不关心被遮蔽的像素数据,
        // 比如由于其他的窗体置于前方时或者渲染的部分内容存在于可视区域外,除非需要读取这些像素获数据进行处理,否则可以开启裁剪获得最佳性能
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        // 创建交换链
        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create swap chain!");
        }

        // 需要注意的是,之前创建交换链步骤中传递了期望的图像大小到字段minImageCount.
        // 而实际的运行,允许创建更多的图像数量,这就解释了为什么需要再一次获取数量
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        // 存储交换链图形格式
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent; // 存储交换链范围(分辨率)
    }

    // 创建图像视图
    void createImageViews()
    {
        // 指定图像视图集合大小
        swapChainImageViews.resize(swapChainImages.size());

        // 给图像视图集赋值
        for (uint32_t i = 0; i < swapChainImages.size(); i++)
        {
            swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }

    // 创建渲染通道
    void createRenderPass()
    {
        // 颜色附件
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = swapChainImageFormat;// 颜色附件的格式,应该与交换链中图像的格式相匹配
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;// 不做多重采样工作,所以采样器设置为1
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // 决定渲染前数据在对应附件的操作行为
        /*
        *VK_ATTACHMENT_LOAD_OP_LOAD: 保存已经存在于当前附件的内容
        *VK_ATTACHMENT_LOAD_OP_CLEAR: 起始阶段以一个常量清理附件内容
        *VK_ATTACHMENT_LOAD_OP_DONT_CARE: 存在的内容未定义,忽略它们
        */
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // 决定渲染后数据在对应附件的操作行为
        /*
        VK_ATTACHMENT_STORE_OP_STORE: 渲染的内容会存储在内存,并在之后进行读取操作
        VK_ATTACHMENT_STORE_OP_DONT_CARE: 帧缓冲区的内容在渲染操作完毕后设置为undefined
        */
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;  // stencilLoadOp应用在模版数据,这里不做任何模版缓冲区操作,所以loading无关紧要
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;// stencilStoreOp应用在模版数据,这里不做任何模版缓冲区操作,所以storing无关紧要
        // 指定图像在开始进入渲染通道render pass前将要使用的布局,VK_IMAGE_LAYOUT_UNDEFINED意为不关心图像之前的布局.
        // 特殊值表明图像的内容不确定会被保留,但是这并不重要,因为无论如何都要清理它,像渲染完毕后使用交换链进行呈现
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; 
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // 指定当渲染通道结束自动变换时使用的布局,VK_IMAGE_LAYOUT_PRESENT_SRC_KHR图像在交换链中被呈现
        /*
        VK_IMAGE_LAYOUT_COLOR_ATTACHMET_OPTIMAL: 图像作为颜色附件
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: 图像在交换链中被呈现
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: 图像作为目标,用于内存COPY操作
        */

        // 深度附件
        VkAttachmentDescription depthAttachment = {};
        depthAttachment.format = findDepthFormat();                     // 查找深度格式
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;                // 不做多重采样工作,所以采样器设置为1
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;           // 同上颜色附件
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;// 同上
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; 
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        /* VkAttachmentReference
        一个单独的渲染通道可以由多个子通道组成.子通道是渲染操作的一个序列
        子通道作用与后续的渲染操作,并依赖之前渲染通道输出到帧缓冲区的内容
        比如说后处理效果的序列通常每一步都依赖之前的操作
        如果将这些渲染操作分组到一个渲染通道中,通过Vulkan将通道中的渲染操作进行重排序,可以节省内存从而获得更好的性能
        */

        // 颜色附件参照
        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // 深度附件参照
        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // 渲染子通道
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;                     // 子渲染通道附件数量
        subpass.pColorAttachments = &colorAttachmentRef;      // 子渲染通道颜色附件
        subpass.pDepthStencilAttachment = &depthAttachmentRef;// 子渲染通道深度附件
        /*
        可以被子通道引用的附件类型如下:
            pInputAttachments: 附件从着色器中读取
            pResolveAttachments: 附件用于颜色附件的多重采样
            pDepthStencilAttachment: 附件用于深度和模版数据
            pPreserveAttachments: 附件不被子通道使用,但是数据被保存
        */

        // 子渲染通道依赖
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // VK_SUBPASS_EXTERNAL指在渲染通道之前或之后的隐式子通道,取决于它是否在srcSubpass或dstSubPass中指定
        dependency.dstSubpass = 0; // 索引0指定子通道,这是第一个也是唯一的,dstSubpass必须始终高于srcSubPass以防止依赖关系出现循环
        // 指定要等待的操作和这些操作发生的阶段,在访问对象之前,需要等待交换链完成对应图像的读取操作,这可以通过等待颜色附件输出的阶段来实现
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; 
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;// 指定要等待的操作和这些操作发生的阶段
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;// 颜色附件阶段操作及涉及颜色附件的读写的操作应该等待

        // 引用颜色和深度两个附件
        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());// 附件数量
        renderPassInfo.pAttachments = attachments.data();                          // 附件数据
        renderPassInfo.subpassCount = 1;                                           // 子渲染通道数量
        renderPassInfo.pSubpasses = &subpass;                                      // 子渲染通道
        renderPassInfo.dependencyCount = 1;                                        // 子渲染通道依赖数量
        renderPassInfo.pDependencies = &dependency;                                // 子渲染通道依赖数据

        // 创建渲染通道
        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    // 创建描述符设置布局 为uniform buffer指定顺序 ubo为0位 texture为1位
    void createDescriptorSetLayout()
    {// 需要在管线创建时,为着色器提供关于每个描述符绑定的详细信息,就像为每个顶点属性和location索引做的一样.
     // 添加一个新的函数来定义所有这些名为createDescritorSetLayout的信息.考虑到会在管线中使用,它应该在管线创建函数之前调用

        // 着色器数据结构布局绑定
        VkDescriptorSetLayoutBinding uboLayoutBinding = {};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1; // 
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr; // pImmutableSamplers字段仅仅与图像采样的描述符有关
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // 描述符在着色器哪个阶段被引用

        // 组合图像采样器描述符
        VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        // stageFlags指定在片段着色器中使用组合图像采样器描述符.这是片段颜色最终被确定的地方.可以在顶点着色器中使用纹理采样,比如通过高度图heightmap动态的变形顶点的网格
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        // 描述符设置布局绑定集
        std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
        // 创建描述符设置布局初始化
        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        // 创建描述符设置布局
        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    // 创建绘图管线
    void createGraphicsPipeline()
    {
        // 加载顶点shader和片元shader
        auto vertShaderCode = readFile("../shaders/shader.vert.spv");
        auto fragShaderCode = readFile("../shaders/shader.frag.spv");

        // 在将shader传递给渲染管线之前,必须将其封装到VkShaderModule对象中
        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        // 将着色器模块分配到管线中的顶点着色器阶段
        VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // 阶段
        vertShaderStageInfo.module = vertShaderModule;          // shader模块
        vertShaderStageInfo.pName = "main";

        // 将着色器模块分配到管线中的片段着色器阶段
        VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        // shader阶段集
        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        // 描述顶点数据的格式,数据传递到vertex shader中
        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        // 根据数据的间隙,确定数据是每个顶点或者是每个instance(instancing)
        auto bindingDescription = Vertex::getBindingDescription();
        // 描述将要进行绑定及加载属性的顶点着色器中的相关属性类型
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;                                                     // 顶点绑定描述数量
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()); // 顶点属性描述数量
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;                                      // 顶点绑定描述符数据信息
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();                           // 顶点属性描述符数据信息

        // 输入装配
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // 图元的拓扑结构类型
        /*
        VK_PRIMITIVE_TOPOLOGY_POINT_LIST: 顶点到点
        VK_PRIMITIVE_TOPOLOGY_LINE_LIST: 两点成线,顶点不共用
        VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: 两点成线,每个线段的结束顶点作为下一个线段的开始顶点
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: 三点成面,顶点不共用
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: 每个三角形的第二个、第三个顶点都作为下一个三角形的前两个顶点
        */
        // 顶点数据按照缓冲区中的序列作为索引,但是也可以通过element buffer缓冲区自行指定顶点数据的索引.通过复用顶点数据提升性能.
        // 如果设置primitiveRestartEnable成员为VK_TRUE,可以通过0xFFFF或者0xFFFFFFFF作为特殊索引来分解线和三角形在_STRIP模式下的图元拓扑结构.
        inputAssembly.primitiveRestartEnable = VK_FALSE;  // 是否启用顶点索重新开始图元

        // 描述framebuffer作为渲染输出结果目标区域 裁切剔除不可见区域
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;  // 视口宽(交换链宽)
        viewport.height = (float)swapChainExtent.height;// 视口高(交换链高)
        viewport.minDepth = 0.0f;                       // 指定framebuffer中深度的范围
        viewport.maxDepth = 1.0f;                       // 指定framebuffer中深度的范围

        // 需要将图像绘制到完整的帧缓冲区中,所以定义裁剪矩形覆盖到整体图像
        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;

        // 视口区域状态
        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;     // 视口区域数量
        viewportState.pViewports = &viewport;// 指定视口区域
        viewportState.scissorCount = 1;      // 指定裁剪数量
        viewportState.pScissors = &scissor;  // 指定裁剪数据


        // 光栅化通过顶点着色器及具体的几何算法将顶点进行塑形,并将图形传递到片段着色器进行着色工作
        // 也会执行深度测试depth testing、面裁切face culling和裁剪测试,可以对输出的片元进行配置,决定是否输出整个图元拓扑或者是边框(线框渲染)

        // 创建管线光栅器状态初始化
        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE; // 超过远近裁剪面的片元会进行收敛,而不是丢弃它们.它在特殊的情况下比较有用,像阴影贴图.使用该功能需要得到GPU的支持,深度测试
        rasterizer.rasterizerDiscardEnable = VK_FALSE; // 设置为VK_TRUE,那么几何图元永远不会传递到光栅化阶段.这是基本的禁止任何输出到framebuffer帧缓冲区的方法
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // polygonMode决定几何产生图片的内容,填充
        /*
        VK_POLYGON_MODE_FILL: 多边形区域填充
        VK_POLYGON_MODE_LINE: 多边形边缘线框绘制
        VK_POLYGON_MODE_POINT: 多边形顶点作为描点绘制
        */
        //使用任何模式填充都需开启GPU功能,lineWidth成员是直接填充的,根据片元的数量描述线的宽度.最大的线宽支持取决于硬件,任何大于1.0的线宽需要开启GPU的wideLines特性支持
        rasterizer.lineWidth = 1.0f; 
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // cullMode变量用于决定面裁剪的类型方式.可以禁止culling,裁剪front faces,cull back faces 或者全部
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // frontFace用于描述作为front-facing面的顶点的顺序,可以是顺时针也可以是逆时针
        rasterizer.depthBiasEnable = VK_FALSE;  // 光栅器可以通过添加常量或者基于片元的斜率来更改深度值.有时对于阴影贴图是有用的,这里不使用,所以设置为VK_FALSE

        // 它通过组合多个多边形的片段着色器结果,光栅化到同一个像素.这主要发生在边缘,这也是最引人注目的锯齿出现的地方.
        // 如果只有一个多边形映射到像素是不需要多次运行片段着色器进行采样的,相比高分辨率来说,它会花费较低的开销.开启该功能需要GPU支持
        // 多重采样-抗锯齿
        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // 深度模板
        VkPipelineDepthStencilStateCreateInfo depthStencil = {};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;// 指定是否应该将新的深度缓冲区与深度缓冲区进行比较,以确认是否应该被丢弃
        // 指定通过深度测试的新的片段深度是否应该被实际写入深度缓冲区.这在绘制透明对象时非常有用.它们应该与之前渲染的不透明对象进行比较,但不会导致更远的透明对象不被绘制
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;// 指定执行保留或者丢弃片段的比较细节.坚持深度值较低的惯例,它意味着更近.所以新的片段的深度应该更小
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        // 片段着色器输出具体的颜色,它需要与帧缓冲区framebuffer中已经存在的颜色进行混合.这个转换的过程成为混色
        // 有两种方式:1.将old和new颜色进行混合产出一个最终的颜色 2.使用按位操作混合old和new颜色的值
        // 有两个结构体用于配置颜色混合.第一个结构体VkPipelineColorBlendAttachmentState包括了每个附加到帧缓冲区的配置
        // 第二个结构体VkPipelineColorBlendStateCreateInfo包含了全局混色的设置


        // 管线颜色混合附件状态
        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        // 掩码会用确定帧缓冲区中具体哪个通道的颜色受到影响
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        // 如果设为VK_FALSE,那么从片段着色器输出的新颜色不会发生变化,否则两个混色操作会计算新的颜色.所得到的结果与colorWriteMask进行AND运算,以确定实际传递的通道
        colorBlendAttachment.blendEnable = VK_FALSE; 
        // blendEnable,大多数的情况下使用混色用于实现alpha blending,新的颜色与旧的颜色进行混合会基于它们的opacity透明通道.finalColor作为最终的输出

        // 全局混色的设置 持有所有帧缓冲区的引用,它允许设置混合操作的常量,该常量可以作为后续计算的混合因子
        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        // 使用第一种方式, 需要设置logicOpEnable为VK_TURE.二进制位操作在logicOp字段中指定.在第一种方式中会自动禁止,等同于为每一个附加的帧缓冲区framebuffer关闭混合操作
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        // 可以在着色器中使用uniform,它是类似与动态状态变量的全局变量,可以在绘画时修改,可以更改着色器的行为而无需重新创建它们.
        // 它们通常用于将变换矩阵传递到顶点着色器或者在片段着色器冲创建纹理采样器
        // 这些uniform数值需要在管线创建过程中,通过VkPipelineLayout对象指定

        // 需要在创建管线的时候指定描述符集合的布局,用以告知Vulkan着色器将要使用的描述符.描述符布局在管线布局对象中指定.修改VkPipelineLayoutCreateInfo引用布局对象
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        // 创建管线布局
        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        // 渲染管线初始化 这个结构体填充上面的所有准备数据
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;                // shader阶段集
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = pipelineLayout; // 管线布局赋给管线初始化布局属性
        pipelineInfo.renderPass = renderPass; // 渲染通道
        pipelineInfo.subpass = 0;             // 子渲染通道
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // 子渲染通道句柄设空

        // 创建图形管线
        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, fragShaderModule, nullptr); // 清除frag shader module
        vkDestroyShaderModule(device, vertShaderModule, nullptr); // 清除vert shader module
    }

    // 帧缓冲区
    void createFramebuffers()
    {
        // 动态调整用于保存framebuffers的容器大小
        swapChainFramebuffers.resize(swapChainImageViews.size());

        // 迭代图像视图并通过它们创建对应的framebuffers
        for (size_t i = 0; i < swapChainImageViews.size(); i++)
        {
            // 图像视图数组
            std::array<VkImageView, 2> attachments = {
                swapChainImageViews[i],
                depthImageView
            };

            // 创建framebuffers是非常直接的.首先需要指定framebuffer需要兼容的renderPass
            // 只能使用与其兼容的渲染通道的帧缓冲区,这大体上意味着它们使用相同的附件数量和类型.
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass; // 渲染通道
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size()); // 图像视图数量
            framebufferInfo.pAttachments = attachments.data(); // 图像视图数组
            framebufferInfo.width = swapChainExtent.width; // 帧缓冲区宽
            framebufferInfo.height = swapChainExtent.height;// 帧缓冲区高
            framebufferInfo.layers = 1; // 这里交换链图像是单个图像,因此层数为1

            // 创建帧缓冲区
            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    // 创建令缓冲区
    void createCommandPool()
    {
        // 命令缓冲区通过将其提交到其中一个设备队列上来执行,如检索的graphics和presentation队列
        // 每个命令对象池只能分配在单一类型的队列上提交的命令缓冲区,也就是要分配的命令需要与队列类型一致,需要记录绘制的命令,这就说明为什么要选择图形队列簇的原因
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        // 创建命令缓冲区初始化
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

        // 创建命令缓冲区
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics command pool!");
        }
    }

    // 创建深度资源
    void createDepthResources()
    {
        // 候选格式列表中 根据期望值的降序原则,检测第一个得到支持的格式
        VkFormat depthFormat = findDepthFormat();

        // 创建图像
        createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, 
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
        // 深度图像视图
        depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

        // 未定义的布局可以作为初始布局,因为深度图像内容无关紧要.需要在 transitionImageLayout 中更新一些逻辑使用正确的子资源
        transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

    // 查找深度格式. 根据期望值的降序原则,检测第一个得到支持的格式
    VkFormat findDepthFormat()
    {
        return findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    // 候选格式列表中 根据期望值的降序原则,检测第一个得到支持的格式
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates)
        {
            /*
            VkFormatProperties 结构体包含三个字段：
                linearTilingFeatures: 使用线性平铺格式
                optimalTilingFeatures: 使用最佳平铺格式
                bufferFeatures: 支持缓冲区
            */
            VkFormatProperties props;
            // 支持的格式依赖于所使用的 tiling mode平铺模式和具体的用法,所以必须包含这些参数.可以使用 vkGetPhysicalDeviceFormatProperties 函数查询格式的支持
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            // 
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    // 判断所选择的深度格式是否包含模版组件
    bool hasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    // 创建加载纹理图片
    void createTextureImage()
    {
        int texWidth, texHeight, texChannels;
        // STBI_rgb_alpha值强制加载图片的alpha通道,即使它本身没有alpha,但是这样做对于将来加载其他的纹理的一致性非常友好,像素在STBI_rgba_alpha的情况下逐行排列,每个像素4个字节
        stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);// 使用文件的路径和通道的数量作为参数加载图片
        VkDeviceSize imageSize = (uint64_t)texWidth * texHeight * 4;// 图片的像素值

        if (!pixels)
        {
            throw std::runtime_error("failed to load texture image!");
        }

        // 图片像素缓冲区
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory; // 图片像素存储
        createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        // 直接从库中加载的图片中拷贝像素到缓冲区
        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device, stagingBufferMemory);

        stbi_image_free(pixels); // 清理原图像的像素数据

        // 创建贴图图像
        createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, 
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

        // 处理布局变换
        transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        // 拷贝暂存缓冲区到贴图图像
        copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        // 变换来准备着色器访问
        transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(device, stagingBuffer, nullptr);   // 清理暂存缓冲区
        vkFreeMemory(device, stagingBufferMemory, nullptr);// 清理分配的内存

    }

    // 创建图像视图访问纹理图像
    void createTextureImageView()
    {
        textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
    }


    /* createTextureSampler
    需要注意的是采样器没有任何地方引用VkImage
    采样器是一个独特的对象,它提供了从纹理中提取颜色的接口
    它可以应用在任何期望的图像中,无论是1D,2D,或者是3D.也与之前很多旧的API是不同的,后者将纹理图像与过滤器混合成单一状态
    */
    // 创建配置采样器对象
    void createTextureSampler()
    {
        // 指定将要应用的过滤器和变换
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;// 指定轴向使用的寻址模式,在这里称为U代替X,这是纹理空间坐标的约定
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;// 指定轴向使用的寻址模式,在这里称为V代替Y,这是纹理空间坐标的约定
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;// 指定轴向使用的寻址模式,在这里称为W代替Z,这是纹理空间坐标的约定
        /*寻址模式有以下几种:
            VK_SAMPLER_ADDRESS_MODE_REPEAT：当超过图像尺寸的时候采用循环填充
            VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT：与循环模式类似,但是当超过图像尺寸的时候,它采用反向镜像效果
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE：当超过图像尺寸的时候,采用边缘最近的颜色进行填充
            VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TOEDGE：与边缘模式类似,但是使用与最近边缘相反的边缘进行填充
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER：当采样超过图像的尺寸时,返回一个纯色填充
        */
        samplerInfo.anisotropyEnable = VK_TRUE;// 指定是否使用各向异性过滤器.没有理由不使用该特性,除非性能是一个问题
        // 限制可用于计算最终颜色的纹素采样的数量.低的数值会得到好性能,但会得到差的质量.当前没有任何的图形硬件可以使用超过16个采样器,因为超过16个采样器的差异可以忽略不计
        samplerInfo.maxAnisotropy = 16;
        // 指定采样范围超过图像时候返回的颜色,与之对应的是边缘寻址模式.可以用float或int返回黑色,白色或透明度.但是不能指定任意颜色
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        // unnormalizedCoordinates指定使用的坐标系统,用于访问图像的纹素.如果字段为VK_TRUE,意味着可以简单的使用坐标范围为(0,texWidth)和(0,texHeight)
        // 如果使用VK_FALSE,意味着每个轴向纹素访问使用[(0,1)范围.真实的应用程序总是使用归一化的坐标.因为这样可以使用完全相同坐标的不同分辨率的纹理
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        // 关闭比较功能；如果开启比较功能,那么纹素首先和值进行比较,并且比较后的值用于过滤操作,主要用在阴影纹理映射的 percentage-closer filtering 即百分比近似过滤器
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        // 创建配置采样器对象
        if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    // 创建图像视图
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
    {
        // 创建图像视图初始化结构
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;// 用于描述图像数据该被如何解释
        viewInfo.format = format;
        // subresourceRange用于描述图像的使用目标是什么,以及可以被访问的有效区域.图像将会作为color targets,没有任何mipmapping levels 或是多层 multiple layers
        // 如果在编写沉浸式的3D应用程序,比如VR,就需要创建支持多层的交换链.并且通过不同的层为每一个图像创建多个视图,以满足不同层的图像在左右眼渲染时对视图的需要
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        // 创建图像视图
        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture image view!");
        }

        return imageView;
    }

    // 创建贴图图像
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
                     VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
    {
        // 图片初始化
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        // 指定图像类型,告知Vulkan采用什么样的坐标系在图像中采集纹素.它可以是1D,2D和3D图像.1D图像用于存储数组数据或者灰度图,2D图像主要用于纹理贴图,3D图像用于存储立体纹素
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format; // Vulkan支持多种图像格式,但无论如何要在缓冲区中为纹素应用与像素一致的格式,否则拷贝操作会失败
        imageInfo.tiling = tiling;
        /*
        tiling字段可以设定两者之一：
            VK_IMAGE_TILING_LINEAR: 纹素基于行主序的布局,如pixels数组
            VK_IMAGE_TILING_OPTIMAL: 纹素基于具体的实现来定义布局,以实现最佳访问
          与图像布局不同的是,tiling模式不能在之后修改.如果需要在内存图像中直接访问纹素,必须使用VK_IMAGE_TILING_LINEAR.
          将会使用暂存缓冲区代替暂存图像,所以这部分不是很有必要.为了更有效的从shader中访问纹素,会使用VK_IMAGE_TILING_OPTIMAL
        */

        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        /*
        图像的initialLayout字段,有两个可选的值：
            VK_IMAGE_LAYOUT_UNDEFINED: GPU不能使用,第一个变换将丢弃纹素.
            VK_IMAGE_LAYOUT_PREINITIALIZED: GPU不能使用,但是第一次变换将会保存纹素.
        */

        // 图像将会被用作缓冲区拷贝的目标,所以应该设置作为传输目的地.还希望从着色器中访问图像对mesh进行着色,因此具体的usage还要包括VK_IMAGE_USAGE_SAMPLED_BIT
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;// 多重采样,这里仅仅适用于作为附件的图像,所以坚持一个采样数值
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;// 因为图像会在一个队列簇中使用：支持图形或者传输操作

        // 创建图形
        if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        // 在真实的生产环境中的应用程序里,不建议为每个缓冲区调用vkAllocateMemory分配内存
        // 内存分配的最大数量受到maxMemoryAllocationCount物理设备所限,即使像NVIDIA GTX1080这样的高端硬件上,也只能提供4096的大小
        // 同一时间,为大量对象分配内存的正确方法是创建一个自定义分配器,许多函数中用到的偏移量offset,将一个大块的可分配内存区域划分为多个可分配内存块,提供缓冲区使用
        if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate image memory!");
        }

        // 为图像工作分配内存
        vkBindImageMemory(device, image, imageMemory, 0);
    }

    // 变换图像布局
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        /*
        通常主流的做法用于处理图像变换是使用 image memory barrier.
        管线的屏障通常用于访问资源时进行同步,也类似缓冲区在读操作之前完成写入操作,也可以用于图像布局的变换以及在使用VK_SHARING_MODE_EXCLUSIVE模式情况下,传输队列簇宿主的变换
        缓冲区有一个等价的 buffer memory barrier
        */

        // 图形存储屏障
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;// 如果针对传输队列簇的宿主使用屏障,需要设置队列簇的索引.如不关心,则必须设置VK_QUEUE_FAMILY_IGNORED(不是默认值)
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;// 如果针对传输队列簇的宿主使用屏障,需要设置队列簇的索引.如不关心,则必须设置VK_QUEUE_FAMILY_IGNORED(不是默认值)
        barrier.image = image;// 指定受到影响的图像

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            // 指定受到影响的图像特定区域
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            // 判断所选择的深度格式是否包含模版组件
            if (hasStencilComponent(format))
            {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            // srcAccessMask|dstAccessMask 屏障主要用于同步目的,所以必须在应用屏障前指定哪一种操作类型及涉及到的资源,同时要指定哪一种操作及资源必须等待屏障
            // 必须这样做尽管使用vkQueueWaitIdle人为的控制同步.正确的值取决于旧的和新的布局,所以知道了要使用的变换,就可以回到布局部分
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else
        {
            throw std::invalid_argument("unsupported layout transition!");
        }

        // 所有类型的管线屏障都使用同样的函数提交
        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
        /* vkCmdPipelineBarrier
        所有类型的管线屏障都使用同样的函数提交.命令缓冲区参数后的第一个参数指定管线的哪个阶段,应用屏障同步之前要执行的前置操作
        第二个参数指定操作将在屏障上等待的管线阶段.在屏障之前和之后允许指定管线阶段取决于在屏障之前和之后如何使用资源
        允许的值列在规范的table表格中.
        比如在屏障之后从uniform中读取,指定使用VK_ACCESS_UNIFORM_READ_BIT以及初始着色器从uniform读取作为管线阶段,例如VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
        为这种类型的指定非着色器管线阶段是没有意义的,并且当指定和使用类型不匹配的管线阶段时候,validation layer 将会提示警告信息
        第三个参数可以设置为0或者VK_DEPENDENCY_BY_REGION_BIT.后者将屏障变换为每个区域的状态.这意味着,例如,允许已经写完资源的区域开始读的操作,更加细的粒度
        最后三个参数引用管线屏障的数组,有三种类型,第一种 memory barriers,第二种, buffer memory barriers, 和 image memory barriers
        需要注意的是这里没有使用VkFormat参数,但是会在深度缓冲区中使用它做一些特殊的变换
        */

        // 结束单个命令集
        endSingleTimeCommands(commandBuffer);
    }

    // 缓冲区拷贝到图像
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        // 命令缓冲区记录
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        // 指定拷贝具体哪一部分到图像的区域
        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, 1 };

        // 缓冲区拷贝到图像的操作
        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        // 结束单次命令集
        endSingleTimeCommands(commandBuffer);
    }

    // 加载模型 tinyobjloader库
    void loadModel()
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, MODEL_PATH.c_str()))
        {
            throw std::runtime_error(err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

        for (const auto& shape : shapes)
        {
            // 给顶点索引加入编号&给顶点集合加入值&给纹理赋值
            for (const auto& index : shape.mesh.indices)
            {
                Vertex vertex = {};

                vertex.pos =
                {
                    attrib.vertices[(uint64_t)3 * index.vertex_index + 0],
                    attrib.vertices[(uint64_t)3 * index.vertex_index + 1],
                    attrib.vertices[(uint64_t)3 * index.vertex_index + 2]
                };

                vertex.texCoord =
                {  // 本项目中,使用坐标从左上角0,0到右下角的1,1来映射纹理,从而简单的填充矩形.在这里可以尝试各种坐标.可以尝试使用低于0或者1以上的坐标来查看寻址模式的不同表现
                   // Vulkan的纹理坐标的起点是左上角,而OBJ格式则是左下角.通过反转纹理坐标的垂直分量来解决这个问题
                    attrib.texcoords[(uint64_t)2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[(uint64_t)2 * index.texcoord_index + 1]
                };

                vertex.color = { 1.0f, 1.0f, 1.0f };

                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                // 把这个顶点索引提交给顶点索引集合
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

    // 画球
    void loadModel2()
    {
        float M_PI = 3.14159265359;
        float R = 0.7f;//球的半径
        int statck = 100;//statck：切片----把球体横向切成几部分
        float statckStep = (float)(M_PI / statck);//单位角度值
        int slice = 300;//纵向切几部分
        float sliceStep = (float)(M_PI / slice);//水平圆递增的角度

        float r0, r1, x0, x1, y0, y1, z0, z1; //r0、r1为圆心引向两个临近切片部分表面的两条线 (x0,y0,z0)和(x1,y1,z1)为临近两个切面的点
        float alpha0 = 0, alpha1 = 0; //前后两个角度
        float beta = 0; //切片平面上的角度
        std::unordered_map<Vertex, uint32_t> uniqueVertices = {};
        std::vector<Vertex> allvertices;
        //外层循环
        for (int i = 0; i < statck; i++)
        {
            alpha0 = (float)(-M_PI / 2 + (i * statckStep));
            alpha1 = (float)(-M_PI / 2 + ((i + 1) * statckStep));
            y0 = (float)(R * std::sin(alpha0));
            r0 = (float)(R * std::cos(alpha0));
            y1 = (float)(R * std::sin(alpha1));
            r1 = (float)(R * std::cos(alpha1));

            //循环每一层圆
            for (int j = 0; j <= (slice * 2); j++)
            {
                beta = j * sliceStep;
                x0 = (float)(r0 * std::cos(beta));
                z0 = -(float)(r0 * std::sin(beta));
                x1 = (float)(r1 * std::cos(beta));
                z1 = -(float)(r1 * std::sin(beta));

                Vertex vertex = {};
                vertex.pos = { x0, y0, z0 };
                vertex.texCoord = { 0.f,0.f };
                vertex.color = { 1.f, 0.0f, 0.0f };

                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(allvertices.size());
                    allvertices.push_back(vertex);
                }

                //indices.push_back(uniqueVertices[vertex]);

                Vertex vertex1 = {};
                vertex1.pos = { x1, y1, z1 };
                vertex1.texCoord = { 0.f,0.f };
                vertex1.color = { 0.f, 1.0f, 0.0f };

                if (uniqueVertices.count(vertex1) == 0)
                {
                    uniqueVertices[vertex1] = static_cast<uint32_t>(allvertices.size());
                    allvertices.push_back(vertex1);
                }

                //indices.push_back(uniqueVertices[vertex1]);
            }
        }

        std::unordered_map<Vertex, uint32_t> uniqueVerticesall = {};
        // 给球拆成3个顶点一组并给每个顶点加Normal(本来是每个三角形的后两点和下一个三角形的前两点共点，改为不共点，因为要计算每个点的normal)
        for (size_t i = 2; i < allvertices.size(); i++)
        {
            Vertex vertex0 = {};
            vertex0.pos = allvertices[i - 2].pos;
            vertex0.texCoord = allvertices[i - 2].texCoord;
            vertex0.color = { 0.5f, 0.5f, 0.5f };

            Vertex vertex1 = {};
            vertex1.pos = allvertices[i - 1].pos;
            vertex1.texCoord = allvertices[i - 1].texCoord;
            vertex1.color = { 0.5f, 0.5f, 0.5f };

            Vertex vertex2 = {};
            vertex2.pos = allvertices[i].pos;
            vertex2.texCoord = allvertices[i].texCoord;
            vertex2.color = { 0.5f, 0.5f, 0.5f };

            // 求这个面的法线
            glm::vec3 vec1 = vertex0.pos - vertex1.pos;
            glm::vec3 vec2 = vertex0.pos - vertex2.pos;
            glm::vec3 normal = glm::normalize(glm::cross(vec1, vec2));
            vertex0.normal = normal;
            vertex1.normal = normal;
            vertex2.normal = normal;

            if (i % 2 == 0)
            {
                // 求这个面的法线
                glm::vec3 vec1 = vertex0.pos - vertex1.pos;
                glm::vec3 vec2 = vertex0.pos - vertex2.pos;
                glm::vec3 normal = glm::normalize(glm::cross(vec1, vec2));
                vertex0.normal = normal;
                vertex1.normal = normal;
                vertex2.normal = normal;

                if (uniqueVerticesall.count(vertex0) == 0)
                {
                    uniqueVerticesall[vertex0] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex0);
                }
                indices.push_back(uniqueVerticesall[vertex0]);

                if (uniqueVerticesall.count(vertex1) == 0)
                {
                    uniqueVerticesall[vertex1] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex1);
                }
                indices.push_back(uniqueVerticesall[vertex1]);

                if (uniqueVerticesall.count(vertex2) == 0)
                {
                    uniqueVerticesall[vertex2] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex2);
                }
                indices.push_back(uniqueVerticesall[vertex2]);

            }
            else
            {
                // 求这个面的法线
                glm::vec3 vec1 = vertex0.pos - vertex1.pos;
                glm::vec3 vec2 = vertex0.pos - vertex2.pos;
                glm::vec3 normal = glm::normalize(glm::cross(vec1, -vec2));
                vertex0.normal = normal;
                vertex1.normal = normal;
                vertex2.normal = normal;

                if (uniqueVerticesall.count(vertex2) == 0)
                {
                    uniqueVerticesall[vertex2] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex2);
                }
                indices.push_back(uniqueVerticesall[vertex2]);

                if (uniqueVerticesall.count(vertex1) == 0)
                {
                    uniqueVerticesall[vertex1] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex1);
                }
                indices.push_back(uniqueVerticesall[vertex1]);

                if (uniqueVerticesall.count(vertex0) == 0)
                {
                    uniqueVerticesall[vertex0] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex0);
                }
                indices.push_back(uniqueVerticesall[vertex0]);
            }
        }
    }

    // 创建顶点缓冲区
    void createVertexBuffer()
    {
        // 计算缓冲区大小直接用sizeof
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        VkBuffer stagingBuffer; // 划分临时缓冲区用来映射、拷贝顶点数据
        VkDeviceMemory stagingBufferMemory; // 临时缓冲区内存
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data); // 内存映射
        memcpy(data, vertices.data(), (size_t)bufferSize);                 // 拷贝数据
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

        // 拷贝临时缓冲区到顶点缓冲区
        copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);   // 清除顶点缓冲区
        vkFreeMemory(device, stagingBufferMemory, nullptr);// 清除顶点缓冲区记录
    }

    // 创建顶点索引缓冲区
    void createIndexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer stagingBuffer; // 临时缓冲区
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

        copyBuffer(stagingBuffer, indexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr); // 清除顶点索引缓冲区
        vkFreeMemory(device, stagingBufferMemory, nullptr); // 清除顶点索引缓冲区记录
    }

    // 创建全局缓冲区 ubo
    void createUniformBuffer()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer, uniformBufferMemory);
    }

    // 创建描述符集合
    void createDescriptorPool()
    {
        // 明确需要使用的描述符池包含的描述符类型与数量
        std::array<VkDescriptorPoolSize, 2> poolSizes = {};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = 1;
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = 1;

        // 创建描述符池初始化
        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = 1;// 指定最大的描述符集合的分配数量

        // 创建描述符池
        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    // 创建描述符设置
    void createDescriptorSet()
    {
        // 存储描述符集合的句柄
        VkDescriptorSetLayout layouts[] = { descriptorSetLayout };
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = layouts;

        // 分配描述符集合
        if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor set!");
        }

        // 指定缓冲区和描述符内部包含的数据的区域
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uniformBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;

        // 描述符集合
        std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSet; // 描述符设置
        descriptorWrites[0].dstBinding = 0; //  uniform buffer 绑定的索引设定为0.描述符可以是数组,所以需要指定要更新的数组索引.在这里没有使用数组,所以简单的设置为0
        descriptorWrites[0].dstArrayElement = 0;// 起始索引
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // 
        descriptorWrites[0].descriptorCount = 1; // 描述多少描述符需要被更新
        descriptorWrites[0].pBufferInfo = &bufferInfo;// 指定描述符引用的缓冲区数据

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSet;
        descriptorWrites[1].dstBinding = 1; // ubiform layout binding = 1
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo; // 用于指定描述符引用的图像数据

        // 描述符的配置更新
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    // 创建缓冲区
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {
        // 创建buffer初始化
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size; // 指定缓冲区字节大小.计算缓冲区每个顶点数据的字节大小可以直接使用sizeof
        bufferInfo.usage = usage;// 指定缓冲区的数据将如何使用.可以使用位操作指定多个使用目的
        // 像交换链中的图像一样,缓冲区也可以由特定的队列簇占有或者多个同时共享.在本项目缓冲区将会被用于图形队列,所以使用独占访问模式exclusive mode
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        // 创建buffer
        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create buffer!");
        }

        // 给缓冲区分配内存
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);// 

        // 分配内存
        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size; // 大小
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties); // 分配内存编号

        // 在真实的生产环境中的应用程序里,不建议为每个缓冲区调用vkAllocateMemory分配内存
        // 内存分配的最大数量受到maxMemoryAllocationCount物理设备所限,即使像NVIDIA GTX1080这样的高端硬件上,也只能提供4096的大小
        // 同一时间,为大量对象分配内存的正确方法是创建一个自定义分配器,许多函数中用到的偏移量offset,将一个大块的可分配内存区域划分为多个可分配内存块,提供缓冲区使用
        // 可以自己实现一个灵活的内存分配器,或者使用GOUOpen提供的VulkanMemoryAllocator库.然而,对于本项目,可以做到为每个资源使用单独的分配,因为不会触达任何资源限制条件

        // 如果内存分配成功,使用vkBindBufferMemory函数将内存关联到缓冲区 (先分配)
        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        // 将内存关联到缓冲区 （再关联）
        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    // 命令缓冲区的记录功能
    VkCommandBuffer beginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        /*
        level参数指定分配的命令缓冲区的主从关系
            VK_COMMAND_BUFFER_LEVEL_PRIMARY: 可以提交到队列执行,但不能从其他的命令缓冲区调用
            VK_COMMAND_BUFFER_LEVEL_SECONDARY: 无法直接提交,但是可以从主命令缓冲区调用
        */
        allocInfo.commandPool = commandPool; // 命令缓冲区通过vkAllocateCommandBuffers函数分配
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        // 指定命令缓冲区在使用过程中的一些具体信息
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        /*
        flags标志位参数用于指定如何使用命令缓冲区.可选的参数类型如下:
            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: 命令缓冲区将在执行一次后立即重新记录
            VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: 这是一个辅助缓冲区,它限制在在一个渲染通道中
            VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: 命令缓冲区也可以重新提交,同时它也在等待执行
        */

        // 开始命令缓冲区
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    // 结束单次命令集
    void endSingleTimeCommands(VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        // 队列提交和同步
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1; // 提交执行命令缓冲区的数量
        submitInfo.pCommandBuffers = &commandBuffer; // 具体被提交执行的命令缓冲区

        // 
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    // 从一个缓冲区拷贝数据到另一个缓冲区
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        // 命令缓冲区记录
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion = {};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);// 拷贝缓冲区

        endSingleTimeCommands(commandBuffer);
    }

    // 分配不同类型的内存  每种类型的内存根据所允许的操作和特性均不相同.需要结合缓冲区与应用程序实际的需要找到正确的内存类型使用
    // typeFilter参数将以位的形式代表适合的内存类型
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties; // 有效的内存类型
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties); // 遍历有效的内存类型

        // 为缓冲区找到合适的内存类型
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {   // typeFilter参数将以位的形式代表适合的内存类型.这意味着通过简单的迭代内存属性集合,并根据需要的类型与每个内存属性的类型进行AND操作,判断是否为1
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    // 创建命令缓冲区
    void createCommandBuffers()
    {
        // 为命令缓冲区集设置大小
        commandBuffers.resize(swapChainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        for (size_t i = 0; i < commandBuffers.size(); i++)
        {
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

            vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

            // 渲染通道初始化
            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass; // 绑定到对应附件的渲染通道
            renderPassInfo.framebuffer = swapChainFramebuffers[i]; // 为每一个交换链的图像创建帧缓冲区,并指定为颜色附件
            renderPassInfo.renderArea.offset = { 0, 0 }; // 数定义渲染区域的大小
            renderPassInfo.renderArea.extent = swapChainExtent; // 渲染区域定义着色器加载和存储将要发生的位置.区域外的像素将具有未定的值.为了最佳的性能它的尺寸应该与附件匹配

            // 
            std::array<VkClearValue, 2> clearValues = {};
            clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f }; // 为了简化操作,定义了clear color为100%黑色
            clearValues[1].depthStencil = { 1.0f, 0 };

            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            // 开启渲染通道
            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            /*
            第一个参数是记录该命令的命令缓冲区.第二个参数指定传递的渲染通道的具体信息.最后的参数控制如何提供render pass将要应用的绘制命令.它使用以下数值任意一个:
                VK_SUBPASS_CONTENTS_INLINE: 渲染过程命令被嵌入在主命令缓冲区中,没有辅助缓冲区执行
                VK_SUBPASS_CONTENTS_SECONDARY_COOMAND_BUFFERS: 渲染通道命令将会从辅助命令缓冲区执行
            */

            // 绑定图形管线 第一个参数 记录该命令的命令缓冲区,第二个参数指定具体管线类型,第三个参数graphics 或者 compute pipeline
            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

            VkBuffer vertexBuffers[] = { vertexBuffer };
            VkDeviceSize offsets[] = { 0 };
            // 绑定顶点缓冲区,像之前设置一样,除了命令缓冲区之外,前两参数指定要为其指定的顶点缓冲区的偏移量和数量,后两参数指定将要绑定的顶点缓冲区的数组及开始读数据的起始偏移量
            vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32); // 绑定索引缓冲区

            // 将描述符集合绑定到实际的着色器的描述符中
            vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

            // 前两个参数指定索引的数量和几何instance数量.没有使用instancing,所以指定1.
            // 索引数表示被传递到顶点缓冲区中的顶点数量.下一个参数指定索引缓冲区的偏移量,使用1将会导致图形卡在第二个索引处开始读取
            // 倒数第二个参数指定索引缓冲区中添加的索引的偏移.最后一个参数指定instancing偏移量,这里没使用该特性
            vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

            // 渲染通道执行完绘制,可以结束渲染作业
            vkCmdEndRenderPass(commandBuffers[i]);

            // 并停止记录命令缓冲区的工作
            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }

    // 创建信号对象
    void createSemaphores()
    {
        // 在当前版本中 除了type不需要赋其他值
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        // 创建信号量对象
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphores!");
        }
    }

    // 更新窗体显示 这个函数会在每次循环创建新的变换矩阵确保几何图形旋转,移动,放缩等变换
    void updateUniformBuffer()
    {
        // 初始化摄像机
        static glm::vec3 position = glm::vec3(2.0f, 2.f, 2.f);// 初始摄像机位置
        static glm::vec3 centre = glm::vec3(0.0f, 0.0f, 0.0f); // 模型中心
        static float FoV = 45.0f;
        glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);// 仰角

        // 鼠标键盘操作
        glfwSetWindowUserPointer(window, this);
        glfwSetMouseButtonCallback(window, OnMouseButton);
        glfwSetCursorPosCallback(window, OnMouseMove);
        glfwSetKeyCallback(window, OnKey);
        glfwSetScrollCallback(window, OnMouseScroll);

        double speed = 0.001f;
        float  i_xpos = g_mouse_delta.x;
        float  i_ypos = g_mouse_delta.y;
        position = glm::vec3(position.x * std::cos(-speed * i_xpos) - position.y * std::sin(-speed * i_xpos),
                             position.x * std::sin(-speed * i_xpos) + position.y * std::cos(-speed * i_xpos), position.z);
        position = glm::vec3(position.x, position.y * std::cos(speed * i_ypos) - position.z * std::sin(speed * i_ypos),
                             position.y * std::sin(speed * i_ypos) + position.z * std::cos(speed * i_ypos));
        if (g_is_scroll_delta && FoV - g_scroll_delta.y > 0.f && FoV - g_scroll_delta.y < 90.f)
        {
            FoV = FoV - g_scroll_delta.y;
            g_is_scroll_delta = false;
        }

        UniformBufferObject ubo = {};
        ubo.view = glm::lookAt(position, centre, up); // 摄像机位置/中心位置/上下仰角
        // 选择使用FOV为45度的透视投影.其他参数是宽高比,近裁剪面和远裁剪面.重要的是使用当前的交换链扩展来计算宽高比,以便在窗体调整大小后参考最新的窗体宽度和高度
        ubo.proj = glm::perspective(glm::radians(FoV), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1; // GLM最初是为OpenGL设计的,它的裁剪坐标的Y是反转的.修正该问题的最简单的方法是在投影矩阵中Y轴的缩放因子反转.如果不这样做图像会被倒置

        // 加一个测试用的点光
        ubo.testlight = glm::vec3(10.f, 10.f, 10.f);

        // 现在定义了所有的变换,所以将UBO中的数据复制到uniform缓冲区.除了没有暂存缓冲区,这与顶点缓冲区的操作完全相同.
        // 使用ubo将并不是经常变化的值传递给着色器是非常有效的方式.相比传递一个更小的数据缓冲区到着色器中,更有效的方式是使用常量
        void* data;
        vkMapMemory(device, uniformBufferMemory, 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, uniformBufferMemory);
    }

    // 绘制帧 从交换链获取图像,在帧缓冲区中使用作为附件的图像来执行命令缓冲区中的命令,将图像返还给交换链最终呈现
    void drawFrame()
    {
        // vkAcquireNextImageKHR前两参数是希望获取到图像的逻辑设备和交换链.第三参数指定获取有效图像的操作timeout,单位纳秒.使用64位无符号最大值禁止timeout
        // 后两参数指定使用的同步对象,当presentation完成图像的呈现后会使用该对象发起信号,这是开始绘制的时间点,它可以指定一个信号量semaphore或栅栏或两者,这里用imageAvailableSemaphore
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            // 重新创建交换链
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        // 队列提交和同步
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // 等待缓冲区信号集
        VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; // 数组对应pWaitSemaphores中具有相同索引的信号
        submitInfo.waitSemaphoreCount = 1; // 等待信号的数量
        submitInfo.pWaitSemaphores = waitSemaphores; // 等待哪些具体信号
        submitInfo.pWaitDstStageMask = waitStages; // 指定pWaitSemaphores中具有相同索引的信号

        submitInfo.commandBufferCount = 1; // 指定被执行的命令缓冲区信号数量
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex]; // 指定哪个命令缓冲区被实际提交执行

        // 渲染结束信号集
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
        submitInfo.signalSemaphoreCount = 1; // 指定被指定的命令缓冲区信号数量
        submitInfo.pSignalSemaphores = signalSemaphores;// 指定具体被提交指定的命令缓冲区

        // 向图像队列提交命令缓冲区
        // 当开销负载比较大的时候,处于效率考虑,函数可以持有VkSubmitInfo结构体数组
        // 最后一个参数引用了一个可选的栅栏,当命令缓冲区执行完毕时候它会被发送信号
        // 这里使用信号量进行同步,所以需要传递VK_NULL_HANDLE
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        // 将结果提交到交换链,使其最终显示在屏幕上
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        // 指定在进行presentation之前要等待的信号量
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        // 交换链集合
        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        // 提交请求呈现交换链中的图像
        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        vkQueueWaitIdle(presentQueue);
    }

    // 在将代码传递给渲染管线之前,必须将其封装到VkShaderModule对象中
    // 创建一个辅助函数createShaderModule实现该逻辑
    VkShaderModule createShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        // 创建shaderModule
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    // 选择交换链表面格式
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        // 最理想的情况是surface没有设置任何偏向性的格式,
        // 这个时候Vulkan会通过仅返回一个VkSurfaceFormatKHR结构表示,
        // 且该结构的format成员设置为VK_FORMAT_UNDEFINED.
        if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
        {
            return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
        }

        // 如果不能自由的设置格式,那么可以通过遍历列表设置具有偏向性的组合
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    // 查询最合适的PresentMode类型
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
    {
        /*
        VK_PRESENT_MODE_IMMEDIATE_KHR: 应用程序提交的图像被立即传输到屏幕呈现,这种模式可能会造成撕裂效果
        VK_PRESENT_MODE_FIFO_KHR:交换链被看作一个队列,当显示内容需要刷新的时候,显示设备从队列的前面获取图像,并且程序将渲染完成的图像插入队列的后面.如果队列是满的程序会等待
           这种规模与视频游戏的垂直同步很类似.显示设备的刷新时刻被设为"垂直中断"
        VK_PRESENT_MODE_FIFO_RELAXED_KHR: 该模式与上一个模式略有不同的地方为,
           如果应用程序存在延迟,即接受最后一个垂直同步信号时队列空了,将不会等待下一个垂直同步信号,而是将图像直接传送.这样做可能导致可见的撕裂效果
        VK_PRESENT_MODE_MAILBOX_KHR: 这是第二种模式的变种.当交换链队列满的时候,选择新的替换旧的图像,从而替代阻塞应用程序的情形
           这种模式通常用来实现三重缓冲区,与标准的垂直同步双缓冲相比,它可以有效避免延迟带来的撕裂效果
        */

        // 逻辑上看仅仅VR_PRESENT_MODE_FIFO_KHR模式保证可用性
        VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

        // 三级缓冲是一个非常好的策略.它允许避免撕裂,同时仍然保持相对低的延迟,通过渲染尽可能新的图像,直到接受垂直同步信号
        // 循环列表,检查它是否可用
        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
            else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                bestMode = availablePresentMode;
            }
        }

        return bestMode;
    }

    // 交换范围 指交换链图像的分辨率
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        // 分辨率的范围被定义在VkSurfaceCapabilitiesKHR结构体中
        // 通过设置currentExtent成员的width和height来匹配窗体的分辨率
        // 一些窗体管理器允许不同的设置,意味着将currentExtent的width和height设置为特殊的数值表示:uint32_t的最大值
        // 在这种情况下,参考窗体minImageExtent和maxImageExtent选择最匹配的分辨率

        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetWindowSize(window, &width, &height);// 获取窗体的大小

            VkExtent2D actualExtent =
            {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            // max和min函数用于将width和height限制在实际支持的minimum和maximum范围中
            // 在<algorithm>头文件
            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    // 获取设备交换链信息
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
    {
        SwapChainSupportDetails details;
        // 查询设备的Surface
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        // 获取format数量
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        // 将format句柄赋给formats数组
        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        // 查询设备的present数量
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        // 将present句柄赋给presentModes数组
        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    // 检查这个设备是否适合要执行的操作
    bool isDeviceSuitable(VkPhysicalDevice device)
    {
        // 查找设备列队簇(就是找到所有的显卡)
        QueueFamilyIndices indices = findQueueFamilies(device);

        // 额外的检查逻辑
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        // 验证交换链是否有足够的支持
        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        // 判断显卡能够执行哪些操作supportedFeatures.samplerAnisotropy
        VkPhysicalDeviceFeatures supportedFeatures;
        // 查询这个设备支持的功能
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        // 尝试查询交换链的支持是在验证完扩展有效性之后进行
        return indices.isComplete() && extensionsSupported && supportedFeatures.samplerAnisotropy;
    }

    // 额外的检查逻辑
    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        // 获取队列数量
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        // 为队列的每个设备句柄赋给数组
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        // 获取每一个设备名字
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        // 如果设备名字列表不为空 则返回真
        return requiredExtensions.empty();
    }

    // 查找列队簇
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        // 获取一共有多少个显卡设备
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        // 声明vector数组 用来记录显卡句柄
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        // 将每一个显卡句柄赋值给vector数组
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        // 获取有效列队数
        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            // 查找具备graphics功能的队列簇
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }

            // 用于检查的核心代码是vkGetPhysicalDeviceSurfaceSupportKHR, 
            // 它将物理设备、队列簇索引和surface作为参数.在VK_QUEUE_GRAPHICS_BIT相同的循环体中添加函数的调用
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            // 查找具备presentation功能的队列簇
            if (queueFamily.queueCount > 0 && presentSupport)
            {
                indices.presentFamily = i; // 存储presentation队列簇的索引
            }

            if (indices.isComplete())
            {
                break;
            }

            i++;
        }

        return indices;
    }

    // 获取所需的扩展
    std::vector<const char*> getRequiredExtensions()
    {
        std::vector<const char*> extensions;

        unsigned int glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        for (unsigned int i = 0; i < glfwExtensionCount; i++)
        {
            extensions.push_back(glfwExtensions[i]);
        }

        if (enableValidationLayers)
        {
            extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }

        return extensions;
    }

    // 检测所有请求的layers是否可用
    bool checkValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr); // 列出所有可用的层

        // 给所有layer赋值
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        // 
        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                // 比较字符 如果不相等
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }

    // 读取文件
    static std::vector<char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    // debug回调
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj,
                                                        size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
    {
        std::cerr << "validation layer: " << msg << std::endl;

        return VK_FALSE;
    }
};

// 主函数
int main()
{
    Application app;

    try
    {
        app.run();
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
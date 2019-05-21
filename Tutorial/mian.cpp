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
// #include "vulkan/vulkan.hpp" // ���������÷�ʽ

const int WIDTH = 1280; // �����
const int HEIGHT = 720; // �����

const std::string MODEL_PATH = "models/chalet.obj"; // objģ��·��
const std::string TEXTURE_PATH = "textures/chalet.jpg"; // ����ͼƬ·��

// �쳣�����
const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };

// ����Ⱦͼ���ύ����Ļ�Ļ������ƣ����ֻ��Ƴ�Ϊ��������
const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
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

void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) 
{
    auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr) 
    {
        func(instance, callback, pAllocator);
    }
}

// ��������ĳ�����ԵĶ��д�����������ṹ�壬��������-1��ʾ"δ�ҵ�"
// ֧��graphics����ĵĶ��дغ�֧��presentation����Ķ��дؿ��ܲ���ͬһ����
struct QueueFamilyIndices
{
    int graphicsFamily = -1;
    int presentFamily = -1;

    bool isComplete() 
    {
        return graphicsFamily >= 0 && presentFamily >= 0;
    }
};

// ���������Ϊ�˲��Խ���������Ч����ԶԶ�����ģ���Ϊ�������ܺܺõ��봰��surface���ݡ�
// ����������ͬ��Ҳ��Ҫ�ܶ����ã�����������Ҫ�˽�һЩ�й����õ�ϸ��
// �������ṹ����ϸ��Ϣ
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

// ��������
struct Vertex
{
    glm::vec3 pos; // ����xyz
    glm::vec3 color; // ��ɫrgb
    glm::vec2 texCoord; // ��������UV

    // ��������� �������������������ݴ��ڴ���ص����ʡ����仰˵����ָ��������Ŀ֮��ļ���ֽ����Լ��Ƿ�ÿ������֮�����ÿ��instance֮���ƶ�����һ����Ŀ
    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;// �ƶ���ÿ����������һ��������Ŀ
        /*
        inputRate�������Ծ߱�һ��ֵ֮һ��
            VK_VERTEX_INPUT_RATE_VERTEX: �ƶ���ÿ����������һ��������Ŀ
            VK_VERTEX_INPUT_RATE_INSTANCE: ��ÿ��instance֮���ƶ�����һ��������Ŀ
        */

        return bindingDescription;
    }

    // �����������
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0; // binding����������Vulkanÿ���������ݵ���Դ
        attributeDescriptions[0].location = 0;// location����������vertex shader��Ϊ�����locationָ�������ɫ���У�locationΪ0����position������32bit����������
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;// format�������������Ե����͡��ø�ʽʹ������ɫ��ʽһ����ö��
        attributeDescriptions[0].offset = offsetof(Vertex, pos);// offset����ָ����ÿ���������ݶ�ȡ���ֽڿ��ƫ����,��һ�μ���һ��Vertex��position����(pos)��ƫ�������ֽ�������Ϊ0�ֽڡ�����ʹ��offsetof macro���Զ������

        attributeDescriptions[1].binding = 0; // binding����������Vulkanÿ��������ɫ���ݵ���Դ
        attributeDescriptions[1].location = 1;// location����������vertex shader��Ϊ�����locationָ�������ɫ���У�locationΪ1����color������32bit����������
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;// format�������������Ե�����
        attributeDescriptions[1].offset = offsetof(Vertex, color); // offset����ָ����ÿ����ɫ���ݶ�ȡ���ֽڿ��ƫ����

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const
    {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

// ��չstd��׼��
namespace std 
{
    template<> struct hash<Vertex> 
    {
        size_t operator()(Vertex const& vertex) const 
        {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

// ��ɫ������
struct UniformBufferObject
{
    glm::mat4 model;  // ģ�;���
    glm::mat4 view;   // ���������
    glm::mat4 proj;   // �������
};

// ������
class Application 
{
public:
    // ����
    void run()
    {
        initWindow(); // ��ʼ������
        initVulkan(); // ��ʼ��vulkan|��Ⱦ����
        mainLoop();   // ��ѭ����ͼ�λ��Ƶ���Ļ
        cleanup();    // �����Դ
    }

private:
    GLFWwindow* window; // ����ʵ��

    VkInstance instance; // vkʵ��
    VkDebugReportCallbackEXT callback; // debug�쳣���
    /*
     ��Ҫ��instance����֮��������������surface
     ��Ϊ����Ӱ�������豸��ѡ��
     ����surface�������VulkanҲ�Ƿ�ǿ�Ƶġ�
     Vulkan����������������ҪͬOpenGLһ������Ҫ��������surface
    */
    VkSurfaceKHR surface; // surface����Vulkan�봰��ϵͳ����������

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // �Կ�|Ӳ���豸
    VkDevice device; // ���һ���µ����Ա���洢�߼��豸���

    VkQueue graphicsQueue; // ��ͼ���о��
    VkQueue presentQueue;  // ��ʾ���о��

    VkSwapchainKHR swapChain; // ����������
    std::vector<VkImage> swapChainImages; // ������ͼ�� ͼ�񱻽�����������Ҳ���ڽ��������ٵ�ͬʱ�Զ��������Բ���Ҫ����������
    VkFormat swapChainImageFormat; // ������ͼ��任
    VkExtent2D swapChainExtent; // ��������չ
    std::vector<VkImageView> swapChainImageViews; // ����ͼ����ͼ�ľ����
    std::vector<VkFramebuffer> swapChainFramebuffers;// ������֡��������

    VkRenderPass renderPass; // ��Ⱦͨ��
    VkDescriptorSetLayout descriptorSetLayout; // ������������
    VkPipelineLayout pipelineLayout; // ���߲���
    VkPipeline graphicsPipeline; // ���ƹ���

    VkCommandPool commandPool; // �����

    VkImage depthImage; // ͼ����ȸ���
    VkDeviceMemory depthImageMemory;// ͼ����ȸ�����¼
    VkImageView depthImageView;// ͼ�������ͼ

    VkImage textureImage; // ����ͼƬ
    VkDeviceMemory textureImageMemory; // ����ͼƬ��¼
    VkImageView textureImageView; // ����ͼ����ͼ
    VkSampler textureSampler; // ���������

    std::vector<Vertex> vertices; // ���㼯��
    /*
    �Ƽ��ڵ����ڴ��з�������Դ���绺����������ʵ���ϣ�Ӧ�ø���һ��ϸ��������Nvidia���������򿪷��߽��齫���������(���㻺����������������)�洢������VkBuffer��
    ��������vkCmdBindVertexBuffers֮���������ʹ��ƫ����
    �ŵ����ڣ�����������£����ݻ���ӳ�ֵ����û��棬��Ϊ����������һ������������ͬһ����Ⱦ�����п��Ը���������ͬ�ڴ��Ķ����Դ�飬ֻҪˢ�����ݼ���
    �ü��ɳ�Ϊ��Ϊaliasing��һЩVulkan��������ȷ�ı�־ָ������������ͼ
    */
    std::vector<uint32_t> indices;// ������������
    VkBuffer vertexBuffer;// ���㻺����
    VkDeviceMemory vertexBufferMemory;// ���㻺������¼
    VkBuffer indexBuffer; // ����������
    VkDeviceMemory indexBufferMemory; // ������������¼

    VkBuffer uniformBuffer; // ͳһ��������
    VkDeviceMemory uniformBufferMemory;// ͳһ����������¼

    VkDescriptorPool descriptorPool; // ��������
    VkDescriptorSet descriptorSet; // ����������

    std::vector<VkCommandBuffer> commandBuffers; // ���������

    VkSemaphore imageAvailableSemaphore; // ��Ⱦ��ʼ�ź�
    VkSemaphore renderFinishedSemaphore; // ��Ⱦ�����ź�

    // ��ʼ������
    void initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // �������� ���óߴ�ͱ���
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Demo", nullptr, nullptr);

        glfwSetWindowUserPointer(window, this);
        glfwSetWindowSizeCallback(window, Application::onWindowResized);
    }

    // ��ʼ��vulkan|��Ⱦ����
    void initVulkan() 
    {
        createInstance();           // ����ʵ��
        setupDebugCallback();       // 
        createSurface();            // �����vulkanʵ������
        pickPhysicalDevice();       // ѡ���豸
        createLogicalDevice();      // �����߼��豸
        createSwapChain();          // ������
        createImageViews();         // ����ͼ����ͼ
        createRenderPass();         // ��Ⱦͨ��
        createDescriptorSetLayout();// �������������ò���
        createGraphicsPipeline();   // ͼ�ι���
        createCommandPool();        // ���������
        createDepthResources();     // �������ͼ��
        createFramebuffers();       // ֡������
        createTextureImage();       // ������������ͼƬ stb��
        createTextureImageView();   // ����ͼ����ͼ��������ͼ��
        createTextureSampler();     // �������ò���������
        loadModel();                // ����ģ�� tinyobjloader��
        createVertexBuffer();       // �������㻺����
        createIndexBuffer();        // ������������������
        createUniformBuffer();      // ����ͳһ������
        createDescriptorPool();     // ��������������
        createDescriptorSet();      // ������������������
        createCommandBuffers();     // �����������
        createSemaphores();         // �����źŶ���
    }

    // ��ѭ����ͼ�λ��Ƶ���Ļ
    void mainLoop()
    {
        while (!glfwWindowShouldClose(window)) 
        {
            glfwPollEvents();

            updateUniformBuffer(); // �����������ÿһ֡�д����µı任������ȷ������ͼ����ת���ƶ��������ȱ任
            drawFrame(); // ����֡
        }

        vkDeviceWaitIdle(device);
    }

    // �����������Դ
    void cleanupSwapChain()
    {
        vkDestroyImageView(device, depthImageView, nullptr);
        vkDestroyImage(device, depthImage, nullptr);
        vkFreeMemory(device, depthImageMemory, nullptr);// 

        // ɾ��֡������
        for (size_t i = 0; i < swapChainFramebuffers.size(); i++)
        {
            vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
        }

        // �������ڴ���������������
        vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

        vkDestroyPipeline(device, graphicsPipeline, nullptr); // ���ͼ�ι���
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr); // ����pipeline layout
        vkDestroyRenderPass(device, renderPass, nullptr); // ������Ⱦͨ������Ⱦͨ���������������������ڶ���ʹ�ã�������Ҫ���˳��׶ν�������

        // ͼ����ͼ��Ҫ��ȷ�Ĵ������̣������ڳ����˳���ʱ����Ҫ���һ��ѭ��ȥ����
        for (size_t i = 0; i < swapChainImageViews.size(); i++)
        {
            vkDestroyImageView(device, swapChainImageViews[i], nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);// �������豸ǰ���ٽ�����
    }

    // �����Դ
    void cleanup()
    {
        cleanupSwapChain(); // ���������

        vkDestroySampler(device, textureSampler, nullptr);// �������������
        vkDestroyImageView(device, textureImageView, nullptr);// ��������ͼ����ͼ

        vkDestroyImage(device, textureImage, nullptr);// �����ͼͼ��
        vkFreeMemory(device, textureImageMemory, nullptr);// �����ͼͼ���¼

        vkDestroyDescriptorPool(device, descriptorPool, nullptr); // �������������

        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        vkDestroyBuffer(device, uniformBuffer, nullptr);
        vkFreeMemory(device, uniformBufferMemory, nullptr);

        vkDestroyBuffer(device, indexBuffer, nullptr); // �����������������
        vkFreeMemory(device, indexBufferMemory, nullptr);// �������������������¼

        vkDestroyBuffer(device, vertexBuffer, nullptr); // ������㻺����
        vkFreeMemory(device, vertexBufferMemory, nullptr);// ������㻺������¼

        vkDestroySemaphore(device, renderFinishedSemaphore, nullptr); // �����Ⱦ�ź�
        vkDestroySemaphore(device, imageAvailableSemaphore, nullptr); // ���ͼ���ź�

        vkDestroyCommandPool(device, commandPool, nullptr);// �����������

        vkDestroyDevice(device, nullptr); // ����߼��豸��Դ
        DestroyDebugReportCallbackEXT(instance, callback, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr); // GLFWû���ṩר�õĺ�������surface,���Լ򵥵�ͨ��Vulkanԭʼ��API
        vkDestroyInstance(instance, nullptr); // ȷ��surface����������instance����֮ǰ���

        glfwDestroyWindow(window); // ���������Դ

        glfwTerminate();
    }

    // ���ڳߴ�仯
    static void onWindowResized(GLFWwindow* window, int width, int height) 
    {
        if (width == 0 || height == 0) return;

        Application * app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        app->recreateSwapChain(); // ���´���������
    }

    // ���´���������
    void recreateSwapChain()
    {
        vkDeviceWaitIdle(device);

        // �����������Դ
        cleanupSwapChain();

        createSwapChain(); // ����������
        createImageViews();// ����ͼ����ͼ
        createRenderPass();// ������Ⱦͨ��
        createGraphicsPipeline();// ����ͼ�����
        createDepthResources(); // �����ź���Դ
        createFramebuffers(); // ����֡������
        createCommandBuffers(); // �����������
    }

    // ����vkʵ��
    void createInstance() 
    {
        if (enableValidationLayers && !checkValidationLayerSupport())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        // ��ʼ������ʱ
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        // ����ʵ����ʼ��
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo; // ������ʱ����ʵ��

        // 
        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (enableValidationLayers) 
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else 
        {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create instance!");
        }
    }

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

    // GLFWû��ʹ�ýṹ�壬����ѡ��ǳ�ֱ�ӵĲ������������ú���
    // vulkan�ʹ��������
    void createSurface() 
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    // ѡ����ʾ�豸
    void pickPhysicalDevice() 
    {
        // ��ȡͼ�ο��б�
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        // ���һ���豸��û�� ֱ���׳��쳣
        if (deviceCount == 0) 
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        // ��������������¼����ͼ�ο��ľ��
        std::vector<VkPhysicalDevice> devices(deviceCount);
        // ����deviceCountͼ�ο�������devices���鸳���ֵ
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // �������е�ͼ�ο�
        for (const auto& device : devices) 
        {
            // �����鿨����Ҫ�� ����鿨�������ǰ�������ľ������ ����ѭ��
            // ��������ҵ�һ�����Ҫ��Ŀ��������Ϊ�����豸
            if (isDeviceSuitable(device))
            {
                physicalDevice = device;
                break;
            }
        }

        // ������������ͼ�ο�û�б���ֵ ���쳣
        if (physicalDevice == VK_NULL_HANDLE) 
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    // �����߼��豸
    void createLogicalDevice() 
    {
        /*
        ������Ҫ���VkDeviceQueueCreateInfo�ṹ��������ͬ���ܵĶ��С�
        һ�����ŵķ�ʽ����Բ�ͬ���ܵĶ��дش���һ��set����ȷ�����дص�Ψһ��
        */

        // �����жӴ�
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        // �жӴ�����ʼ������
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

        // �����豸�������е�Ԫ��
        float queuePriority = 1.0f;// Vulkan����ʹ��0.0��1.0֮��ĸ���������������ȼ���Ӱ���������ִ�еĵ��á���ʹֻ��һ������Ҳ�Ǳ����
        for (int queueFamily : uniqueQueueFamilies)
        {
            // �����豸�жӳ�ʼ��
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // ��ȷ����Ϣ�й��豸Ҫʹ�õĹ�������
        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;// ��������������
        /*
        �������ǿ��ʹ�ø��������˲�����Ҳ���Լ򵥵�ͨ�������趨����ʹ������
            samplerInfo.anisotropyEnable = VK_FALSE;
            samplerInfo.maxAnisotropy = 1;
        */

        // ʹ������������ṹ�壬���ǿ������VkDeviceCreateInfo�ṹ
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        // �ѳ�ʼ�����˶����鸳��������ʼ��VkDeviceCreateInfo
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        // ���豸�������Ը�ֵ��������ʼ��VkDeviceCreateInfo
        createInfo.pEnabledFeatures = &deviceFeatures;

        // 
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else 
        {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create logical device!");
        }

        // ���ÿ�����д��ж��еľ�����������߼��豸�����дأ����������ʹ洢��ȡ���б��������ָ�롣
        //��Ϊ����ֻ�Ǵ�������дش���һ�����У�������Ҫʹ������0
        vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
        // 
        vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);
    }

    // ����������
    // ����Ⱦͼ���ύ����Ļ�Ļ������Ƴ���������
    void createSwapChain() 
    {
        // ��ȡ�������ṹ����ϸ��Ϣ
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        // ÿ��VkSurfaceFormatKHR�ṹ������һ��format��һ��colorSpace��Ա��
        // format��Ա����ָ��ɫ��ͨ�������͡�
        // ���磬VK_FORMAT_B8G8R8A8_UNORM������ʹ��B,G,R��alpha�����ͨ������ÿһ��ͨ��Ϊ�޷���8bit������ÿ�������ܼ�32bits��
        // colorSpace��Ա����SRGB��ɫ�ռ��Ƿ�ͨ��VK_COLOR_SPACE_SRGB_NONLINEAR_KHR��־֧�֡�
        // ��Ҫע������ڽ���汾�Ĺ淶�У������־��ΪVK_COLORSPACE_SRGB_NONLINEAR_KHR��
        // ������Ծ�����ʹ��SRGB(��ɫ����Э��)����Ϊ����õ������׸�֪�ġ���ȷ��ɫ�ʡ�
        // ֱ����SRGB��ɫ�򽻵��ǱȽ�����ս�ģ�����ʹ�ñ�׼��RGB��Ϊ��ɫ��ʽ����Ҳ��ͨ��ʹ�õ�һ����ʽVK_FORMAT_B8G8R8A8_UNORM��

        // ����surface��ʽ������formats��Ϊ�����Ĳ���������ΪSwapChainSupportDetails
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        // presentationģʽ���ڽ������ǳ���Ҫ��������������Ļ����ͼ�������
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        // �������е�ͼ���������������Ϊ���еĳ��ȡ���ָ������ʱͼ�����С���������Դ���1��ͼ����������ʵ�����ػ���
        // maxImageCount��ֵΪ0��������ڴ�֮��û�����ƣ������Ϊʲô��Ҫ���
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        // ��Vulkan��������Ĵ�������һ��������������Ҳ��Ҫ�������Ľṹ��
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        // �������󶨵������surface֮����Ҫָ��������ͼ���йص���ϸ��Ϣ
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1; // imageArrayLayersָ��ÿ��ͼ����ɵĲ������������ǿ���3DӦ�ó��򣬷���ʼ��Ϊ1
        // imageUsageλ�ֶ�ָ���ڽ������ж�ͼ����еľ������
        // ֱ�Ӷ����ǽ�����Ⱦ������ζ��������Ϊ��ɫ������Ҳ�������Ƚ�ͼ����ȾΪ������ͼ�񣬽��к��������
        // ����������¿���ʹ����VK_IMAGE_USAGE_TRANSFER_DST_BIT������ֵ����ʹ���ڴ��������Ⱦ��ͼ���䵽������ͼ����С�
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        // 
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // ͼ����Ա�������дط��ʣ�����Ҫ��ȷ����Ȩ������ϵ
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;// ͬһʱ��ͼ��ֻ�ܱ�һ�����д�ռ�ã�����������д���Ҫ������Ȩ��Ҫ��ȷָ�������ַ�ʽ�ṩ����õ�����
        }

        // ���������֧��(supportedTransforms in capabilities),���ǿ���Ϊ������ͼ��ָ��ĳЩת���߼�
        // ����90��˳ʱ����ת����ˮƽ��ת���������Ҫ�κ�transoform���������Լ򵥵�����ΪcurrentTransoform

        // ��ǰ�任��ֵ��preTransform
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        // ���Alpha�ֶ�ָ��alphaͨ���Ƿ�Ӧ�����������Ĵ���ϵͳ���л�ϲ�����������Ըù��ܣ��򵥵���VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        // presentModeָ���Լ������clipped��Ա����ΪVK_TRUE����ζ�Ų����ı��ڱε��������ݣ�
        // �������������Ĵ�������ǰ��ʱ������Ⱦ�Ĳ������ݴ����ڿ�������֮�⣬���������Ҫ��ȡ��Щ���ػ����ݽ��д���������Կ����ü����������ܡ�
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        // ����������
        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create swap chain!");
        }

        // ��Ҫע����ǣ�֮ǰ���������������д�����������ͼ���С���ֶ�minImageCount��
        // ��ʵ�ʵ����У������������ͼ����������ͽ�����Ϊʲô��Ҫ��һ�λ�ȡ����
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    // ����ͼ����ͼ
    void createImageViews()
    {
        // ����ͼ����ͼ���ϵĴ�С
        swapChainImageViews.resize(swapChainImages.size());

        // ѭ���������еĽ�����ͼ��
        for (uint32_t i = 0; i < swapChainImages.size(); i++)
        {
            swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }

    // ��Ⱦͨ��
    void createRenderPass() 
    {
        // ��ɫ����������
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = swapChainImageFormat;// ��ɫ�����ĸ�ʽ��Ӧ���뽻������ͼ��ĸ�ʽ��ƥ��
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;// �������κζ��ز����Ĺ��������Բ���������Ϊ1
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // ��������Ⱦǰ�����ڶ�Ӧ�����Ĳ�����Ϊ
        /*
        VK_ATTACHMENT_LOAD_OP_LOAD: �����Ѿ������ڵ�ǰ����������
        VK_ATTACHMENT_LOAD_OP_CLEAR: ��ʼ�׶���һ����������������
        VK_ATTACHMENT_LOAD_OP_DONT_CARE: ���ڵ�����δ���壬��������
        */
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // ��������Ⱦ�������ڶ�Ӧ�����Ĳ�����Ϊ
        /*
        VK_ATTACHMENT_STORE_OP_STORE: ��Ⱦ�����ݻ�洢���ڴ棬����֮����ж�ȡ����
        VK_ATTACHMENT_STORE_OP_DONT_CARE: ֡����������������Ⱦ������Ϻ�����Ϊundefined
        */
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;// stencilLoadOp/stencilStoreOpӦ����ģ�����ݡ�����Ӧ�ó��򲻻����κ�ģ�滺�����Ĳ�������������loading��storing�޹ؽ�Ҫ
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;// 
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // ָ��ͼ���ڿ�ʼ������Ⱦͨ��render passǰ��Ҫʹ�õĲ��ֽṹ��VK_IMAGE_LAYOUT_UNDEFINED��Ϊ������ͼ��֮ǰ�Ĳ��֡�����ֵ����ͼ������ݲ�ȷ���ᱻ�����������Ⲣ����Ҫ����Ϊ����������Ƕ�Ҫ������,����Ⱦ��Ϻ�ʹ�ý��������г���
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // ָ������Ⱦͨ�������Զ��任ʱʹ�õĲ���,VK_IMAGE_LAYOUT_PRESENT_SRC_KHRͼ���ڽ������б�����
        /*
        VK_IMAGE_LAYOUT_COLOR_ATTACHMET_OPTIMAL: ͼ����Ϊ��ɫ����
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: ͼ���ڽ������б�����
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: ͼ����ΪĿ�꣬�����ڴ�COPY����
        */

        // 
        VkAttachmentDescription depthAttachment = {};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // 
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        /* VkAttachmentReference
        һ����������Ⱦͨ�������ɶ����ͨ����ɡ���ͨ������Ⱦ������һ������
        ��ͨ���������������Ⱦ������������֮ǰ��Ⱦͨ�������֡������������
        ����˵����Ч��������ͨ��ÿһ��������֮ǰ�Ĳ���
        �������Щ��Ⱦ�������鵽һ����Ⱦͨ���У�ͨ��Vulkan��ͨ���е���Ⱦ�������������򣬿��Խ�ʡ�ڴ�Ӷ���ø��õ�����
        */

        // ��Ⱦ��ͨ������1 ��ɫ
        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // ��Ⱦ��ͨ������2 ���
        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // ��Ⱦ��ͨ��
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1; // �����������е�����ֱ�Ӵ�Ƭ����ɫ�����ã��� layout(location = 0) out vec4 outColor ָ��
        subpass.pColorAttachments = &colorAttachmentRef; // ����������ɫ����
        subpass.pDepthStencilAttachment = &depthAttachmentRef;// ����������Ⱥ�ģ������
        /*
        ���Ա���ͨ�����õĸ�����������:
            pInputAttachments: ��������ɫ���ж�ȡ
            pResolveAttachments: ����������ɫ�����Ķ��ز���
            pDepthStencilAttachment: ����������Ⱥ�ģ������
            pPreserveAttachments: ����������ͨ��ʹ�ã��������ݱ�����
        */

        // ������ͨ��������ϵ
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // VK_SUBPASS_EXTERNAL��ָ����Ⱦͨ��֮ǰ����֮�����ʽ��ͨ����ȡ�������Ƿ���srcSubpass����dstSubPass��ָ��
        dependency.dstSubpass = 0; // ����0ָ�����ǵ���ͨ�������ǵ�һ��Ҳ��Ψһ��,dstSubpass����ʼ�ո���srcSubPass�Է�ֹ������ϵ����ѭ��
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // ָ��Ҫ�ȴ��Ĳ�������Щ���������Ľ׶�,�����ǿ��Է��ʶ���֮ǰ,������Ҫ�ȴ���������ɶ�Ӧͼ��Ķ�ȡ����,�����ͨ���ȴ���ɫ��������Ľ׶���ʵ��
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;// ָ��Ҫ�ȴ��Ĳ�������Щ���������Ľ׶�
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;// ��ɫ�����׶εĲ������漰��ɫ�����Ķ�ȡ��д��Ĳ���Ӧ�õȴ�

        // ������ɫ�������������
        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1; // ָ����Ⱦ��ͨ������
        renderPassInfo.pSubpasses = &subpass; // ָ����Ⱦ��ͨ��
        renderPassInfo.dependencyCount = 1; // ָ������������
        renderPassInfo.pDependencies = &dependency; // ָ������������

        // ������Ⱦͨ��
        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    // �������������ò���
    void createDescriptorSetLayout() 
    {// ��Ҫ�ڹ��ߴ���ʱ��Ϊ��ɫ���ṩ����ÿ���������󶨵���ϸ��Ϣ������Ϊÿ���������Ժ�location��������һ����
     // ���һ���µĺ���������������Щ��ΪcreateDescritorSetLayout����Ϣ�����ǵ����ڹ�����ʹ�ã���Ӧ���ڹ��ߴ�������֮ǰ����

        // ��ɫ�����ݽṹ���ְ�
        VkDescriptorSetLayoutBinding uboLayoutBinding = {};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1; // 
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr; // pImmutableSamplers�ֶν�����ͼ��������������й�
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // ����������ɫ���ĸ��׶α�����

        // ���ͼ�������������
        VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;// ȷ��stageFlags��ȷ����,ָ��������Ƭ����ɫ����ʹ�����ͼ�������������.�����Ƭ����ɫ���ձ�ȷ���ĵط�.�����ڶ�����ɫ����ʹ���������������ͨ���߶�ͼheightmap��̬�ı��ζ��������

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    // ����ͼ�ι���
    void createGraphicsPipeline() 
    {
        // ���ض���shader��ƬԪshader
        auto vertShaderCode = readFile("shaders/vert.spv");
        auto fragShaderCode = readFile("shaders/frag.spv");

        // �ڽ����봫�ݸ���Ⱦ����֮ǰ�����Ǳ��뽫���װ��VkShaderModule������
        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        // ����ɫ��ģ����䵽�����еĶ�����ɫ���׶�
        VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // ״̬
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        // ����ɫ��ģ����䵽�����е�Ƭ����ɫ���׶�
        VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        // ��������ṹ��Ĵ�������ͨ�����鱣�棬�ⲿ�����ý�����ʵ�ʵĹ��ߴ�����ʼ
        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        // �����������ݵĸ�ʽ�����ݴ��ݵ�vertex shader��
        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        // �������ݵļ�϶��ȷ��������ÿ�����������ÿ��instance(instancing)
        auto bindingDescription = Vertex::getBindingDescription();
        // ������Ҫ���а󶨼��������ԵĶ�����ɫ���е������������
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1; // �������������
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()); // ����������������
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // ָ��ṹ�����飬���ڽ�һ���������صĶ���������Ϣ
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // ָ��ṹ�����飬���ڽ�һ���������صĶ���������Ϣ

        // 
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // ͼԪ�����˽ṹ����
        /*
        VK_PRIMITIVE_TOPOLOGY_POINT_LIST: ���㵽��
        VK_PRIMITIVE_TOPOLOGY_LINE_LIST: ������ߣ����㲻����
        VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: ������ߣ�ÿ���߶εĽ���������Ϊ��һ���߶εĿ�ʼ����
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: ������棬���㲻����
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: ÿ������ѵ�ĵڶ��������������㶼��Ϊ��һ�������ε�ǰ��������
        */
        // �������ݰ��ջ������е�������Ϊ����������Ҳ����ͨ��element buffer����������ָ���������ݵ�������ͨ�����ö��������������ܡ�
        // �������primitiveRestartEnable��ԱΪVK_TRUE������ͨ��0xFFFF����0xFFFFFFFF��Ϊ�����������ֽ��ߺ���������_STRIPģʽ�µ�ͼԪ���˽ṹ��
        inputAssembly.primitiveRestartEnable = VK_FALSE;  // �Ƿ����ö��������¿�ʼͼԪ
        
        // ����framebuffer��Ϊ��Ⱦ������Ŀ������
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f; // ָ��framebuffer����ȵķ�Χ
        viewport.maxDepth = 1.0f; // ָ��framebuffer����ȵķ�Χ

        // ��Ҫ��ͼ����Ƶ�������֡������framebuffer�У����Զ���ü����θ��ǵ�����ͼ��
        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;

        // �ӿ�����
        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;


        // ��դ��ͨ��������ɫ��������ļ����㷨������������Σ�����ͼ�δ��ݵ�Ƭ����ɫ��������ɫ����
        // ��Ҳ��ִ����Ȳ���depth testing�������face culling�Ͳü����ԣ������Զ������ƬԪ�������ã������Ƿ��������ͼԪ���˻����Ǳ߿�(�߿���Ⱦ)

        // ��դ��
        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE; // ����Զ���ü����ƬԪ����������������Ƕ������ǡ��������������±Ƚ����ã�����Ӱ��ͼ��ʹ�øù�����Ҫ�õ�GPU��֧��
        rasterizer.rasterizerDiscardEnable = VK_FALSE; // ����ΪVK_TRUE����ô����ͼԪ��Զ���ᴫ�ݵ���դ���׶Ρ����ǻ����Ľ�ֹ�κ������framebuffer֡�������ķ���
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // polygonMode�������β���ͼƬ������
        /*
        VK_POLYGON_MODE_FILL: ������������
        VK_POLYGON_MODE_LINE: ����α�Ե�߿����
        VK_POLYGON_MODE_POINT: ����ζ�����Ϊ������
        */
        rasterizer.lineWidth = 1.0f; //ʹ���κ�ģʽ��䶼��Ҫ����GPU����,lineWidth��Ա��ֱ�����ģ�����ƬԪ�����������ߵĿ�ȡ������߿�֧��ȡ����Ӳ�����κδ���1.0���߿���Ҫ����GPU��wideLines����֧��
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // cullMode�������ھ�����ü������ͷ�ʽ�����Խ�ֹculling���ü�front faces��cull back faces ����ȫ��
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // frontFace����������Ϊfront-facing��Ķ����˳�򣬿�����˳ʱ��Ҳ��������ʱ��
        rasterizer.depthBiasEnable = VK_FALSE;  // ��դ������ͨ����ӳ������߻���ƬԪ��б�����������ֵ��һЩʱ�������Ӱ��ͼ�����õģ����ﲻʹ�ã���������depthBiasEnableΪVK_FALSE

        // ��ͨ����϶������ε�Ƭ����ɫ���������դ����ͬһ�����ء�����Ҫ�����ڱ�Ե����Ҳ��������עĿ�ľ�ݳ��ֵĵط���
        // ���ֻ��һ�������ӳ�䵽�����ǲ���Ҫ�������Ƭ����ɫ�����в����ģ���ȸ߷ֱ�����˵�����Ứ�ѽϵ͵Ŀ����������ù�����ҪGPU֧��
        // ���ز���-�����
        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // ��Ȼ�����
        VkPipelineDepthStencilStateCreateInfo depthStencil = {};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;// ָ���Ƿ�Ӧ�ý��µ���Ȼ���������Ȼ��������бȽϣ���ȷ���Ƿ�Ӧ�ñ�����
        depthStencil.depthWriteEnable = VK_TRUE;// ָ��ͨ����Ȳ��Ե��µ�Ƭ������Ƿ�Ӧ�ñ�ʵ��д����Ȼ����������ڻ���͸�������ʱ��ǳ����á�����Ӧ����֮ǰ��Ⱦ�Ĳ�͸��������бȽϣ������ᵼ�¸�Զ��͸�����󲻱�����
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;// ָ��ִ�б������߶���Ƭ�εıȽ�ϸ�ڡ�������ֵ�ϵ͵Ĺ���������ζ�Ÿ����������µ�Ƭ�ε����Ӧ�ø�С
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        // Ƭ����ɫ������������ɫ������Ҫ��֡������framebuffer���Ѿ����ڵ���ɫ���л�ϡ����ת���Ĺ��̳�Ϊ��ɫ
        // �����ַ�ʽ:1.��old��new��ɫ���л�ϲ���һ�����յ���ɫ 2.ʹ�ð�λ�������old��new��ɫ��ֵ
        // �������ṹ������������ɫ��ϡ���һ���ṹ��VkPipelineColorBlendAttachmentState������ÿ�����ӵ�֡������������
        // �ڶ����ṹ��VkPipelineColorBlendStateCreateInfo������ȫ�ֻ�ɫ������

        // ��ɫ
        // ÿ�����ӵ�֡������������
        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;// �������ȷ��֡�������о����ĸ�ͨ������ɫ�ܵ�Ӱ��
        colorBlendAttachment.blendEnable = VK_FALSE; // ���blendEnable����ΪVK_FALSE,��ô��Ƭ����ɫ�����������ɫ���ᷢ���仯������������ɫ����������µ���ɫ�����õ��Ľ����colorWriteMask����AND���㣬��ȷ��ʵ�ʴ��ݵ�ͨ��
        // blendEnable,������������ʹ�û�ɫ����ʵ��alpha blending���µ���ɫ��ɵ���ɫ���л�ϻ�������ǵ�opacity͸��ͨ����finalColor��Ϊ���յ����

        // ȫ�ֻ�ɫ������ ��������֡�����������ã����������û�ϲ����ĳ������ó���������Ϊ��������Ļ������
        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;// ʹ�õ�һ�ַ�ʽ, ��Ҫ����logicOpEnableΪVK_TURE��������λ������logicOp�ֶ���ָ�����ڵ�һ�ַ�ʽ�л��Զ���ֹ����ͬ��Ϊÿһ�����ӵ�֡������framebuffer�رջ�ϲ���
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        // ��������ɫ����ʹ��uniform�����������붯̬״̬������ȫ�ֱ����������ڻ滭ʱ�޸ģ����Ը�����ɫ������Ϊ���������´������ǡ�
        // ����ͨ�����ڽ��任���󴫵ݵ�������ɫ��������Ƭ����ɫ���崴�����������
        // ��Щuniform��ֵ��Ҫ�ڹ��ߴ��������У�ͨ��VkPipelineLayout����ָ��

        // ��Ҫ�ڴ������ߵ�ʱ��ָ�����������ϵĲ��֣����Ը�֪Vulkan��ɫ����Ҫʹ�õ��������������������ڹ��߲��ֶ�����ָ�����޸�VkPipelineLayoutCreateInfo���ò��ֶ���
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        // �������߲���
        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        // ��Ⱦ���߳�ʼ�� ����ṹ��������������׼������
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = pipelineLayout; // ���߲��ָ������߳�ʼ����������
        pipelineInfo.renderPass = renderPass; // ��Ⱦͨ��
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        // ����ͼ�ι���
        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, fragShaderModule, nullptr); // ���frag shader module
        vkDestroyShaderModule(device, vertShaderModule, nullptr); // ���vert shader module
    }

    // ֡������
    void createFramebuffers()
    {
        // ��̬�������ڱ���framebuffers��������С
        swapChainFramebuffers.resize(swapChainImageViews.size());

        // ����ͼ����ͼ��ͨ�����Ǵ�����Ӧ��framebuffers
        for (size_t i = 0; i < swapChainImageViews.size(); i++)
        {
            // ͼ����ͼ����
            std::array<VkImageView, 2> attachments = {
                swapChainImageViews[i],
                depthImageView
            };

            // ����framebuffers�Ƿǳ�ֱ�ӵġ�������Ҫָ��framebuffer��Ҫ���ݵ�renderPass
            // ֻ��ʹ��������ݵ���Ⱦͨ����֡�����������������ζ������ʹ����ͬ�ĸ������������͡�
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass; // ��Ⱦͨ��
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size()); // ͼ����ͼ����
            framebufferInfo.pAttachments = attachments.data(); // ͼ����ͼ����
            framebufferInfo.width = swapChainExtent.width; // ֡��������
            framebufferInfo.height = swapChainExtent.height;// ֡��������
            framebufferInfo.layers = 1; // ���ｻ����ͼ���ǵ���ͼ����˲���Ϊ1

            // ����֡������
            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    // ���������
    void createCommandPool() 
    {
        // �������ͨ�������ύ������һ���豸��������ִ�У������Ǽ�����graphics��presentation����
        // ÿ����������ֻ�ܷ����ڵ�һ���͵Ķ������ύ�����������Ҳ����Ҫ�����������Ҫ���������һ�£���Ҫ��¼���Ƶ�������˵��ΪʲôҪѡ��ͼ�ζ��дص�ԭ��
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        // �������������ʼ��
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

        // �����������
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create graphics command pool!");
        }
    }

    // �������ͼ����Դ
    void createDepthResources()
    {
        // ��ѡ��ʽ�б��� ��������ֵ�Ľ���ԭ�򣬼���һ���õ�֧�ֵĸ�ʽ
        VkFormat depthFormat = findDepthFormat();

        // ����ͼ��
        createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
        // ���ͼ����ͼ
        depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

        // δ����Ĳ��ֿ�����Ϊ��ʼ���֣���Ϊ���ͼ�������޹ؽ�Ҫ��������Ҫ�� transitionImageLayout �и���һЩ�߼�ʹ����ȷ������Դ
        transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

    // ��ѡ��ʽ�б��� ��������ֵ�Ľ���ԭ��,����һ���õ�֧�ֵĸ�ʽ
    VkFormat findDepthFormat()
    {
        return findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    // ��ѡ��ʽ�б��� ��������ֵ�Ľ���ԭ�򣬼���һ���õ�֧�ֵĸ�ʽ
    VkFormat findSupportedFormat(const std::vector<VkFormat> & candidates, VkImageTiling tiling, VkFormatFeatureFlags features) 
    {
        for (VkFormat format : candidates) 
        {
            /*
            VkFormatProperties �ṹ����������ֶΣ�
                linearTilingFeatures: ʹ������ƽ�̸�ʽ
                optimalTilingFeatures: ʹ�����ƽ�̸�ʽ
                bufferFeatures: ֧�ֻ�����
            */
            VkFormatProperties props;
            // ֧�ֵĸ�ʽ��������ʹ�õ� tiling modeƽ��ģʽ�;�����÷������Ա��������Щ����������ʹ�� vkGetPhysicalDeviceFormatProperties ������ѯ��ʽ��֧��
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

    // �ж���ѡ�����ȸ�ʽ�Ƿ����ģ�����
    bool hasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    // ������������ͼƬ
    void createTextureImage() 
    {
        int texWidth, texHeight, texChannels;
        // STBI_rgb_alphaֵǿ�Ƽ���ͼƬ��alphaͨ������ʹ������û��alpha���������������ڽ������������������һ���Էǳ��Ѻã�������STBI_rgba_alpha��������������У�ÿ������4���ֽ�
        stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);// ʹ���ļ���·����ͨ����������Ϊ��������ͼƬ
        VkDeviceSize imageSize = (uint64_t)texWidth * texHeight * 4;// ͼƬ������ֵ

        if (!pixels)
        { 
            throw std::runtime_error("failed to load texture image!");
        }

        // ͼƬ���ػ�����
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory; // ͼƬ���ش洢
        createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        // ֱ�Ӵӿ��м��ص�ͼƬ�п������ص�������
        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device, stagingBufferMemory);

        stbi_image_free(pixels); // ����ԭͼ�����������

        // ������ͼͼ��
        createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

        // �����ֱ任
        transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        // �����ݴ滺��������ͼͼ��
        copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        // �任��׼����ɫ������
        transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(device, stagingBuffer, nullptr);// �����ݴ滺����
        vkFreeMemory(device, stagingBufferMemory, nullptr);// ���������ڴ�
    }

    // ����ͼ����ͼ��������ͼ��
    void createTextureImageView()
    {
        textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    
    /* createTextureSampler
    ��Ҫע����ǲ�����û���κεط�����VkImage
    ��������һ�����صĶ������ṩ�˴���������ȡ��ɫ�Ľӿ�
    ������Ӧ�����κ�������ͼ���У�������1D��2D��������3D��Ҳ��֮ǰ�ܶ�ɵ�API�ǲ�ͬ�ģ����߽�����ͼ�����������ϳɵ�һ״̬
    */
    // �������ò���������
    void createTextureSampler()
    {
        // ָ����ҪӦ�õĹ������ͱ任
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;// ָ������ʹ�õ�Ѱַģʽ,�������ΪU����X,��������ռ������Լ��
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;// ָ������ʹ�õ�Ѱַģʽ,�������ΪV����Y,��������ռ������Լ��
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;// ָ������ʹ�õ�Ѱַģʽ,�������ΪW����Z,��������ռ������Լ��
        /*Ѱַģʽ�����¼���:
            VK_SAMPLER_ADDRESS_MODE_REPEAT��������ͼ��ߴ��ʱ�����ѭ�����
            VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT����ѭ��ģʽ���ƣ����ǵ�����ͼ��ߴ��ʱ�������÷�����Ч��
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE��������ͼ��ߴ��ʱ�򣬲��ñ�Ե�������ɫ�������
            VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TOEDGE�����Եģʽ���ƣ�����ʹ���������Ե�෴�ı�Ե�������
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER������������ͼ��ĳߴ�ʱ������һ����ɫ���
        */
        samplerInfo.anisotropyEnable = VK_TRUE;// ָ���Ƿ�ʹ�ø������Թ�������û�����ɲ�ʹ�ø����ԣ�����������һ������
        samplerInfo.maxAnisotropy = 16;// ���ƿ����ڼ���������ɫ�����ز������������͵���ֵ��õ������ܣ�����õ��������.��ǰû���κε�ͼ��Ӳ������ʹ�ó���16������������Ϊ����16���������Ĳ�����Ժ��Բ���
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;// ָ��������Χ����ͼ��ʱ�򷵻ص���ɫ����֮��Ӧ���Ǳ�ԵѰַģʽ��������float��int���غ�ɫ����ɫ��͸���ȡ����ǲ���ָ��������ɫ
        samplerInfo.unnormalizedCoordinates = VK_FALSE;// ָ��ʹ�õ�����ϵͳ�����ڷ���ͼ������ء�����ֶ�ΪVK_TRUE����ζ�ſ��Լ򵥵�ʹ�����귶ΧΪ [ 0, texWidth ) �� [ 0, texHeight )�����ʹ��VK_FALSE����ζ��ÿ���������ط���ʹ��  [ 0, 1) ��Χ����ʵ��Ӧ�ó�������ʹ�ù�һ�������ꡣ��Ϊ��������ʹ����ȫ��ͬ����Ĳ�ͬ�ֱ��ʵ�����
        samplerInfo.compareEnable = VK_FALSE;// �رձȽϹ��ܣ���������ȽϹ��ܣ���ô�������Ⱥ�ֵ���бȽϣ����ұȽϺ��ֵ���ڹ��˲���,��Ҫ������Ӱ����ӳ��� percentage-closer filtering ���ٷֱȽ��ƹ�����
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        // �������ò���������
        if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    // ����ͼ����ͼ
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
    {
        // ����ͼ����ͼ��ʼ���ṹ
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;// ��������ͼ�����ݸñ���ν���
        viewInfo.format = format;
        // subresourceRange��������ͼ���ʹ��Ŀ����ʲô���Լ����Ա����ʵ���Ч����ͼ�񽫻���Ϊcolor targets��û���κ�mipmapping levels ���Ƕ�� multiple layers
        // ����ڱ�д����ʽ��3DӦ�ó��򣬱���VR������Ҫ����֧�ֶ��Ľ�����������ͨ����ͬ�Ĳ�Ϊÿһ��ͼ�񴴽������ͼ�������㲻ͬ���ͼ������������Ⱦʱ����ͼ����Ҫ
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        // ����ͼ����ͼ
        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture image view!");
        }

        return imageView;
    }

    // ������ͼͼ��
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image, VkDeviceMemory & imageMemory)
    {
        // ͼƬ��ʼ��
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;// imageType�ֶ�ָ��ͼ�����ͣ���֪Vulkan����ʲô��������ϵ��ͼ���вɼ����ء���������1D��2D��3Dͼ��1Dͼ�����ڴ洢�������ݻ��߻Ҷ�ͼ��2Dͼ����Ҫ����������ͼ��3Dͼ�����ڴ洢��������
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format; // Vulkan֧�ֶ���ͼ���ʽ���������������Ҫ�ڻ�������Ϊ����Ӧ��������һ�µĸ�ʽ�����򿽱�������ʧ��
        imageInfo.tiling = tiling;
        /*
        tiling�ֶο����趨����֮һ��
            VK_IMAGE_TILING_LINEAR: ���ػ���������Ĳ��֣���pixels����
            VK_IMAGE_TILING_OPTIMAL: ���ػ��ھ����ʵ�������岼�֣���ʵ����ѷ���
          ��ͼ�񲼾ֲ�ͬ���ǣ�tilingģʽ������֮���޸ġ������Ҫ���ڴ�ͼ����ֱ�ӷ������أ�����ʹ��VK_IMAGE_TILING_LINEAR��
          ����ʹ���ݴ滺���������ݴ�ͼ�������ⲿ�ֲ��Ǻ��б�Ҫ��Ϊ�˸���Ч�Ĵ�shader�з������أ���ʹ��VK_IMAGE_TILING_OPTIMAL
        */

        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        /*
        ͼ���initialLayout�ֶΣ���������ѡ��ֵ��
            VK_IMAGE_LAYOUT_UNDEFINED: GPU����ʹ�ã���һ���任���������ء�
            VK_IMAGE_LAYOUT_PREINITIALIZED: GPU����ʹ�ã����ǵ�һ�α任���ᱣ�����ء�
        */

        imageInfo.usage = usage;// ͼ�񽫻ᱻ����������������Ŀ�꣬����Ӧ��������Ϊ����Ŀ�ĵء����ǻ�ϣ������ɫ���з���ͼ������ǵ�mesh������ɫ����˾����usage��Ҫ����VK_IMAGE_USAGE_SAMPLED_BIT
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;// ���ز���,���������������Ϊ������ͼ�����Լ��һ��������ֵ
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;// ��Ϊͼ�����һ�����д���ʹ�ã�֧��ͼ�λ��ߴ������

        // ����ͼ��
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

        // ����ʵ�����������е�Ӧ�ó����������Ϊÿ������������vkAllocateMemory�����ڴ�
        // �ڴ�������������ܵ�maxMemoryAllocationCount�����豸���ޣ���ʹ��NVIDIA GTX1080�����ĸ߶�Ӳ���ϣ�Ҳֻ���ṩ4096�Ĵ�С
        // ͬһʱ�䣬Ϊ������������ڴ����ȷ�����Ǵ���һ���Զ����������ͨ��ʹ����������ຯ�����õ���ƫ����offset����һ�����Ŀɷ����ڴ����򻮷�Ϊ����ɷ����ڴ�飬�ṩ������ʹ��
        if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate image memory!");
        }

        // Ϊͼ���������ڴ�
        vkBindImageMemory(device, image, imageMemory, 0);
    }

    // �����ֱ任 ��������
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        /*
        ͨ���������������ڴ���ͼ��任��ʹ�� image memory barrier��
        һ�����ߵ�����ͨ�����ڷ�����Դ��ʱ�����ͬ����Ҳ���ƻ������ڶ�����֮ǰ���д���������ȻҲ��������ͼ�񲼾ֵı任�Լ���ʹ�� VK_SHARING_MODE_EXCLUSIVE ģʽ����£�������д������ı任
        ��������һ���ȼ۵� buffer memory barrier
        */

        // ͼ�δ洢����
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;// �����Դ�����дص�����ʹ�����ϣ���Ҫ���ö��дص���������������ģ����������VK_QUEUE_FAMILY_IGNORED(����Ĭ��ֵ)
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;// �����Դ�����дص�����ʹ�����ϣ���Ҫ���ö��дص���������������ģ����������VK_QUEUE_FAMILY_IGNORED(����Ĭ��ֵ)
        barrier.image = image;// ָ���ܵ�Ӱ���ͼ��

        // subresourceRangeָ���ܵ�Ӱ���ͼ���ض�����
        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            // �ж���ѡ�����ȸ�ʽ�Ƿ����ģ�����
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
            // srcAccessMask|dstAccessMask ������Ҫ����ͬ��Ŀ�ģ����Ա�����Ӧ������ǰָ����һ�ֲ������ͼ��漰������Դ��ͬʱҪָ����һ�ֲ�������Դ����ȴ�����
            // ��������������ʹ��vkQueueWaitIdle��Ϊ�Ŀ���ͬ������ȷ��ֵȡ���ھɵĺ��µĲ��֣�����֪����Ҫʹ�õı任���Ϳ��Իص����ֲ���
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

        // �������͵Ĺ������϶�ʹ��ͬ���ĺ����ύ
        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
        /* vkCmdPipelineBarrier
        �������͵Ĺ������϶�ʹ��ͬ���ĺ����ύ���������������ĵ�һ������ָ�����ߵ��ĸ��׶Σ�Ӧ������ͬ��֮ǰҪִ�е�ǰ�ò���
        �ڶ�������ָ���������������ϵȴ��Ĺ��߽׶Ρ�������֮ǰ��֮������ָ�����߽׶�ȡ����������֮ǰ��֮�����ʹ����Դ
        �����ֵ���ڹ淶�� table ����С����磬Ҫ������֮��� uniform �ж�ȡ��ָ��ʹ��VK_ACCESS_UNIFORM_READ_BIT�Լ���ʼ��ɫ���� uniform �ж�ȡ��Ϊ���߽׶Σ����� VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
        Ϊ�������͵�ָ������ɫ�����߽׶���û������ģ����ҵ�ָ����ʹ�����Ͳ�ƥ��Ĺ��߽׶�ʱ��validation layer ������ʾ������Ϣ
        ������������������Ϊ0����VK_DEPENDENCY_BY_REGION_BIT�����߽����ϱ任Ϊÿ�������״̬������ζ�ţ����磬�����Ѿ�д����Դ������ʼ���Ĳ���������ϸ������
        ��������������ù������ϵ����飬���������ͣ���һ�� memory barriers���ڶ���, buffer memory barriers, �� image memory barriers
        ��Ҫע���������û��ʹ��VkFormat���������ǻ�����Ȼ�������ʹ������һЩ����ı任
        */


        // �����������
        endSingleTimeCommands(commandBuffer);
    }

    // ������������ͼ��
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        // ���������¼
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        // ָ������������һ���ֵ�ͼ�������
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

        // ������������ͼ��Ĳ���
        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        // �����������
        endSingleTimeCommands(commandBuffer);
    }

    // ����ģ�� tinyobjloader��
    void loadModel()
    {
        // ����objģ��
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
            // ����������������&�����㼯�ϼ���ֵ&������ֵ
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
                {  // ����Ŀ��,ʹ����������Ͻ�0,0�����½ǵ�1,1��ӳ������,�Ӷ��򵥵�������.��������Գ��Ը�������.���Գ���ʹ�õ���0����1���ϵ��������鿴Ѱַģʽ�Ĳ�ͬ����
                   // ����Vulkan�������������������Ͻǣ���OBJ��ʽ�������½ǡ�ͨ����ת��������Ĵ�ֱ����������������
                    attrib.texcoords[(uint64_t)2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[(uint64_t)2 * index.texcoord_index + 1]
                };

                vertex.color = { 1.0f, 1.0f, 1.0f };

                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                // ��������������ύ��������������
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

    // �������㻺����
    void createVertexBuffer()
    {
        // ���㻺������Сֱ����sizeof
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        VkBuffer stagingBuffer; // ����stagingBufferMemory����������ӳ�䡢������������
        VkDeviceMemory stagingBufferMemory; // 
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data); // �ڴ�ӳ��
        memcpy(data, vertices.data(), (size_t)bufferSize); // ��������
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

        // 
        copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr); // ������㻺����
        vkFreeMemory(device, stagingBufferMemory, nullptr);// ������㻺������¼
    }

    // ������������������
    void createIndexBuffer()
    {
        // ���û�������Сֱ����sizeof
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

        copyBuffer(stagingBuffer, indexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr); // �����������������
        vkFreeMemory(device, stagingBufferMemory, nullptr); // �������������������¼
    }

    // ����ͳһ������ ubo
    void createUniformBuffer() 
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer, uniformBufferMemory);
    }

    // ��������������
    void createDescriptorPool()
    {
        // ��ȷ��Ҫʹ�õ����������ϰ���������������������
        std::array<VkDescriptorPoolSize, 2> poolSizes = {};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = 1;
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = 1;

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = 1;// ָ���������������ϵķ�������

        // ��������������
        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    // ������������������
    void createDescriptorSet()
    {
        // �洢���������ϵľ��
        VkDescriptorSetLayout layouts[] = { descriptorSetLayout };
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = layouts;

        // ��������������
        if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor set!");
        }

        // ָ�����������������ڲ����������ݵ�����
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uniformBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;

        // ����������
        std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSet; // ����������
        descriptorWrites[0].dstBinding = 0; //  uniform buffer �󶨵������趨Ϊ0�����������������飬������Ҫָ��Ҫ���µ�����������������û��ʹ�����飬���Լ򵥵�����Ϊ0
        descriptorWrites[0].dstArrayElement = 0;// ��ʼ����
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // 
        descriptorWrites[0].descriptorCount = 1; // ����������������Ҫ������
        descriptorWrites[0].pBufferInfo = &bufferInfo;// ָ�����������õĻ���������

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSet;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo; // ����ָ�����������õ�ͼ������

        // �����������ø���
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    // ����������
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) 
    {
        // ����buffer��ʼ��
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;// ����
        bufferInfo.size = size; // ָ���������ֽڴ�С�����㻺����ÿ���������ݵ��ֽڴ�С����ֱ��ʹ��sizeof
        bufferInfo.usage = usage;// ָ�������������ݽ����ʹ�á�����ʹ��λ����ָ�����ʹ��Ŀ��
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;// �񽻻����е�ͼ��һ����������Ҳ�������ض��Ķ��д�ռ�л��߶��ͬʱ�����ڱ���Ŀ���������ᱻ����ͼ�ζ��У�����ʹ�ö�ռ����ģʽexclusive mode

        // ����buffer
        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create buffer!");
        }

        // �������������ڴ�
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);// 

        // �����ڴ�
        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size; // ��С
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties); // �����ڴ���

        // ����ʵ�����������е�Ӧ�ó����������Ϊÿ������������vkAllocateMemory�����ڴ�
        // �ڴ�������������ܵ�maxMemoryAllocationCount�����豸���ޣ���ʹ��NVIDIA GTX1080�����ĸ߶�Ӳ���ϣ�Ҳֻ���ṩ4096�Ĵ�С
        // ͬһʱ�䣬Ϊ������������ڴ����ȷ�����Ǵ���һ���Զ����������ͨ��ʹ����������ຯ�����õ���ƫ����offset����һ�����Ŀɷ����ڴ����򻮷�Ϊ����ɷ����ڴ�飬�ṩ������ʹ��
        // �����Լ�ʵ��һ�������ڴ������������ʹ��GOUOpen�ṩ��VulkanMemoryAllocator�⡣Ȼ�������ڱ���Ŀ����������Ϊÿ����Դʹ�õ����ķ��䣬��Ϊ���ᴥ���κ���Դ��������.

        // ����ڴ����ɹ���ʹ��vkBindBufferMemory�������ڴ������������
        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        // ���ڴ������������
        vkBindBufferMemory(device, buffer, bufferMemory, 0);// ���ĸ������ڴ������ƫ����,��Ϊ����ڴ汻ר��Ϊ���㻺��������,ƫ��������Ϊ0.���ƫ����non-zero,��ô��Ҫͨ��memRequirements.alignment����
    }

    // ��������ļ�¼����
    VkCommandBuffer beginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        /*
        level����ָ�������������������ӹ�ϵ
            VK_COMMAND_BUFFER_LEVEL_PRIMARY: �����ύ������ִ�У������ܴ�����������������á�
            VK_COMMAND_BUFFER_LEVEL_SECONDARY: �޷�ֱ���ύ�����ǿ��Դ�������������á�
        */
        allocInfo.commandPool = commandPool; // �������ͨ��vkAllocateCommandBuffers��������
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        // ָ�����������ʹ�ù����е�һЩ������Ϣ
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        /*
        flags��־λ��������ָ�����ʹ�������������ѡ�Ĳ�����������:
            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: �����������ִ��һ�κ��������¼�¼��
            VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: ����һ������������������������һ����Ⱦͨ���С�
            VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: �������Ҳ���������ύ��ͬʱ��Ҳ�ڵȴ�ִ�С�
        */

        // ��ʼ�������
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    // �����������
    void endSingleTimeCommands(VkCommandBuffer commandBuffer) 
    {
        vkEndCommandBuffer(commandBuffer);

        // �����ύ��ͬ��
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1; // �ύִ���������������
        submitInfo.pCommandBuffers = &commandBuffer; // ���屻�ύִ�е��������

        // 
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    // ��һ���������������ݵ���һ��������
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        // ���������¼
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion = {};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);// ����������

        endSingleTimeCommands(commandBuffer);
    }

    // ���䲻ͬ���͵��ڴ�  ÿ�����͵��ڴ����������Ĳ��������Ծ�����ͬ����Ҫ��ϻ�������Ӧ�ó���ʵ�ʵ���Ҫ�ҵ���ȷ���ڴ�����ʹ��
    // typeFilter��������λ����ʽ�����ʺϵ��ڴ�����
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) 
    {
        VkPhysicalDeviceMemoryProperties memProperties; // ��Ч���ڴ�����
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties); // ������Ч���ڴ�����

        // Ϊ�������ҵ����ʵ��ڴ�����
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {   // typeFilter��������λ����ʽ�����ʺϵ��ڴ����͡�����ζ��ͨ���򵥵ĵ����ڴ����Լ��ϣ���������Ҫ��������ÿ���ڴ����Ե����ͽ���AND�������ж��Ƿ�Ϊ1
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    // �����������
    void createCommandBuffers() 
    {
        // Ϊ������������ô�С
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

            // ��Ⱦͨ����ʼ��
            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass; // �󶨵���Ӧ��������Ⱦͨ��
            renderPassInfo.framebuffer = swapChainFramebuffers[i]; // Ϊÿһ����������ͼ�񴴽�֡����������ָ��Ϊ��ɫ����
            renderPassInfo.renderArea.offset = { 0, 0 }; // ��������Ⱦ����Ĵ�С
            renderPassInfo.renderArea.extent = swapChainExtent; // ��Ⱦ��������ɫ�����غʹ洢��Ҫ������λ�á�����������ؽ�����δ����ֵ��Ϊ����ѵ��������ĳߴ�Ӧ���븽��ƥ��

            // 
            std::array<VkClearValue, 2> clearValues = {};
            clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f }; // Ϊ�˼򻯲�����������clear colorΪ100%��ɫ
            clearValues[1].depthStencil = { 1.0f, 0 };

            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size()); 
            renderPassInfo.pClearValues = clearValues.data();

            // ������Ⱦͨ��
            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            /*
            ��һ���������Ǽ�¼�����������������ڶ�������ָ�����Ǵ��ݵ���Ⱦͨ���ľ�����Ϣ�����Ĳ�����������ṩrender pass��ҪӦ�õĻ��������ʹ��������ֵ����һ��:
                VK_SUBPASS_CONTENTS_INLINE: ��Ⱦ�������Ƕ��������������У�û�и���������ִ�С�
                VK_SUBPASS_CONTENTS_SECONDARY_COOMAND_BUFFERS: ��Ⱦͨ�������Ӹ����������ִ�С�
            */

            // ��ͼ�ι��� ��һ������ ��¼�����������������ڶ�������ָ������������ͣ�����������graphics ���� compute pipeline��
            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

            VkBuffer vertexBuffers[] = { vertexBuffer };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);// ���ڰ󶨶��㻺����������֮ǰ������һ���������������֮�⣬ǰ��������ָ��������ҪΪ��ָ���Ķ��㻺������ƫ����������,����������ָ���˽�Ҫ�󶨵Ķ��㻺���������鼰��ʼ��ȡ���ݵ���ʼƫ����

            vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32); // ������������

            // �����������ϰ󶨵�ʵ�ʵ���ɫ������������
            vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

            // ǰ��������ָ�������������ͼ���instance������û��ʹ��instancing������ָ��1��
            // ��������ʾ�����ݵ����㻺�����еĶ�����������һ������ָ��������������ƫ������ʹ��1���ᵼ��ͼ�ο��ڵڶ�����������ʼ��ȡ
            // �����ڶ�������ָ����������������ӵ�������ƫ�ơ����һ������ָ��instancingƫ����������û��ʹ�ø�����
            vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

            // ��Ⱦͨ��ִ������ƣ����Խ�����Ⱦ��ҵ
            vkCmdEndRenderPass(commandBuffers[i]);

            // ��ֹͣ��¼��������Ĺ���
            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }

    // �����ź�������
    void createSemaphores() 
    {
        // �ڵ�ǰ�汾�� ����type����Ҫ������ֵ
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        // �����ź�������
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphores!");
        }
    }

    // ���� �����������ÿһ֡�д����µı任������ȷ������ͼ����ת���ƶ��������ȱ任
    void updateUniformBuffer()
    {
        // ������
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos); // ��ȡ��ǰ���λ��



        // ʱ�������ת
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 3000.0f;

        UniformBufferObject ubo = {};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(1.7f, 1.7f, 1.7f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // �����λ��/����λ��/��������
        ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);// ѡ��ʹ��FOVΪ45�ȵ�͸��ͶӰ�����������ǿ�߱ȣ����ü����Զ�ü��档��Ҫ����ʹ�õ�ǰ�Ľ�������չ�������߱ȣ��Ա��ڴ��������С��ο����µĴ����Ⱥ͸߶�
        ubo.proj[1][1] *= -1; // GLM�����ΪOpenGL��Ƶģ����Ĳü������Y�Ƿ�ת�ġ��������������򵥵ķ�������ͶӰ������Y����������ӷ�ת�������������ͼ��ᱻ����

        // ���ڶ��������еı任�����Խ�UBO�е����ݸ��Ƶ�uniform������������û���ݴ滺���������붥�㻺�����Ĳ�����ȫ��ͬ
        // ʹ��ubo�������Ǿ����仯��ֵ���ݸ���ɫ���Ƿǳ���Ч�ķ�ʽ����ȴ���һ����С�����ݻ���������ɫ���У�����Ч�ķ�ʽ��ʹ�ó���
        void* data;
        vkMapMemory(device, uniformBufferMemory, 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, uniformBufferMemory);
    }

    // ����֡ �ӽ�������ȡͼ����֡��������ʹ����Ϊ������ͼ����ִ����������е������ͼ�񷵻������������ճ���
    void drawFrame()
    {
        // vkAcquireNextImageKHRǰ��������������ϣ����ȡ��ͼ����߼��豸�ͽ�����������������ָ����ȡ��Чͼ��Ĳ���timeout����λ���롣����ʹ��64λ�޷������ֵ��ֹtimeout
        // ����������ָ��ʹ�õ�ͬ�����󣬵�presentation���������ͼ��ĳ��ֺ��ʹ�øö������ź�,����ǿ�ʼ���Ƶ�ʱ���,������ָ��һ���ź���semaphore����դ����������,����ʹ��imageAvailableSemaphore
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        // ������󱨸�
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            // ���´���������
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        // �����ύ��ͬ��
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        
        // �ȴ��������źż�
        VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; // �����ӦpWaitSemaphores�о�����ͬ�������ź�
        submitInfo.waitSemaphoreCount = 1; // �ȴ��źŵ�����
        submitInfo.pWaitSemaphores = waitSemaphores; // �ȴ���Щ�����ź�
        submitInfo.pWaitDstStageMask = waitStages; // ָ��pWaitSemaphores�о�����ͬ�������ź�

        submitInfo.commandBufferCount = 1; // ָ����ִ�е���������ź�����
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex]; // ָ���ĸ����������ʵ���ύִ��

        // ��Ⱦ�����źż�
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
        submitInfo.signalSemaphoreCount = 1; // ָ����ָ������������ź�����
        submitInfo.pSignalSemaphores = signalSemaphores;// ָ�����屻�ύָ�����������

        // ��ͼ������ύ�������
        // ���������رȽϴ��ʱ�򣬴���Ч�ʿ��ǣ��������Գ���VkSubmitInfo�ṹ������
        // ���һ������������һ����ѡ��դ�������������ִ�����ʱ�����ᱻ�����ź�
        // ����ʹ���ź�������ͬ��������������Ҫ����VK_NULL_HANDLE
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        // ������ύ����������ʹ��������ʾ����Ļ��
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        // ָ���ڽ���presentation֮ǰҪ�ȴ����ź�����
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        // ����������
        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        // �ύ������ֽ������е�ͼ��
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

    // �ڽ����봫�ݸ���Ⱦ����֮ǰ�����뽫���װ��VkShaderModule������
    // ����һ����������createShaderModuleʵ�ָ��߼�
    VkShaderModule createShaderModule(const std::vector<char> & code) 
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        // ����shaderModule
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }
    
    // ѡ�񽻻��������ʽ
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> & availableFormats) 
    {
        // ������������surfaceû�������κ�ƫ���Եĸ�ʽ��
        // ���ʱ��Vulkan��ͨ��������һ��VkSurfaceFormatKHR�ṹ��ʾ��
        // �Ҹýṹ��format��Ա����ΪVK_FORMAT_UNDEFINED��
        if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) 
        {
            return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
        }

        // ����������ɵ����ø�ʽ����ô����ͨ�������б����þ���ƫ���Ե����
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    // ��ѯ����ʵ�PresentMode����
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
    {
        /*
        VK_PRESENT_MODE_IMMEDIATE_KHR: Ӧ�ó����ύ��ͼ���������䵽��Ļ���֣�����ģʽ���ܻ����˺��Ч����
        VK_PRESENT_MODE_FIFO_KHR: ������������һ�����У�����ʾ������Ҫˢ�µ�ʱ����ʾ�豸�Ӷ��е�ǰ���ȡͼ�񣬲��ҳ�����Ⱦ��ɵ�ͼ�������еĺ��档������������ĳ����ȴ���
           ���ֹ�ģ����Ƶ��Ϸ�Ĵ�ֱͬ�������ơ���ʾ�豸��ˢ��ʱ�̱���Ϊ����ֱ�жϡ���
        VK_PRESENT_MODE_FIFO_RELAXED_KHR: ��ģʽ����һ��ģʽ���в�ͬ�ĵط�Ϊ��
           ���Ӧ�ó�������ӳ٣����������һ����ֱͬ���ź�ʱ���п��ˣ�������ȴ���һ����ֱͬ���źţ����ǽ�ͼ��ֱ�Ӵ��͡����������ܵ��¿ɼ���˺��Ч����
        VK_PRESENT_MODE_MAILBOX_KHR: ���ǵڶ���ģʽ�ı��֡�����������������ʱ��ѡ���µ��滻�ɵ�ͼ�񣬴Ӷ��������Ӧ�ó�������Ρ�
           ����ģʽͨ������ʵ�����ػ����������׼�Ĵ�ֱͬ��˫������ȣ���������Ч�����ӳٴ�����˺��Ч����
        */

        // �߼��Ͽ�����VR_PRESENT_MODE_FIFO_KHRģʽ��֤������
        VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

        // ����������һ���ǳ��õĲ��ԡ����������˺�ѣ�ͬʱ��Ȼ������Ե͵��ӳ٣�ͨ����Ⱦ�������µ�ͼ��ֱ�����ܴ�ֱͬ���ź�
        // ѭ���б�������Ƿ����
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

    // ������Χ ָ������ͼ��ķֱ���
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities) 
    {
        // �ֱ��ʵķ�Χ��������VkSurfaceCapabilitiesKHR�ṹ����
        // Vulkan��������ͨ������currentExtent��Ա��width��height��ƥ�䴰��ķֱ���
        // һЩ�������������ͬ�����ã���ζ�Ž�currentExtent��width��height����Ϊ�������ֵ��ʾ:uint32_t�����ֵ��
        // ����������£��ο�����minImageExtent��maxImageExtentѡ����ƥ��ķֱ��ʡ�

        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetWindowSize(window, &width, &height);// ��ȡ����Ĵ�С

            VkExtent2D actualExtent =
            {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            // max��min�������ڽ�width��height������ʵ��֧�ֵ�minimum��maximum��Χ��
            // ��<algorithm>ͷ�ļ�
            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    // ��ȡ�豸��������Ϣ
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
    {
        SwapChainSupportDetails details;

        // ��ѯ�豸��Surface
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        // ��ȡformat����
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        // ��format�������formats����
        if (formatCount != 0) 
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        // ��ѯ�豸��present����
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        // ��present�������presentModes����
        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    // �������Կ��Ƿ��ʺ�Ҫִ�еĲ���
    bool isDeviceSuitable(VkPhysicalDevice device)
    {
        // �����豸�жӴ�(�����ҵ����е��Կ�)
        QueueFamilyIndices indices = findQueueFamilies(device);

        // ����ļ���߼�
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        // ��֤�������Ƿ����㹻��֧��
        bool swapChainAdequate = false;
        if (extensionsSupported) 
        {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        // �ж��Կ��ܹ�ִ����Щ����supportedFeatures.samplerAnisotropy
        VkPhysicalDeviceFeatures supportedFeatures;
        // ��ѯ����豸֧�ֵĹ���
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        // ���Բ�ѯ��������֧��������֤����չ��Ч��֮�����
        return indices.isComplete() && extensionsSupported && supportedFeatures.samplerAnisotropy;
    }

    // ����ļ���߼�
    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        // ��ȡ��������
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        // Ϊ���е�ÿ���豸�����������
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        // ��ȡÿһ���豸����
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        // ����豸�����б�Ϊ�� �򷵻���
        return requiredExtensions.empty();
    }

    // �����жӴ�
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) 
    {
        QueueFamilyIndices indices;

        // ��ȡһ���ж��ٸ��Կ��豸
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        // ����vector���� ������¼�Կ����
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        // ��ÿһ���Կ������ֵ��vector����
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        // ��ȡ��Ч�ж���
        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            // ���Ҿ߱�graphics���ܵĶ��д�
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }

            // ���ڼ��ĺ��Ĵ�����vkGetPhysicalDeviceSurfaceSupportKHR, 
            // ���������豸�����д�������surface��Ϊ��������VK_QUEUE_GRAPHICS_BIT��ͬ��ѭ��������Ӻ����ĵ���
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            // ���Ҿ߱�presentation���ܵĶ��д�
            if (queueFamily.queueCount > 0 && presentSupport)
            {
                indices.presentFamily = i; // �洢presentation���дص�����
            }

            if (indices.isComplete())
            {
                break;
            }

            i++;
        }

        return indices;
    }

    // ��ȡ�������չ
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

    // �����֤��֧��
    bool checkValidationLayerSupport() 
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) 
            {
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

    // ��ȡ�ļ�
    static std::vector<char> readFile(const std::string & filename)
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

    // debug�ص�
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
    {
        std::cerr << "validation layer: " << msg << std::endl;

        return VK_FALSE;
    }
};

// ������
int main()
{
    Application app;

    try
    {
        app.run();
    }
    catch (const std::runtime_error & e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
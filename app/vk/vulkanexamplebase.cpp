#include "vulkanexamplebase.h"

std::vector<const char*> VulkanExampleBase::args;

VkResult VulkanExampleBase::createInstance(bool enableValidation)
{
    this->settings.validation = enableValidation;

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = name.c_str();
    appInfo.pEngineName = name.c_str();
    appInfo.apiVersion = VK_API_VERSION_1_0;

    std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

    // Enable surface extensions depending on os
    instanceExtensions.push_back("VK_KHR_win32_surface");

    if (enabledInstanceExtensions.size() > 0)
    {
        for (auto enabledExtension : enabledInstanceExtensions) 
        {
            instanceExtensions.push_back(enabledExtension);
        }
    }

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = NULL;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    if (instanceExtensions.size() > 0)
    {
        if (settings.validation)
        {
            instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }
        instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    }
    if (settings.validation)
    {
        instanceCreateInfo.enabledLayerCount = VkAPI::debug::validationLayerCount;
        instanceCreateInfo.ppEnabledLayerNames = VkAPI::debug::validationLayerNames;
    }
    return vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
}

std::string VulkanExampleBase::getWindowTitle()
{
    std::string device(deviceProperties.deviceName);
    std::string windowTitle;
    windowTitle = title + " - " + device;
    if (!settings.overlay) {
        windowTitle += " - " + std::to_string(frameCounter) + " fps";
    }
    return windowTitle;
}

const std::string VulkanExampleBase::getAssetPath()
{
    std::string resource_path = RESOURCE_PATH "/assets/";
    return  resource_path;
}

bool VulkanExampleBase::checkCommandBuffers()
{
    for (auto& cmdBuffer : drawCmdBuffers)
    {
        if (cmdBuffer == VK_NULL_HANDLE)
        {
            return false;
        }
    }
    return true;
}

void VulkanExampleBase::createCommandBuffers()
{
    // Create one command buffer for each swap chain image and reuse for rendering
    drawCmdBuffers.resize(swapChain.imageCount);

    VkCommandBufferAllocateInfo cmdBufAllocateInfo =
        VkAPI::initializers::commandBufferAllocateInfo(
            cmdPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            static_cast<uint32_t>(drawCmdBuffers.size()));

    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, drawCmdBuffers.data()));
}

void VulkanExampleBase::destroyCommandBuffers()
{
    vkFreeCommandBuffers(device, cmdPool, static_cast<uint32_t>(drawCmdBuffers.size()), drawCmdBuffers.data());
}

VkCommandBuffer VulkanExampleBase::createCommandBuffer(VkCommandBufferLevel level, bool begin)
{
    VkCommandBuffer cmdBuffer;

    VkCommandBufferAllocateInfo cmdBufAllocateInfo =
        VkAPI::initializers::commandBufferAllocateInfo(
            cmdPool,
            level,
            1);

    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &cmdBuffer));

    // If requested, also start the new command buffer
    if (begin)
    {
        VkCommandBufferBeginInfo cmdBufInfo = VkAPI::initializers::commandBufferBeginInfo();
        VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
    }

    return cmdBuffer;
}

void VulkanExampleBase::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
{
    if (commandBuffer == VK_NULL_HANDLE)
    {
        return;
    }

    VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK_RESULT(vkQueueWaitIdle(queue));

    if (free)
    {
        vkFreeCommandBuffers(device, cmdPool, 1, &commandBuffer);
    }
}

void VulkanExampleBase::createPipelineCache()
{
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VK_CHECK_RESULT(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache));
}

// ׼��
void VulkanExampleBase::prepare()
{
    if (vulkanDevice->enableDebugMarkers)
    {
        VkAPI::debugmarker::setup(device);
    }
    initSwapchain();
    createCommandPool();
    setupSwapChain();
    createCommandBuffers();
    setupDepthStencil();
    setupRenderPass();
    createPipelineCache();
    setupFrameBuffer();
    settings.overlay = settings.overlay && (!benchmark.active);
    if (settings.overlay)
    {
        VkAPI::UIOverlayCreateInfo overlayCreateInfo = {};
        // Setup default overlay creation info
        overlayCreateInfo.device = vulkanDevice;
        overlayCreateInfo.copyQueue = queue;
        overlayCreateInfo.framebuffers = frameBuffers;
        overlayCreateInfo.colorformat = swapChain.colorFormat;
        overlayCreateInfo.depthformat = depthFormat;
        overlayCreateInfo.width = width;
        overlayCreateInfo.height = height;
        // Virtual function call for example to customize overlay creation
        OnSetupUIOverlay(overlayCreateInfo);
        // Load default shaders if not specified by example
        if (overlayCreateInfo.shaders.size() == 0) 
        {
            overlayCreateInfo.shaders = 
            {
                loadShader(getAssetPath() + "shaders/ui/uioverlay.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
                loadShader(getAssetPath() + "shaders/ui/uioverlay.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT),
            };
        }
        UIOverlay = new VkAPI::UIOverlay(overlayCreateInfo);
        updateOverlay();
    }
}

// ������ɫ��
VkPipelineShaderStageCreateInfo VulkanExampleBase::loadShader(std::string fileName, VkShaderStageFlagBits stage)
{
    VkPipelineShaderStageCreateInfo shaderStage = {};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = stage;
    shaderStage.module = VkAPI::tools::loadShader(fileName.c_str(), device);
    shaderStage.pName = "main"; // todo : make param
    assert(shaderStage.module != VK_NULL_HANDLE);
    shaderModules.push_back(shaderStage.module);
    return shaderStage;
}

// ��Ⱦ֡
void VulkanExampleBase::renderFrame()
{
    auto tStart = std::chrono::high_resolution_clock::now();
    if (viewUpdated)
    {
        viewUpdated = false;
        viewChanged();
    }

    render();
    frameCounter++;
    auto tEnd = std::chrono::high_resolution_clock::now();
    auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
    frameTimer = (float)tDiff / 1000.0f;
    camera.update(frameTimer);
    if (camera.moving())
    {
        viewUpdated = true;
    }
    // Convert to clamped timer value
    if (!paused)
    {
        timer += timerSpeed * frameTimer;
        if (timer > 1.0)
        {
            timer -= 1.0f;
        }
    }
    fpsTimer += (float)tDiff;
    if (fpsTimer > 1000.0f)
    {
        lastFPS = static_cast<uint32_t>((float)frameCounter * (1000.0f / fpsTimer));
        if (!settings.overlay)
        {
            std::string windowTitle = getWindowTitle();
            SetWindowText(window, windowTitle.c_str());
        }
        fpsTimer = 0.0f;
        frameCounter = 0;
    }
    // TODO: Cap UI overlay update rates
    updateOverlay();
}

// ��Ⱦѭ��
void VulkanExampleBase::renderLoop()
{
    if (benchmark.active)
    {
        benchmark.run([=] { render(); }, vulkanDevice->properties);
        vkDeviceWaitIdle(device);
        if (benchmark.filename != "")
        {
            benchmark.saveResults();
        }
        return;
    }

    destWidth = width;
    destHeight = height;
    MSG msg;
    bool quitMessageReceived = false;
    while (!quitMessageReceived)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                quitMessageReceived = true;
                break;
            }
        }
        renderFrame();
    }
    // Flush device to make sure all resources can be freed
    if (device != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(device);
    }
}

// ����UI
void VulkanExampleBase::updateOverlay()
{
    if (!settings.overlay)
        return;

    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DeltaTime = frameTimer;

    io.MousePos = ImVec2(mousePos.x, mousePos.y);
    io.MouseDown[0] = mouseButtons.left;
    io.MouseDown[1] = mouseButtons.right;

    ImGui::NewFrame();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("Vulkan Example", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::TextUnformatted(title.c_str());
    ImGui::TextUnformatted(deviceProperties.deviceName);
    ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / lastFPS), lastFPS);

    ImGui::PushItemWidth(110.0f * UIOverlay->scale);
    OnUpdateUIOverlay(UIOverlay);
    ImGui::PopItemWidth();

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::Render();

    UIOverlay->update();
}

// ׼��֡
void VulkanExampleBase::prepareFrame()
{
    // Acquire the next image from the swap chain
    VkResult err = swapChain.acquireNextImage(semaphores.presentComplete, &currentBuffer);
    // Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
    if ((err == VK_ERROR_OUT_OF_DATE_KHR) || (err == VK_SUBOPTIMAL_KHR))
    {
        windowResize();
    }
    else
    {
        VK_CHECK_RESULT(err);
    }
}

void VulkanExampleBase::submitFrame()
{
    bool submitOverlay = settings.overlay && UIOverlay->visible;

    if (submitOverlay)
    {
        // Wait for color attachment output to finish before rendering the text overlay
        VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submitInfo.pWaitDstStageMask = &stageFlags;

        // Set semaphores
        // Wait for render complete semaphore
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &semaphores.renderComplete;
        // Signal ready with UI overlay complete semaphpre
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &semaphores.overlayComplete;

        // Submit current UI overlay command buffer
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &UIOverlay->cmdBuffers[currentBuffer];
        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

        // Reset stage mask
        submitInfo.pWaitDstStageMask = &submitPipelineStages;
        // Reset wait and signal semaphores for rendering next frame
        // Wait for swap chain presentation to finish
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &semaphores.presentComplete;
        // Signal ready with offscreen semaphore
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &semaphores.renderComplete;
    }

    VK_CHECK_RESULT(swapChain.queuePresent(queue, currentBuffer, submitOverlay ? semaphores.overlayComplete : semaphores.renderComplete));

    VK_CHECK_RESULT(vkQueueWaitIdle(queue));
}

VulkanExampleBase::VulkanExampleBase(bool enableValidation)
{
    // Check for a valid asset path
    struct stat info;
    if (stat(getAssetPath().c_str(), &info) != 0)
    {
        std::string msg = "Could not locate asset path in \"" + getAssetPath() + "\" !";
        MessageBox(NULL, msg.c_str(), "Fatal error", MB_OK | MB_ICONERROR);
        exit(-1);
    }

    settings.validation = enableValidation;

    char* numConvPtr;

    // Parse command line arguments
    for (size_t i = 0; i < args.size(); i++)
    {
        if (args[i] == std::string("-validation"))
        {
            settings.validation = true;
        }
        if (args[i] == std::string("-vsync"))
        {
            settings.vsync = true;
        }
        if ((args[i] == std::string("-f")) || (args[i] == std::string("--fullscreen")))
        {
            settings.fullscreen = true;
        }
        if ((args[i] == std::string("-w")) || (args[i] == std::string("-width"))) 
        {
            uint32_t w = strtol(args[i + 1], &numConvPtr, 10);
            if (numConvPtr != args[i + 1]) { width = w; };
        }
        if ((args[i] == std::string("-h")) || (args[i] == std::string("-height"))) 
        {
            uint32_t h = strtol(args[i + 1], &numConvPtr, 10);
            if (numConvPtr != args[i + 1]) { height = h; };
        }
        // Benchmark
        if ((args[i] == std::string("-b")) || (args[i] == std::string("--benchmark"))) 
        {
            benchmark.active = true;
            VkAPI::tools::errorModeSilent = true;
        }
        // Warmup time (in seconds)
        if ((args[i] == std::string("-bw")) || (args[i] == std::string("--benchwarmup")))
        {
            if (args.size() > i + 1)
            {
                uint32_t num = strtol(args[i + 1], &numConvPtr, 10);
                if (numConvPtr != args[i + 1]) 
                {
                    benchmark.warmup = num;
                }
                else 
                {
                    std::cerr << "Warmup time for benchmark mode must be specified as a number!" << std::endl;
                }
            }
        }
        // Benchmark runtime (in seconds)
        if ((args[i] == std::string("-br")) || (args[i] == std::string("--benchruntime"))) 
        {
            if (args.size() > i + 1) 
            {
                uint32_t num = strtol(args[i + 1], &numConvPtr, 10);
                if (numConvPtr != args[i + 1]) 
                {
                    benchmark.duration = num;
                }
                else 
                {
                    std::cerr << "Benchmark run duration must be specified as a number!" << std::endl;
                }
            }
        }
        // Bench result save filename (overrides default)
        if ((args[i] == std::string("-bf")) || (args[i] == std::string("--benchfilename"))) 
        {
            if (args.size() > i + 1) 
            {
                if (args[i + 1][0] == '-') 
                {
                    std::cerr << "Filename for benchmark results must not start with a hyphen!" << std::endl;
                }
                else 
                {
                    benchmark.filename = args[i + 1];
                }
            }
        }
        // Output frame times to benchmark result file
        if ((args[i] == std::string("-bt")) || (args[i] == std::string("--benchframetimes")))
        {
            benchmark.outputFrameTimes = true;
        }
    }

    // Enable console if validation is active
    // Debug message callback will output to it
    if (this->settings.validation)
    {
        setupConsole("Vulkan validation output");
    }
    setupDPIAwareness();
}

VulkanExampleBase::~VulkanExampleBase()
{
    // Clean up Vulkan resources
    swapChain.cleanup();
    if (descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    }
    destroyCommandBuffers();
    vkDestroyRenderPass(device, renderPass, nullptr);
    for (uint32_t i = 0; i < frameBuffers.size(); i++)
    {
        vkDestroyFramebuffer(device, frameBuffers[i], nullptr);
    }

    for (auto& shaderModule : shaderModules)
    {
        vkDestroyShaderModule(device, shaderModule, nullptr);
    }
    vkDestroyImageView(device, depthStencil.view, nullptr);
    vkDestroyImage(device, depthStencil.image, nullptr);
    vkFreeMemory(device, depthStencil.mem, nullptr);

    vkDestroyPipelineCache(device, pipelineCache, nullptr);

    vkDestroyCommandPool(device, cmdPool, nullptr);

    vkDestroySemaphore(device, semaphores.presentComplete, nullptr);
    vkDestroySemaphore(device, semaphores.renderComplete, nullptr);
    vkDestroySemaphore(device, semaphores.overlayComplete, nullptr);

    if (UIOverlay) {
        delete UIOverlay;
    }

    delete vulkanDevice;

    if (settings.validation)
    {
        VkAPI::debug::freeDebugCallback(instance);
    }

    vkDestroyInstance(instance, nullptr);
}

bool VulkanExampleBase::initVulkan()
{
    VkResult err;

    // Vulkan instance
    err = createInstance(settings.validation);
    if (err) {
        VkAPI::tools::exitFatal("Could not create Vulkan instance : \n" + VkAPI::tools::errorString(err), err);
        return false;
    }

    // If requested, we enable the default validation layers for debugging
    if (settings.validation)
    {
        // The report flags determine what type of messages for the layers will be displayed
        // For validating (debugging) an appplication the error and warning bits should suffice
        VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        // Additional flags include performance info, loader and layer debug messages, etc.
        VkAPI::debug::setupDebugging(instance, debugReportFlags, VK_NULL_HANDLE);
    }

    // Physical device
    uint32_t gpuCount = 0;
    // Get number of available physical devices
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr));
    assert(gpuCount > 0);
    // Enumerate devices
    std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
    err = vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());
    if (err) {
        VkAPI::tools::exitFatal("Could not enumerate physical devices : \n" + VkAPI::tools::errorString(err), err);
        return false;
    }

    // GPU selection

    // Select physical device to be used for the Vulkan example
    // Defaults to the first device unless specified by command line
    uint32_t selectedDevice = 0;

    // GPU selection via command line argument
    for (size_t i = 0; i < args.size(); i++)
    {
        // Select GPU
        if ((args[i] == std::string("-g")) || (args[i] == std::string("-gpu")))
        {
            char* endptr;
            uint32_t index = strtol(args[i + 1], &endptr, 10);
            if (endptr != args[i + 1])
            {
                if (index > gpuCount - 1)
                {
                    std::cerr << "Selected device index " << index << " is out of range, reverting to device 0 (use -listgpus to show available Vulkan devices)" << std::endl;
                }
                else
                {
                    std::cout << "Selected Vulkan device " << index << std::endl;
                    selectedDevice = index;
                }
            };
            break;
        }
        // List available GPUs
        if (args[i] == std::string("-listgpus"))
        {
            uint32_t gpuCount = 0;
            VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr));
            if (gpuCount == 0)
            {
                std::cerr << "No Vulkan devices found!" << std::endl;
            }
            else
            {
                // Enumerate devices
                std::cout << "Available Vulkan devices" << std::endl;
                std::vector<VkPhysicalDevice> devices(gpuCount);
                VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &gpuCount, devices.data()));
                for (uint32_t i = 0; i < gpuCount; i++) {
                    VkPhysicalDeviceProperties deviceProperties;
                    vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
                    std::cout << "Device [" << i << "] : " << deviceProperties.deviceName << std::endl;
                    std::cout << " Type: " << VkAPI::tools::physicalDeviceTypeString(deviceProperties.deviceType) << std::endl;
                    std::cout << " API: " << (deviceProperties.apiVersion >> 22) << "." << ((deviceProperties.apiVersion >> 12) & 0x3ff) << "." << (deviceProperties.apiVersion & 0xfff) << std::endl;
                }
            }
        }
    }

    physicalDevice = physicalDevices[selectedDevice];

    // Store properties (including limits), features and memory properties of the phyiscal device (so that examples can check against them)
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

    // Derived examples can override this to set actual features (based on above readings) to enable for logical device creation
    getEnabledFeatures();

    // Vulkan device creation
    // This is handled by a separate class that gets a logical device representation
    // and encapsulates functions related to a device
    vulkanDevice = new VkAPI::VulkanDevice(physicalDevice);
    VkResult res = vulkanDevice->createLogicalDevice(enabledFeatures, enabledDeviceExtensions);
    if (res != VK_SUCCESS) {
        VkAPI::tools::exitFatal("Could not create Vulkan device: \n" + VkAPI::tools::errorString(res), res);
        return false;
    }
    device = vulkanDevice->logicalDevice;

    // Get a graphics queue from the device
    vkGetDeviceQueue(device, vulkanDevice->queueFamilyIndices.graphics, 0, &queue);

    // Find a suitable depth format
    VkBool32 validDepthFormat = VkAPI::tools::getSupportedDepthFormat(physicalDevice, &depthFormat);
    assert(validDepthFormat);

    swapChain.connect(instance, physicalDevice, device);

    // Create synchronization objects
    VkSemaphoreCreateInfo semaphoreCreateInfo = VkAPI::initializers::semaphoreCreateInfo();
    // Create a semaphore used to synchronize image presentation
    // Ensures that the image is displayed before we start submitting new commands to the queu
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.presentComplete));
    // Create a semaphore used to synchronize command submission
    // Ensures that the image is not presented until all commands have been sumbitted and executed
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.renderComplete));
    // Create a semaphore used to synchronize command submission
    // Ensures that the image is not presented until all commands for the UI overlay have been sumbitted and executed
    // Will be inserted after the render complete semaphore if the UI overlay is enabled
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.overlayComplete));

    // Set up submit info structure
    // Semaphores will stay the same during application lifetime
    // Command buffer submission info is set by each example
    submitInfo = VkAPI::initializers::submitInfo();
    submitInfo.pWaitDstStageMask = &submitPipelineStages;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphores.presentComplete;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphores.renderComplete;

    return true;
}

void VulkanExampleBase::setupConsole(std::string title)
{
    AllocConsole();
    AttachConsole(GetCurrentProcessId());
    FILE* stream;
    freopen_s(&stream, "CONOUT$", "w+", stdout);
    freopen_s(&stream, "CONOUT$", "w+", stderr);
    SetConsoleTitle(TEXT(title.c_str()));
}

void VulkanExampleBase::setupDPIAwareness()
{
    using SetProcessDpiAwarenessFunc = HRESULT(*)(PROCESS_DPI_AWARENESS);

    HMODULE shCore = LoadLibraryA("Shcore.dll");
    if (shCore)
    {
        SetProcessDpiAwarenessFunc setProcessDpiAwareness =
            (SetProcessDpiAwarenessFunc)GetProcAddress(shCore, "SetProcessDpiAwareness");

        if (setProcessDpiAwareness != nullptr)
        {
            setProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
        }

        FreeLibrary(shCore);
    }
}

// ���ô���
HWND VulkanExampleBase::setupWindow(HINSTANCE hinstance, WNDPROC wndproc)
{
    this->windowInstance = hinstance;

    WNDCLASSEX wndClass;

    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = wndproc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hinstance;
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = name.c_str();
    wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

    if (!RegisterClassEx(&wndClass))
    {
        std::cout << "Could not register window class!\n";
        fflush(stdout);
        exit(1);
    }

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    if (settings.fullscreen)
    {
        DEVMODE dmScreenSettings;
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = screenWidth;
        dmScreenSettings.dmPelsHeight = screenHeight;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        if ((width != (uint32_t)screenWidth) && (height != (uint32_t)screenHeight))
        {
            if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
            {
                if (MessageBox(NULL, "Fullscreen Mode not supported!\n Switch to window mode?", "Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
                {
                    settings.fullscreen = false;
                }
                else
                {
                    return nullptr;
                }
            }
        }

    }

    DWORD dwExStyle;
    DWORD dwStyle;

    if (settings.fullscreen)
    {
        dwExStyle = WS_EX_APPWINDOW;
        dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    }
    else
    {
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    }

    RECT windowRect;
    windowRect.left = 0L;
    windowRect.top = 0L;
    windowRect.right = settings.fullscreen ? (long)screenWidth : (long)width;
    windowRect.bottom = settings.fullscreen ? (long)screenHeight : (long)height;

    AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

    std::string windowTitle = getWindowTitle();
    window = CreateWindowEx(0,
        name.c_str(),
        windowTitle.c_str(),
        dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0,
        0,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        NULL,
        NULL,
        hinstance,
        NULL);

    if (!settings.fullscreen)
    {
        // Center on screen
        uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
        uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
        SetWindowPos(window, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }

    if (!window)
    {
        printf("Could not create window!\n");
        fflush(stdout);
        return nullptr;
        exit(1);
    }

    ShowWindow(window, SW_SHOW);
    SetForegroundWindow(window);
    SetFocus(window);

    return window;
}

void VulkanExampleBase::handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        prepared = false;
        DestroyWindow(hWnd);
        PostQuitMessage(0);
        break;
    case WM_PAINT:
        ValidateRect(window, NULL);
        break;
    case WM_KEYDOWN:
        switch (wParam)
        {
        case KEY_P:
            paused = !paused;
            break;
        case KEY_F1:
            if (settings.overlay) {
                UIOverlay->visible = !UIOverlay->visible;
            }
            break;
        case KEY_ESCAPE:
            PostQuitMessage(0);
            break;
        }

        if (camera.firstperson)
        {
            switch (wParam)
            {
            case KEY_W:
                camera.keys.up = true;
                break;
            case KEY_S:
                camera.keys.down = true;
                break;
            case KEY_A:
                camera.keys.left = true;
                break;
            case KEY_D:
                camera.keys.right = true;
                break;
            }
        }

        keyPressed((uint32_t)wParam);
        break;
    case WM_KEYUP:
        if (camera.firstperson)
        {
            switch (wParam)
            {
            case KEY_W:
                camera.keys.up = false;
                break;
            case KEY_S:
                camera.keys.down = false;
                break;
            case KEY_A:
                camera.keys.left = false;
                break;
            case KEY_D:
                camera.keys.right = false;
                break;
            }
        }
        break;
    case WM_LBUTTONDOWN:
        mousePos = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
        mouseButtons.left = true;
        break;
    case WM_RBUTTONDOWN:
        mousePos = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
        mouseButtons.right = true;
        break;
    case WM_MBUTTONDOWN:
        mousePos = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
        mouseButtons.middle = true;
        break;
    case WM_LBUTTONUP:
        mouseButtons.left = false;
        break;
    case WM_RBUTTONUP:
        mouseButtons.right = false;
        break;
    case WM_MBUTTONUP:
        mouseButtons.middle = false;
        break;
    case WM_MOUSEWHEEL:
    {
        short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        zoom += (float)wheelDelta * 0.005f * zoomSpeed;
        camera.translate(glm::vec3(0.0f, 0.0f, (float)wheelDelta * 0.005f * zoomSpeed));
        viewUpdated = true;
        break;
    }
    case WM_MOUSEMOVE:
    {
        handleMouseMove(LOWORD(lParam), HIWORD(lParam));
        break;
    }
    case WM_SIZE:
        if ((prepared) && (wParam != SIZE_MINIMIZED))
        {
            if ((resizing) || ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED)))
            {
                destWidth = LOWORD(lParam);
                destHeight = HIWORD(lParam);
                windowResize();
            }
        }
        break;
    case WM_ENTERSIZEMOVE:
        resizing = true;
        break;
    case WM_EXITSIZEMOVE:
        resizing = false;
        break;
    }
}

void VulkanExampleBase::viewChanged() {}

void VulkanExampleBase::keyPressed(uint32_t) {}

void VulkanExampleBase::mouseMoved(double x, double y, bool& handled) {}

void VulkanExampleBase::buildCommandBuffers() {}

void VulkanExampleBase::createCommandPool()
{
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = swapChain.queueNodeIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &cmdPool));
        }

void VulkanExampleBase::setupDepthStencil()
{
    VkImageCreateInfo image = {};
    image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image.pNext = NULL;
    image.imageType = VK_IMAGE_TYPE_2D;
    image.format = depthFormat;
    image.extent = { width, height, 1 };
    image.mipLevels = 1;
    image.arrayLayers = 1;
    image.samples = VK_SAMPLE_COUNT_1_BIT;
    image.tiling = VK_IMAGE_TILING_OPTIMAL;
    image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image.flags = 0;

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    VkImageViewCreateInfo depthStencilView = {};
    depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthStencilView.pNext = NULL;
    depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthStencilView.format = depthFormat;
    depthStencilView.flags = 0;
    depthStencilView.subresourceRange = {};
    depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    depthStencilView.subresourceRange.baseMipLevel = 0;
    depthStencilView.subresourceRange.levelCount = 1;
    depthStencilView.subresourceRange.baseArrayLayer = 0;
    depthStencilView.subresourceRange.layerCount = 1;

    VkMemoryRequirements memReqs;

    VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &depthStencil.image));
    vkGetImageMemoryRequirements(device, depthStencil.image, &memReqs);
    mem_alloc.allocationSize = memReqs.size;
    mem_alloc.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device, &mem_alloc, nullptr, &depthStencil.mem));
    VK_CHECK_RESULT(vkBindImageMemory(device, depthStencil.image, depthStencil.mem, 0));

    depthStencilView.image = depthStencil.image;
    VK_CHECK_RESULT(vkCreateImageView(device, &depthStencilView, nullptr, &depthStencil.view));
}

void VulkanExampleBase::setupFrameBuffer()
{
    VkImageView attachments[2];

    // Depth/Stencil attachment is the same for all frame buffers
    attachments[1] = depthStencil.view;

    VkFramebufferCreateInfo frameBufferCreateInfo = {};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.pNext = NULL;
    frameBufferCreateInfo.renderPass = renderPass;
    frameBufferCreateInfo.attachmentCount = 2;
    frameBufferCreateInfo.pAttachments = attachments;
    frameBufferCreateInfo.width = width;
    frameBufferCreateInfo.height = height;
    frameBufferCreateInfo.layers = 1;

    // Create frame buffers for every swap chain image
    frameBuffers.resize(swapChain.imageCount);
    for (uint32_t i = 0; i < frameBuffers.size(); i++)
    {
        attachments[0] = swapChain.buffers[i].view;
        VK_CHECK_RESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &frameBuffers[i]));
    }
}

void VulkanExampleBase::setupRenderPass()
{
    std::array<VkAttachmentDescription, 2> attachments = {};
    // Color attachment
    attachments[0].format = swapChain.colorFormat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // Depth attachment
    attachments[1].format = depthFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference = {};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthReference = {};
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = nullptr;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = nullptr;
    subpassDescription.pResolveAttachments = nullptr;

    // Subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
}

void VulkanExampleBase::getEnabledFeatures()
{
    // Can be overriden in derived class
}

void VulkanExampleBase::windowResize()
{
    if (!prepared)
    {
        return;
    }
    prepared = false;

    // Ensure all operations on the device have been finished before destroying resources
    vkDeviceWaitIdle(device);

    // Recreate swap chain
    width = destWidth;
    height = destHeight;
    setupSwapChain();

    // Recreate the frame buffers
    vkDestroyImageView(device, depthStencil.view, nullptr);
    vkDestroyImage(device, depthStencil.image, nullptr);
    vkFreeMemory(device, depthStencil.mem, nullptr);
    setupDepthStencil();
    for (uint32_t i = 0; i < frameBuffers.size(); i++) {
        vkDestroyFramebuffer(device, frameBuffers[i], nullptr);
    }
    setupFrameBuffer();

    // Command buffers need to be recreated as they may store
    // references to the recreated frame buffer
    destroyCommandBuffers();
    createCommandBuffers();
    buildCommandBuffers();

    vkDeviceWaitIdle(device);

    if (settings.overlay) {
        UIOverlay->resize(width, height, frameBuffers);
    }

    camera.updateAspectRatio((float)width / (float)height);

    // Notify derived class
    windowResized();
    viewChanged();

    prepared = true;
}

void VulkanExampleBase::handleMouseMove(int32_t x, int32_t y)
{
    int32_t dx = (int32_t)mousePos.x - x;
    int32_t dy = (int32_t)mousePos.y - y;

    bool handled = false;

    if (settings.overlay) {
        ImGuiIO& io = ImGui::GetIO();
        handled = io.WantCaptureMouse;
    }
    mouseMoved((float)x, (float)y, handled);

    if (handled) {
        mousePos = glm::vec2((float)x, (float)y);
        return;
    }

    if (mouseButtons.left) {
        rotation.x += dy * 1.25f * rotationSpeed;
        rotation.y -= dx * 1.25f * rotationSpeed;
        camera.rotate(glm::vec3(dy * camera.rotationSpeed, -dx * camera.rotationSpeed, 0.0f));
        viewUpdated = true;
    }
    if (mouseButtons.right) {
        zoom += dy * .005f * zoomSpeed;
        camera.translate(glm::vec3(-0.0f, 0.0f, dy * .005f * zoomSpeed));
        viewUpdated = true;
    }
    if (mouseButtons.middle) {
        cameraPos.x -= dx * 0.01f;
        cameraPos.y -= dy * 0.01f;
        camera.translate(glm::vec3(-dx * 0.01f, -dy * 0.01f, 0.0f));
        viewUpdated = true;
    }
    mousePos = glm::vec2((float)x, (float)y);
}

void VulkanExampleBase::windowResized()
{
    // Can be overriden in derived class
}

void VulkanExampleBase::initSwapchain()
{
    swapChain.initSurface(windowInstance, window);
}

void VulkanExampleBase::setupSwapChain()
{
    swapChain.create(&width, &height, settings.vsync);
}

void VulkanExampleBase::OnSetupUIOverlay(VkAPI::UIOverlayCreateInfo& createInfo) {}
void VulkanExampleBase::OnUpdateUIOverlay(VkAPI::UIOverlay* overlay) {}
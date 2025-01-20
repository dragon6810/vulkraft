#include <Renderer/Renderer.h>

#include <stdio.h>
#include <assert.h>
#include <vector>

#include <VkBootstrap.h>

#include <Testing/Testing.h>

#define RENDERER_DOVALIDATIONLAYERS true
#define RENDERER_VERBOSE_LOGGING true
#define RENDERER_DEVICE_COUNT 1
#define RENDERER_DEVICE_GFX 0

#define RENDERER_USE_VALIDATION_LAYERS true

void Renderer::VulkanAssertImpl(VkResult result, const char* expr, const char* file, int line)
{
    const char *errorstr;

    if (result == VK_SUCCESS)
        return;

    switch (result) 
    {
        case VK_NOT_READY: errorstr = "VK_NOT_READY"; break;
        case VK_TIMEOUT: errorstr = "VK_TIMEOUT"; break;
        case VK_EVENT_SET: errorstr = "VK_EVENT_SET"; break;
        case VK_EVENT_RESET: errorstr = "VK_EVENT_RESET"; break;
        case VK_INCOMPLETE: errorstr = "VK_INCOMPLETE"; break;
        case VK_ERROR_OUT_OF_HOST_MEMORY: errorstr = "VK_ERROR_OUT_OF_HOST_MEMORY"; break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: errorstr = "VK_ERROR_OUT_OF_DEVICE_MEMORY"; break;
        case VK_ERROR_INITIALIZATION_FAILED: errorstr = "VK_ERROR_INITIALIZATION_FAILED"; break;
        case VK_ERROR_DEVICE_LOST: errorstr = "VK_ERROR_DEVICE_LOST"; break;
        case VK_ERROR_MEMORY_MAP_FAILED: errorstr = "VK_ERROR_MEMORY_MAP_FAILED"; break;
        case VK_ERROR_LAYER_NOT_PRESENT: errorstr = "VK_ERROR_LAYER_NOT_PRESENT"; break;
        case VK_ERROR_EXTENSION_NOT_PRESENT: errorstr = "VK_ERROR_EXTENSION_NOT_PRESENT"; break;
        case VK_ERROR_FEATURE_NOT_PRESENT: errorstr = "VK_ERROR_FEATURE_NOT_PRESENT"; break;
        case VK_ERROR_INCOMPATIBLE_DRIVER: errorstr = "VK_ERROR_INCOMPATIBLE_DRIVER"; break;
        case VK_ERROR_TOO_MANY_OBJECTS: errorstr = "VK_ERROR_TOO_MANY_OBJECTS"; break;
        case VK_ERROR_FORMAT_NOT_SUPPORTED: errorstr = "VK_ERROR_FORMAT_NOT_SUPPORTED"; break;
        case VK_ERROR_FRAGMENTED_POOL: errorstr = "VK_ERROR_FRAGMENTED_POOL"; break;
        case VK_ERROR_UNKNOWN: errorstr = "VK_ERROR_UNKNOWN"; break;
        default: errorstr = "UNKNOWN_ERROR_CODE"; break;
    }

    // Print error message and terminate
    fprintf(stderr, "Vulkan assertion failed!\n");
    fprintf(stderr, "Error: %s (code: %d)\n", errorstr, result);
    fprintf(stderr, "Expression: %s\n", expr);
    fprintf(stderr, "File: %s\n", file);
    fprintf(stderr, "Line: %d\n", line);
    exit(EXIT_FAILURE);
}

Renderer::FrameData* Renderer::GetFrame(void)
{
    return &this->frames[this->curframe % RENDERER_MAX_FIF];
}

void Renderer::DestroySwapchain(void)
{
    int i;

    vkDestroySwapchainKHR(this->vkdevice, this->swapchain, NULL);
    for(i=0; i<this->swapchainimageviews.size(); i++)
        vkDestroyImageView(this->vkdevice, this->swapchainimageviews[i], NULL);
}

void Renderer::Cleanup(void)
{
    int i;

    if(!this->initialized)
        return;

    vkDeviceWaitIdle(this->vkdevice);

    for(i=0; i<RENDERER_MAX_FIF; i++)
        vkDestroyCommandPool(this->vkdevice, this->frames[i].cmdpool, NULL);

    DestroySwapchain();
    vkDestroyDevice(this->vkdevice, NULL);
    vkDestroySurfaceKHR(this->vkinstance, this->surface, NULL);
    vkb::destroy_debug_utils_messenger(this->vkinstance, this->debugmessenger);
    vkDestroyInstance(this->vkinstance, NULL);
    glfwDestroyWindow(win);
    glfwTerminate();
}

void Renderer::MakeCommandStructures(void)
{
    int i;

    VkCommandPoolCreateInfo poolcreateinfo;
    VkCommandBufferAllocateInfo buffallocinfo;

    poolcreateinfo = {};
    poolcreateinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolcreateinfo.pNext = NULL;
    poolcreateinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolcreateinfo.queueFamilyIndex = this->queues[RENDERER_QUEUE_GRAPHICS].family;

    for(i=0; i<RENDERER_MAX_FIF; i++)
    {
        VulkanAssert(vkCreateCommandPool(this->vkdevice, &poolcreateinfo, NULL, &this->frames[i].cmdpool));
    
        buffallocinfo = {};
        buffallocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        buffallocinfo.pNext = NULL;
        buffallocinfo.commandPool = this->frames[i].cmdpool;
        buffallocinfo.commandBufferCount = 1;
        buffallocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        VulkanAssert(vkAllocateCommandBuffers(this->vkdevice, &buffallocinfo, &this->frames[i].maincmdbuffer));
    }
}

void Renderer::MakeSwapchain(int w, int h)
{
    vkb::SwapchainBuilder builder { this->vkphysicaldevice, this->vkdevice, this->surface };
    vkb::Swapchain vkbswapchain;

    this->swapchainimgformat = VK_FORMAT_B8G8R8A8_UNORM;

    builder.set_desired_format(VkSurfaceFormatKHR { .format = this->swapchainimgformat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR });
    builder.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR);
    builder.set_desired_extent(w, h);
    builder.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    vkbswapchain = builder.build().value();
    this->swapchain = vkbswapchain.swapchain;
    this->swapchainextent = vkbswapchain.extent;
    this->swapchainimages = vkbswapchain.get_images().value();
    this->swapchainimageviews = vkbswapchain.get_image_views().value();
}

void Renderer::MakeDevice(void)
{
    VkPhysicalDeviceSynchronization2Features sync2features{};
    VkPhysicalDeviceVulkan12Features features12;
    vkb::PhysicalDeviceSelector selector { this->vkbinstance };
    vkb::Result<vkb::PhysicalDevice> selectres { vkb::Error() };
    vkb::PhysicalDevice vkbphysicaldevice;
    vkb::DeviceBuilder builder { vkbphysicaldevice };
    vkb::Device vkbdevice;

    sync2features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
    sync2features.pNext = NULL;
    sync2features.synchronization2 = true;

    features12 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    selector.set_minimum_version(1, 2);
    selector.set_required_features_12(features12);
    selector.add_required_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    selector.add_required_extension(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
    selector.add_required_extension(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    selector.add_required_extension(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    selector.add_required_extension(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    selector.add_required_extension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    selector.add_required_extension_features(sync2features);
    selector.set_surface(this->surface);

    selectres = selector.select();

    VulkraftAssert(selectres);

    vkbphysicaldevice = selectres.value();
    builder = vkb::DeviceBuilder(vkbphysicaldevice);
    vkbdevice = builder.build().value();
    
    this->vkphysicaldevice = vkbdevice.physical_device;
    this->vkdevice = vkbdevice.device;

    // Queues

    this->queues[RENDERER_QUEUE_GRAPHICS].queue = vkbdevice.get_queue(vkb::QueueType::graphics).value();
    this->queues[RENDERER_QUEUE_GRAPHICS].family = vkbdevice.get_queue_index(vkb::QueueType::graphics).value();
}

void Renderer::MakeSurface(void)
{
    VulkanAssert(glfwCreateWindowSurface(this->vkinstance, this->win, NULL, &this->surface));
}

void Renderer::MakeVkInstance(void)
{
    int i;

    vkb::InstanceBuilder builder;
    vkb::Result<vkb::Instance> buildres { vkb::Error() };

    builder.set_app_name("vulkraft");
    builder.request_validation_layers(RENDERER_USE_VALIDATION_LAYERS);
    builder.use_default_debug_messenger();
    builder.require_api_version(1, 2, 0);
    
#ifdef MACOS
    builder.enable_extension("VK_EXT_metal_surface");
    builder.enable_extension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

    buildres = builder.build();
    VulkraftAssert(buildres);

    this->vkbinstance = buildres.value();

    this->vkinstance = this->vkbinstance.instance;
    this->debugmessenger = this->vkbinstance.debug_messenger;

    return;
}

void Renderer::MakeVulkan(void)
{
    int w, h;

    MakeVkInstance();
    MakeSurface();
    MakeDevice();
    glfwGetWindowSize(win, &w, &h);
    MakeSwapchain(w, h);
    MakeCommandStructures();
}

void Renderer::MakeWindow(void)
{
    assert(!this->win);
    assert(glfwInit());

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    this->win = glfwCreateWindow(1280, 720, "vulkraft", nullptr, nullptr);
}

void Renderer::Kill(void)
{
    Cleanup();
}

void Renderer::Launch(void)
{
    while(!glfwWindowShouldClose(win))
    {
        glfwPollEvents();
    }
}

void Renderer::Initialize(void)
{
    assert(!this->initialized);
    
    MakeWindow();
    MakeVulkan();

    printf("renderer initialize.\n");
    this->initialized = true;
}
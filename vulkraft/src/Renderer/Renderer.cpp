#include <Renderer/Renderer.h>

#include <stdio.h>
#include <assert.h>
#include <vector>

#include <VkBootstrap.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <Testing/Testing.h>
#include <Renderer/VulkanHelper.h>

#define RENDERER_DOVALIDATIONLAYERS true
#define RENDERER_VERBOSE_LOGGING true
#define RENDERER_DEVICE_COUNT 1
#define RENDERER_DEVICE_GFX 0

#define RENDERER_USE_VALIDATION_LAYERS true

#define S_TO_NS(s) (1000000000 * s)

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

void Renderer::CleanupCommandPools(void)
{
    int i;

    for(i=0; i<this->allcommandpools.size(); i++)
        vkDestroyCommandPool(this->vkdevice, this->allcommandpools[i], NULL);
}

void Renderer::CleanupFences(void)
{
    int i;

    for(i=0; i<this->allfences.size(); i++)
        vkDestroyFence(this->vkdevice, this->allfences[i], NULL);
}

void Renderer::CleanupSemaphores(void)
{
    int i;

    for(i=0; i<this->allsemaphores.size(); i++)
        vkDestroySemaphore(this->vkdevice, this->allsemaphores[i], NULL);
}

void Renderer::CleanupImageViews(void)
{
    int i;

    for(i=0; i<this->allimageviews.size(); i++)
        vkDestroyImageView(this->vkdevice, this->allimageviews[i], NULL);
}

void Renderer::CleanupImages(void)
{
    int i;

    for(i=0; i<this->allimages.size(); i++)
        vkDestroyImage(this->vkdevice, this->allimages[i], NULL); 
}

void Renderer::CleanupSwapchain(void)
{
    vkDestroySwapchainKHR(this->vkdevice, this->swapchain, NULL);
}

void Renderer::CleanupAllocator(void)
{
    vmaDestroyAllocator(this->allocator);
}

void Renderer::CleanupDevice(void)
{
    vkDestroyDevice(this->vkdevice, NULL);
}

void Renderer::CleanupSurface(void)
{
    vkDestroySurfaceKHR(this->vkinstance, this->surface, NULL);
}

void Renderer::CleanupDebugMessenger(void)
{
    vkb::destroy_debug_utils_messenger(this->vkinstance, this->debugmessenger);
}

void Renderer::CleanupInstance(void)
{
    vkDestroyInstance(this->vkinstance, NULL);
}

void Renderer::CleanupGLFW(void)
{
    glfwDestroyWindow(win);
    glfwTerminate();
}

void Renderer::Cleanup(void)
{
    int i;

    if(!this->initialized)
        return;

    vkDeviceWaitIdle(this->vkdevice);

    this->deletionqueue.Flush();
}

void Renderer::DrawBackground(VkCommandBuffer cmd)
{
    VkClearColorValue clearcol;
    VkImageSubresourceRange clearrange;

    clearcol = { { 0.0, sinf((float) curframe / 120.0) / 2.0f + 0.5f, 0.0 } };
    clearrange = {};
    clearrange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clearrange.baseMipLevel = 0;
    clearrange.levelCount = VK_REMAINING_MIP_LEVELS;
    clearrange.baseArrayLayer = 0;
    clearrange.layerCount = VK_REMAINING_ARRAY_LAYERS;
    vkCmdClearColorImage(cmd, this->drawimg.img, VK_IMAGE_LAYOUT_GENERAL, &clearcol, 1, &clearrange);
}

void Renderer::Draw(void)
{
    uint32_t iswapchainimg;
    VkCommandBufferBeginInfo cmdbuffinfo;
    VkPipelineStageFlags dstmask;
    VkSubmitInfo queuesubmitinfo;
    VkPresentInfoKHR presentinfo;

    VulkanAssert(vkWaitForFences(this->vkdevice, 1, &GetFrame()->renderfence, true, S_TO_NS(1)));
    VulkanAssert(vkResetFences(this->vkdevice, 1, &GetFrame()->renderfence));

    VulkanAssert(vkAcquireNextImageKHR(this->vkdevice, this->swapchain, S_TO_NS(1), GetFrame()->swapchainsemaphore, NULL, &iswapchainimg));

    VulkanAssert(vkResetCommandBuffer(GetFrame()->maincmdbuffer, 0));
    cmdbuffinfo = VulkanHelper::CmdBuffBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VulkanAssert(vkBeginCommandBuffer(GetFrame()->maincmdbuffer, &cmdbuffinfo));

    this->drawimgextent.width = this->drawimg.extent.width;
    this->drawimgextent.height = this->drawimg.extent.height;

    VulkanHelper::ImgChangeLayout(GetFrame()->maincmdbuffer, this->drawimg.img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    
    DrawBackground(GetFrame()->maincmdbuffer);

    VulkanHelper::ImgChangeLayout(GetFrame()->maincmdbuffer, this->drawimg.img, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    VulkanHelper::ImgChangeLayout(GetFrame()->maincmdbuffer, this->swapchainimages[iswapchainimg], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VulkanHelper::ImgBlitToImg(GetFrame()->maincmdbuffer, this->drawimg.img, this->swapchainimages[iswapchainimg], this->drawimgextent, this->swapchainextent);
    VulkanHelper::ImgChangeLayout(GetFrame()->maincmdbuffer, this->swapchainimages[iswapchainimg], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VulkanAssert(vkEndCommandBuffer(GetFrame()->maincmdbuffer));

    dstmask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    queuesubmitinfo = {};
    queuesubmitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    queuesubmitinfo.pNext = NULL;
    queuesubmitinfo.waitSemaphoreCount = 1;
    queuesubmitinfo.pWaitSemaphores = &GetFrame()->swapchainsemaphore;
    queuesubmitinfo.pWaitDstStageMask = &dstmask;
    queuesubmitinfo.commandBufferCount = 1;
    queuesubmitinfo.pCommandBuffers = &GetFrame()->maincmdbuffer;
    queuesubmitinfo.signalSemaphoreCount = 1;
    queuesubmitinfo.pSignalSemaphores = &GetFrame()->rendersemaphore;

    VulkanAssert(vkQueueSubmit(this->queues[RENDERER_QUEUE_GRAPHICS].queue, 1, &queuesubmitinfo, GetFrame()->renderfence));

    presentinfo = {};
    presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentinfo.pNext = NULL;
    presentinfo.swapchainCount = 1;
    presentinfo.pSwapchains = &this->swapchain;
    presentinfo.waitSemaphoreCount = 1;
    presentinfo.pWaitSemaphores = &GetFrame()->rendersemaphore;
    presentinfo.pImageIndices = &iswapchainimg;

    VulkanAssert(vkQueuePresentKHR(this->queues[RENDERER_QUEUE_GRAPHICS].queue, &presentinfo));

    curframe++;
}

void Renderer::PopulateDeletionQueue(void)
{
    this->deletionqueue.Push([&] { CleanupGLFW(); });
    this->deletionqueue.Push([&] { CleanupInstance(); });
    this->deletionqueue.Push([&] { CleanupDebugMessenger(); });
    this->deletionqueue.Push([&] { CleanupSurface(); });
    this->deletionqueue.Push([&] { CleanupDevice(); });
    this->deletionqueue.Push([&] { CleanupAllocator(); });
    this->deletionqueue.Push([&] { CleanupSwapchain(); });
    this->deletionqueue.Push([&] { CleanupImages(); });
    this->deletionqueue.Push([&] { CleanupImageViews(); });
    this->deletionqueue.Push([&] { CleanupSemaphores(); });
    this->deletionqueue.Push([&] { CleanupFences(); });
    this->deletionqueue.Push([&] { CleanupCommandPools(); });
}

void Renderer::MakeSyncStructures(void)
{
    int i;

    VkFenceCreateInfo fencecreateinfo;
    VkSemaphoreCreateInfo semaphorecreateinfo;

    fencecreateinfo = {};
    fencecreateinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fencecreateinfo.pNext = NULL;
    fencecreateinfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    semaphorecreateinfo = {};
    semaphorecreateinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphorecreateinfo.pNext = NULL;
    semaphorecreateinfo.flags = 0;

    for(i=0; i<RENDERER_MAX_FIF; i++)
    {
        VulkanAssert(vkCreateFence(this->vkdevice, &fencecreateinfo, NULL, &this->frames[i].renderfence));

        VulkanAssert(vkCreateSemaphore(this->vkdevice, &semaphorecreateinfo, NULL, &this->frames[i].rendersemaphore));
        VulkanAssert(vkCreateSemaphore(this->vkdevice, &semaphorecreateinfo, NULL, &this->frames[i].swapchainsemaphore));
    }
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
    int i;

    vkb::SwapchainBuilder builder { this->vkphysicaldevice, this->vkdevice, this->surface };
    vkb::Swapchain vkbswapchain;
    VkImageCreateInfo drawimgcreateinfo;
    VkImageViewCreateInfo drawimgviewcreateinfo;
    VmaAllocationCreateInfo drawimgalloccreateinfo;

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

    for(i=0; i<this->swapchainimages.size(); i++)
        this->allimages.push_back(this->swapchainimages[i]);

    for(i=0; i<this->swapchainimageviews.size(); i++)
        this->allimageviews.push_back(this->swapchainimageviews[i]);

    this->drawimg.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    this->drawimg.extent = { (unsigned int) w, (unsigned int) h, 1 };
    drawimgcreateinfo = VulkanHelper::ImgCreateInfo
    (
        this->drawimg.format,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        this->drawimg.extent
    );

    drawimgalloccreateinfo = {};
    drawimgalloccreateinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    drawimgalloccreateinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VulkanAssert(vmaCreateImage
    (
        this->allocator,
        &drawimgcreateinfo,
        &drawimgalloccreateinfo,
        &this->drawimg.img,
        &this->drawimg.alloc,
        NULL
    ));

    drawimgviewcreateinfo = VulkanHelper::ImgViewCreateInfo(this->drawimg.format, this->drawimg.img, VK_IMAGE_ASPECT_COLOR_BIT);
    VulkanAssert(vkCreateImageView(this->vkdevice, &drawimgviewcreateinfo, NULL, &this->drawimg.view));
    this->allimages.push_back(this->drawimg.img);
    this->allimageviews.push_back(this->drawimg.view);
}

void Renderer::MakeAllocator(void)
{
    VmaAllocatorCreateInfo createinfo;

    createinfo = {};
    createinfo.physicalDevice = this->vkphysicaldevice;
    createinfo.device = this->vkdevice;
    createinfo.instance = this->vkinstance;
    createinfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    VulkanAssert(vmaCreateAllocator(&createinfo, &this->allocator));
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
    MakeAllocator();
    glfwGetWindowSize(win, &w, &h);
    MakeSwapchain(w, h);
    MakeCommandStructures();
    MakeSyncStructures();
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

        Draw();
    }

    alldone = true;
}

void Renderer::Initialize(void)
{
    assert(!this->initialized);
    
    MakeWindow();
    MakeVulkan();

    this->initialized = true;
}
#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

#include <Renderer/DeletionQueue.h>
#include <Renderer/VulkanHelper.h>
#include <Renderer/DescriptorAllocator.h>

#define RENDERER_MAX_FIF 2

#define RENDERER_QUEUE_COUNT 1
#define RENDERER_QUEUE_GRAPHICS 0

#define VulkanAssert(result) Renderer::VulkanAssertImpl((result), #result, __FILE__, __LINE__)

class Renderer
{
public:
    struct FrameData
    {
        VkCommandPool cmdpool;
        VkCommandBuffer maincmdbuffer;

        VkFence renderfence;
        VkSemaphore rendersemaphore;
        VkSemaphore swapchainsemaphore;
    };

    struct Queue
    {
        VkQueue queue;
        unsigned int family;
    };
private:
    bool initialized = false;
    int framenum = 0;
    bool stop = false;

    GLFWwindow *win = 0;

    vkb::Instance vkbinstance;

    DeletionQueue deletionqueue;

    VkInstance vkinstance;
    VkDebugUtilsMessengerEXT debugmessenger;
    VmaAllocator allocator;
    VkPhysicalDevice vkphysicaldevice;
    VkDevice vkdevice;
    VkSurfaceKHR surface;

    VkSwapchainKHR swapchain;
    VkFormat swapchainimgformat;
    std::vector<VkImage> swapchainimages;
    std::vector<VkImageView> swapchainimageviews;
    VkExtent2D swapchainextent;

    std::vector<VkImage> allimages;
    std::vector<VkImageView> allimageviews;
    std::vector<VkSemaphore> allsemaphores;
    std::vector<VkFence> allfences;
    std::vector<VkCommandPool> allcommandpools;
    std::vector<DescriptorAllocator*> alldescriptorallocators;

    VulkanHelper::AllocatedImg drawimg;
    VkExtent2D drawimgextent;

    DescriptorAllocator gpdescriptoralloc;
    VkDescriptorSet drawimgdescriptors;
    VkDescriptorSetLayout drawimgdescriptorlayout;

    VkPipeline gradpipeline;
	VkPipelineLayout gradpipelinelayout;

    unsigned long int curframe = 0;
    FrameData frames[RENDERER_MAX_FIF];

    Queue queues[RENDERER_QUEUE_COUNT];
public:
    bool alldone = false;
private:
    void MakeWindow(void);
    void MakeVulkan(void);
    void MakeVkInstance(void);
    void MakeSurface(void);
    void MakeDevice(void);
    void MakeAllocator(void);
    void MakeSwapchain(int w, int h);
    void MakeCommandStructures(void);
    void MakeSyncStructures(void);
    void MakeDescriptors(void);
    void MakePipelines(void);
    void PopulateDeletionQueue(void);

    void MakeGradientPipeline(void);

    void Draw(void);
    void DrawBackground(VkCommandBuffer cmd);

    void Cleanup(void);

    void CleanupGLFW(void);
    void CleanupInstance(void);
    void CleanupDebugMessenger(void);
    void CleanupSurface(void);
    void CleanupDevice(void);
    void CleanupAllocator(void);
    void CleanupImages(void);
    void CleanupSwapchain(void);
    void CleanupImageViews(void);
    void CleanupSemaphores(void);
    void CleanupFences(void);
    void CleanupCommandPools(void);
    void CleanupDescriptorAllocators(void);
    void CleanupPipelines(void);
    void CleanupPipelineLayouts(void);
public:
    static void VulkanAssertImpl(VkResult result, const char* expr, const char* file, int line);

    void Initialize(void);
    void Launch(void);
    void Kill(void);

    FrameData* GetFrame(void);
};
#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>

// what is the maximum number of cpu threads we'll use while rendering?
#define RENDERER_THREAD_COUNT 1

#define RENDERER_QUEUE_COUNT 4
#define RENDERER_QUEUE_GRAPHICS 0
#define RENDERER_QUEUE_COMPUTE 1
#define RENDERER_QUEUE_TRANSFER 2
#define RENDERER_QUEUE_PRESENTATION 3

#define VulkanAssert(result) Renderer::VulkanAssertImpl((result), #result, __FILE__, __LINE__)

class Renderer
{
private:
    bool initialized = false;
    int framenum = 0;
    bool stop = false;

    GLFWwindow *win = 0;

    vkb::Instance vkbinstance;

    VkInstance vkinstance;
    VkDebugUtilsMessengerEXT debugmessenger;
    VkPhysicalDevice vkphysicaldevice;
    VkDevice vkdevice;
    VkSurfaceKHR surface;
    
    VkSwapchainKHR swapchain;
    VkFormat swapchainimgformat;
    std::vector<VkImage> swapchainimages;
    std::vector<VkImageView> swapchainimageviews;
    VkExtent2D swapchainextent;
private:
    void MakeWindow(void);
    void MakeVulkan(void);
    void MakeVkInstance(void);
    void MakeSurface(void);
    void MakeDevice(void);
    void MakeSwapchain(int w, int h);

    void DestroySwapchain(void);
    void Cleanup(void);
public:
    static void VulkanAssertImpl(VkResult result, const char* expr, const char* file, int line);

    void Initialize(void);
    void Launch(void);
    void Kill(void);
};
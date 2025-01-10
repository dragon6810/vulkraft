#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

// what is the maximum number of cpu threads we'll use while rendering?
#define RENDERER_THREAD_COUNT 1

#define RENDERER_QUEUE_COUNT 3
#define RENDERER_QUEUE_GRAPHICS 0
#define RENDERER_QUEUE_COMPUTE 1
#define RENDERER_QUEUE_TRANSFER 2

#define VulkanAssert(result) Renderer::VulkanAssertImpl((result), #result, __FILE__, __LINE__)

class Renderer
{
private:
    typedef struct queue_s
    {
        int familyindex;
        VkDeviceQueueCreateInfo createinfo;
    } queue_t;

    typedef struct device_s
    {
        VkPhysicalDevice *physicaldevice;
        VkDevice logicaldevice;

        std::array<std::vector<VkCommandPool>, RENDERER_QUEUE_COUNT> cmdpools;

        std::array<queue_t, RENDERER_QUEUE_COUNT> queues;
    } device_t;
private:
    bool initialized = false;
    int framenum = 0;
    bool stop = false;

    GLFWwindow *win = 0;
    VkExtent2D windowdims = { 1280, 720 };
    
    std::vector<VkPhysicalDevice> allphysicaldevices;
    std::vector<device_t> devices;

    const std::vector<const char*> validationlayers = { "VK_LAYER_KHRONOS_validation" };

    VkInstance vkinstance;
private:
    void MakeWindow(void);
    void MakeVulkan(void);
    void MakeVkInstance(void);
    void FindPhysicalDevices(void);
    void PrintPhysicalDevicesInfo(void);
    void PrintPhysicalDeviceInfo(VkPhysicalDevice* device);
    void ChoosePhysicalDevice(void);
    void ChooseDeviceQueueFamilies(void);
    void MakeQueues(void);
    void MakeDeviceQueues(device_t* device);
    void MakeLogicalDevices(void);
    void MakeCommandPools(void);
    void MakeDeviceCommandPools(device_t* device);
    void MakeCommandBuffers(void);
public:
    static void VulkanAssertImpl(VkResult result, const char* expr, const char* file, int line);

    void Initialize(void);
    void Launch(void);
    void Kill(void);
};
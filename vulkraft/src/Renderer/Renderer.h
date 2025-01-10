#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#define RENDERER_QUEUE_COUNT 3
#define RENDERER_QUEUE_GRAPHICS 0
#define RENDERER_QUEUE_COMPUTE 1
#define RENDERER_QUEUE_TRANSFER 2

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
public:
    void Initialize(void);
    void Launch(void);
    void Kill(void);
};
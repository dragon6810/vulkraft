#include <Renderer/Renderer.h>

#include <stdio.h>
#include <assert.h>
#include <vector>

#define RENDERER_DOVALIDATIONLAYERS true
#define RENDERER_VERBOSE_LOGGING true
#define RENDERER_DEVICE_COUNT 1
#define RENDERER_DEVICE_GFX 0

void Renderer::MakeLogicalDevices(void)
{
    int i, j;

    VkDeviceCreateInfo createinfo;
    std::vector<VkDeviceQueueCreateInfo> queuecreateinfos;

    assert(this->devices.size() == RENDERER_DEVICE_COUNT);

    for(i=0; i<this->devices.size(); i++)
    {
        queuecreateinfos.resize(this->devices[i].queues.size());
        for(j=0; j<queuecreateinfos.size(); j++)
            queuecreateinfos[j] = this->devices[i].queues[j].createinfo;
        
        createinfo = {};
        createinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createinfo.pNext = nullptr;
        createinfo.flags = 0;
        createinfo.queueCreateInfoCount = queuecreateinfos.size();
        createinfo.pQueueCreateInfos = queuecreateinfos.data();

        assert(vkCreateDevice(*this->devices[i].physicaldevice, &createinfo, nullptr, &this->devices[i].logicaldevice) == VK_SUCCESS);
    }
}

void Renderer::MakeDeviceQueues(device_t* device)
{
    int i;

    queue_t *curqueue;
    float priority;

    priority = 0.5;
    for(i=0; i<device->queues.size(); i++)
    {
        curqueue = &device->queues[i];

        curqueue->createinfo = {};
        curqueue->createinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        curqueue->createinfo.pNext = nullptr;
        curqueue->createinfo.flags = 0;
        curqueue->createinfo.queueFamilyIndex = curqueue->familyindex;
        curqueue->createinfo.queueCount = 1;
        curqueue->createinfo.pQueuePriorities = &priority;
    }   
}

void Renderer::MakeQueues(void)
{
    int i;

    for(i=0; i<this->devices.size(); i++)
        MakeDeviceQueues(&this->devices[i]);
}

void Renderer::ChooseDeviceQueueFamilies(void)
{
    int i, j;

    std::vector<VkQueueFamilyProperties> familyprops;
    uint32_t nfamilyprops;
    
    assert(this->devices.size() == RENDERER_DEVICE_COUNT);

    for(i=0; i<this->devices.size(); i++)
    {
        vkGetPhysicalDeviceQueueFamilyProperties(*this->devices[i].physicaldevice, &nfamilyprops, nullptr);
        familyprops.resize(nfamilyprops);
        vkGetPhysicalDeviceQueueFamilyProperties(*this->devices[i].physicaldevice, &nfamilyprops, familyprops.data());
        this->devices[i].queues[RENDERER_QUEUE_GRAPHICS].familyindex = 0;
        this->devices[i].queues[RENDERER_QUEUE_COMPUTE].familyindex = 1;
        this->devices[i].queues[RENDERER_QUEUE_TRANSFER].familyindex = 2;
        for(j=0; j<familyprops.size(); j++)
        {
            if(RENDERER_VERBOSE_LOGGING)
            {
                printf("queue family %d:\n", j);
                printf("    queue count: %u.\n", familyprops[j].queueCount);
                printf("    type:\n");
                if(familyprops[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    printf("        graphics.\n");
                if(familyprops[j].queueFlags & VK_QUEUE_COMPUTE_BIT)
                    printf("        compute.\n");
                if(familyprops[j].queueFlags & VK_QUEUE_TRANSFER_BIT)
                    printf("        transfer.\n");
                if(familyprops[j].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
                    printf("        sparse binding.\n");
                if(familyprops[j].queueFlags & VK_QUEUE_PROTECTED_BIT)
                    printf("        protected.\n");
                if(familyprops[j].queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
                    printf("        video decode.\n");
                if(familyprops[j].queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
                    printf("        video encode.\n");
                if(familyprops[j].queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV)
                    printf("        optical flow.\n");
            }
        }
    }
}

void Renderer::ChoosePhysicalDevice(void)
{
    assert(this->allphysicaldevices.data());

    this->devices.resize(RENDERER_DEVICE_COUNT);

    // TODO: make a heuristic or something to choose a good device

    this->devices[RENDERER_DEVICE_GFX].physicaldevice = &this->allphysicaldevices[0];
}

void Renderer::PrintPhysicalDeviceInfo(VkPhysicalDevice* device)
{
    VkPhysicalDeviceProperties props;

    assert(device);

    vkGetPhysicalDeviceProperties(*device, &props);

    printf("physical device \"%s\":\n", props.deviceName);
    printf("    type: ");
    switch(props.deviceType)
    {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        printf("unkown.\n");
        break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        printf("integrated gpu.\n");
        break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        printf("discrete gpu.\n");
        break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        printf("virtual gpu.\n");
        break;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        printf("cpu.\n");
        break;
    }
    printf("    limits:\n");
    printf("        max 1d image dimension: %u.\n", props.limits.maxImageDimension1D);
    printf("        max 2d image dimension: %u.\n", props.limits.maxImageDimension2D);
    printf("        max 3d image dimension: %u.\n", props.limits.maxImageDimension3D);
    printf("        max image array layers: %u.\n", props.limits.maxImageArrayLayers);
    printf("        max framebuffer width: %u.\n", props.limits.maxFramebufferWidth);
    printf("        max framebuffer height: %u.\n", props.limits.maxFramebufferHeight);
}

void Renderer::PrintPhysicalDevicesInfo(void)
{
    int i;

    for(i=0; i<this->allphysicaldevices.size(); i++)
        PrintPhysicalDeviceInfo(&this->allphysicaldevices[i]);
}

void Renderer::FindPhysicalDevices(void)
{
    uint32_t ndevices;

    assert(vkEnumeratePhysicalDevices(this->vkinstance, &ndevices, nullptr) == VK_SUCCESS);
    this->allphysicaldevices.resize(ndevices);
    assert(vkEnumeratePhysicalDevices(this->vkinstance, &ndevices, this->allphysicaldevices.data()) == VK_SUCCESS);
}

void Renderer::MakeVkInstance(void)
{
    int i;

    unsigned int nglfwexts;
    const char **glfwexts;
    std::vector<const char*> exts;
    VkInstanceCreateInfo createinfo{};
    VkApplicationInfo appinfo{};

    glfwexts = glfwGetRequiredInstanceExtensions(&nglfwexts);
    exts.resize(nglfwexts);
    for(i=0; i<nglfwexts; i++)
        exts[i] = glfwexts[i];
    exts.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appinfo.pNext = nullptr;
    appinfo.pApplicationName = "vulkraft";
    appinfo.applicationVersion = 1;
    appinfo.pEngineName = "vulkraft engine";
    appinfo.engineVersion = 1;
    appinfo.apiVersion = VK_API_VERSION_1_3;

    createinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createinfo.pNext = nullptr;
    createinfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    createinfo.pApplicationInfo = &appinfo;
    createinfo.enabledExtensionCount = exts.size();
    createinfo.ppEnabledExtensionNames = exts.data();

    assert(vkCreateInstance(&createinfo, nullptr, &this->vkinstance) == VK_SUCCESS);
}

void Renderer::MakeVulkan(void)
{
    MakeVkInstance();
    FindPhysicalDevices();
    if(RENDERER_VERBOSE_LOGGING)
        PrintPhysicalDevicesInfo();
    ChoosePhysicalDevice();
    ChooseDeviceQueueFamilies();
    MakeQueues();
    MakeLogicalDevices();
}

void Renderer::MakeWindow(void)
{
    assert(!this->win);
    assert(glfwInit());

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    this->win = glfwCreateWindow(this->windowdims.width, this->windowdims.height, "vulkraft", nullptr, nullptr);
}

void Renderer::Kill(void)
{

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
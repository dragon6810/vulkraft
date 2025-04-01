#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

class VulkanHelper
{
public:
    struct AllocatedImg
    {
        VkImage img;
        VkImageView view;
        VmaAllocation alloc;
        VkExtent3D extent;
        VkFormat format;
    };
public:
    static VkCommandBufferBeginInfo CmdBuffBeginInfo(VkCommandBufferUsageFlags flags);
    
    static VkImageCreateInfo ImgCreateInfo(VkFormat format, VkImageUsageFlags usageflags, VkExtent3D extent);
    static VkImageViewCreateInfo ImgViewCreateInfo(VkFormat format, VkImage img, VkImageAspectFlags aspectflags);
    static void ImgChangeLayout(VkCommandBuffer cmd, VkImage img, VkImageLayout oldlay, VkImageLayout newlay);
    static void ImgBlitToImg(VkCommandBuffer cmd, VkImage src, VkImage dst, VkExtent2D srcext, VkExtent2D dstext);

    static bool ShaderLoadModule(VkDevice device, const char* filepath, VkShaderModule* module);
};
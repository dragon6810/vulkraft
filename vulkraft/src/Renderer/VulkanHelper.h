#pragma once

#include <vulkan/vulkan.h>

class VulkanHelper
{
public:
    static VkCommandBufferBeginInfo CmdBuffBeginInfo(VkCommandBufferUsageFlags flags);
    
    static void ImgChangeLayout(VkCommandBuffer cmd, VkImage img, VkImageLayout oldlay, VkImageLayout newlay);
};
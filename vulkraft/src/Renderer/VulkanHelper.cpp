#include <Renderer/VulkanHelper.h>

void VulkanHelper::ImgChangeLayout(VkCommandBuffer cmd, VkImage img, VkImageLayout oldlay, VkImageLayout newlay)
{
    VkImageMemoryBarrier imgbarrier;
    VkImageSubresourceRange subimg;

    imgbarrier = {};
    imgbarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imgbarrier.pNext = NULL;

    imgbarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    imgbarrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;

    imgbarrier.oldLayout = oldlay;
    imgbarrier.newLayout = newlay;

    subimg = {};
    subimg.aspectMask = (newlay == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    subimg.baseMipLevel = 0;
    subimg.levelCount = VK_REMAINING_MIP_LEVELS;
    subimg.baseArrayLayer = 0;
    subimg.layerCount = VK_REMAINING_ARRAY_LAYERS;

    imgbarrier.subresourceRange = subimg;
    imgbarrier.image = img;

    vkCmdPipelineBarrier
    (
        cmd,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0,
        0,
        NULL,
        0,
        NULL,
        1, &imgbarrier
    );
}

VkCommandBufferBeginInfo VulkanHelper::CmdBuffBeginInfo(VkCommandBufferUsageFlags flags)
{
    VkCommandBufferBeginInfo begininfo;

    begininfo = {};
    begininfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begininfo.pNext = NULL;
    begininfo.flags = flags;
    begininfo.pInheritanceInfo = NULL;

    return begininfo;
}
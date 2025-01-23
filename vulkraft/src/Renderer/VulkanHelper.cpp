#include <Renderer/VulkanHelper.h>

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

VkImageCreateInfo VulkanHelper::ImgCreateInfo(VkFormat format, VkImageUsageFlags usageflags, VkExtent3D extent)
{
    VkImageCreateInfo createinfo;

    createinfo = {};
    createinfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createinfo.pNext = NULL;
    createinfo.imageType = VK_IMAGE_TYPE_2D;
    createinfo.format = format;
    createinfo.extent = extent;
    createinfo.mipLevels = 1;
    createinfo.arrayLayers = 1;
    createinfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createinfo.usage = usageflags;

    return createinfo;
}

VkImageViewCreateInfo VulkanHelper::ImgViewCreateInfo(VkFormat format, VkImage img, VkImageAspectFlags aspectflags)
{
    VkImageViewCreateInfo createinfo;

    createinfo = {};
    createinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createinfo.pNext = NULL;
    createinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createinfo.image = img;
    createinfo.format = format;
    createinfo.subresourceRange.baseMipLevel = 0;
    createinfo.subresourceRange.levelCount = 1;
    createinfo.subresourceRange.baseArrayLayer = 0;
    createinfo.subresourceRange.layerCount = 1;
    createinfo.subresourceRange.aspectMask = aspectflags;

    return createinfo;
}

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

void VulkanHelper::ImgBlitToImg(VkCommandBuffer cmd, VkImage src, VkImage dst, VkExtent2D srcext, VkExtent2D dstext)
{
    VkImageBlit blitregion;

    blitregion = {};
    blitregion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitregion.srcSubresource.baseArrayLayer = 0;
    blitregion.srcSubresource.layerCount = 1;
    blitregion.srcSubresource.mipLevel = 0;
    blitregion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitregion.dstSubresource.baseArrayLayer = 0;
    blitregion.dstSubresource.layerCount = 1;
    blitregion.dstSubresource.mipLevel = 0;
    blitregion.srcOffsets[1] = { (int) srcext.width, (int) srcext.height, 1 };
    blitregion.dstOffsets[1] = { (int) dstext.width, (int) dstext.height, 1 };

    vkCmdBlitImage
    (
        cmd,
        src,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dst,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &blitregion,
        VK_FILTER_LINEAR
    );
}
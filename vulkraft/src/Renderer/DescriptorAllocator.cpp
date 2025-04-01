#include <renderer/DescriptorAllocator.h>

#include <vector>

#include <Renderer/Renderer.h>

DescriptorAllocator::DescriptorAllocator()
{
    
}

DescriptorAllocator::DescriptorAllocator(VkDevice* device)
{
    assert(device);

    this->device = device;
}

VkDescriptorSet DescriptorAllocator::Allocate(VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo allocinfo;
    VkDescriptorSet set;

    allocinfo = {};
    allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocinfo.pNext = NULL;
    allocinfo.descriptorPool = this->pool;
    allocinfo.descriptorSetCount = 1;
    allocinfo.pSetLayouts = &layout;

    VulkanAssert(vkAllocateDescriptorSets(*this->device, &allocinfo, &set));

    return set;
}

void DescriptorAllocator::DestroyPool(void)
{
    vkDestroyDescriptorPool(*this->device, this->pool, NULL);
}

void DescriptorAllocator::ClearDescriptors(void)
{
    VulkanAssert(vkResetDescriptorPool(*this->device, this->pool, 0));   
}

void DescriptorAllocator::InitializePool(unsigned int maxsets, std::span<PoolSizeRatio> poolratios)
{
    int i;

    std::vector<VkDescriptorPoolSize> sizes;
    VkDescriptorPoolCreateInfo createinfo;

    sizes.resize(poolratios.size());
    for(i=0; i<poolratios.size(); i++)
        sizes[i] = { poolratios[i].type, (uint32_t) (poolratios[i].ratio * maxsets) };
    
    createinfo = {};
    createinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createinfo.pNext = NULL;
    createinfo.flags = 0;
    createinfo.maxSets = maxsets;
    createinfo.poolSizeCount = sizes.size();
    createinfo.pPoolSizes = sizes.data();

    VulkanAssert(vkCreateDescriptorPool(*this->device, &createinfo, NULL, &this->pool));
}
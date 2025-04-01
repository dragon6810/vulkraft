#pragma once

#include <span>

#include <vulkan/vulkan.h>

class DescriptorAllocator
{
public:
    DescriptorAllocator();
    DescriptorAllocator(VkDevice* device);
public:
    struct PoolSizeRatio
    {
        VkDescriptorType type;
        float ratio;
    };
private:
    VkDevice* device;

    VkDescriptorPool pool;
public:
    void InitializePool(unsigned int maxsets, std::span<PoolSizeRatio> poolratios);
    void ClearDescriptors(void);
    void DestroyPool(void);

    VkDescriptorSet Allocate(VkDescriptorSetLayout layout);
};
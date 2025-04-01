#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class DescriptorLayoutBuilder
{
private:
    std::vector<VkDescriptorSetLayoutBinding> bindings;
public:
    void AddBinding(unsigned int binding, VkDescriptorType type);
    void ClearBindings(void);
    VkDescriptorSetLayout GenerateLayout(VkDevice device, VkShaderStageFlags stageflags, void* next = NULL, VkDescriptorSetLayoutCreateFlags flags = 0);
};
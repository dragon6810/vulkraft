#include <Renderer/DescriptorLayoutBuilder.h>

#include <Renderer/Renderer.h>

void DescriptorLayoutBuilder::AddBinding(unsigned int binding, VkDescriptorType type)
{
    VkDescriptorSetLayoutBinding newbinding;

    newbinding = {};
    newbinding.binding = binding;
    newbinding.descriptorCount = 1;
    newbinding.descriptorType = type;

    this->bindings.push_back(newbinding);
}

void DescriptorLayoutBuilder::ClearBindings(void)
{
    this->bindings.clear();
}

VkDescriptorSetLayout DescriptorLayoutBuilder::GenerateLayout(VkDevice device, VkShaderStageFlags stageflags, void* next, VkDescriptorSetLayoutCreateFlags flags)
{
    int i;

    VkDescriptorSetLayoutCreateInfo createinfo;
    VkDescriptorSetLayout set;

    for(i=0; i<this->bindings.size(); i++)
        this->bindings[i].stageFlags |= stageflags;

    createinfo = {};
    createinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createinfo.pNext = next;
    createinfo.bindingCount = this->bindings.size();
    createinfo.pBindings = this->bindings.data();
    createinfo.flags = flags;

    VulkanAssert(vkCreateDescriptorSetLayout(device, &createinfo, NULL, &set));

    return set;   
}
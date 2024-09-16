#pragma once
#include "globals.h"
#include "vkdecl.h"

class DescSet {
    public:
        void wrt(VkWriteDescriptorSet* wrtDesc, ui32 binding);

        VkDescriptorSet handle;
        VkDevice        _dvc;
};

class DescPool {
    public:

        VkResult create(const VulkanData& vkdata);
        void     dstr();

        VkResult allocDescSet(DescSet* outSet); 
        void     freeDescSet(DescSet* outSet);

        VulkanData _vkdata;

        VkDescriptorSetLayout  _lytHandle;
        VkDescriptorPool        handle;

        ui32 _maxSets = 10;
        std::vector<VkDescriptorSetLayoutBinding> _lytBindings = {
            { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
            { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
        };
};

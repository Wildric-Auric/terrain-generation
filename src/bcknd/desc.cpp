#include "desc.h"

void DescSet::wrt(VkWriteDescriptorSet* wrtDesc, ui32 binding) {
    wrtDesc->sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wrtDesc->dstSet     = this->handle;
    wrtDesc->dstBinding = binding;
    vkUpdateDescriptorSets(_dvc, 1, wrtDesc, 0, nullptr);
}

VkResult DescPool::create(const VulkanData& vkdata) {
    VkDescriptorPoolCreateInfo      poolCrtInfo{};
    VkDescriptorSetLayoutCreateInfo crtInfo{}; 
    std::vector<VkDescriptorPoolSize> poolSizes(_lytBindings.size());
    VkResult  res;
    _vkdata = vkdata;

    crtInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    crtInfo.bindingCount = _lytBindings.size(); 
    crtInfo.pBindings    = _lytBindings.data();
    res = vkCreateDescriptorSetLayout(_vkdata.dvc, &crtInfo, nullptr, &_lytHandle);

    for (arch i = 0; i < _lytBindings.size(); ++i) {
        poolSizes[i].type            = _lytBindings[i].descriptorType;
        poolSizes[i].descriptorCount = _maxSets; 
    }

    poolCrtInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCrtInfo.poolSizeCount = poolSizes.size();
    poolCrtInfo.pPoolSizes    = poolSizes.data();
    poolCrtInfo.maxSets       = _maxSets;

    res = vkCreateDescriptorPool(_vkdata.dvc, &poolCrtInfo, nullptr, &handle);

    return res;
}
    
VkResult DescPool::allocDescSet(DescSet* outSet) {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = handle;
    allocInfo.descriptorSetCount = 1; 
    allocInfo.pSetLayouts        = &_lytHandle;

    outSet->_dvc = _vkdata.dvc;
    return vkAllocateDescriptorSets(_vkdata.dvc, &allocInfo, &outSet->handle);
}

void DescPool::freeDescSet(DescSet* outSet) {
    vkFreeDescriptorSets(_vkdata.dvc, handle, 1, &outSet->handle);

}

void DescPool::dstr() {
   vkDestroyDescriptorPool(_vkdata.dvc, handle, nullptr); 
   vkDestroyDescriptorSetLayout(_vkdata.dvc, _lytHandle, nullptr);
}

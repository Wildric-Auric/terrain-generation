#include "sync.h"

VkSemaphoreCreateInfo& Semaphore::fillCrtInfo(const VulkanData& data, bool isBinary) {
    _vkdata = data;
    _typeCrtInfo.sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    _typeCrtInfo.pNext         = nullptr;
    _typeCrtInfo.semaphoreType = isBinary ? VK_SEMAPHORE_TYPE_BINARY : VK_SEMAPHORE_TYPE_TIMELINE;
    _typeCrtInfo.initialValue  = 0;
    _crtInfo.flags = 0;
    _crtInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    _crtInfo.pNext = &_typeCrtInfo;
    return _crtInfo;
}

VkResult Semaphore::create() {
    VkResult res = vkCreateSemaphore(_vkdata.dvc, &_crtInfo, nullptr, &handle);
    return res;
}
    
void Semaphore::dstr() {
    if (!handle) return;
    vkDestroySemaphore(_vkdata.dvc, handle, nullptr);
}


bool Semaphore::isBin() {
    return _typeCrtInfo.semaphoreType == VK_SEMAPHORE_TYPE_BINARY;
}

VkResult Semaphore::signal(ui64 val) {
    VkSemaphoreSignalInfo inf{};
    inf.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
    inf.value = val;
    inf.semaphore = handle;
    return vkSignalSemaphore(_vkdata.dvc, &inf);
}

VkResult Semaphore::wait(ui64 val) {
    VkSemaphoreWaitInfo inf{};
    inf.semaphoreCount = 1;
    inf.pSemaphores = &handle;
    inf.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    inf.pValues = &val;
    return vkWaitSemaphores(_vkdata.dvc, &inf, (uint64_t)-1);
}

VkFenceCreateInfo& Fence::fillCrtInfo(const VulkanData& data, bool isSignaled) {
    _vkdata = data;
    _crtInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    _crtInfo.flags = isSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
    _crtInfo.pNext = 0;
    return _crtInfo;
}

VkResult Fence::create() {
    return vkCreateFence(_vkdata.dvc, &_crtInfo, nullptr, &handle);
}

VkResult Fence::wait() {
    return vkWaitForFences(_vkdata.dvc, 1, &handle, 1, (ui64)-1);
}

void    Fence::dstr() {
    if (!handle) return;
    vkDestroyFence(_vkdata.dvc, handle, nullptr);
}

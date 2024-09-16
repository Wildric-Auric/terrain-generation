#include "cmd_buffer.h"
#include "support.h"


VkResult CmdBufferPool::create(const VulkanData& vkdata, i32 queueFamilyIndex) {
    _vkdata = vkdata;
    VulkanSupport::QueueFamIndices qfam;
    VulkanSupport::findQueues(qfam, _vkdata);    

    crtInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    crtInfo.queueFamilyIndex = queueFamilyIndex;
    crtInfo.flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
    return vkCreateCommandPool(vkdata.dvc, &crtInfo, nullptr, &handle);
}

void     CmdBufferPool::dstr() {
    vkDestroyCommandPool(_vkdata.dvc, handle, nullptr);
}


VkResult CmdBufferPool::allocCmdBuff(CmdBuff* outBuff, VkQueue queue) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = handle;
    allocInfo.commandBufferCount = 1; 
    outBuff->queue = queue;
    return vkAllocateCommandBuffers(_vkdata.dvc, &allocInfo, &outBuff->handle);
}

void CmdBufferPool::freeCmdBuff(const CmdBuff& cmdBuff) {
    vkFreeCommandBuffers(_vkdata.dvc, handle, 1, &cmdBuff.handle);
}

void CmdBufferPool::execBegin(CmdBuff* outBuff, i32 index) {
    VkCommandBufferAllocateInfo allocInfo{};
    VkCommandBufferBeginInfo beginInfo{};
    VkCommandBuffer buff;
    VkQueue q;

    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool        = handle;

    vkAllocateCommandBuffers(_vkdata.dvc, &allocInfo, &buff);
    
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(buff, &beginInfo);

    q = VulkanSupport::getQueue(_vkdata, index);

    outBuff->handle = buff;
    outBuff->queue  = q;
    return; 
}

void CmdBufferPool::execEnd(CmdBuff& buff) { 
    vkEndCommandBuffer(buff.handle);
    buff.submit();
    vkQueueWaitIdle(buff.queue); //TODO::Use fence instead 
    vkFreeCommandBuffers(_vkdata.dvc, handle, 1, &buff.handle);
}


VkResult CmdBuff::submit(VkFence fence) {
    VkSubmitInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.commandBufferCount = 1;
    info.pCommandBuffers = &handle; 
    return vkQueueSubmit(queue, 1, &info, fence);
}

#pragma once
#include "vkdecl.h"
#include "globals.h"

//TODO::an abstraction for command buffer, abstracting queue submission, a comamnd buffer must have 
class CmdBuff {
    public:
        VkQueue         queue;
        VkCommandBuffer handle;
        VkResult        submit(VkFence = NULL);
};


class CmdBufferPool {
public:

    VkResult create(const VulkanData&, i32); 
    VkResult allocCmdBuff(CmdBuff* outBuff, VkQueue queue);
    void     freeCmdBuff(const CmdBuff& cmdBuff);
    void     dstr(); 
    
    void execBegin(CmdBuff* outBuff, i32 queueIndex = -1);
    void execEnd(CmdBuff&);

    VulkanData _vkdata;
    VkCommandPool handle;
    VkCommandPoolCreateInfo crtInfo{};
};




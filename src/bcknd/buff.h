#pragma once
#include <vulkan/vulkan.h>
#include "vkdecl.h" 
#include "globals.h"
#include "cmd_buffer.h"


class Buffer {
public:
    VkResult create(const VulkanData&, ui32);
    void     dstr();
    VkBufferCreateInfo& fillCrtInfo();

    void cpyFrom(CmdBufferPool cmdPool, const Buffer& other);
    void cpyTo(CmdBufferPool   cmdPool, Buffer& other);

    void mapMem(void**);
    void unmapMem(void**);
    void wrt(void* mappedDst, void* src, arch size); 

    VulkanData         _vkdata;
    ui32               _size;

    VkBuffer           handle;
    VkDeviceMemory     mem; 

    VkBufferCreateInfo    crtInfo{};
    VkMemoryPropertyFlags memProp = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
};
//Basically a buffer in host memory
class UniBuff {
    public:
    VkResult create(const VulkanData&, ui32 uniformSize);
    void     dstr();   
    void     wrt(void* uniformStruct);

    Buffer _buff;
    void*  _mapped = nullptr;
};

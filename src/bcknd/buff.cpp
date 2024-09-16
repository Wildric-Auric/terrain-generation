#include "buff.h"
#include "support.h"

void Buffer::mapMem(void** adr) {
    vkMapMemory(_vkdata.dvc, mem, 0, _size, 0, adr);
}

void Buffer::unmapMem(void** adr) {
    vkUnmapMemory(_vkdata.dvc, mem);
}

void Buffer::wrt(void* dst, void* src, arch size) {
    memcpy(dst, src, size);
}

VkResult Buffer::create(const VulkanData& vkdata, ui32 size) {
    VkMemoryAllocateInfo allocInfo{};
    VkResult res;
    

    _size   = size;
    _vkdata = vkdata;

    crtInfo.size = size;
    res = vkCreateBuffer(_vkdata.dvc, &crtInfo, nullptr, &handle);

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(_vkdata.dvc, handle, &memReq);

    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize  = memReq.size; //memReq.size is different from size; seems like memReq.size has is 2^n 
    allocInfo.memoryTypeIndex = VulkanSupport::findMem(_vkdata, memReq.memoryTypeBits, memProp);
    
    vkAllocateMemory(vkdata.dvc, &allocInfo, nullptr, &mem);
    vkBindBufferMemory(vkdata.dvc, handle, mem, 0);

    return res;
}

VkBufferCreateInfo& Buffer::fillCrtInfo() {

    crtInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    crtInfo.size  = 0;
    crtInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    crtInfo.usage       = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    return crtInfo;
}
    
void Buffer::dstr() {
    vkDestroyBuffer(_vkdata.dvc, handle, nullptr);
    vkFreeMemory(_vkdata.dvc, mem, nullptr);
}
    
void Buffer::cpyFrom(CmdBufferPool pool, const Buffer& other) {
    VkBufferCopy cpyRgn;
    cpyRgn.srcOffset = 0;
    cpyRgn.dstOffset = 0;
    cpyRgn.size      = other._size;

    CmdBuff buff;

    pool.execBegin(&buff, offsetof(VulkanSupport::QueueFamIndices, gfx) );

    vkCmdCopyBuffer(buff.handle, other.handle, handle, 1, &cpyRgn);

    pool.execEnd(buff);
}

void Buffer::cpyTo(CmdBufferPool pool, Buffer& other) {
   other.cpyFrom(pool, *this);
}


VkResult UniBuff::create(const VulkanData& vkdata, ui32 uniformSize) {
    _buff.fillCrtInfo();
    _buff.memProp = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    _buff.crtInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; 
    VkResult res = _buff.create(vkdata, uniformSize);
    _buff.mapMem(&_mapped);
    return res;
}

void     UniBuff::dstr() {
    _buff.unmapMem(&_mapped);
    _buff.dstr();
}

void     UniBuff::wrt(void* uniformStruct) {
    _buff.wrt(_mapped, uniformStruct, _buff._size);
}



#include "vertex.h"

std::vector<VkVertexInputBindingDescription>   Vertex::bindings;
std::vector<VkVertexInputAttributeDescription> Vertex::attribs;

const VkVertexInputBindingDescription*  Vertex::getBindings(arch* outSize) {
       if (outSize != nullptr)
           *outSize = bindings.size();
       return bindings.data(); 
}

const VkVertexInputAttributeDescription* Vertex::getAttribs(arch* outSize) {
    if (outSize != nullptr)
        *outSize = attribs.size();
    return attribs.data();
}
        
const VkVertexInputBindingDescription*   Vertex::setBindings(arch* outSize) {
    bindings.resize(1, {});
    bindings[0].binding = 0;
    bindings[0].stride  = sizeof(VertexData);
    bindings[0].inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;

    if (outSize != nullptr)
        *outSize = bindings.size();
    return bindings.data();
}

const VkVertexInputAttributeDescription* Vertex::setAttribs(arch* outSize) {
    attribs.resize(2, {});
    attribs[0].binding = 0;
    attribs[0].location= 0;
    attribs[0].offset  = offsetof(VertexData, pos);
    attribs[0].format  = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;

    attribs[1].binding = 0;
    attribs[1].location= 1;
    attribs[1].offset  = offsetof(VertexData, uv);
    attribs[1].format  = VkFormat::VK_FORMAT_R32G32_SFLOAT;

    if (outSize != nullptr)
        *outSize = attribs.size();
    return attribs.data();
}

void VertexObject::create(const VulkanData& vkdata, CmdBufferPool pool, float* strides, ui32 size, bool isDynamic) {
     _vkdata = vkdata;
     Buffer stagingBuffer;
     stagingBuffer.fillCrtInfo();
     stagingBuffer.memProp = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
     stagingBuffer.crtInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; 
     stagingBuffer.create(_vkdata, size);

     buff.fillCrtInfo();

    
     buff.memProp = isDynamic ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT 
                                       : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

     buff.crtInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; 
     buff.create(_vkdata, size);

     void* dst;
     void* src = strides;
     stagingBuffer.mapMem(&dst);
     stagingBuffer.wrt(dst, src, size);

     buff.cpyFrom(pool, stagingBuffer);

     stagingBuffer.dstr();
}

void VertexObject::createIndexBuff(const VulkanData& vkdata, CmdBufferPool pool, ui32* indexArray, ui32 size, bool isDynamic) {
     _vkdata = vkdata;
     Buffer stagingBuffer;
     stagingBuffer.fillCrtInfo();
     stagingBuffer.memProp = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
     stagingBuffer.crtInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; 
     stagingBuffer.create(_vkdata, size);

     indexBuff.fillCrtInfo();

     indexBuff.memProp = isDynamic ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT 
                                       : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

     indexBuff.crtInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; 
     indexBuff.create(_vkdata, size);

     void* dst;
     void* src = indexArray;
     stagingBuffer.mapMem(&dst);
     stagingBuffer.wrt(dst, src, size);

     indexBuff.cpyFrom(pool, stagingBuffer);

     stagingBuffer.dstr();
}

void VertexObject::dstr() {
    buff.dstr();
    indexBuff.dstr();
}


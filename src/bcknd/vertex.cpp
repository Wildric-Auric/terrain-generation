#include "vertex.h"


VertexInfo::VertexInfo(const std::vector<VertexAttributeInfo>& inf, uchar binding) {
    setBinding(binding);
    setAttribs(inf); 
}

const VkVertexInputBindingDescription*  VertexInfo::getBinding() {
       return &binding; 
}

const VkVertexInputAttributeDescription* VertexInfo::getAttribs(arch* outSize) {
    if (outSize != nullptr)
        *outSize = attribs.size();
    return attribs.data();
}
        
const VkVertexInputBindingDescription*   VertexInfo::setBinding(uchar b) {
    binding.binding = b;
    binding.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
    return &binding;
}

const VkVertexInputAttributeDescription* VertexInfo::setAttribs(const std::vector<VertexAttributeInfo >& strides, arch* outSize) {
    ui32 off  = 0;

    attribs.resize(strides.size(), {});

    for (int i = 0; i < strides.size(); ++i) {
        attribs[i].binding = binding.binding;
        attribs[i].location= i;
        attribs[i].offset  = off; 
        attribs[i].format  = strides[i].fmt;
        off += strides[i].byteSize;
    }

    if (outSize != nullptr)
        *outSize = attribs.size();
    binding.stride  = off;

    return attribs.data();
}
    
void Vertex::wrt(VertexInfo& inf,  void*  input) {
    arch datSize = bytes.size();
    bytes.resize(datSize + inf.getBinding()->stride );
    memcpy(&bytes[datSize], input, inf.getBinding()->stride);
};

void Vertex::read(VertexInfo& inf, void* output, ui32 offset) {
    memcpy(output, &bytes[offset], inf.getBinding()->stride);
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


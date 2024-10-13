#pragma once
#include "vkdecl.h"
#include "globals.h"
#include "buff.h"

#include <vulkan/vulkan.h>
#include <vector>


struct VertexAttributeInfo {
    VkFormat fmt;
    ui32     byteSize;
};

class VertexInfo {
    public: 

        VkVertexInputBindingDescription   binding;
        std::vector<VkVertexInputAttributeDescription> attribs;

        VertexInfo(const std::vector<VertexAttributeInfo >&, uchar binding);

        const VkVertexInputBindingDescription*   getBinding();
        const VkVertexInputAttributeDescription* getAttribs(arch* outSize  = nullptr);
        
        const VkVertexInputBindingDescription*   setBinding(uchar binding);
        const VkVertexInputAttributeDescription* setAttribs(const std::vector<VertexAttributeInfo>&, arch* outSize  = nullptr);
};

class Vertex {
    public:
    std::vector<char> bytes;

    inline Vertex() {};
    template<typename T>
    inline Vertex(std::initializer_list<T> l) {
        arch byteSize = l.size() * sizeof(T);
        bytes.resize(byteSize);
        memcpy(&bytes[0], l.begin(), byteSize);
    };

    void  wrt(VertexInfo&,  void*  input);
    void  read(VertexInfo&, void* output, ui32 offset);
};


class VertexObject {  
    public:

    void create(const VulkanData& vkdata, CmdBufferPool pool, float* strides, ui32 size, bool isDynamic = false);
    void createIndexBuff(const VulkanData& vkdata, CmdBufferPool, ui32* indexArray, ui32 size, bool isDynamic = false);
    void dstr();

    VulkanData _vkdata;

    Buffer buff;
    Buffer indexBuff;
};

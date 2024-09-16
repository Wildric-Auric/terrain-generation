#pragma once
#include "vkdecl.h"
#include "globals.h"
#include "buff.h"

#include <vulkan/vulkan.h>
#include <vector>

struct VertexData {
    fvec3 pos;
    fvec2 uv;
};

class Vertex {
    public:
    
        VertexData data;

        static std::vector<VkVertexInputBindingDescription>   bindings;
        static std::vector<VkVertexInputAttributeDescription> attribs;

        static const VkVertexInputBindingDescription*   getBindings(arch* outSize = nullptr);
        static const VkVertexInputAttributeDescription* getAttribs(arch* outSize  = nullptr);
        
        static const VkVertexInputBindingDescription*   setBindings(arch* outSize = nullptr);
        static const VkVertexInputAttributeDescription* setAttribs(arch* outSize  = nullptr);
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

#pragma once
#include "bcknd/vkdecl.h"
#include "bcknd/pipeline.h"
#include "bcknd/desc.h"
#include "bcknd/frame.h"
#include "bcknd/frame.h"

#include "shapes.h"
struct ShaderLoc {
    std::string vert;
    std::string frag;
    std::string geom;
    std::string tess;
};

class GfxContext {
    public:
    void setup(const ShaderLoc&, Renderpass*, 
               const std::vector<VkDescriptorSetLayoutBinding>* descPoolBinding = nullptr);
    void create();
    void dstr();
    void bind(CmdBuff&);

    Pipeline          pipeline; 
    DescPool          descPool;
    Renderpass*       rdrpass;
    //TODO::Remove this from here
    Shader vertS, fragS, geomS;
};

class GfxObject {
    public:
    void init(GfxContext*, Shape*);
    void update(const CmdBuff&);
    void draw();
    void dstr();

    DescSet         _descSet;
    Shape*          _shape;
    GfxContext*     _gtx;
};

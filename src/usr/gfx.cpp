#include "gfx.h"
#include "shared.h"
#include "bcknd/io.h"

static void loadShaders(Vkapp& app, const char* vertPath, const char* fragPath, Shader& vertS, Shader& fragS) {
    std::vector<char> vsrc, fsrc;
    io::readBin(vertPath, vsrc);
    io::readBin(fragPath, fsrc);
    vertS.fillCrtInfo((const ui32*)vsrc.data(), vsrc.size());
    fragS.fillCrtInfo((const ui32*)fsrc.data(), fsrc.size()); 
    vertS.create(app.data);
    fragS.create(app.data);
    vertS.fillStageCrtInfo(VK_SHADER_STAGE_VERTEX_BIT);
    fragS.fillStageCrtInfo(VK_SHADER_STAGE_FRAGMENT_BIT);
}

void GfxContext::setup(const ShaderLoc& loc, const Renderpass* r,const std::vector<VkDescriptorSetLayoutBinding>* descPoolBinding) {

    loadShaders(*GlobalData::app, loc.vert.c_str(), loc.frag.c_str(), vertS, fragS);

    VkPipelineShaderStageCreateInfo* stages = new VkPipelineShaderStageCreateInfo[2] {
        vertS.stageCrtInfo,
        fragS.stageCrtInfo
    };
    
    rdrpass = r;
    pipeline.fillCrtInfo(rdrpass->_subpasses._strideInfo[0].colLen);
    pipeline.crtInfo.stageCount           = 2;
    pipeline.crtInfo.pStages              = stages;
    pipeline.crtInfo.renderPass           = rdrpass->handle;
    pipeline.layoutCrtInfo.setLayoutCount = 1;
    pipeline.inputAsmState.topology       = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    
    if (descPoolBinding != nullptr) {
        descPool._lytBindings = *descPoolBinding;
    }
}

void GfxContext::create() {
    descPool.create(GlobalData::app->data);
    pipeline.layoutCrtInfo.pSetLayouts    = &descPool._lytHandle;
    pipeline.create(GlobalData::app->data);
	delete[] pipeline.crtInfo.pStages;
}

void GfxContext::dstr() {
    descPool.dstr();
    pipeline.dstr(); 
}

void GfxContext::bind(CmdBuff& cmdBuff) {
    vkCmdBindPipeline(cmdBuff.handle, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, 
    pipeline.handle);
}


void GfxObject::init(GfxContext* gtx, Shape* s) {
    _gtx   = gtx;
    _shape = s;
    _gtx->descPool.allocDescSet(&_descSet);
}

void GfxObject::update(const CmdBuff& cmdBuff) {
    vkCmdBindDescriptorSets(cmdBuff.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, _gtx->pipeline._layout, 0, 1, &_descSet.handle, 0, nullptr);
}

void GfxObject::draw() {
    _shape->draw();
}

void GfxObject::dstr() {
	_gtx->descPool.freeDescSet(&_descSet);
}


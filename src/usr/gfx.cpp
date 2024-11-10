#include "gfx.h"
#include "shared.h"
#include "bcknd/io.h"

static void loadShaders(Vkapp& app, const char* vertPath, const char* fragPath, Shader* vertS, Shader* fragS, Shader* geomS = nullptr, const char* geomPath = nullptr) {
    std::vector<char> vsrc, fsrc, gsrc;

    if (vertPath != nullptr) {
        io::readBin(vertPath, vsrc);
        vertS->fillCrtInfo((const ui32*)vsrc.data(), vsrc.size());
        vertS->create(app.data);
        vertS->fillStageCrtInfo(VK_SHADER_STAGE_VERTEX_BIT);
    }

    if (fragPath != nullptr) {
        io::readBin(fragPath, fsrc);
        fragS->fillCrtInfo((const ui32*)fsrc.data(), fsrc.size()); 
        fragS->create(app.data);
        fragS->fillStageCrtInfo(VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    if (geomPath != nullptr) {
        io::readBin(geomPath, gsrc);
        geomS->fillCrtInfo((const ui32*)gsrc.data(), gsrc.size());
        geomS->create(app.data);
        geomS->fillStageCrtInfo(VK_SHADER_STAGE_GEOMETRY_BIT);
    }
}

void GfxContext::setup(const ShaderLoc& loc, Renderpass* r,const std::vector<VkDescriptorSetLayoutBinding>* descPoolBinding) {
    
    loadShaders(*GlobalData::app, 
                loc.vert.empty() ? nullptr : loc.vert.c_str(),
                loc.frag.empty() ? nullptr : loc.frag.c_str(), 
                &vertS, &fragS, &geomS, 
                loc.geom.empty() ? nullptr : loc.geom.c_str()
               );

    uchar n = !loc.vert.empty() + !loc.frag.empty() + !loc.geom.empty();
    VkPipelineShaderStageCreateInfo* stages = new VkPipelineShaderStageCreateInfo[n];


    uchar cur = 0;

    !loc.vert.empty() ? stages[cur++] = vertS.stageCrtInfo : stages[cur];
    !loc.frag.empty() ? stages[cur++] = fragS.stageCrtInfo : stages[cur];
    !loc.geom.empty() ? stages[cur++] = geomS.stageCrtInfo : stages[cur];

    rdrpass = r;
    pipeline.fillCrtInfo(rdrpass->_subpasses._strideInfo[0].colLen);
    pipeline.crtInfo.stageCount           = n;
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


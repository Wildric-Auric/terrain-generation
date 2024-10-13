#include "pipeline.h"
#include "params.h"
#include "vertex.h"

VkResult Pipeline::create(const VulkanData& vkdata) {
    _vkdata = vkdata;

    _vinf.setAttribs(sinf);
    _vinf.setBinding(0);

    arch temp0;
    vrtxInputState.pVertexAttributeDescriptions    = _vinf.getAttribs(&temp0);
    vrtxInputState.vertexAttributeDescriptionCount = temp0;
    vrtxInputState.pVertexBindingDescriptions      = _vinf.getBinding();
    vrtxInputState.vertexBindingDescriptionCount   = 1;

    vkCreatePipelineLayout(vkdata.dvc, &layoutCrtInfo, nullptr, &_layout);
    crtInfo.layout = _layout;
    return vkCreateGraphicsPipelines(vkdata.dvc, VK_NULL_HANDLE, 1, &crtInfo, nullptr, &handle); 
}

VkGraphicsPipelineCreateInfo& Pipeline::fillCrtInfo(arch attNum) { 
    _msaaState.sType     = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    _dynState.sType      = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    vrtxInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    inputAsmState.sType  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    tesState.sType       = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    rasterState.sType    = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    depthState.sType     = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    blendState.sType     = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    viewportState.sType  = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

    inputAsmState.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAsmState.primitiveRestartEnable = VK_FALSE;
 
    rasterState.cullMode    = VK_CULL_MODE_BACK_BIT;
    rasterState.frontFace   = VK_FRONT_FACE_CLOCKWISE;
    rasterState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterState.lineWidth   = 1.0f;

    depthState.depthTestEnable =  VK_TRUE;
    depthState.depthWriteEnable = VK_TRUE;
    depthState.depthCompareOp   = VK_COMPARE_OP_LESS;
    depthState.depthBoundsTestEnable = VK_FALSE;
    depthState.minDepthBounds        = 0.0f;
    depthState.maxDepthBounds        = 1.0f;
    depthState.stencilTestEnable     = VK_FALSE;

    VkPipelineColorBlendAttachmentState temp;
    temp.blendEnable = VK_TRUE;
    temp.colorWriteMask  = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; 
    temp.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    temp.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    temp.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    temp.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    temp.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    temp.alphaBlendOp        = VK_BLEND_OP_ADD;
    temp.colorBlendOp        = VK_BLEND_OP_ADD;

    _colBlendAtt = std::vector<VkPipelineColorBlendAttachmentState>(attNum, temp);

    blendState.logicOpEnable   = VK_FALSE;
    blendState.attachmentCount = _colBlendAtt.size();
    blendState.pAttachments    = _colBlendAtt.data();

    _msaaState.sampleShadingEnable = VK_FALSE;
    _msaaState.rasterizationSamples = (VkSampleCountFlagBits) GfxParams::inst.msaa;
    _msaaState.alphaToOneEnable     = VK_FALSE;
    _msaaState.alphaToCoverageEnable = VK_FALSE;

    _dynState.dynamicStateCount = sizeof(_dynamicStates) / sizeof(_dynamicStates[0]);
    _dynState.pDynamicStates    = _dynamicStates;

    _scissor.extent = {480,360};
    _scissor.offset = {0,0};
    _viewport.width = 480;
    _viewport.height= 360;

    viewportState.scissorCount  = 1;
    viewportState.viewportCount = 1;
    viewportState.pScissors     = &_scissor;
    viewportState.pViewports    = &_viewport;
   
    layoutCrtInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCrtInfo.setLayoutCount = 0;

    //---
    crtInfo.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    crtInfo.stageCount          = 0;
    crtInfo.pStages             = NULL;
    crtInfo.pVertexInputState   = &vrtxInputState;
    crtInfo.pInputAssemblyState = &inputAsmState;
    crtInfo.pTessellationState  = &tesState;
    crtInfo.pMultisampleState   = &_msaaState;
    crtInfo.pDepthStencilState  = &depthState;
    crtInfo.pColorBlendState    = &blendState;
    crtInfo.pDynamicState       = &_dynState;
    crtInfo.pViewportState      = &viewportState;
    crtInfo.pRasterizationState = &rasterState;
    crtInfo.layout              = NULL;
    crtInfo.renderPass          = NULL;
    crtInfo.subpass             = 0;
    return crtInfo;
}

void Pipeline::dstr() {
    vkDestroyPipelineLayout(_vkdata.dvc, _layout, nullptr);
    vkDestroyPipeline(_vkdata.dvc, handle, nullptr);
}



VkShaderModuleCreateInfo& Shader::fillCrtInfo(const ui32* const bytecode, const arch size) {
    crtInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    crtInfo.codeSize = size;
    crtInfo.pCode    = bytecode;
    return crtInfo;
}

VkPipelineShaderStageCreateInfo& Shader::fillStageCrtInfo(VkShaderStageFlagBits type) {
    _type              = type;
    stageCrtInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageCrtInfo.stage = type;
    stageCrtInfo.module = handle;
    stageCrtInfo.pName  = "main";
    return stageCrtInfo;
}

VkResult Shader::create(const VulkanData& vkdata) {
    _vkdata = vkdata;
    return vkCreateShaderModule(vkdata.dvc, &crtInfo, nullptr, &handle);
}

void Shader::dstr() {
    vkDestroyShaderModule(_vkdata.dvc, handle, nullptr);
}
    

VkComputePipelineCreateInfo& ComputePipeline::fillCrtInfo() {

    crtInfo.sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    crtInfo.layout             = NULL;
    crtInfo.basePipelineIndex  = 0;
    crtInfo.basePipelineHandle = VK_NULL_HANDLE;

    lytCrtInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    return crtInfo;
}

VkResult ComputePipeline::create(const VulkanData& vkdata) { 
    VkResult res;
    _vkdata = vkdata;
    vkCreatePipelineLayout(_vkdata.dvc, &lytCrtInfo, nullptr, &_lyt);
    crtInfo.layout = _lyt;
    res = vkCreateComputePipelines(_vkdata.dvc, nullptr, 1, &crtInfo, nullptr, &handle);
    return res;
} 

void ComputePipeline::dstr() {
    vkDestroyPipeline(_vkdata.dvc, handle, nullptr);
    vkDestroyPipelineLayout(_vkdata.dvc, _lyt, nullptr);
}

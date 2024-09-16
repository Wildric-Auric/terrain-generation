#pragma once

#include "nwin/vulkan_surface.h"
#include "vulkan/vulkan_core.h"

#include "bcknd/vkapp.h"
#include "bcknd/vkdecl.h"
#include "bcknd/support.h"
#include "bcknd/params.h"
#include "bcknd/io.h"
#include "bcknd/vertex.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

inline i32 loop(Vkapp& vkapp, bool wireframe = false) {

    CmdBufferPool gfxCmdPool;
    DescPool      descPool;
    Renderpass    renderpass;
    Swapchain     swpchain;
    
    VulkanSupport::QueueFamIndices qfam; 
    VulkanSupport::findQueues(qfam, vkapp.data);
    VK_CHECK_EXTENDED(gfxCmdPool.create(vkapp.data, qfam.gfx), "command pool");

    VK_CHECK_EXTENDED(descPool.create(vkapp.data), "Descriptor pool");
   

    renderpass._subpasses.setup(2, 10);

    AttachmentContainer att;
    AttachmentContainer att1;

    Attachment* tmp = att.add(); 
    tmp->desc.format = VK_FORMAT_R8G8B8A8_SRGB; 
    tmp->desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    att.add();
    att.addDepth(); 

    tmp = att1.add();
    tmp->desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    att1.add();
    att1.addDepth();

    renderpass._subpasses.add(vkapp.win, vkapp.data, att, nullptr);
    renderpass._subpasses.add(vkapp.win, vkapp.data, att1, nullptr);

    VK_CHECK_EXTENDED(renderpass.create(vkapp.data, vkapp.win), "rndpass");

    VK_CHECK_EXTENDED(swpchain.create(vkapp.data, vkapp.win, renderpass), "Failed to create swapchain");

    Pipeline      pipeline;
    Pipeline      wireframePipeline; 
    
    std::vector<char> frag;
    std::vector<char> vert;

    io::readBin("..\\build\\bin\\trifrag.spv", frag );
    io::readBin("..\\build\\bin\\trivert.spv", vert );
    Shader fragS, vertS;
    fragS.fillCrtInfo((const ui32*)frag.data(), frag.size());
    vertS.fillCrtInfo((const ui32*)vert.data(), vert.size());
    VK_CHECK_EXTENDED(fragS.create(vkapp.data), "fragshader");
    VK_CHECK_EXTENDED(vertS.create(vkapp.data), "vertshader");
    fragS.fillStageCrtInfo(VK_SHADER_STAGE_FRAGMENT_BIT);
    vertS.fillStageCrtInfo(VK_SHADER_STAGE_VERTEX_BIT);
    //Tessellation
    std::vector<char> tesc;
    std::vector<char> tese;
    io::readBin("..\\build\\bin\\testtese.spv", tese);
    io::readBin("..\\build\\bin\\testtesc.spv", tesc);
    Shader teseS, tescS;
    teseS.fillCrtInfo((const ui32*)tese.data(), tese.size());
    tescS.fillCrtInfo((const ui32*)tesc.data(), tesc.size());
    teseS.create(vkapp.data);
    tescS.create(vkapp.data);
    teseS.fillStageCrtInfo(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    tescS.fillStageCrtInfo(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    //Geometry
    std::vector<char> geom;
    io::readBin("..\\build\\bin\\testgeom.spv", geom);
    Shader geomS;
    geomS.fillCrtInfo((const ui32*)geom.data(), geom.size());
    geomS.create(vkapp.data);
    geomS.fillStageCrtInfo(VK_SHADER_STAGE_GEOMETRY_BIT);

    VkPipelineShaderStageCreateInfo stages[] = {
        vertS.stageCrtInfo,
        fragS.stageCrtInfo,
        tescS.stageCrtInfo,
        teseS.stageCrtInfo,
        geomS.stageCrtInfo
    };

    pipeline.fillCrtInfo(renderpass._subpasses._strideInfo[0].colLen);
    pipeline.crtInfo.stageCount = sizeof(stages) / sizeof(stages[0]);
    pipeline.crtInfo.pStages    = stages;
    pipeline.crtInfo.renderPass = renderpass.handle;

    pipeline.layoutCrtInfo.setLayoutCount = 1;
    pipeline.layoutCrtInfo.pSetLayouts    = &descPool._lytHandle;
    
    pipeline.inputAsmState.topology      = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    pipeline.tesState.patchControlPoints = 4;

    wireframePipeline.fillCrtInfo(renderpass._subpasses._strideInfo[0].colLen);
    wireframePipeline.crtInfo.stageCount = pipeline.crtInfo.stageCount; 
    wireframePipeline.crtInfo.pStages    = stages;
    wireframePipeline.crtInfo.renderPass = renderpass.handle;
    wireframePipeline.layoutCrtInfo.setLayoutCount = 1;
    wireframePipeline.layoutCrtInfo.pSetLayouts    = &descPool._lytHandle;
    wireframePipeline.inputAsmState.topology       = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    wireframePipeline.tesState.patchControlPoints  = 4;

    wireframePipeline.rasterState.polygonMode = VK_POLYGON_MODE_LINE; 

    
    Pipeline subpass1pipeline;
    subpass1pipeline.fillCrtInfo(renderpass._subpasses._strideInfo[0].colLen);
    subpass1pipeline.crtInfo.stageCount = pipeline.crtInfo.stageCount; 
    subpass1pipeline.crtInfo.pStages    = stages;
    subpass1pipeline.crtInfo.renderPass = renderpass.handle;
    subpass1pipeline.layoutCrtInfo.setLayoutCount = 1;
    subpass1pipeline.layoutCrtInfo.pSetLayouts    = &descPool._lytHandle;
    subpass1pipeline.inputAsmState.topology       = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    subpass1pipeline.tesState.patchControlPoints  = 4;
    subpass1pipeline.crtInfo.subpass              = 1;

    VK_CHECK_EXTENDED(pipeline.create(vkapp.data), "Failed to create Pipeline");
    VK_CHECK_EXTENDED(wireframePipeline.create(vkapp.data), "Failed to create wireframe pipeline");
    VK_CHECK_EXTENDED(subpass1pipeline.create(vkapp.data), "Failed to create pipeline");
 
    fragS.dstr();
    vertS.dstr();
    tescS.dstr();
    teseS.dstr();
    geomS.dstr();

    DescSet     descSet{};
    //Creating image texture
    ui32 imgsize;
    ivec2 vecsize;
    i32   channels; 
    stbi_uc* pixels = stbi_load("../res/brick.jpg", &vecsize.x, &vecsize.y, &channels, STBI_rgb_alpha);
    imgsize = vecsize.x * vecsize.y * 4;

    Sampler smpler;
    smpler.fillCrtInfo(vkapp.data);
    smpler.create(vkapp.data);

    Buffer tempImg;
    Img    img0;
    ImgView view;

    tempImg.fillCrtInfo();
    tempImg.crtInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    tempImg.memProp       = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    tempImg.create(vkapp.data, imgsize);
    void* mapped;
    tempImg.mapMem(&mapped);
    memcpy(mapped, pixels, imgsize);
    tempImg.unmapMem(&mapped);

    img0.fillCrtInfo();
    img0.crtInfo.usage         = VK_IMAGE_USAGE_SAMPLED_BIT      | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | 
                                 VK_IMAGE_USAGE_TRANSFER_DST_BIT; 

    img0.crtInfo.extent.width  = vecsize.x;
    img0.crtInfo.extent.height = vecsize.y;
    img0.setMaxmmplvl();
    img0.create(vkapp.data);
    img0.cpyFrom(gfxCmdPool, tempImg, vecsize, 0);
    img0.genmmp(gfxCmdPool, offsetof(VulkanSupport::QueueFamIndices, gfx));

    view.fillCrtInfo(img0);
    view.create(vkapp.data);


    //img0.changeLyt(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, gfxCmdPool);
    //-----
    fvec3   tran = {0.0,0.0,0.0};
    Matrix4<float> projMat(1);
    Matrix4<float> viewMat(1);
    Matrix4<float> modelMat(1);

    PerspectiveMat(projMat, 70, 1.0, 0.001, 100.0);
    TranslateMat(modelMat, tran);
    RotateMat(modelMat, 0.0, {0.0, 1.0, 0.0});
    LookAt(viewMat, {0.3,-1.0,0.0}, tran, {0.0, 1.0, 0.0});
    //Matrix4<float> mvp = projMat * viewMat * modelMat; 

    struct {
        Matrix4<float> view;
        Matrix4<float> model;
        Matrix4<float> proj;
        float subdiv = 1.0; 
    } unfData;

    UniBuff unf;
    //descPool._lytBindings.pop_back();
    unf.create(vkapp.data, sizeof(unfData));
    descPool.allocDescSet(&descSet);


    VkWriteDescriptorSet wrt{};

    VkDescriptorBufferInfo inf{};
    inf.buffer      = unf._buff.handle; inf.range       = unf._buff._size;
    wrt.pBufferInfo = &inf; 
    wrt.descriptorCount = 1;
    wrt.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descSet.wrt(&wrt, 0);

    VkDescriptorImageInfo imInf{};
    imInf.sampler     = smpler.handle;
    imInf.imageView   = view.handle;
    imInf.imageLayout = img0.crtInfo.initialLayout;    

    wrt = VkWriteDescriptorSet{};
    wrt.descriptorCount = 1;
    wrt.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    wrt.pImageInfo      = &imInf;
    
    descSet.wrt(&wrt, 1);
    unf.wrt(&unfData);

    VertexObject vobj;
    VertexData strides[] = {
        { {-0.5, -0.5, 0.5},  {0.0, 0.0} },      
        { { 0.5, -0.5, 0.5}, {1.0, 0.0} },    
        { {-0.5,  0.5, 0.5}, {0.0, 1.0} },
        { { 0.5,  0.5, 0.5}, {1.0, 1.0} },
        
        { {0.5, -0.5, 0.5},  {0.0, 0.0} },   
        { {0.5, -0.5, -.5}, {1.0, 0.0} },    
        { {0.5,  0.5, 0.5}, {0.0, 1.0} },
        { {0.5,  0.5, -.5}, {1.0, 1.0} },

        { {-0.5, -0.5, -.5},  {0.0, 0.0} },      
        { { 0.5, -0.5, -.5}, {1.0, 0.0} },    
        { {-0.5,  -0.5, 0.5}, {0.0, 1.0} },
       { { 0.5,  -0.5, 0.5}, {1.0, 1.0} },
        
       { {-0.5, -0.5, -.5}, {0.0, 0.0} },    
        { {-0.5, -0.5, 0.5},  {1.0, 0.0} },      
        { {-0.5,  0.5, -.5}, {0.0, 1.0} },
        { {-0.5,  0.5, 0.5},  {1.0, 1.0} },

        { { 0.5, -0.5, -.5}, {1.0, 0.0} },    
        { {-0.5, -0.5, -.5},  {0.0, 0.0} },      
        { { 0.5,  0.5, -.5}, {1.0, 1.0} },
        { {-0.5,  0.5, -.5}, {0.0, 1.0} },
        
    };

    ui32 indices[] = {
        0,1,2,
        3,2,1, 
        
        4+0,4+1,4+2,
        4+3,4+2,4+1, 
        
        8+0,8+1,8+2,
        8+3,8+2,8+1, 

        12+0,12+1,12+2,
        12+3,12+2,12+1, 
        
        16+0,16+1,16+2,
        16+3,16+2,16+1, 

    };
    
    vobj.create(vkapp.data, gfxCmdPool, (float*)strides,  sizeof(strides), 1);
    vobj.createIndexBuff(vkapp.data, gfxCmdPool, indices, sizeof(indices));

    Frame frame;
    FrameData data;
    data.win = &vkapp.win;
    data.swpchain = &swpchain;
    data.cmdBuffPool = &gfxCmdPool;
    data.rdrpass     = &renderpass;
    frame._data = data;
    renderpass.fillBeginInfo(vkapp.win, {0.009, 0.001, 0.02 });
    frame.create(vkapp.data);

    VkResult res;

    VkPhysicalDeviceProperties prop;

    while (vkapp.win.ptr->shouldLoop()) {        
        if (!frame.begin()) {
            continue;
        }
        renderpass.begin(frame.cmdBuff, frame.swpIndex);
        Pipeline* ptemp = &pipeline;
        if (vkapp.win.ptr->_getKeyboard().isKeyPressed((NWin::Key)'W')) {
            ptemp = &wireframePipeline;
        }

        vkCmdBindPipeline(frame.cmdBuff.handle, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, ptemp->handle);
        //Dynamic states 
        VkViewport viewport;
        VkRect2D   scissor;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        viewport.width =  Max<i32>(500, 5);
        viewport.height = Max<i32>(500, 5);
        viewport.y = Max<i32>(-0.5 * viewport.height + vkapp.win.drawArea.y * 0.5, 0.0);
        viewport.x = Max<i32>(-0.5 * viewport.width  + vkapp.win.drawArea.x * 0.5, 0.0);
        scissor.extent =  {(ui32)viewport.width, (ui32)viewport.height};
        scissor.offset =  { (i32)viewport.x, (i32)viewport.y };
        vkCmdSetViewport(frame.cmdBuff.handle, 0, 1, &viewport);
        vkCmdSetScissor(frame.cmdBuff.handle, 0, 1, &scissor);
        VkDeviceSize voff = 0;

        static float sub = 1.0;
        static float t = 0.0;
        t += 1.;
        projMat = Matrix4<float>(1);
        viewMat = Matrix4<float>(1);
        modelMat = Matrix4<float>(1);
        PerspectiveMat(projMat, 60, 1.0, 0.001, 100.0);
        RotateMat(modelMat, t, { 0.0, 1.0, 0.0 });
        TranslateMat(modelMat, tran);
        LookAt(viewMat, { 0.0,-1.0, 2.0 }, tran, { 0.0, 1.0, 0.0 });

        unfData.view  = viewMat;
        unfData.proj  = projMat;
        unfData.model = modelMat;
        
        if (vkapp.win.ptr->_getKeyboard().onKeyRelease(NWin::Key::NWIN_KEY_RIGHT)) {
            sub += 1.0;
            system("cls");
            std::cout << "Subdivions: " << sub;
        }

        static bool fullscreen = 0;
        if (vkapp.win.ptr->_getKeyboard().onKeyRelease(NWin::Key('F'))) {
            fullscreen = !fullscreen;
            if (fullscreen)
                vkapp.win.ptr->enableFullscreen();
            else
                vkapp.win.ptr->disableFullscreen();
        }

        if (vkapp.win.ptr->_getKeyboard().onKeyRelease(NWin::Key::NWIN_KEY_LEFT)) {
            sub -= 1.0;
            if (sub < 1.0)
                sub = 1.0;
            system("cls");
            std::cout << "Subdivions: " << sub;
        }

        unfData.subdiv = sub;

        unf.wrt(&unfData);


        vkCmdBindDescriptorSets(frame.cmdBuff.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, ptemp->_layout, 0, 1, &descSet.handle, 0, nullptr);
        vkCmdBindVertexBuffers(frame.cmdBuff.handle, 0, 1, &vobj.buff.handle, &voff);
        vkCmdBindIndexBuffer(frame.cmdBuff.handle, vobj.indexBuff.handle, voff, VkIndexType::VK_INDEX_TYPE_UINT32);
        vkCmdDraw(frame.cmdBuff.handle, sizeof(strides)/sizeof(strides[0]), 1, 0, 0);
        
        vkCmdNextSubpass(frame.cmdBuff.handle, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindDescriptorSets(frame.cmdBuff.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, subpass1pipeline._layout, 0, 1, &descSet.handle, 0, nullptr);
        vkCmdBindVertexBuffers(frame.cmdBuff.handle, 0, 1, &vobj.buff.handle, &voff);
        vkCmdBindIndexBuffer(frame.cmdBuff.handle, vobj.indexBuff.handle, voff, VkIndexType::VK_INDEX_TYPE_UINT32);
        vkCmdBindPipeline(frame.cmdBuff.handle, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, subpass1pipeline.handle);
        vkCmdDraw(frame.cmdBuff.handle, sizeof(strides)/sizeof(strides[0]), 1, 0, 0);
        renderpass.end(frame.cmdBuff);

        frame.end(); 
    }
        //vkCmdDrawIndexed(frame.cmdBuff.handle, sizeof(indices) / sizeof(indices[0]), 1, 0, 0, 0);

    frame.dstr();
    vobj.dstr();
    unf.dstr();
    smpler.dstr();
    tempImg.dstr();
    img0.dstr();
    view.dstr();
    pipeline.dstr();
    wireframePipeline.dstr();

    swpchain.dstr(); 
    renderpass.dstr();
    gfxCmdPool.dstr();
    descPool.dstr();

    return 0;
}

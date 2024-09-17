#include "engine.h"
#include "bcknd/cmd_buffer.h"
#include "bcknd/desc.h"
#include "bcknd/frame.h"
#include "bcknd/support.h"
#include "bcknd/io.h"
#include "bcknd/params.h"

#include "shared.h"
#include "shapes.h"

#define P0PATHVERT "../build/bin/mrt.vert.spv"
#define P0PATHFRAG "../build/bin/mrt.frag.spv"
#define P0PATHCOMP "../build/bin/def.comp.spv"

#define P1PATHVERT "../build/bin/def.vert.spv"
#define P1PATHFRAG "../build/bin/def.frag.spv"




//TODO::Refactor and integrate this to vkutil
VkResult setupRdrpassFmbuffs(Renderpass& rdrpass, Window& win, VulkanData& _vkdata, Swapchain& swpchain) {
    VkResult res;
    VkExtent2D imageExtent;
    VulkanSupport::SwpchainCap spec; 
    
    VulkanSupport::getSwapchaincap(_vkdata, spec);

    swpchain.chooseExtent(win, spec.cap, &imageExtent);

    VkSurfaceFormatKHR srfcFmt = spec.srfcFormats[VulkanSupport::selSrfcFmt(spec)];
    ui32 imgCount = spec.cap.minImageCount + 1;
    if (spec.cap.maxImageCount > 0 && imgCount > spec.cap.maxImageCount) {
        imgCount = spec.cap.maxImageCount;
    }

    Img fake;
    std::vector<VkImage>& imgs = swpchain.imgs;
    std::vector<ImgView>& views = swpchain.views;


    rdrpass.fmbuffs.resize(imgCount);
    for (arch i = 0; i < imgCount; ++i) {
        fake.crtInfo.format = srfcFmt.format;
        fake.crtInfo.samples = (VkSampleCountFlagBits)GfxParams::inst.msaa;
        fake.handle         = imgs[i];

        rdrpass.fmbuffs[i].fillCrtInfo();
        rdrpass.fmbuffs[i].crtInfo.width  = imageExtent.width;
        rdrpass.fmbuffs[i].crtInfo.height = imageExtent.height;

        std::vector<VkImageView> att;

        for (arch k = 0; k < rdrpass._subpasses.descs.size(); ++k) {
            AttachmentData* depth = rdrpass._subpasses.getStrideDepth(k);
            auto colIter = rdrpass._subpasses.getStrideColIterBegin(k, nullptr);
            auto resIter = rdrpass._subpasses.getStrideColResolveBegin(k, nullptr);
            arch j = 0;
            if (resIter != rdrpass._subpasses.resources.end()) {
                for (;j < rdrpass._subpasses._strideInfo[k].colLen;++j) {
                    att.push_back((resIter++)->view.handle);
                }
            }

            if (colIter != rdrpass._subpasses.resources.end()) {
                for (j=0;j < rdrpass._subpasses._strideInfo[k].colLen;++j) {
                    if (rdrpass._subpassHjckIndex == k && rdrpass._attHjckIndex == j)  {
                        att.push_back(views[i].handle);
                        ++colIter;
                        continue;
                    }
                    att.push_back((colIter++)->view.handle);
                }
            }

            if (depth != nullptr)
                att.push_back(depth->view.handle);
        }


        res = rdrpass.fmbuffs[i].create(_vkdata, rdrpass.handle, att.data(), att.size());
    }
    return res;
}


void loadShaders(Vkapp& app, const char* vertPath, const char* fragPath, Shader& vertS, Shader& fragS) {
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

void Engine::run(Vkapp& app) {
    CmdBufferPool gfxCmdPool;
    CmdBufferPool compCmdPool;
    CmdBuff       compCmdBuff;
    DescPool      descPool;
    DescPool      descPool2;
    Renderpass    rdrpass;
    Renderpass    rdrpass0;
    Swapchain     swpchain;
    Pipeline      pline;
    Pipeline      pline2;
    Frame         frame;
    FrameData     frameData;
    Sampler       defSampler;
    
    //-----------Creating Resources----------- 
    VulkanSupport::QueueFamIndices qfam;
    VulkanSupport::findQueues(qfam, app.data);
    gfxCmdPool.create(app.data, qfam.gfx );
    compCmdPool.create(app.data, qfam.com);
    VkQueue q = VulkanSupport::getQueue(app.data, offsetof(VulkanSupport::QueueFamIndices, gfx));

    descPool.create(app.data);

    descPool2._lytBindings.resize(3);
    descPool2._lytBindings[0] = {0, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    descPool2._lytBindings[1] = {1, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    descPool2._lytBindings[2] = {2, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    descPool2.create(app.data);

    //-----------Setup and create renderpass----------- 
    //Fist subpass MRT
    AttachmentContainer att;
    auto colAtt = att.add();

    att.addDepth();
    auto normalAtt     = att.add();
    auto reflectionAtt = att.add();


    rdrpass._subpasses.setup(1,4);
    rdrpass._subpasses.add(app.win, app.data, att, nullptr);  //First subpass
    colAtt->desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    rdrpass.setSwpChainHijack(-1, 0);
    rdrpass.create(app.data, app.win);
    rdrpass.fillBeginInfo(app.win);


    AttachmentContainer att0;
    auto colAtt0 = att0.add();
    colAtt0->desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; 
    colAtt0->desc.format      = VK_FORMAT_R8G8B8A8_SRGB; 
    rdrpass0._subpasses.setup(1,1);
    rdrpass0._subpasses.add(app.win, app.data, att0, nullptr);
    rdrpass0.setSwpChainHijack(0, 0); 
    rdrpass0.create(app.data, app.win);
    rdrpass0.fillBeginInfo(app.win);

    //-------------Swapchain---------------
    swpchain.create(app.data, app.win, rdrpass0);
    //-----------Create Second rdrpass framebuffers----------
    setupRdrpassFmbuffs(rdrpass, app.win, app.data, swpchain);
    //-----------Setup Shaders----------- 

    std::vector<char> code;
    Shader fragS, vertS;
    Shader fragS2, vertS2;
    loadShaders(app,P0PATHVERT, P0PATHFRAG, fragS, vertS);
    loadShaders(app,P1PATHVERT, P1PATHFRAG, fragS2, vertS2);

    VkPipelineShaderStageCreateInfo stages[] = {
        vertS.stageCrtInfo,
        fragS.stageCrtInfo
    };

    VkPipelineShaderStageCreateInfo stages2[] = {
        vertS2.stageCrtInfo,
        fragS2.stageCrtInfo
    };
    
    //-----------Create Pipelines----------- 
    pline.fillCrtInfo(rdrpass._subpasses._strideInfo[0].colLen);
    pline.crtInfo.stageCount = 2;
    pline.crtInfo.pStages    = stages;
    pline.crtInfo.renderPass = rdrpass.handle;
    pline.layoutCrtInfo.setLayoutCount = 1;
    pline.layoutCrtInfo.pSetLayouts    = &descPool._lytHandle;
    pline.inputAsmState.topology       = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pline.create(app.data);

    pline2.fillCrtInfo(rdrpass0._subpasses._strideInfo[0].colLen);
    pline2.crtInfo.stageCount = 2;
    pline2.crtInfo.pStages    = stages2;
    pline2.crtInfo.renderPass = rdrpass0.handle;
    pline2.layoutCrtInfo.setLayoutCount = 1;
    pline2.layoutCrtInfo.pSetLayouts    = &descPool2._lytHandle;
    pline2.inputAsmState.topology       = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pline2.crtInfo.subpass              = 0;
    pline2.create(app.data);
    //-----------Free Shaders----------- 
    vertS.dstr();
    fragS.dstr();
    fragS2.dstr();
    vertS2.dstr();
    //----------Allocate Descriptor---------
    DescSet mrtDescSet{};
    descPool.allocDescSet(&mrtDescSet);

    DescSet descSet{};
    descPool2.allocDescSet(&descSet);
    //----------Create Sampler--------------
    defSampler.fillCrtInfo(app.data);
    defSampler.create(app.data);
    //----------Create Frame----------------

    frameData.win = &app.win;
    frameData.swpchain = &swpchain;
    frameData.cmdBuffPool = &gfxCmdPool;
    frameData.rdrpass = &rdrpass0;
    frame._data = frameData;
    frame.create(app.data);
    //------------Set Global Data-------------
    GlobalData::cmdBuff = &frame.cmdBuff;
    GlobalData::cmdBuffPool = &gfxCmdPool;
    GlobalData::app         = &app;

    //-----------Loop------------
    Quad t;
    Quad t0;
    Cube cube;

    cube.init();
    t.init(); 

    t0.setInitSize({2.0f,2.0f});
    t0.init();

    struct {
        Matrix4<float> view;
        Matrix4<float> model;
        Matrix4<float> proj;
    } unfData;

    UniBuff unf;
    unf.create(app.data, sizeof(unfData));

    ImgView v0, v1, v2;
    while (app.win.ptr->shouldLoop()) {

       frame.submitInfo.waitSemaphoreCount = 1;
       if (!frame.begin())
           continue;

        v0.dstr();
        v1.dstr();
        v2.dstr();

       rdrpass.begin(frame.cmdBuff, frame.swpIndex);
        
       VkViewport viewport;
       VkRect2D   scissor;
       viewport.minDepth = 0.0f;
       viewport.maxDepth = 1.0f;
       viewport.width =  Max<i32>(500, 5);
       viewport.height = Max<i32>(500, 5);
       viewport.y = Max<i32>(-0.5 * viewport.height + app.win.drawArea.y * 0.5, 0.0);
       viewport.x = Max<i32>(-0.5 * viewport.width  + app.win.drawArea.x * 0.5, 0.0);
       scissor.extent =  {(ui32)viewport.width, (ui32)viewport.height};
       scissor.offset =  { (i32)viewport.x, (i32)viewport.y };
       vkCmdSetViewport(frame.cmdBuff.handle, 0, 1, &viewport);
       vkCmdSetScissor(frame.cmdBuff.handle, 0, 1, &scissor);
        
        static float t = 0.0;
        t += 1.;
        unfData.proj     = Matrix4<float>(1);
        unfData.view     = Matrix4<float>(1);
        unfData.model    = Matrix4<float>(1);
        PerspectiveMat(unfData.proj, 60, 1.0, 0.001, 100.0);
        RotateMat(unfData.model, t, { 0.0, 1.0, 0.0 });
        TranslateMat(unfData.model, {0.0, 0.0, 0.0});
        LookAt(unfData.view, { 0.0,-1.0, 2.0 }, {0.0,0.0,0.0}, { 0.0, 1.0, 0.0 });
        unf.wrt(&unfData);

       VkWriteDescriptorSet   wrt0{};
       VkDescriptorBufferInfo buffInf{};
       buffInf.offset = 0;
       buffInf.range  = unf._buff._size;
       buffInf.buffer = unf._buff.handle;

       wrt0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
       wrt0.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
       wrt0.descriptorCount = 1; 
       wrt0.pBufferInfo     = &buffInf;
       mrtDescSet.wrt(&wrt0, 0);

       vkCmdBindDescriptorSets(frame.cmdBuff.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pline._layout, 0, 1, &mrtDescSet.handle, 0, nullptr);
       vkCmdBindPipeline(frame.cmdBuff.handle, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pline.handle);
       cube.draw();

//       unfData.proj     = Matrix4<float>(1);
//       unfData.view     = Matrix4<float>(1);
//       unfData.model    = Matrix4<float>(1);
//       PerspectiveMat(unfData.proj, 60, 1.0, 0.001, 100.0);
//       RotateMat(unfData.model, 90, { 1.0, 0.0, 0.0 });
//       TranslateMat(unfData.model, {0.0, -1.0, 0.0});
//       unf.wrt(&unfData);
//       mrtDescSet.wrt(&wrt0, 0);  
       t0.draw();
        

       rdrpass.end(frame.cmdBuff);
       frame.submitInfo.signalSemaphoreCount = 0;
       vkEndCommandBuffer(frame.cmdBuff.handle);
       vkQueueSubmit(frame.cmdBuff.queue, 1, &frame.submitInfo, frame.fenQueueSubmitComplete);
       vkWaitForFences(app.data.dvc, 1, &frame.fenQueueSubmitComplete, VK_TRUE, UINT64_MAX);
       vkResetFences(app.data.dvc, 1, &frame.fenQueueSubmitComplete);
       vkResetCommandBuffer(frame.cmdBuff.handle, 0);
       vkBeginCommandBuffer(frame.cmdBuff.handle, &frame.beginInfo);
       frame.submitInfo.signalSemaphoreCount = 1;   
       vkQueueWaitIdle(q); //I need timeline semaphores and generally sync abstraction
       
       rdrpass0.begin(frame.cmdBuff, frame.swpIndex);
      
       vkCmdSetViewport(frame.cmdBuff.handle, 0, 1, &viewport);
       vkCmdSetScissor(frame.cmdBuff.handle, 0, 1, &scissor);
       vkCmdBindPipeline(frame.cmdBuff.handle, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pline2.handle);

       //----------Update Descriptor set and uniform----------
       VkWriteDescriptorSet  wrt = {};
       VkDescriptorImageInfo inf{};
       

       auto iter = rdrpass._subpasses.getStrideColIterBegin(0, nullptr);
       iter->image.crtInfo.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
       iter->image.changeLyt(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, *frame._data.cmdBuffPool);
       v0.fillCrtInfo(iter->image); v0.create(app.data);
       inf.sampler = defSampler.handle;
       inf.imageView = v0.handle;
       inf.imageLayout = iter->image.crtInfo.initialLayout; //VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 
       wrt.descriptorCount = 1;
       wrt.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
       wrt.pImageInfo = &inf;
       descSet.wrt(&wrt, 0);
       ++iter;
       iter->image.crtInfo.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
       iter->image.changeLyt(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, *frame._data.cmdBuffPool);
       v1.fillCrtInfo(iter->image); v1.create(app.data);
       inf.imageView = v1.handle;
       descSet.wrt(&wrt, 1);
       ++iter;
       iter->image.crtInfo.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
       iter->image.changeLyt(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, *frame._data.cmdBuffPool);
       v2.fillCrtInfo(iter->image); v2.create(app.data);
       inf.imageView = v2.handle;
       descSet.wrt(&wrt, 2);
       //-------------------
       vkCmdBindDescriptorSets(frame.cmdBuff.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pline2._layout, 0, 1, 
                               &descSet.handle, 0, nullptr);

       t0.draw();

       rdrpass0.end(frame.cmdBuff); 
       
       frame.submitInfo.waitSemaphoreCount = 0;
       //frame.submitInfo.pSignalSemaphores
       frame.end();

       //v2.dstr(); v1.dstr(); v0.dstr();

    }
    //-----------------------Clean Resources------------------
    frame.dstr();
    pline.dstr();
    pline2.dstr();
    swpchain.dstr();
    rdrpass.dstr();
    rdrpass0.dstr();
    descPool.dstr();
    descPool2.dstr();
    defSampler.dstr(); 
    gfxCmdPool.dstr();
    compCmdPool.dstr();
    t.destroy();
    t0.destroy();
    cube.destroy();
    unf.dstr();
    v0.dstr();
    v1.dstr();
    v2.dstr();
}


#include "engine.h"

#include "ui.h"

#include "bcknd/cmd_buffer.h"
#include "bcknd/desc.h"
#include "bcknd/frame.h"
#include "bcknd/support.h"
#include "bcknd/params.h"

#include "shared.h"
#include "shapes.h"
#include "gfx.h"
#include "cam.h"

#include "scene.h"
#include "components/mesh_renderer.h"
#include "terrain.h"


#define P0PATHVERT "../build/bin/gpu_terrain.mrt.vert.spv"
#define P0PATHFRAG "../build/bin/gpu_terrain.mrt.frag.spv"
#define P0PATHGEOM "../build/bin/gpu_terrain.mrt.geom.spv"

#define P1PATHVERT "../build/bin/def.vert.spv"
#define P1PATHFRAG "../build/bin/def.frag.spv"

#define P2PATHVERT "../build/bin/shadow.vert.spv"
#define P2PATHFRAG "../build/bin/shadow.frag.spv"

//Temporary
//struct LightData {
//    fvec3 pos = fvec3(-0.4, 0.81, -0.2);
//    fvec3 col = fvec3(1.0, 1.0, 1.0);
//};

UniBuff lightBuffer;

float FOV = 60.0;

LightData defaultLight;
Cam       defaultCam;
Transform cubeTrans(nullptr);
GfxObject  terObj;
GfxObject  cubeObj;
GfxObject  rendObj;

GfxContext gtx;          //Context for MRT 
GfxContext gtxShadow;    //Context for Shadow Map
GfxContext gtxDef;       //Final deferred rendering ctx 

GfxContext gtxBase;

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

//just to be legible

fvec3 trans = {0.0f,0.0f,0.f};

typedef struct {
        Matrix4<float> view;
        Matrix4<float> model;
        Matrix4<float> proj;
} UnfData;
typedef struct {
    UnfData unfData;
    UniBuff unf;
} CbkData;

void setViewPort(ui32 x, ui32 y, Frame& frame) {
       VkViewport viewport;
       VkRect2D   scissor;
       viewport.minDepth = 0.0f;
       viewport.maxDepth = 1.0f;
       viewport.width =  Max<i32>(x, 5);
       viewport.height = Max<i32>(y, 5);
       viewport.y = Max<i32>(-0.5 * viewport.height + y * 0.5, 0.0);
       viewport.x = Max<i32>(-0.5 * viewport.width  + x * 0.5, 0.0);
       scissor.extent =  {(ui32)viewport.width, (ui32)viewport.height};
       scissor.offset =  { (i32)viewport.x, (i32)viewport.y };
       vkCmdSetViewport(frame.cmdBuff.handle, 0, 1, &viewport);
       vkCmdSetScissor(frame.cmdBuff.handle, 0, 1, &scissor);
}

static void shadowRndPass(Renderpass& rdrpassShadow, Frame& frame, Vkapp& app, GfxContext& gtxShadow, 
                          UnfData& unfData, UniBuff& unf, GfxObject& obj, VkQueue& q) {
       rdrpassShadow.begin(frame.cmdBuff, frame.swpIndex);

       setViewPort(app.win.drawArea.x, app.win.drawArea.y, frame);

       gtxShadow.bind(frame.cmdBuff);

       unfData.proj     = Matrix4<float>(1);
       unfData.view     = Matrix4<float>(1);
       unfData.model    = Matrix4<float>(1);

       static float tme = 0.0f;
       tme  += 0.1;


       VkWriteDescriptorSet   wrt0{};
       VkDescriptorBufferInfo buffInf{};

       buffInf.offset = 0;
       buffInf.range  = unf._buff._size;
       buffInf.buffer = unf._buff.handle;

       wrt0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
       wrt0.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
       wrt0.descriptorCount = 1; 
       wrt0.pBufferInfo     = &buffInf;

       unfData.proj     = Matrix4<float>(1);
       unfData.view     = Matrix4<float>(1);
       unfData.model    = Matrix4<float>(1);
       PerspectiveMat(unfData.proj, 60, 1.0, 0.001, 100.0);
       RotateMat(unfData.model, 90, { 1.0, 0.0, 0.0 });
       TranslateMat(unfData.model, trans);
       LookAt(unfData.view, 
       { 1.0,-0.5, -0.6 }, trans, { 0.0, 1.0, 0.0 });
       unf.wrt(&unfData);
       obj._descSet.wrt(&wrt0, 0);  
       obj.update(frame.cmdBuff);

       obj.draw();
        
       rdrpassShadow.end(frame.cmdBuff);


       frame.submitInfo.signalSemaphoreCount = 0;
       vkEndCommandBuffer(frame.cmdBuff.handle);
       vkQueueSubmit(frame.cmdBuff.queue, 1, &frame.submitInfo, frame.fenQueueSubmitComplete.handle);
       vkWaitForFences(app.data.dvc, 1, &frame.fenQueueSubmitComplete.handle, VK_TRUE, UINT64_MAX);
       vkResetFences(app.data.dvc, 1, &frame.fenQueueSubmitComplete.handle);
       vkResetCommandBuffer(frame.cmdBuff.handle, 0);
       vkBeginCommandBuffer(frame.cmdBuff.handle, &frame.beginInfo);
       frame.submitInfo.signalSemaphoreCount = 1;   
       vkQueueWaitIdle(q); //I need timeline semaphores and generally sync abstraction
}

static void mrtRndPass(Renderpass& rdrpass, Frame& frame, Vkapp& app, GfxContext& gtx, UnfData& unfData, UniBuff& unf, GfxObject& obj, VkQueue& q) {
       //TODO::Pass Model matrix (transform)
       rdrpass.begin(frame.cmdBuff, frame.swpIndex);        
        
       gtx.bind(frame.cmdBuff);

       setViewPort(frame._data.win->drawArea.x, frame._data.win->drawArea.y, frame);

       VkWriteDescriptorSet   wrt0{};
       VkDescriptorBufferInfo buffInf{};
       buffInf.offset = 0;
       buffInf.range  = unf._buff._size;
       buffInf.buffer = unf._buff.handle;

       wrt0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
       wrt0.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
       wrt0.descriptorCount = 1; 
       wrt0.pBufferInfo     = &buffInf;


       defaultCam.setPerp(FOV, 1.0, 0.0001);
//       Rect r{};
//       r.buttom = -1.0;
//       r.top    = 1.0;
//       r.right  = 1.0;
//       r.left   = -1.0;
//       defaultCam.setOrtho(r, 0.00, 100.0);
//       defaultCam.updateView(); 
//       terData.rot.x = 90.0;
       defaultCam.updateView(); 
//       terData.rot.x = 90.0;
//
//       unfData.model = terData.setModel();
       unfData.proj = defaultCam._proj;
       unfData.view = defaultCam._view;

       unf.wrt(&unfData);
       obj._descSet.wrt(&wrt0, 0);  
       obj.update(frame.cmdBuff);

       obj.draw();

       gtxBase.bind(frame.cmdBuff);

       cubeTrans.scale = {0.1,0.3,0.1};
       unfData.model = cubeTrans.setModel();
       unfData.proj  = defaultCam._proj;
       unfData.view  = defaultCam._view;
       static UniBuff unf0;

       unf0.create(app.data,sizeof(unfData));
       unf0.wrt(&unfData);
       buffInf.buffer = unf0._buff.handle; 
       cubeObj._descSet.wrt(&wrt0, 0);
       cubeObj.update(frame.cmdBuff);

       cubeObj.draw();
    
       rdrpass.end(frame.cmdBuff);

       frame.submitInfo.signalSemaphoreCount = 0;
       frame.submitInfo.waitSemaphoreCount   = 0; //We wait only for the first renderpss, the wait semaphore is img available
       vkEndCommandBuffer(frame.cmdBuff.handle);
       vkQueueSubmit(frame.cmdBuff.queue, 1, &frame.submitInfo, frame.fenQueueSubmitComplete.handle);
       vkWaitForFences(app.data.dvc, 1, &frame.fenQueueSubmitComplete.handle, VK_TRUE, UINT64_MAX);
       vkResetFences(app.data.dvc, 1, &frame.fenQueueSubmitComplete.handle);
       vkResetCommandBuffer(frame.cmdBuff.handle, 0);
       vkBeginCommandBuffer(frame.cmdBuff.handle, &frame.beginInfo);
       frame.submitInfo.signalSemaphoreCount = 1;   
       vkQueueWaitIdle(q); 

       unf0.dstr();     

}



void terUpdate(Obj* obj) {
    MeshRenderer* mr = obj->get<MeshRenderer>();
    Transform*    tr = obj->get<Transform>();
    auto& vkdat = mr->_gobj._gtx->rdrpass->_vkdata;
    CbkData& dat = *(CbkData*)obj->updateCbkData;

       VkWriteDescriptorSet   wrt0{};
       VkDescriptorBufferInfo buffInf{};
       buffInf.offset = 0;
       buffInf.range  = dat.unf._buff._size;
       buffInf.buffer = dat.unf._buff.handle;

       wrt0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
       wrt0.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
       wrt0.descriptorCount = 1; 
       wrt0.pBufferInfo     = &buffInf;

       defaultCam.setPerp(FOV, 1.0, 0.0001);
       defaultCam.updateView(); 
       tr->rot.x = 90.0;

       dat.unfData.model = tr->setModel();
       dat.unfData.proj  = defaultCam._proj;
       dat.unfData.view  = defaultCam._view;

       dat.unf.wrt(&dat.unfData);
       mr->_gobj._descSet.wrt(&wrt0, 0);  
       mr->_gobj.update(*GlobalData::cmdBuff);

       mr->_gobj.draw();
}

void terInit(Obj* obj) {
    MeshRenderer* mr = obj->get<MeshRenderer>();
    auto& vkdat = mr->_gobj._gtx->rdrpass->_vkdata;
    CbkData& dat = *(CbkData*)obj->updateCbkData;
    dat.unf.create( vkdat, sizeof(UnfData) );
}

static void mrtRndPass2(Scene& scene, Frame& frame, Vkapp& app, VkQueue& q) {

       Renderpass& rdrpass = *scene.objs.begin()->first->rdrpass;
       rdrpass.begin(frame.cmdBuff, frame.swpIndex);        
       
       setViewPort(frame._data.win->drawArea.x, frame._data.win->drawArea.y, frame);
       scene.update();
    
       rdrpass.end(frame.cmdBuff);

       frame.submitInfo.signalSemaphoreCount = 0;
       vkEndCommandBuffer(frame.cmdBuff.handle);
       vkQueueSubmit(frame.cmdBuff.queue, 1, &frame.submitInfo, frame.fenQueueSubmitComplete.handle);
       vkWaitForFences(app.data.dvc, 1, &frame.fenQueueSubmitComplete.handle, VK_TRUE, UINT64_MAX);
       vkResetFences(app.data.dvc, 1, &frame.fenQueueSubmitComplete.handle);
       vkResetCommandBuffer(frame.cmdBuff.handle, 0);
       vkBeginCommandBuffer(frame.cmdBuff.handle, &frame.beginInfo);
       frame.submitInfo.signalSemaphoreCount = 1;   
       vkQueueWaitIdle(q); //I need timeline semaphores and generally sync abstraction

//       frame.submitInfo.signalSemaphoreCount = 0;
//       frame.submitInfo.waitSemaphoreCount   = 0; //We wait only for the first renderpss, the wait semaphore is img available
//       vkEndCommandBuffer(frame.cmdBuff.handle);
//       vkQueueSubmit(frame.cmdBuff.queue, 1, &frame.submitInfo, frame.fenQueueSubmitComplete);
//       vkWaitForFences(app.data.dvc, 1, &frame.fenQueueSubmitComplete, VK_TRUE, UINT64_MAX);
//       vkResetFences(app.data.dvc, 1, &frame.fenQueueSubmitComplete);
//       vkResetCommandBuffer(frame.cmdBuff.handle, 0);
//       vkBeginCommandBuffer(frame.cmdBuff.handle, &frame.beginInfo);
//       frame.submitInfo.signalSemaphoreCount = 1;   
//       vkQueueWaitIdle(q); 


}

static void defRndPass(Renderpass& rdrpassDef, Renderpass& rdrpass, 
                       Frame& frame, Vkapp& app, GfxContext& gtxDef, 
                       VkQueue& q,
                       GfxObject& rendObj, ImgView& v0, ImgView& v1, ImgView& v2, Sampler& defSampler) {

       rdrpassDef.begin(frame.cmdBuff, frame.swpIndex); 
       setViewPort(app.win.drawArea.x, app.win.drawArea.y, frame);
       gtxDef.bind(frame.cmdBuff);
       //----------Update Descriptor set and uniform----------
       VkWriteDescriptorSet  wrt = {};
       VkWriteDescriptorSet  wrt0 {};
       VkDescriptorImageInfo inf{};
       VkDescriptorBufferInfo infB{};
       

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
       rendObj._descSet.wrt(&wrt, 0);
       ++iter;
       iter->image.crtInfo.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
       iter->image.changeLyt(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, *frame._data.cmdBuffPool);
       v1.fillCrtInfo(iter->image); v1.create(app.data);
       inf.imageView = v1.handle;
       rendObj._descSet.wrt(&wrt, 1);
       ++iter;
       iter->image.crtInfo.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
       iter->image.changeLyt(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, *frame._data.cmdBuffPool);
       v2.fillCrtInfo(iter->image); v2.create(app.data);
       inf.imageView = v2.handle;
       rendObj._descSet.wrt(&wrt, 2);

       //-------------------
       lightBuffer.wrt(&defaultLight);
       wrt0.descriptorCount = 1;
       wrt0.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
       wrt0.pBufferInfo    = &infB;
       infB.range          = sizeof(LightData);
       infB.buffer         = lightBuffer._buff.handle;
       rendObj._descSet.wrt(&wrt0, 3);
       //-------------------
       rendObj.update(frame.cmdBuff);
       rendObj.draw();

       UIBegin(); 
       UIRender();
       UIEnd();

       rdrpassDef.end(frame.cmdBuff); 
       
       frame.submitInfo.waitSemaphoreCount = 0;

}





void Engine::run(Vkapp& app) {
    //Set default values
    defaultCam.trans.pos = {0., 0.25, 1.15};
    defaultCam.trans.rot = {28.05, 0., 0.0};
    //------------------
    CmdBufferPool gfxCmdPool;
    CmdBufferPool compCmdPool;
    CmdBuff       compCmdBuff;
    DescPool      descPool;
    DescPool      descPool2;
    Renderpass    rdrpass;
    Renderpass    rdrpassDef;
    Renderpass    rdrpassShadow;
    Swapchain     swpchain;

    Frame         frame;
    FrameData     frameData;
    Sampler       defSampler;
    
    //-----------Creating Resources----------- 

    VulkanSupport::QueueFamIndices qfam;
    VulkanSupport::findQueues(qfam, app.data);
    gfxCmdPool.create(app.data, qfam.gfx );
    compCmdPool.create(app.data, qfam.com);
    VkQueue q = VulkanSupport::getQueue(app.data, offsetof(VulkanSupport::QueueFamIndices, gfx));

    lightBuffer.create(app.data, sizeof(LightData));

    defaultCam.trans.pos = {5.0, 20.2, 101.};
    defaultLight.pos.z = 100.0f;
    //-----------Setup and create renderpass----------- 
    //Fist rdrpass MRT
    AttachmentContainer att;
    auto colAtt = att.add();
    colAtt->desc.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    att.addDepth();

    auto normalAtt         = att.add();
    normalAtt->desc.format = VK_FORMAT_R16G16B16A16_SFLOAT;

    auto posAtt = att.add();
    posAtt->desc.format    = VK_FORMAT_R16G16B16A16_SFLOAT;



    rdrpass._subpasses.setup(1,4);
    rdrpass._subpasses.add(app.win, app.data, att, nullptr);  //First subpass
    rdrpass.setSwpChainHijack(-1, 0);
    rdrpass.create(app.data, app.win);
    rdrpass.fillBeginInfo(app.win, {0.0f, 0.0, 0.0, 0.0f});

    //Second rdrpass for rendering
    AttachmentContainer att0;
    auto colAtt0 = att0.add();
    colAtt0->desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; 
    colAtt0->desc.format      = VK_FORMAT_R8G8B8A8_SRGB; 
    rdrpassDef._subpasses.setup(1,1);
    rdrpassDef._subpasses.add(app.win, app.data, att0, nullptr);
    rdrpassDef.setSwpChainHijack(0, 0); 
    rdrpassDef.create(app.data, app.win);
    rdrpassDef.fillBeginInfo(app.win);

    //Renderpass for shadowmap
    AttachmentContainer att1;
    Attachment* colAtt1 = att1.add();
    att1.addDepth();
    rdrpassShadow._subpasses.setup(1,2);
    rdrpassShadow._subpasses.add(app.win, app.data, att1, nullptr);
    rdrpassShadow.setSwpChainHijack(-1,0);
    rdrpassShadow.create(app.data, app.win);
    rdrpassShadow.fillBeginInfo(app.win);

    //-------------Swapchain---------------
    swpchain.create(app.data, app.win, rdrpassDef);
    //-----------Create rdrpass framebuffers----------
    setupRdrpassFmbuffs(rdrpass, app.win, app.data, swpchain);
    setupRdrpassFmbuffs(rdrpassShadow, app.win, app.data, swpchain);
    //----------Create Sampler--------------
    defSampler.fillCrtInfo(app.data);
    defSampler.create(app.data);
    //----------Create Frame----------------

    frameData.win = &app.win;
    frameData.swpchain = &swpchain;
    frameData.cmdBuffPool = &gfxCmdPool;
    frameData.rdrpass = &rdrpassDef;
    frame._data = frameData;
    frame.create(app.data);
    //------------Set Global Data-------------
    GlobalData::cmdBuff = &frame.cmdBuff;
    GlobalData::cmdBuffPool = &gfxCmdPool;
    GlobalData::app         = &app;
    


    //---------------Create Abstraction for gfx---------------------

    Quad t;
    Quad t0;
    Cube cube;
    SubdivQuad subQuad;

    std::vector<VkDescriptorSetLayoutBinding> lytBindings;
    lytBindings =  
    {
     {0, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
     {1, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
     {2, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
     {3, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
    };
  
    //gtx.setup({P0PATHVERT, P0PATHFRAG, P0PATHGEOM}, &rdrpass);
    //gtx.pipeline.rasterState.polygonMode = VK_POLYGON_MODE_LINE;
    //gtx.create();

    gtxDef.setup({ P1PATHVERT, P1PATHFRAG }, &rdrpassDef, &lytBindings);
    gtxDef.create();
    
    SubdivQuad sq;
    sq.init(1, 0.5, 0); Scene scene;
    GfxContext* mrtGtx = scene.push();
    mrtGtx->setup({ P0PATHVERT, P0PATHFRAG, P0PATHGEOM }, &rdrpass);
    //mrtGtx->pipeline.rasterState.cullMode = VK_CULL_MODE_NONE; //Temporary, i want to flip terrain
    //mrtGtx->pipeline.rasterState.polygonMode = VK_POLYGON_MODE_LINE;
    mrtGtx->create();

    Obj& terrain     = scene.push(mrtGtx);
    void (*initProc)(Obj*) = [](Obj* obj) -> void { 
        obj->get<Terrain>()->init();
    };
    void (*updateProc)(Obj*) = [](Obj* obj) -> void { 
        defaultCam.setPerp(FOV, 1.0, 0.0001);
        defaultCam.updateView(); 
        obj->get<Terrain>()->update();
    }; 

     Terrain* tt = terrain.add<Terrain>();
     tt->gtx = mrtGtx;
     tt->cam = &defaultCam;
     terrain.setInitCbk( initProc );
     terrain.setUpdateCkb( updateProc );

//    MeshRenderer* mr = terrain.add<MeshRenderer>();
//    terrain.add<Transform>();
//    mr->_gobj.init(mrtGtx,  &sq._data); 
//    CbkData d;
//    terrain.setInitCbk(terInit, &d);
//    terrain.setUpdateCkb(terUpdate, &d);

    scene.init();

    gtxShadow.setup({P2PATHVERT, P2PATHFRAG}, &rdrpassShadow);
    gtxShadow.create();

    gtxBase.setup({"../build/bin/base.mrt.vert.spv","../build/bin/base.mrt.frag.spv"}, &rdrpass);
    gtxBase.create();

    //subQuad.init(10, 0.0, 1); 
    //cube.init();
    t0.setInitSize({2.0f,2.0f});
    t0.init();

    //terObj.init(&gtx, &subQuad._data);
    //cubeObj.init(&gtx, &cube);
    rendObj.init(&gtxDef, &t0);


    ImgView v0, v1, v2;
//    UniBuff unf;
//    UnfData unfData{};
//    unf.create(app.data, sizeof(unfData));
    //------------Init ImGui------------------
    DescPool imguiPool;
    imguiPool._lytBindings = {
        { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
    };
    imguiPool.create(app.data);

    VkExtent2D imageExtent;
    VulkanSupport::SwpchainCap spec;  
    VulkanSupport::getSwapchaincap(app.data, spec);
    swpchain.chooseExtent(app.win, spec.cap, &imageExtent);
    VkSurfaceFormatKHR srfcFmt = spec.srfcFormats[VulkanSupport::selSrfcFmt(spec)];
    ui32 imgCount = spec.cap.minImageCount + 1;
    if (spec.cap.maxImageCount > 0 && imgCount > spec.cap.maxImageCount) {
        imgCount = spec.cap.maxImageCount;
    }
    
    ImGui_ImplVulkan_InitInfo info{};
    info.Instance  = app.data.inst;
    info.PhysicalDevice = app.data.phyDvc;
    info.Device = app.data.dvc;
    info.Queue  = q;
    info.Subpass = 0;
    info.Allocator = nullptr;
    info.ImageCount = imgCount;
    info.MSAASamples    = VK_SAMPLE_COUNT_1_BIT;
    info.MinImageCount  = spec.cap.minImageCount;
    info.RenderPass     = rdrpassDef.handle;
    info.DescriptorPool = imguiPool.handle;
   
    Init(&info);
    //-----------Loop------------
    while (app.win.ptr->shouldLoop()) {

       frame.submitInfo.waitSemaphoreCount = 1;
       if (!frame.begin())
           continue;

        v0.dstr();
        v1.dstr();
        v2.dstr();

       //shadowRndPass(rdrpassShadow, frame, app, gtxShadow, unfData, unf, terObj, q);
       //mrtRndPass(rdrpass, frame, app, gtx, unfData, unf, terObj, q);
       mrtRndPass2(scene, frame, app, q);
       defRndPass(rdrpassDef, rdrpass, frame, app, gtxDef, q, rendObj, v0, v1, v2, defSampler);

       frame.end();
    }
    //-----------------------Clean Resources------------------
    frame.dstr();

	//gtx.dstr();
    gtxDef.dstr();

    swpchain.dstr();
    rdrpass.dstr();
    rdrpassDef.dstr();

	gfxCmdPool.dstr();
	compCmdPool.dstr();

    defSampler.dstr(); 
    t.destroy();
    t0.destroy();
    cube.destroy();
    //unf.dstr();
    v0.dstr();
    v1.dstr();
    v2.dstr();
}

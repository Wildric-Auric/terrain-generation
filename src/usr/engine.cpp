
#include "engine.h"

#include "ui.h"

#include "bcknd/cmd_buffer.h"
#include "bcknd/desc.h"
#include "bcknd/frame.h"
#include "bcknd/support.h"
#include "bcknd/params.h"
#include "bcknd/vkimg.h"

#include "shared.h"
#include "shapes.h"
#include "gfx.h"
#include "cam.h"

#include "scene.h"
#include "components/mesh_renderer.h"
#include "terrain.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"


#define P0PATHVERT "../build/bin/gpu_terrain.mrt.vert.spv"
#define P0PATHFRAG "../build/bin/gpu_terrain.mrt.frag.spv"
#define P0PATHGEOM "../build/bin/gpu_terrain.mrt.geom.spv"

#define P1PATHVERT "../build/bin/def.vert.spv"
#define P1PATHFRAG "../build/bin/def.frag.spv"

#define P2PATHVERT "../build/bin/shadow.vert.spv"
#define P2PATHFRAG "../build/bin/shadow.frag.spv"

#define PATHSHADOWFRAG "../build/bin/shadow.frag.spv"
UniBuff lightBuffer;

float FOV = 60.0;

LightData defaultLight;
Cam       defaultCam;
Transform cubeTrans(nullptr);
GfxObject  terObj;
GfxObject  cubeObj;
GfxObject  rendObj;

GfxContext gtxDef;       //Final deferred rendering ctx 

Img     im; 
ImgView imgView;
Sampler smpler;

void loadImg(const std::string& path, Img& im, ImgView& imView) {
    Buffer  imBuff;
    ui32  imgSize;
    ivec2 vecSize;
    i32 channels;
    stbi_uc* pixels = stbi_load(path.c_str(), &vecSize.x, &vecSize.y, &channels, STBI_rgb_alpha);
    imgSize = vecSize.x * vecSize.y * 4;

    imBuff.fillCrtInfo();
    imBuff.crtInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    imBuff.memProp       = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    imBuff.create(GlobalData::app->data, imgSize);
    void* mem;
    imBuff.mapMem(&mem);
    memcpy(mem, pixels, imgSize);
    imBuff.unmapMem(&mem);

    im.fillCrtInfo();
    im.crtInfo.extent.width  = vecSize.x;
    im.crtInfo.extent.height = vecSize.y;
    im.crtInfo.usage         = VK_IMAGE_USAGE_SAMPLED_BIT      
                             | VK_IMAGE_USAGE_TRANSFER_SRC_BIT 
                             | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    im.setMaxmmplvl();
    im.create(GlobalData::app->data);
    im.cpyFrom(*GlobalData::cmdBuffPool, imBuff, vecSize, 0);
    im.genmmp(*GlobalData::cmdBuffPool, offsetof(VulkanSupport::QueueFamIndices, gfx));
    im.changeLyt(VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    *GlobalData::cmdBuffPool); 

    imView.fillCrtInfo(im);
    imView.create(GlobalData::app->data);

    imBuff.dstr();
}

typedef struct {
        Matrix4<float> view;
        Matrix4<float> model;
        Matrix4<float> proj;
} UnfData;
typedef struct {
    UnfData unfData;
    UniBuff unf;
} CbkData;

void setViewPort(ui32 x, ui32 y, Frame& frame, bool m = 0) {
       VkViewport viewport;
       VkRect2D   scissor;
       viewport.minDepth = 0.0f;
       viewport.maxDepth = 1.0f;
       ui32 v = Min<i32>(x,y);
       if (m) {
       viewport.width =  Max<i32>(x, 5);
       viewport.height = Max<i32>(y, 5);
       }
       else {
       viewport.width =  Max<i32>(v, 5);
       viewport.height = Max<i32>(v, 5);
       }
       viewport.y = Max<i32>(-0.5 * viewport.height + y * 0.5, 0.0);
       viewport.x = Max<i32>(-0.5 * viewport.width  + x * 0.5, 0.0);
       scissor.extent =  {(ui32)viewport.width, (ui32)viewport.height};
       scissor.offset =  { (i32)viewport.x, (i32)viewport.y };
       vkCmdSetViewport(frame.cmdBuff.handle, 0, 1, &viewport);
       vkCmdSetScissor(frame.cmdBuff.handle, 0, 1, &scissor);
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

static void shadowRndPass2(Scene& scene, Frame& frame, Vkapp& app, VkQueue& q) {

       Renderpass& rdrpass = *scene.objs.begin()->first->rdrpass;
       rdrpass.begin(frame.cmdBuff, frame.swpIndex);        
       
       setViewPort(frame._data.win->drawArea.x, frame._data.win->drawArea.y, frame);
       scene.update();
    
       rdrpass.end(frame.cmdBuff);
}

static void mrtRndPass2(Scene& scene, Frame& frame, Vkapp& app, VkQueue& q) {
       Renderpass& rdrpass = *scene.objs.begin()->first->rdrpass;
       rdrpass.begin(frame.cmdBuff, frame.swpIndex);         
       setViewPort(frame._data.win->drawArea.x, frame._data.win->drawArea.y, frame);
       scene.update(); 
       rdrpass.end(frame.cmdBuff);
}

static void defRndPass(Renderpass& rdrpassDef, Renderpass& rdrpass, 
                       Frame& frame, Vkapp& app, GfxContext& gtxDef, 
                       VkQueue& q,
                       GfxObject& rendObj, ImgView& v0, ImgView& v1, ImgView& v2, Sampler& defSampler) {

       rdrpassDef.begin(frame.cmdBuff, frame.swpIndex); 
       setViewPort(app.win.drawArea.x, app.win.drawArea.y, frame, 1);
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
    RenderpassContainer rdrcnt;
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

    defaultCam.trans.pos = {8.6,1.8, 49.4};
    defaultCam.trans.rot = {33.1,-1.0, 0.0};
    defaultLight.pos     = {10.0,0.5,10.0};
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


    Renderpass& rdrpass = rdrcnt.add();
    Renderpass& rdrpassDef = rdrcnt.add();
    Renderpass& rdrpassShadow = rdrcnt.add();


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
    att1.addDepth();
    rdrpassShadow._subpasses.setup(1,1);
    rdrpassShadow._subpasses.add(app.win, app.data, att1, nullptr);
    rdrpassShadow.setSwpChainHijack(-1,-1);
    rdrpassShadow.create(app.data, app.win);
    rdrpassShadow.fillBeginInfo(app.win);
    
    //Create swapchain and framebuffers
    swpchain.create(app.data, app.win);
    rdrpassDef.createFmbuffs(swpchain);
    rdrpass.createFmbuffs(swpchain);
    rdrpassShadow.createFmbuffs(swpchain);
    defSampler.fillCrtInfo(app.data);
    defSampler.create(app.data);

    //Create frame
    frameData.win = &app.win;
    frameData.swpchain = &swpchain;
    frameData.cmdBuffPool = &gfxCmdPool;
    frameData.rdrpassCnt = &rdrcnt;
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
  
    gtxDef.setup({ P1PATHVERT, P1PATHFRAG }, &rdrpassDef, &lytBindings);
    gtxDef.create();
    
    SubdivQuad sq;
    sq.init(1, 0.5, 0); Scene scene;
    GfxContext* mrtGtx = scene.push();

    //Temp----------------------
    loadImg("../res/heightmap.png", im, imgView);
    smpler.fillCrtInfo(app.data);
    smpler.create(app.data);
    //--------------------------- 




    std::vector<VkDescriptorSetLayoutBinding> descPoolBindings = {
        { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
        { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_GEOMETRY_BIT, nullptr}
    };

    mrtGtx->setup({ P0PATHVERT, P0PATHFRAG, P0PATHGEOM }, 
                  &rdrpass, &descPoolBindings);
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

    scene.init();

    //Shadow scene-------
    Scene shadowScene;
    GfxContext* shadowGtx = shadowScene.push();
    shadowGtx->setup({ P0PATHVERT, "", P0PATHGEOM},
                     &rdrpassShadow, &descPoolBindings);

    shadowGtx->create();
    Obj& terrainShadowObj  = shadowScene.push(shadowGtx);
    Terrain* terrainShadow = terrainShadowObj.add<Terrain>();
    terrainShadow->gtx     = shadowGtx;
    terrainShadow->cam     = &defaultCam;
    terrainShadowObj.setInitCbk(initProc);
    terrainShadowObj.setUpdateCkb(updateProc);
    shadowScene.init();
    //-------------------

    //subQuad.init(10, 0.0, 1); 
    //cube.init();
    t0.setInitSize({2.0f,2.0f});
    t0.init();

    //terObj.init(&gtx, &subQuad._data);
    //cubeObj.init(&gtx, &cube);
    rendObj.init(&gtxDef, &t0);


    ImgView v0, v1, v2;
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
       if (!frame.begin())
           continue;

        v0.dstr();
        v1.dstr();
        v2.dstr();

       shadowRndPass2(shadowScene, frame, app, q);
       frame.nextRdrpass();
       mrtRndPass2(scene, frame, app, q);
       frame.nextRdrpass();
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
    v0.dstr();
    v1.dstr();
    v2.dstr();
}

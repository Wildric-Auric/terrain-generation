#include "frame.h" 
#include "support.h" 
#include "vkimg.h"
#include "params.h"


bool Renderpass::setSwpChainHijack(ui32 subpassIndex, ui32 attIndex) {
    //if (_subpassHjckIndex >= _subpasses.descs.size() || attIndex >= _subpasses._strideInfo[_subpassHjckIndex].colLen) 
    //    return 0;
    _subpassHjckIndex = subpassIndex;
    _attHjckIndex     = attIndex;
    return 1;
}

VkResult Swapchain::create(const VulkanData& vkdata, const Window& win) {
    VkResult res;
    VkSwapchainCreateInfoKHR crtInfo{};

    VulkanSupport::SwpchainCap spec;
    _vkdata = vkdata;
    
    VulkanSupport::getSwapchaincap(_vkdata, spec);
    _srfcFmt = spec.srfcFormats[VulkanSupport::selSrfcFmt(spec)];

    _imgCount = spec.cap.minImageCount + 1;
    if (spec.cap.maxImageCount > 0 && _imgCount > spec.cap.maxImageCount) {
        _imgCount = spec.cap.maxImageCount;
    }

    crtInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    crtInfo.surface = vkdata.srfc;

    crtInfo.imageFormat =     _srfcFmt.format;
    crtInfo.imageColorSpace = _srfcFmt.colorSpace;

    crtInfo.minImageCount = _imgCount;
    crtInfo.imageArrayLayers = 1; 
    chooseExtent(win, spec.cap, &crtInfo.imageExtent);
    _extent = ivec2(crtInfo.imageExtent.width, crtInfo.imageExtent.height);
    crtInfo.imageUsage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //Render directly
    crtInfo.presentMode = VulkanSupport::selPresent();

    crtInfo.preTransform   = spec.cap.currentTransform;
    crtInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    crtInfo.clipped        = VK_TRUE;
    crtInfo.oldSwapchain   = VK_NULL_HANDLE;


    VulkanSupport::QueueFamIndices fam;
    VulkanSupport::findQueues(fam, _vkdata);
    //Only present family and gfx have access for now
    i32 indices[] = {
        fam.gfx,
        fam.pre
    };

    if (fam.pre == fam.gfx) {
        crtInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        crtInfo.pQueueFamilyIndices = nullptr;
        crtInfo.queueFamilyIndexCount = 0;
    }
    else {
        crtInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        crtInfo.pQueueFamilyIndices   = (const ui32*)indices;
        crtInfo.queueFamilyIndexCount = sizeof(indices) / sizeof(indices[0]);
    }
    res = vkCreateSwapchainKHR(vkdata.dvc, &crtInfo, nullptr, &handle);
    VK_CHECK_EXTENDED(res, "Failed to create swapchain");
    vkGetSwapchainImagesKHR(vkdata.dvc, handle, &_imgCount, nullptr);
    imgs.resize(_imgCount);
    vkGetSwapchainImagesKHR(vkdata.dvc, handle, &_imgCount, imgs.data());
    //Create swapchaine image views and framebuffers
    Img fake;
    views.resize(_imgCount); 
    for (arch i = 0; i < _imgCount; ++i) {
        fake.crtInfo.format = _srfcFmt.format;
        fake.crtInfo.samples = (VkSampleCountFlagBits)GfxParams::inst.msaa;
        fake.handle         = imgs[i];
        views[i].fillCrtInfo(fake);
        views[i].create(_vkdata);
    }
    return res;
}

void Swapchain::chooseExtent(const Window& win, const VkSurfaceCapabilitiesKHR& cap, VkExtent2D* const outExt  ) {

    if (cap.currentExtent.width != (ui32)(-1)) {
        *outExt = cap.currentExtent;
        return;
    }

    ivec2 size = win.drawArea;
    outExt->width = Clamp<ui32>(size.x, cap.minImageExtent.width, cap.maxImageExtent.width);
    outExt->height= Clamp<ui32>(size.y, cap.minImageExtent.height, cap.maxImageExtent.height);
}

void Swapchain::dstr() {
    vkDeviceWaitIdle(_vkdata.dvc);
    for (arch i = 0; i < imgs.size(); ++i) {
        views[i].dstr();
    }
    vkDestroySwapchainKHR(_vkdata.dvc, handle, nullptr);
}

void Attachment::setup() {
    desc.format      = VK_FORMAT_R8G8B8A8_UNORM;
    desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    desc.finalLayout   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    desc.samples       = (VkSampleCountFlagBits)GfxParams::inst.msaa;
    desc.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
    desc.storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
    desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    desc.stencilStoreOp= VK_ATTACHMENT_STORE_OP_DONT_CARE;
    ref.layout         = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

Attachment* AttachmentContainer::getDepth() {
    if (_hasDepth) return nullptr;
    return &depth;
}

Attachment* AttachmentContainer::get(arch index) {
    return &_container[index];
}

Attachment* AttachmentContainer::add(bool hasResolve) {
    _hasResolve = hasResolve;
    _container.push_back({});
    Attachment* att = &_container.back();
    att->setup();
    return att;
}

Attachment*  AttachmentContainer::addDepth() {
    Attachment* att = &depth;
    depth.setup();
    att->desc.format      = VK_FORMAT_D24_UNORM_S8_UINT;
    att->desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    att->desc.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
    att->ref.layout       = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

   _hasDepth = true; 
    return att;
}

void SubpassContainer::addDepthRes(const Window& win, const VulkanData& _vkdata, const VkAttachmentDescription& desc) {
        resources[_ptrAttContainer] = {};
        AttachmentData& depth = resources[_ptrAttContainer];

        depth.image.fillCrtInfo();
        depth.image.crtInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        depth.image.crtInfo.samples       = (VkSampleCountFlagBits)GfxParams::inst.msaa;
        depth.image.crtInfo.format        =  desc.format;
        depth.image.crtInfo.extent.width  =  win.drawArea.x; 
        depth.image.crtInfo.extent.height =  win.drawArea.y;
        depth.image.crtInfo.initialLayout =  VK_IMAGE_LAYOUT_UNDEFINED;
        depth.image.create(_vkdata);

        depth.view.fillCrtInfo(depth.image);
        depth.view.crtInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depth.view.create(_vkdata);
}

void SubpassContainer::addColorRes(const Window& win, const VulkanData& vkdata, const VkAttachmentDescription& desc) {
      //TODO::Parameter for format
       resources[_ptrAttContainer] = {};
       AttachmentData& color = resources[_ptrAttContainer];

       color.image.fillCrtInfo();
       color.image.crtInfo.format  = desc.format;
       color.image.crtInfo.usage   = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; //Changed
       color.image.crtInfo.samples = VK_SAMPLE_COUNT_1_BIT; /*(VkSampleCountFlagBits)GfxParams::inst.msaa*/;
       color.image.crtInfo.extent.width  = win.drawArea.x;
       color.image.crtInfo.extent.height = win.drawArea.y;
       color.image.create(vkdata);

       color.view.fillCrtInfo(color.image);
       color.view.crtInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
       color.view.create(vkdata);
}

void SubpassContainer::addResolveRes(const Window& win, const VulkanData& vkdata, const VkAttachmentDescription& desc) {
       resources[_ptrAttContainer] = {};
       AttachmentData& resolve = resources[_ptrAttContainer];
       resolve.image.fillCrtInfo();
       resolve.image.crtInfo.format        = desc.format;
       resolve.image.crtInfo.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
       resolve.image.crtInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
       resolve.image.crtInfo.samples       = (VkSampleCountFlagBits)GfxParams::inst.msaa;
       resolve.image.crtInfo.extent.width  = win.drawArea.x;
       resolve.image.crtInfo.extent.height = win.drawArea.y;
       resolve.image.create(vkdata);
       resolve.view.fillCrtInfo(resolve.image);
       resolve.view.crtInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
       resolve.view.create(vkdata);
}

void SubpassContainer::setup(arch subpassNum, arch totalAttNum) {
    _dpn.resize(subpassNum);
	descs.resize(subpassNum);
    _strideInfo.resize(subpassNum);
	attDescs.resize(totalAttNum);
	attRefs.resize(totalAttNum);
	resources.resize(totalAttNum);
}

std::vector<AttachmentData>::iterator SubpassContainer::getStrideColIterBegin(ui32 strideIndex, std::vector<AttachmentData>::iterator* end) {
    if (strideIndex >= descs.size() || _strideInfo[strideIndex].colOffset == -1) return resources.end();
    if (end != nullptr) 
        *end= resources.begin() + _strideInfo[strideIndex].colLen; 
    return resources.begin() + _strideInfo[strideIndex].colOffset;

}
std::vector<AttachmentData>::iterator SubpassContainer::getStrideColResolveBegin(ui32 strideIndex, std::vector<AttachmentData>::iterator* end) {
    i32 base = _strideInfo[strideIndex].colOffset;
    if (strideIndex >= descs.size() || _strideInfo[strideIndex].localResOffset == -1) return resources.end();
    if (end != nullptr) 
        *end= resources.begin() + _strideInfo[strideIndex].localResOffset + base + _strideInfo[strideIndex].colLen; 
    return resources.begin() + _strideInfo[strideIndex].localResOffset + base;
}

AttachmentData*  SubpassContainer::getStrideDepth(ui32 strideIndex) {
    i32 base = _strideInfo[strideIndex].colOffset;
    if (strideIndex >= descs.size() || _strideInfo[strideIndex].localDepthOffset == -1) return nullptr;
    return &resources[base + _strideInfo[strideIndex].localDepthOffset];
}

void SubpassContainer::add(const Window& win, const VulkanData& vkdata, AttachmentContainer& atts, VkSubpassDependency** depedencyWithPrevious) {
    Subpass s;
    ui32 beg = _ptrAttContainer;
    ui32 cur = _ptrAttContainer;
    ui32 resOffset  = 0;
    ui32 dpthOffset = 0;
    StrideData& sInfo = _strideInfo[_ptrSPContainer];

    sInfo.colOffset = beg;
    for (Attachment& att : atts._container) {
        att.ref.attachment = cur++; 
        attDescs[_ptrAttContainer] = att.desc;
        attRefs[_ptrAttContainer]   = att.ref;
        addColorRes(win, vkdata, att.desc);
        _ptrAttContainer++;
    }
    sInfo.colLen = cur - beg;
    
    if (GfxParams::inst.msaa != MSAAvalue::x1) {
        sInfo.localResOffset = sInfo.colLen;
        resOffset = cur;
        for (Attachment& att : atts._container) {
           VkAttachmentReference ref{};
           VkAttachmentDescription desc{};
           desc.samples       = VK_SAMPLE_COUNT_1_BIT;
           desc.format        = att.desc.format;
           desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
           desc.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; 
           desc.loadOp        = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
           desc.storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
           ref.layout         = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
           ref.attachment     = cur++;
           attDescs[_ptrAttContainer]  = desc;
           attRefs[_ptrAttContainer]   = ref;
           addResolveRes(win, vkdata, desc);
           _ptrAttContainer++;
        }
    }

    if (atts._hasDepth) {
        sInfo.localDepthOffset = sInfo.colLen + sInfo.colLen * (i32)( sInfo.localResOffset != -1);
        dpthOffset = cur;
        atts.depth.ref.attachment = cur++;
        attDescs[_ptrAttContainer] = atts.depth.desc;
        attRefs[_ptrAttContainer]  = atts.depth.ref;
        addDepthRes(win, vkdata, atts.depth.desc);
        _ptrAttContainer++;
    }


    s.desc.pColorAttachments       = attRefs.data() + beg;
    s.desc.colorAttachmentCount    = atts._container.size();
    s.desc.pResolveAttachments     = resOffset == 0 ? nullptr:attRefs.data() + resOffset; 
    s.desc.pDepthStencilAttachment = dpthOffset == 0? nullptr:attRefs.data() + dpthOffset;
    s.desc.pInputAttachments       = 0; //TODO::Add input attachment setup
    s.desc.inputAttachmentCount    = 0;

    descs[_ptrSPContainer] = s.desc;
    _dpn[_ptrSPContainer]  = {};

    VkSubpassDependency& t = _dpn[_ptrSPContainer];
    _ptrSPContainer++;

    if (_ptrSPContainer == 1) {
        t.srcSubpass           = VK_SUBPASS_EXTERNAL;
        t.dstSubpass           = 0;

        t.srcStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        t.dstStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; 

        t.srcAccessMask        = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        t.dstAccessMask        = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
  
    else {
        //Default is subpasses wait for previous to finish
        t.srcSubpass           = _ptrSPContainer - 2;
        t.dstSubpass           = _ptrSPContainer - 1;

        t.srcStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        t.dstStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; 

        t.srcAccessMask        = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        t.dstAccessMask        = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    }
    if (depedencyWithPrevious != nullptr)
        *depedencyWithPrevious = &t;
    return ;
}


VkResult Renderpass::create(const VulkanData& vkdata, const Window& win) {
    VkRenderPassCreateInfo crtInfo{};

    _vkdata = vkdata;
     
    crtInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    crtInfo.dependencyCount = _subpasses._ptrSPContainer;
    crtInfo.pDependencies   = _subpasses._dpn.data();
    crtInfo.subpassCount    = _subpasses._ptrSPContainer;
    crtInfo.pSubpasses      = _subpasses.descs.data();
    crtInfo.attachmentCount = _subpasses._ptrAttContainer;
    crtInfo.pAttachments    = _subpasses.attDescs.data();
    
    VkResult res = vkCreateRenderPass(_vkdata.dvc, &crtInfo, nullptr, &handle);
 
    return res;
}

void Renderpass::dstr() {
    vkDestroyRenderPass(_vkdata.dvc, handle, nullptr);
    dstrFmbuffs();
    dstrRes();
}

void Renderpass::dstrRes() {
    for (AttachmentData& d : _subpasses.resources) {
        d.view.dstr();
        d.image.dstr();  
    }
}

Renderpass& RenderpassContainer::add() {
    _passes.push_back({});
    return _passes.back();
}

Renderpass& RenderpassContainer::get(arch index) {
    return _passes[index];
}

void RenderpassContainer::dstr() {
    for (Renderpass& rdrpass : _passes) 
        rdrpass.dstr();
}

void Renderpass::dstrFmbuffs() {
    for (ui32 i = 0; i < fmbuffs.size(); ++i) {
        fmbuffs[i].dstr();
    }
}

void Renderpass::resizeFmbuff(const Swapchain& swpchain) {
    dstrFmbuffs(); 
    createFmbuffs(swpchain);
}

void Renderpass::resizeRes(const ivec2& newsize) {
    dstrRes();
    for(int i = 0; i < _subpasses.resources.size();++i) {
        AttachmentData& data = _subpasses.resources[i];
        if (data.view.handle == nullptr) continue;
        data.image.crtInfo.extent.width  = newsize.x;
        data.image.crtInfo.extent.height = newsize.y;
        data.image.crtInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        data.image.create(_vkdata);
        data.view.crtInfo.image = data.image.handle;
        data.view.create(_vkdata);
    }
    _rdrpassInfo.renderArea.extent = {(ui32)newsize.x, (ui32)newsize.y};
}

void Renderpass::resize(const Swapchain& swpchain) {
    resizeRes(swpchain._extent);
    resizeFmbuff(swpchain);
}

VkResult Renderpass::createFmbuffs(const Swapchain& swpchain) {
    VkResult res;
    fmbuffs.resize(swpchain._imgCount);
    for (arch i = 0; i < swpchain._imgCount; ++i) {
        fmbuffs[i].fillCrtInfo();
        fmbuffs[i].crtInfo.width  = swpchain._extent.x;
        fmbuffs[i].crtInfo.height = swpchain._extent.y;

        std::vector<VkImageView> att;

        for (arch k = 0; k < _subpasses.descs.size(); ++k) {
            AttachmentData* depth = _subpasses.getStrideDepth(k);
            auto colIter = _subpasses.getStrideColIterBegin(k, nullptr);
            auto resIter = _subpasses.getStrideColResolveBegin(k, nullptr);
            arch j = 0;
            if (resIter != _subpasses.resources.end()) {
                for (;j < _subpasses._strideInfo[k].colLen;++j) {
                    att.push_back((resIter++)->view.handle);
                }
            }

            if (colIter != _subpasses.resources.end()) {
                for (j=0;j < _subpasses._strideInfo[k].colLen;++j) {
                    if (_subpassHjckIndex == k && _attHjckIndex == j)  {
                        att.push_back(swpchain.views[i].handle);
                        ++colIter;
                        continue;
                    }
                    att.push_back((colIter++)->view.handle);
                }
            }

            if (depth != nullptr)
                att.push_back(depth->view.handle);
        }
        res = fmbuffs[i].create(_vkdata, handle, att.data(), att.size());
    }
    return res;
}

VkRenderPassBeginInfo& Renderpass::fillBeginInfo(const Window& win, const fvec4& clrCol) {
    _clearCol.clear();
    for (const auto& res : _subpasses.resources) {
        if (VulkanSupport::isDepthStencil(res.view.crtInfo.format)) {
            _clearCol.push_back({1.0, 0.0});
        }
        else {
            _clearCol.push_back({clrCol.x, clrCol.y, clrCol.z, clrCol.w});
        }
    } 
    _rdrpassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    _rdrpassInfo.renderPass        = handle;
    _rdrpassInfo.renderArea.offset = {0,0};
    _rdrpassInfo.renderArea.extent = {(ui32)win.drawArea.x, (ui32)win.drawArea.y};

    _rdrpassInfo.clearValueCount     = _clearCol.size();
    _rdrpassInfo.pClearValues        = _clearCol.data();

    return _rdrpassInfo;
}

void Renderpass::begin(CmdBuff& cmdbuff, ui32 swpIndex) { 
    _rdrpassInfo.framebuffer = fmbuffs[swpIndex].handle;
    vkCmdBeginRenderPass(cmdbuff.handle, &_rdrpassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Renderpass::end(CmdBuff& cmdbuff) {
    vkCmdEndRenderPass(cmdbuff.handle);
}

VkResult Frame::create(const VulkanData& vkdata) {
    _vkdata = vkdata;
    VkFenceCreateInfo     fenCrtInfo;
    VkSemaphoreCreateInfo semCrtInfo;

    fenCrtInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenCrtInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    fenCrtInfo.pNext = nullptr;

    semCrtInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semCrtInfo.pNext = nullptr;
    semCrtInfo.flags = NULL;

    fenQueueSubmitComplete.fillCrtInfo(_vkdata, 1);
    semImgAvailable.fillCrtInfo(_vkdata, 1);
    semRdrFinished.fillCrtInfo(_vkdata, 1);

    VkResult res =  fenQueueSubmitComplete.create();//vkCreateFence(_vkdata.dvc, &fenCrtInfo, nullptr, &fenQueueSubmitComplete);
    VkResult res0 = semImgAvailable.create();       //vkCreateSemaphore(_vkdata.dvc, &semCrtInfo, nullptr, &semImgAvailable);
    VkResult res1 = semRdrFinished.create();        //vkCreateSemaphore(_vkdata.dvc, &semCrtInfo, nullptr, &semRdrFinished);

    if (res != VK_SUCCESS) {
        return res; 
    }
    else if (res0 != VK_SUCCESS) {
        return res0; 
    }
    else if (res1 != VK_SUCCESS) {
        return res1;
    }


    VkQueue q = VulkanSupport::getQueue(_vkdata, offsetof(VulkanSupport::QueueFamIndices, gfx));
    _data.cmdBuffPool->allocCmdBuff(&cmdBuff, q);
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = NULL;
    beginInfo.pNext = nullptr;
    beginInfo.pInheritanceInfo = nullptr;

    NWin::Vec2 size;
    _data.win->ptr->getDrawAreaSize(size);

    submitInfo.sType =  VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &cmdBuff.handle;
    submitInfo.waitSemaphoreCount   = 1;

    submitInfo.pWaitSemaphores    = &semImgAvailable.handle;
    submitInfo.pWaitDstStageMask  = &waitDstStage;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &semRdrFinished.handle;

    preInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    preInfo.swapchainCount     = 1;
    preInfo.pSwapchains        = &_data.swpchain->handle;
    preInfo.pImageIndices      = &swpIndex;
    preInfo.waitSemaphoreCount = 1;
    preInfo.pWaitSemaphores    = &semRdrFinished.handle;

    
    VulkanSupport::QueueFamIndices qfam; VulkanSupport::findQueues(qfam, _vkdata);

    vkGetDeviceQueue(_vkdata.dvc, qfam.gfx, 0, &gfxQueue);
    vkGetDeviceQueue(_vkdata.dvc, qfam.pre, 0, &preQueue);

    return VK_SUCCESS;
}

void Frame::dstr() {

    vkQueueWaitIdle(gfxQueue); 
    _data.cmdBuffPool->freeCmdBuff(cmdBuff);

    semImgAvailable.dstr();
    semRdrFinished.dstr();
    fenQueueSubmitComplete.dstr();
}

void Frame::processSwpchainRec() {
    NWin::Vec2 size;
    _data.win->ptr->getDrawAreaSize(size);
    _data.win->drawArea.x = size.x;
    _data.win->drawArea.y = size.y;
    while (size.x == 0 || size.y == 0) {
        _data.win->ptr->update();
        _data.win->ptr->getDrawAreaSize(size);
        _data.win->drawArea.x = size.x;
        _data.win->drawArea.y = size.y;
    }
    _data.swpchain->dstr(); 
    ivec2 s = ivec2((ui32)size.x, (ui32)size.y);
    _data.swpchain->create(_vkdata, *_data.win);
    for (Renderpass& rdrpass : _data.rdrpassCnt->_passes) {
        rdrpass.resize(*_data.swpchain);
    }
    _data.win->ptr->_getKeyboard().update();
    _data.win->ptr->update();
}

bool Frame::begin() {
        _rdrpassIndex = 0;
        VkResult res;
        Frame& frame = *this;
        VulkanData& data = _vkdata;
        fenQueueSubmitComplete.wait();
        res = vkAcquireNextImageKHR(data.dvc, _data.swpchain->handle, UINT64_MAX, frame.semImgAvailable.handle, VK_NULL_HANDLE, &swpIndex);
        //Swapchain recreation
        if (res == VK_ERROR_OUT_OF_DATE_KHR) { 
            processSwpchainRec();
            return 0;
        }
        frame.fenQueueSubmitComplete.reset();
        vkResetCommandBuffer(cmdBuff.handle, 0);
        vkBeginCommandBuffer(cmdBuff.handle, &beginInfo);
        return 1;
}

void Frame::end() {
        submitInfo.waitSemaphoreCount   = _rdrpassIndex == 0;
        submitInfo.signalSemaphoreCount = 1;

        VkResult res;
        vkEndCommandBuffer(cmdBuff.handle);
        vkQueueSubmit(cmdBuff.queue, 1, &submitInfo ,fenQueueSubmitComplete.handle);
        res = vkQueuePresentKHR(preQueue, &preInfo);
        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || _data.win->consumesignal()) {
            processSwpchainRec();
        }
        _data.win->ptr->_getKeyboard().update();
        _data.win->ptr->update();
        
}

void Frame::nextRdrpass() {
    submitInfo.waitSemaphoreCount   = _rdrpassIndex == 0;
    submitInfo.signalSemaphoreCount = 0;
    

    vkEndCommandBuffer(cmdBuff.handle);
    vkQueueSubmit(cmdBuff.queue, 1, &submitInfo, fenQueueSubmitComplete.handle);
    fenQueueSubmitComplete.wait();
    fenQueueSubmitComplete.reset();
    vkResetCommandBuffer(cmdBuff.handle, 0);
    vkBeginCommandBuffer(cmdBuff.handle, &beginInfo);
    
    ++_rdrpassIndex;
}

#include"vkimg.h"
#include "params.h"
#include "support.h"

 VkImageViewCreateInfo& ImgView::fillCrtInfo(const Img& im) {
    crtInfo.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    crtInfo.format       = im.crtInfo.format;
    crtInfo.image        = im.handle;
    crtInfo.viewType     = VK_IMAGE_VIEW_TYPE_2D;
    crtInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    crtInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    crtInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    crtInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY; 
    crtInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    crtInfo.subresourceRange.baseMipLevel   = 0;
    crtInfo.subresourceRange.baseArrayLayer = 0;
    crtInfo.subresourceRange.layerCount     = 1;
    crtInfo.subresourceRange.levelCount     = 1; 

    return crtInfo;
}

VkResult ImgView::create(const VulkanData& vkdata) {
    _vkdata = vkdata;
    return vkCreateImageView(vkdata.dvc, &crtInfo, nullptr, &handle);
}

void ImgView::dstr() {
    if (handle == nullptr) return;
    vkDestroyImageView(_vkdata.dvc, handle, nullptr);
}
    
VkImageCreateInfo& Img::fillCrtInfo() {
    crtInfo.sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    crtInfo.imageType = VK_IMAGE_TYPE_2D;
    crtInfo.format    = VK_FORMAT_R8G8B8A8_SRGB;
    crtInfo.tiling    = VK_IMAGE_TILING_OPTIMAL;
    crtInfo.extent.width  = 0;
    crtInfo.extent.height = 0;
    crtInfo.extent.depth = 1;
    crtInfo.mipLevels    = 1;
    crtInfo.arrayLayers  = 1;
    crtInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    crtInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    crtInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    crtInfo.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    return crtInfo;
};
    
void Img::setMaxmmplvl() {
    crtInfo.mipLevels    = (int)(Max<float>(std::log2(crtInfo.extent.width), std::log2(crtInfo.extent.height))) + 1;
}

VkResult Img::create(const VulkanData& vkdata) {
    VkMemoryRequirements memReq;
    VkMemoryAllocateInfo allocInfo{};
    _vkdata = vkdata;

    VkResult res = vkCreateImage(vkdata.dvc, &crtInfo, nullptr, &handle);
    
    vkGetImageMemoryRequirements(_vkdata.dvc, handle, &memReq);

    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = VulkanSupport::findMem(_vkdata, memReq.memoryTypeBits, memProp);

    vkAllocateMemory(vkdata.dvc, &allocInfo, nullptr, &_mem);
    vkBindImageMemory(vkdata.dvc, handle, _mem, 0);
    return res;
}

void Img::dstr() {
    if (handle == nullptr) return;
    vkDestroyImage(_vkdata.dvc, handle, nullptr);
    vkFreeMemory(_vkdata.dvc, _mem, nullptr);
}

VkFramebufferCreateInfo& Framebuffer::fillCrtInfo() {
    crtInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO; 
    crtInfo.layers = 1;
    return crtInfo;
}

VkResult Framebuffer::create(const VulkanData& vkdata, VkRenderPass rdrpass, VkImageView* attchments, ui32 attLen) {
   _vkdata = vkdata;
   crtInfo.renderPass      = rdrpass;
   crtInfo.attachmentCount = attLen;
   crtInfo.pAttachments    = attchments;
   return vkCreateFramebuffer(_vkdata.dvc, &crtInfo, nullptr, &handle);
}

void Framebuffer::dstr() {
    vkDestroyFramebuffer(_vkdata.dvc, handle, nullptr);
}

struct LytData {
    VkImageLayout* lyt; 
    VkPipelineStageFlags* stage;
    VkAccessFlags*        access;
};


bool defaultMemNexecDep(VkImageLayout oldlyt, VkImageLayout newlyt, 
                        VkPipelineStageFlags& srcStage, VkPipelineStageFlags& dstStage, 
                        VkAccessFlags& srcAccessMask, VkAccessFlags& dstAccessMask) {
    uchar i = 0;
    VkImageLayout ptrLyt;
    std::vector<LytData> vec = { {&oldlyt, &srcStage, &srcAccessMask}, {&newlyt, &dstStage, &dstAccessMask} };
    
    for (LytData& d : vec) {
         if (*d.lyt == VK_IMAGE_LAYOUT_UNDEFINED) {
               *d.stage   = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
               *d.access  = 0;
         }
         if (*d.lyt == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                *d.stage       = VK_PIPELINE_STAGE_TRANSFER_BIT;
                *d.access      = VK_ACCESS_TRANSFER_WRITE_BIT;
         }
         if (*d.lyt == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
                *d.stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                *d.access = VK_ACCESS_TRANSFER_READ_BIT;
         }
         if (*d.lyt == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                *d.stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                *d.access = VK_ACCESS_SHADER_READ_BIT;
         }
         if (*d.lyt == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                *d.stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT   | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                *d.access = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
         }
         //Added 
         if (*d.lyt == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
             *d.stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
             *d.access = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
         }
    }

    return 1;
}

VkResult Img::changeLyt(VkImageLayout newlyt, CmdBufferPool& p, lytproc proc ) {
    VkImageLayout oldlyt = crtInfo.initialLayout;
    if (oldlyt == newlyt) 
        return VK_SUCCESS;

    VkImageMemoryBarrier barrier{};
    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;


    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = crtInfo.initialLayout;
    barrier.newLayout = newlyt; 
    barrier.image     = handle;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = crtInfo.mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    barrier.subresourceRange.aspectMask = newlyt == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ? 
                                        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : 
                                        VK_IMAGE_ASPECT_COLOR_BIT;
    
    if (proc == nullptr)
        defaultMemNexecDep(oldlyt, newlyt, srcStage, dstStage, barrier.srcAccessMask, barrier.dstAccessMask);    
    else 
        proc(oldlyt, newlyt, srcStage, dstStage, barrier.srcAccessMask, barrier.dstAccessMask);


    CmdBuff cmdBuff;
    crtInfo.initialLayout = newlyt;
    p.execBegin(&cmdBuff, offsetof(VulkanSupport::QueueFamIndices, gfx));
    vkCmdPipelineBarrier(cmdBuff.handle, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    p.execEnd(cmdBuff);
    return VK_SUCCESS;
}

void Img::cpyFrom(CmdBufferPool& p, Img& other, const ivec2& size, const ivec2& offset) {
   CmdBuff cmdBuff;
   VkImageCopy rgn{};

   rgn.extent.depth = 1;
   rgn.extent.width = size.x;
   rgn.extent.height = size.y;

   rgn.srcSubresource.mipLevel = 0; 
   rgn.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
   rgn.srcSubresource.layerCount = 1;
   
   rgn.dstSubresource.mipLevel   = 0; 
   rgn.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
   rgn.dstSubresource.layerCount = 1;

   p.execBegin(&cmdBuff, offsetof(VulkanSupport::QueueFamIndices, gfx));
   vkCmdCopyImage(cmdBuff.handle, 
                  other.handle, other.crtInfo.initialLayout, handle, crtInfo.initialLayout, 1, &rgn);
   p.execEnd(cmdBuff);
}

void Img::cpyFrom(CmdBufferPool& p, Buffer& buff, const ivec2& size, ui32 offset) { 
    CmdBuff cmdbuff;
    VkBufferImageCopy rgn{};
    rgn.imageExtent.width  = size.x;
    rgn.imageExtent.height = size.y;
    rgn.imageExtent.depth  = 1;
    rgn.bufferOffset       = offset;

    rgn.imageSubresource.mipLevel = 0;
    rgn.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    rgn.imageSubresource.layerCount = 1;
    rgn.imageSubresource.baseArrayLayer = 0;
    //Change layout first    
    changeLyt(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, p);
    //Cpy
    p.execBegin(&cmdbuff, offsetof(VulkanSupport::QueueFamIndices, gfx));
    vkCmdCopyBufferToImage(cmdbuff.handle,
                           buff.handle, 
                           handle, crtInfo.initialLayout,
                           1, &rgn);
    p.execEnd(cmdbuff);
}

void Img::genmmp(CmdBufferPool& p, ui32 queueIndex) {
    i32 qoff = offsetof(VulkanSupport::QueueFamIndices, gfx);
    VkImageMemoryBarrier br{};
    VkQueue q = VulkanSupport::getQueue(_vkdata, qoff);
    CmdBuff cmdBuff;
    ivec2 mipSize = {(i32)crtInfo.extent.width, (i32)crtInfo.extent.height};

    br.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    br.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    br.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    br.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    br.subresourceRange.baseArrayLayer = 0;
    br.subresourceRange.layerCount     = 1;
    br.subresourceRange.levelCount     = 1;
    br.image = handle;

    p.execBegin(&cmdBuff, qoff);

    for (int i = 1; i < crtInfo.mipLevels; ++i) {
        br.subresourceRange.baseMipLevel = i - 1; 
        br.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        br.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        br.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        br.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(cmdBuff.handle, VK_PIPELINE_STAGE_TRANSFER_BIT , VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, 0, 0, 0, 1, &br);

        VkImageBlit rgn{};

        rgn.srcOffsets[0] = {0,0,0};
        rgn.srcOffsets[1] = {mipSize.x, mipSize.y, 1};
        
        rgn.srcSubresource.mipLevel = i - 1;
        rgn.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        rgn.srcSubresource.layerCount = 1;

        mipSize.x = mipSize.x > 1 ? mipSize.x / 2 : mipSize.x;
        mipSize.y = mipSize.y > 1 ? mipSize.y / 2 : mipSize.y;

        rgn.dstOffsets[0] = {0,0,0};
        rgn.dstOffsets[1] = {mipSize.x, mipSize.y, 1};

        rgn.dstSubresource.mipLevel = i;
        rgn.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        rgn.dstSubresource.layerCount = 1;
        
        vkCmdBlitImage(cmdBuff.handle, handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, handle , VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &rgn, VkFilter::VK_FILTER_LINEAR);

        br.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        br.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        br.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        br.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cmdBuff.handle, VK_PIPELINE_STAGE_TRANSFER_BIT , VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, 0, 0, 0, 1, &br);             
    }
    
    br.subresourceRange.baseMipLevel = crtInfo.mipLevels - 1;
    br.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    br.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    br.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    br.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmdBuff.handle, VK_PIPELINE_STAGE_TRANSFER_BIT , VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, 0, 0, 0, 1, &br);
    
    crtInfo.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    p.execEnd(cmdBuff);
    
}

VkResult Sampler::create(const VulkanData& vkdata) {
    _vkdata = vkdata;
    return vkCreateSampler(_vkdata.dvc, &crtInfo, nullptr, &handle);
}
    
VkSamplerCreateInfo& Sampler::fillCrtInfo(const VulkanData& vkdata) {
    crtInfo.sType     = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    crtInfo.minFilter = VK_FILTER_LINEAR;
    crtInfo.magFilter = VK_FILTER_LINEAR;
    crtInfo.minLod    = 0.0f;
    crtInfo.maxLod    = VK_LOD_CLAMP_NONE;
    crtInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    crtInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    crtInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    crtInfo.anisotropyEnable = GfxParams::inst.anisotropy != 0;
    crtInfo.maxAnisotropy    = Min<float>(VulkanSupport::getMaxAniso(vkdata), GfxParams::inst.anisotropy);
    crtInfo.mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    crtInfo.borderColor      = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    return crtInfo;
}


void    Sampler::dstr() {
    vkDestroySampler(_vkdata.dvc, handle, nullptr);
}


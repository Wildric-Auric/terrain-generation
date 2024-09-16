#pragma once

#include "globals.h"
#include "vkapp.h"

#include <vulkan/vulkan.h>

namespace VulkanSupport {
    struct QueueFamIndices {
        i32 gfx = -1;   //graphics
        i32 pre = -1;   //present 
        i32 com = -1;   //compute
        i32 trs = -1;   //transfer
    };

    struct SwpchainCap {
        VkSurfaceCapabilitiesKHR cap;
        std::vector<VkSurfaceFormatKHR> srfcFormats;
        std::vector<VkPresentModeKHR>   prsntModes;
        bool valid  = 0;
    };

    bool isDepthStencil(VkFormat);
    //Returns a boolean indicating if extension, passed as parameter, is available
    bool extSupport(const char* ext);
    //Returns the index of the last array extension that is not supported,
    //the array is indexed starting by 1.
    //If return value is 0 then all extensions are supported
    ui32 extsSupport(const char** array, ui32 arrCount);
    bool layerSupport(const char* layer);
    ui32 layersSupport(const char** array, ui32 arrCount);

    VkPhysicalDevice selPhyDvc(VkPhysicalDevice* arr, ui32 count);

    void findQueues(QueueFamIndices&, const VulkanData&);

    VkQueue getQueue(const VulkanData& _vkdata, i32 off);
   
    //Swapchain related
    void getSwapchaincap(VulkanData&, SwpchainCap&);
    i32  selSrfcFmt(const SwpchainCap&);

    VkPresentModeKHR selPresent();

    ui32 findMem(VulkanData&,ui32 memTypeBits, VkMemoryPropertyFlags memProp);

    float getMaxAniso(const VulkanData& vkdata);

}

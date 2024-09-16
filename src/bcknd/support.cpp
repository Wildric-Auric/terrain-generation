#include "support.h"
#include "vkapp.h"
#include "vulkan/vulkan_core.h"

namespace VulkanSupport {
    
    //TODO::Complete and add other functions for fmt check
    bool isDepthStencil(VkFormat fmt) {
        return fmt == VK_FORMAT_D24_UNORM_S8_UINT || fmt == VK_FORMAT_D32_SFLOAT_S8_UINT || fmt == VK_FORMAT_D16_UNORM_S8_UINT;
    }

    bool extSupport(const char* ext) {
        ui32 count;
        VkExtensionProperties* prop;
        
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
        prop = new VkExtensionProperties[count];
        vkEnumerateInstanceExtensionProperties(nullptr, &count, prop);
        for (ui32 i = 0; i < count; ++i) {
            if (!strcmp(ext, prop[i].extensionName)) {
                delete[] prop;
                return 1;
            }
        }
        delete[] prop;
        return 0;
    }

    ui32 extsSupport(const char** array, ui32 arrCount) {
        ui32 count;
        VkExtensionProperties* prop;
        ui32 ret  = 0;
        ui32 temp = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
        prop = new VkExtensionProperties[count];
        vkEnumerateInstanceExtensionProperties(nullptr, &count, prop);

        for (ui32 i = 0; i < arrCount; ++i) {
            temp = i + 1;
            for (ui32 j = 0; j < count; ++j) {
                if ( !strcmp(array[i], prop[j].extensionName) ) {
                    temp = 0;
                }
            }
            ret = temp != 0 ? temp : ret;
        }
        delete[] prop;
        return ret;
    }
    
    bool layerSupport(const char* layer) {
        ui32 count;
        VkLayerProperties* prop;
        
        vkEnumerateInstanceLayerProperties(&count, nullptr);
        prop = new VkLayerProperties[count];
        vkEnumerateInstanceLayerProperties(&count, prop);
        for (ui32 i = 0; i < count; ++i) {
            if (!strcmp(layer, prop[i].layerName)) {
                delete[] prop;
                return 1;
            }
        }
        delete[] prop;
        return 0;
    }

    ui32 layersSupport(const char** array, ui32 arrCount) { 
        ui32 count;
        VkLayerProperties* prop;
        ui32 ret  = 0;
        ui32 temp = 0; 
        vkEnumerateInstanceLayerProperties(&count, nullptr);
        prop = new VkLayerProperties[count];
        vkEnumerateInstanceLayerProperties(&count, prop);
        for (ui32 i = 0; i < arrCount; ++i) {
            temp = i + 1;
            for (ui32 j = 0; j < count; ++j) {
                if (!strcmp(array[i], prop[j].layerName)) {
                    temp = 0; 
                }
            }
            ret = temp != 0 ? temp : ret;
        }
        delete[] prop;
        return 0;
    }


    VkPhysicalDevice selPhyDvc(VkPhysicalDevice* arr, ui32 count) {
        return arr[0];
    }
    
    void findQueues(QueueFamIndices& ind, const VulkanData& vkdata) {
        ui32 count; 
        VkBool32                sup;
        VkQueueFamilyProperties prop;
        vkGetPhysicalDeviceQueueFamilyProperties(vkdata.phyDvc, &count, nullptr);
        VkQueueFamilyProperties* famArr = new VkQueueFamilyProperties[count];
        vkGetPhysicalDeviceQueueFamilyProperties(vkdata.phyDvc, &count, famArr);

        for (ui32 i = 0; i < count; ++i) {
            const VkQueueFamilyProperties& q = famArr[i];
            if (q.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
                ind.gfx = i;
            if (q.queueFlags & VK_QUEUE_TRANSFER_BIT)
                ind.trs = i;
            if (q.queueFlags & VK_QUEUE_COMPUTE_BIT) 
                ind.com = i;
            vkGetPhysicalDeviceSurfaceSupportKHR(vkdata.phyDvc, i, vkdata.srfc, &sup);
            if (sup) {
                ind.pre = i;
            }
        }
        delete[] famArr;
    }

    VkQueue getQueue(const VulkanData& _vkdata, i32 off) {
        VkQueue q;
        VulkanSupport::QueueFamIndices qfam; 
        VulkanSupport::findQueues(qfam, _vkdata);
        i32 index     = *(i32*)((arch)(&qfam) + off);

        vkGetDeviceQueue(_vkdata.dvc, index, 0, &q);
        return q;
    }
    
    void getSwapchaincap(VulkanData& vkdata, SwpchainCap& capRef) {
        ui32 count;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkdata.phyDvc, vkdata.srfc, &capRef.cap);
        vkGetPhysicalDeviceSurfaceFormatsKHR(vkdata.phyDvc, vkdata.srfc, &count, nullptr);
        capRef.srfcFormats.resize(count);   
        vkGetPhysicalDeviceSurfaceFormatsKHR(vkdata.phyDvc, vkdata.srfc, &count, capRef.srfcFormats.data());
        
        vkGetPhysicalDeviceSurfacePresentModesKHR(vkdata.phyDvc, vkdata.srfc, &count, nullptr);
        capRef.prsntModes.resize(count);   
        vkGetPhysicalDeviceSurfacePresentModesKHR(vkdata.phyDvc, vkdata.srfc, &count, capRef.prsntModes.data());

        capRef.valid = capRef.prsntModes.size() && capRef.srfcFormats.size();
    }

    i32 selSrfcFmt(const SwpchainCap& specRef) {
        for (arch i = 0; i < specRef.srfcFormats.size(); ++i) {
           if (specRef.srfcFormats[i].format == VK_FORMAT_R8G8B8A8_SRGB 
               && specRef.srfcFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
               return i;
        }
        return 0;
    }

    
    VkPresentModeKHR selPresent() {
       return VK_PRESENT_MODE_FIFO_KHR; 
    }
    
    ui32 findMem(VulkanData& vkdata,ui32 typeFilter, VkMemoryPropertyFlags prop) {
        VkPhysicalDeviceMemoryProperties memProp;
        vkGetPhysicalDeviceMemoryProperties(vkdata.phyDvc, &memProp);
        for (ui32 i = 0; i < memProp.memoryTypeCount; ++i) {
            if ((typeFilter & (1 << i)) && (memProp.memoryTypes[i].propertyFlags & prop) == prop) {
                return i;
            }
        }
        return -1;
    }
    
    float getMaxAniso(const VulkanData& vkdata) {
        VkPhysicalDeviceProperties prop;
        vkGetPhysicalDeviceProperties( vkdata.phyDvc,  &prop);
        return prop.limits.maxSamplerAnisotropy;
    }

}

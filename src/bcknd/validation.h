#pragma once
#include "vulkan/vulkan_core.h"
#include <vulkan/vulkan.h>
#include "vkdecl.h" 

class DebugMessenger {
public:
    DebugMessenger() = default;
    VkDebugUtilsMessengerCreateInfoEXT& fillCrtInfo();
    VkResult    create(VulkanData& vkdata);
    void        dstr();

    VkDebugUtilsMessengerEXT _data          = nullptr; 
    VkDebugUtilsMessengerCreateInfoEXT _crtInfo{};
    VulkanData _vkdata;

 
    static VKAPI_ATTR VkBool32 VKAPI_CALL _defaultCallback(
	        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	        VkDebugUtilsMessageTypeFlagsEXT messageType,
	        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,void* usrDataCallback);

    VKAPI_ATTR VkBool32 (*_pCallback)
        (VkDebugUtilsMessageSeverityFlagBitsEXT,
                                      VkDebugUtilsMessageTypeFlagsEXT,
                                      const VkDebugUtilsMessengerCallbackDataEXT*,
                                      void*) = &DebugMessenger::_defaultCallback;

};

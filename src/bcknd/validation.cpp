#include "validation.h"
#include "vulkan/vulkan_core.h"

#include <iostream>

VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger::_defaultCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	if (messageSeverity != VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		std::cerr << "Validation layer " << messageSeverity << ": " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;

}

VkDebugUtilsMessengerCreateInfoEXT& DebugMessenger::fillCrtInfo() {
	_crtInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	_crtInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	_crtInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	_crtInfo.pfnUserCallback = _pCallback;
    return _crtInfo;
}

VkResult DebugMessenger::create(VulkanData& vkdata) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkdata.inst, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
        _vkdata = vkdata;
		return func(vkdata.inst, &_crtInfo, nullptr, &_data);
	}
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DebugMessenger::dstr() {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_vkdata.inst, "vkDestroyDebugUtilsMessengerEXT");
	if (func == nullptr) return;
	func(_vkdata.inst,_data, nullptr);
}



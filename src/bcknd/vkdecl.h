#pragma once
#include "vulkan/vulkan.h"
#include "nwin/window.h"
#include "globals.h"


#define VK_STR_VALIDATION_LAYER "VK_LAYER_KHRONOS_validation"

typedef NWin::Window* pWin;

struct VulkanData {
    VkPhysicalDevice phyDvc;
    VkDevice         dvc;
    VkInstance       inst;
    VkSurfaceKHR     srfc;
};

//crtInfo can be modified befored calling VkApp::init() so that it window be customized 
class Window {
    public:
    NWin::WindowCrtInfo crtInfo; //creation info
    pWin ptr        = nullptr;
    bool _rszsignal = false; 
    ivec2 drawArea  = {500, 500};
    bool getRszSignal();
    bool consumesignal();

    static Window* cur;
    static void   rszcallback(NWin::winHandle handle, NWin::Vec2 size);
};

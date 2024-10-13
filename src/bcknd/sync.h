#include "vkdecl.h"

class Semaphore {
    public:
    VkSemaphore handle = nullptr;
    VkSemaphoreCreateInfo _crtInfo;
    VkSemaphoreTypeCreateInfo _typeCrtInfo;
    VulkanData            _vkdata;

    VkSemaphoreCreateInfo& fillCrtInfo(const VulkanData&, bool isBinary = 1);
    VkResult create();
    void     dstr();
    bool     isBin();
    //----Only for timeline sem----
    VkResult signal(ui64 val);
    VkResult wait(ui64 val);
};

//Usage of timeline semaphore over fences is preferable
class Fence {
    public:
    VkFence handle = nullptr;
    VulkanData _vkdata;
    VkFenceCreateInfo _crtInfo;

    VkFenceCreateInfo& fillCrtInfo(const VulkanData& data, bool isSignaled = 0);
    VkResult create();
    VkResult wait();
    void     dstr();
};

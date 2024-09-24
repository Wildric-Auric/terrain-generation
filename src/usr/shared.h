#pragma once
#include "bcknd/vkapp.h"
#include "bcknd/cmd_buffer.h"

class GlobalData {
    public:
    static Vkapp* app;
    static CmdBufferPool* cmdBuffPool;
    static CmdBuff*       cmdBuff;
};

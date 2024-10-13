#pragma once

#include "basic.h"
#include "../gfx.h"

class MeshRenderer : public Component {
    public:
    INHERIT_COMPONENT(MeshRenderer)

    GfxObject _gobj;

    void init();
    void update();

};

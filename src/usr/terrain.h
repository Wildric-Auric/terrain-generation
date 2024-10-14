#pragma once
#include "shapes.h"
#include "obj/obj.h"
#include "components/basic.h"
#include "components/mesh_renderer.h"
#include "cam.h"

#include <deque>

typedef struct {
    MVPData unfData;
    UniBuff unf;
    void*   terrainPtr;
} TerrainCbkData;

class Terrain : public Component {
public:
    INHERIT_COMPONENT(Terrain)

    std::deque<SubdivQuad>   _chuncks;
    std::deque<Obj> _objs;
    std::deque<TerrainCbkData> _dat;

    fvec2 _chunckSize = fvec2(10.0, 10.0);
    ivec2 _chunckNum  = ivec2(10, 10);
    //temporary, needs to integrate correctly
    GfxContext* gtx   = nullptr;
    Cam* cam;


    void init()   override;
    void update() override; 
};

#pragma once
#include "shapes.h"
#include "obj/obj.h"
#include "components/basic.h"
#include "components/mesh_renderer.h"
#include "cam.h"

#include <deque>
#include <unordered_map>


typedef struct {
    MVPData unfData;
    float   time;
} TerrainUniformData ;

typedef struct {
    TerrainUniformData unfData;
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
    GfxContext* gtx   = nullptr;
    Cam* cam;

    void addChunck(const fvec3 pos, const fvec2& uvoffset);
    void deleteChunck(ui32);

    void init()   override;
    void update() override; 
    void clean()  override;
};

//Dynamic Terrain

struct DynChunckCbk {
    MVPData camData;
    UniBuff unf;
    void*   terrainPtr; 
};

struct DynChunck {
    SubdivQuad   quad;
    Obj          obj;
    DynChunckCbk cbkData;
    
};

class DynamicTerrain : public Component {
public:
    INHERIT_COMPONENT(DynamicTerrain);
    /*Level of details as key, a container */
    std::unordered_map<ui8, std::list<DynChunck>> pools;
    /*Chunck coordinate as key, a pointer to the chunck as value */
    std::unordered_map<ui16, DynChunck*> chuncks;
    
    Cam* cam;

    void init()   override;
    void update() override; 
    void clean()  override;
};

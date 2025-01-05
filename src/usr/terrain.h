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
    SubdivQuad quad;
    Obj          obj;
    DynChunckCbk cbkData;
    bool isFree = true;     /*Always true for now*/
};

struct ChunckMapObj {
    std::vector<DynChunck> pool;
    arch cursor = 0;
};

class DynamicTerrain : public Component {
public:
    INHERIT_COMPONENT(DynamicTerrain);
    /*A map having LOD as key  pool containing all quads*/
    std::unordered_map<ui32, ChunckMapObj> _pools;
    /*Rendered  quads*/
    std::unordered_map<DynChunck*, DynChunck*> _usedChunks;
    /*Parameters for chunck usage strategy*/
    /*Original quad size, cented on 0, maybe add offset later */
    fvec2 size = fvec2(10.0f, 10.0f);
    const int MAX_ELE = 200; //TODO::Make this dynamic, should depend on rad
    int rad = 5;
    int maxLod = 8;
    
    std::vector<ui32>  usedIndices; 
    Transform*  observer = nullptr;
    GfxContext* gtx      = nullptr;
    Cam*        cam      = nullptr;

    DynChunck* getFreeChunck();
    DynChunck* addChunk();
    DynChunck* subdivChunk();

    int  getLod(int i, int j, float y);
    void addChunck();
    void highDetailsPart(const fvec3&);
    void infinitePart(const fvec3&);

    void init()   override;
    void update() override; 
    void clean()  override;
};

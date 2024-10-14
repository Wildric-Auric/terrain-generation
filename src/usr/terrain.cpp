#include "terrain.h"
#include "shared.h"

void terrainObjUpdate(Obj* obj) {
    MeshRenderer& mr  = *obj->get<MeshRenderer>();    
    Transform*    tr  = obj->get<Transform>();
    auto& vkdat = mr._gobj._gtx->rdrpass->_vkdata;
    TerrainCbkData& dat = *(TerrainCbkData*)obj->updateCbkData;
    Terrain*        ter = (Terrain*)dat.terrainPtr;  
    VkWriteDescriptorSet   wrt0{};
    VkDescriptorBufferInfo buffInf{};
    buffInf.offset = 0;
    buffInf.range  = dat.unf._buff._size;
    buffInf.buffer = dat.unf._buff.handle;

    wrt0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wrt0.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    wrt0.descriptorCount = 1; 
    wrt0.pBufferInfo     = &buffInf;

    dat.unfData.model = tr->_model;
    dat.unfData.proj  = ter->cam->_proj;
    dat.unfData.view  = ter->cam->_view;

    dat.unf.wrt(&dat.unfData);
    mr._gobj._descSet.wrt(&wrt0, 0);  
    mr._gobj.update(*GlobalData::cmdBuff);
    mr._gobj.draw();
}

void Terrain::init() {
    fvec3 offset;
    fvec2 uvoffset;
    for (int i = 0; i < _chunckNum.x; ++i) {
        offset.z   = 0.0f; 
        uvoffset.y = 0.0f; 
        for (int j = 0; j < _chunckNum.y; ++j) {
           _chuncks.push_back({});
           _objs.push_back({});
           _dat.push_back({});
           TerrainCbkData& cbk = _dat.back();
           cbk.unf.create(gtx->pipeline._vkdata,sizeof(MVPData));
           cbk.terrainPtr = this;

           SubdivQuad&   q = _chuncks.back();

           Obj& obj = _objs.back();
           MeshRenderer* mr = obj.add<MeshRenderer>();
           Transform*    tr = obj.add<Transform>();

           int LOD = 8;
           if (j % 4 == 0 && i % 3 == 0) {
               LOD = 8;
           }

           q.init(LOD, _chunckSize.x * 0.5, 1, uvoffset);
           mr->init();
           mr->_gobj.init(gtx, &q._data);
           //tr->size = fvec3(_chunckSize.x, _chunckSize.y, 1.0);
           tr->pos  = offset;
           tr->rot.x  = 90.0f;
           tr->setModel();
           obj.setUpdateCkb(terrainObjUpdate);
           obj.updateCbkData = &cbk; 

           offset.z   += _chunckSize.y ;
           uvoffset.y += 1.0;
        }
        offset.x += _chunckSize.x;
        uvoffset.x += 1.0f;
    }
}

void Terrain::update() {    
    for (Obj& o : _objs) {
        o.update();
    }
}

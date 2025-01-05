#include "terrain.h"
#include "shared.h"

extern Sampler smpler;
extern Img     im;
extern ImgView imgView;

float ti = 0.0f;

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
    
    VkWriteDescriptorSet   wrt1{};
    VkDescriptorImageInfo  imgInf;

    imgInf.sampler     = smpler.handle;
    imgInf.imageLayout = im.crtInfo.initialLayout;
    imgInf.imageView   = imgView.handle;
    wrt1.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wrt1.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    wrt1.descriptorCount = 1;
    wrt1.pImageInfo      = &imgInf;
    mr._gobj._descSet.wrt(&wrt1, 1);

//    fmat4 trans(1.0);
//    if (tr->pos.z == (ter->_chunckNum.x - 1)* ter->_chunckSize.x) {
//        TranslateMat(trans, fvec3(0.0,0.0,0.0));
//    }

    dat.unfData.unfData.model = tr->_model;
    dat.unfData.unfData.proj  = ter->cam->_proj;
    dat.unfData.unfData.view  = ter->cam->_view;
    dat.unfData.time  = ti;

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
           addChunck(offset, uvoffset);
           offset.z   += _chunckSize.y ;
           uvoffset.y += 1.0;
        }
        offset.x += _chunckSize.x;
        uvoffset.x += 1.0f;
    }
}

void Terrain::addChunck(const fvec3 pos, const fvec2& uvoffset) {
           _chuncks.push_back({});
           _objs.push_back({});
           _dat.push_back({});
           TerrainCbkData& cbk = _dat.back();
           cbk.unf.create(gtx->pipeline._vkdata,sizeof(TerrainCbkData));
           cbk.terrainPtr = this;

           SubdivQuad&   q = _chuncks.back();

           Obj& obj = _objs.back();
           MeshRenderer* mr = obj.add<MeshRenderer>();
           Transform*    tr = obj.add<Transform>();

           //int LOD = 7 - 6 * j / _chunckNum.x ;
           
           int LOD = 4;
           if ((int)uvoffset.y >= _chunckNum.x - 1) 
                LOD = 8;
           else if ((int)uvoffset.y >= _chunckNum.x - 3 ) 
                LOD = 7;
           q.init(LOD, _chunckSize.x * 0.5, 1, uvoffset);
           mr->init();
           mr->_gobj.init(gtx, &q._data);
           //tr->size = fvec3(_chunckSize.x, _chunckSize.y, 1.0);
           tr->pos    = pos;
           tr->rot.x  = 90.0f;
           tr->setModel();
           obj.setUpdateCkb(terrainObjUpdate, &cbk);
}

void Terrain::deleteChunck(ui32 i) {
        _dat[i].unf.dstr();
        _chuncks[i].dstr();
        _objs[i].get<MeshRenderer>()->_gobj.dstr();
}

void Terrain::clean() {
    for (int i = 0; i < _objs.size(); ++i )  {
        deleteChunck(i); 
    }
    _dat.clear();
    _chuncks.clear();
    _objs.clear();
}

void Terrain::update() {    
    for (Obj& o : _objs) {
        o.update();
    }
}


//--------------------------


void dynTerUpdateCbk(Obj* obj) {
    VkDescriptorBufferInfo buffInf{};
    VkWriteDescriptorSet   wrt0{};

    MeshRenderer& mr  = *obj->get<MeshRenderer>();    
    Transform*    tr  = obj->get<Transform>();
    VulkanData& vkdat = mr._gobj._gtx->rdrpass->_vkdata;
    DynChunckCbk& dat = *(DynChunckCbk*)obj->updateCbkData;
    DynamicTerrain* ter = (DynamicTerrain*)dat.terrainPtr;  

    dat.camData.proj = ter->cam->_proj;
    dat.camData.view = ter->cam->_view;
    dat.camData.model= tr->setModel();
    
    dat.unf.wrt(& dat.camData);

    buffInf.offset = 0;
    buffInf.range  = dat.unf._buff._size;
    buffInf.buffer = dat.unf._buff.handle;
    wrt0.descriptorCount = 1;
    wrt0.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    wrt0.pBufferInfo = &buffInf;
    
    mr._gobj._descSet.wrt(&wrt0, 0);
    mr._gobj.update(*GlobalData::cmdBuff);
    mr._gobj.draw();
}

void DynamicTerrain::init() {
    VulkanData* vkdata;
    DynChunck* c;
    MeshRenderer* mr;
    Transform*    tr;
 
    vkdata = &gtx->pipeline._vkdata;

    for (ui32 lod = 0; lod <= maxLod; ++lod ) {
    _pools.insert(std::make_pair(lod, ChunckMapObj()));
    _pools[lod].pool.resize(MAX_ELE);
    for (int i = 0; i < MAX_ELE; ++i) {    
        c = &_pools[lod].pool[i];
        c->cbkData.terrainPtr = this;
        c->cbkData.unf.create(*vkdata, sizeof(MVPData));
        c->quad.init(lod);

        mr = c->obj.add<MeshRenderer>();
        tr = c->obj.add<Transform>();
        mr->init();
        mr->_gobj.init(gtx, &c->quad._data);

        c->obj.setUpdateCkb(dynTerUpdateCbk, &c->cbkData);
    }
    }
}

void DynamicTerrain::addChunck() {

}

DynChunck* DynamicTerrain::getFreeChunck() {
    //for (DynChunck& c : _pools.begin()->second.pool) {
    //    if (c.isFree) return &c;
    //}
    return nullptr;
}

int DynamicTerrain::getLod(int i, int j, float y) {
   int lod = 0;
   int m   = maxLod / (abs((int)(y / 30.0 )) + 1);
   float mdist = Max<int>(abs(i), abs(j));
   m = Clamp<int>(m,0,maxLod);
   if (mdist >= m)
       return lod;
    
   lod = m - (mdist<=2 ? 0 : mdist);
   return lod;
}

void DynamicTerrain::update() {
    fvec3 oPos;
    if (observer) 
        oPos = fvec3( (int)(observer->pos.x / size.x) * size.x, 
                      (int)(observer->pos.y), 
                      (int)(observer->pos.z / size.y) * size.y);

    _usedChunks.clear();
    highDetailsPart(oPos);
    infinitePart(oPos);
    int r = rad;
} 

void DynamicTerrain::highDetailsPart(const fvec3& oPos) {
    for (int i = -rad; i <= rad; i++) {
        for (int j = -rad; j <= rad; j++) {
            int  lod   = getLod(i,j,oPos.y);
            arch index = _pools[lod].cursor; 

            _usedChunks.insert(std::make_pair(&_pools[lod].pool[index], &_pools[lod].pool[index]));
            Transform* tr = _pools[lod].pool[index].obj.get<Transform>();
            tr->size = fvec3(size.x,size.y,1.0);
            tr->pos  = fvec3(oPos.x + i * size.x, 0.0, oPos.z + j * size.y);
            tr->rot.x = 90.0f;
            tr->setModel();
            _pools[lod].pool[index].obj.update();
            _pools[lod].cursor = (_pools[lod].cursor + 1) % _pools[lod].pool.size();
        }
    }
}

void DynamicTerrain::infinitePart(const fvec3& origin) {
    int w = (2 * rad + 1);
    fvec2 curSize = size; 
    float rem = 0.0;
    float f   = 1.0;
    float zoff = 0.0;
    while (w != 0) {
        rem  = w / 2.0f - (float)(w/2);  
        w   /= 2; 
        f    = rem / (float)w;
        fvec2 last = curSize;
        curSize = curSize * (2.0f + f); 
        zoff += (curSize.x / 2.0 + last.x / 2.0);
        for (int i = -w/2; i <= w/2; i++) {
            int lod = 0;
            arch index = _pools[lod].cursor; 
            _usedChunks.insert(std::make_pair(&_pools[lod].pool[index], &_pools[lod].pool[index]));
            Transform* tr = _pools[lod].pool[index].obj.get<Transform>();
            tr->size = fvec3(curSize.x, curSize.y,1.0);
            tr->pos  = fvec3(origin.x + i * curSize.x, 0.0, origin.z - rad * size.y - zoff);
            tr->rot.x = 90.0f;
            tr->setModel();
            _pools[lod].pool[index].obj.update();
            _pools[lod].cursor = (_pools[lod].cursor + 1) % _pools[lod].pool.size();
        }
    }
}

void DynamicTerrain::clean() {

}

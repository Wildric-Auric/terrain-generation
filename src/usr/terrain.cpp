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
           obj.setUpdateCkb(terrainObjUpdate);
           obj.updateCbkData = &cbk; 
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


void DynamicTerrain::init() {

}

void DynamicTerrain::update() {
    for (auto& pr : chuncks) {
        pr.second->obj.update();
    }
} 

void DynamicTerrain::clean() {

}

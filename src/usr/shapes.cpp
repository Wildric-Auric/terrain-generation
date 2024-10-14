#include "shapes.h"
#include "shared.h"

struct BaseVertexData {
    fvec3 pos;
    fvec2 uv;
};

void Shape::init(bool isDynamic) {}

void Shape::draw() {
        VkDeviceSize voff = 0;
        vkCmdBindVertexBuffers(GlobalData::cmdBuff->handle, 0, 1, &_vobj.buff.handle, &voff);
        vkCmdBindIndexBuffer(GlobalData::cmdBuff->handle, _vobj.indexBuff.handle, voff, VkIndexType::VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(GlobalData::cmdBuff->handle, _indexCount, 1, 0, 0, 0);
}

void Shape::destroy() {
    _vobj.dstr();
}

void Quad::init(bool isDynamic) { 
    std::vector<BaseVertexData> strides = {
        { {-0.5, -0.5, 0.5},  {0.0, 0.0} },      
        { { 0.5, -0.5, 0.5}, {1.0, 0.0} },    
        { {-0.5,  0.5, 0.5}, {0.0, 1.0} },
        { { 0.5,  0.5, 0.5}, {1.0, 1.0} }
    };
    
    strides[0].pos = strides[0].pos * fvec3(size.x, size.y, 1.0);
    strides[1].pos = strides[1].pos * fvec3(size.x, size.y, 1.0);
    strides[2].pos = strides[2].pos * fvec3(size.x, size.y, 1.0);
    strides[3].pos = strides[3].pos * fvec3(size.x, size.y, 1.0);

    ui32 indices[] = {
        0,1,2,
        3,2,1, 
    };

    _indexCount = sizeof(indices) / sizeof(indices[0]);
    _vobj.create(GlobalData::app->data, *GlobalData::cmdBuffPool, (float*)strides.data(), strides.size() * sizeof(strides[0]), isDynamic); 
    _vobj.createIndexBuff(GlobalData::app->data, *GlobalData::cmdBuffPool, indices, sizeof(indices), isDynamic);
}


void Quad::setInitSize(const fvec2& size) {
     this->size = size;
}


void Cube::init(bool isDynamic) {

     BaseVertexData strides[] = {
        { {-0.5, -0.5, 0.5}, {0.0, 0.0} },      
        { { 0.5, -0.5, 0.5}, {1.0, 0.0} },    
        { {-0.5,  0.5, 0.5}, {0.0, 1.0} },
        { { 0.5,  0.5, 0.5}, {1.0, 1.0} },
        
        { {0.5, -0.5, 0.5},  {0.0, 0.0} },   
        { {0.5, -0.5, -.5}, {1.0, 0.0} },    
        { {0.5,  0.5, 0.5}, {0.0, 1.0} },
        { {0.5,  0.5, -.5}, {1.0, 1.0} },

        { {-0.5, -0.5, -.5},  {0.0, 0.0} },      
        { { 0.5, -0.5, -.5}, {1.0, 0.0} },    
        { {-0.5,  -0.5, 0.5}, {0.0, 1.0} },
       { { 0.5,  -0.5, 0.5}, {1.0, 1.0} },
        
       { {-0.5, -0.5, -.5}, {0.0, 0.0} },    
        { {-0.5, -0.5, 0.5},  {1.0, 0.0} },      
        { {-0.5,  0.5, -.5}, {0.0, 1.0} },
        { {-0.5,  0.5, 0.5},  {1.0, 1.0} },

        { { 0.5, -0.5, -.5}, {1.0, 0.0} },    
        { {-0.5, -0.5, -.5},  {0.0, 0.0} },      
        { { 0.5,  0.5, -.5}, {1.0, 1.0} },
        { {-0.5,  0.5, -.5}, {0.0, 1.0} },
        
    };

    ui32 indices[] = {
        0,1,2,
        3,2,1, 
        
        4+0,4+1,4+2,
        4+3,4+2,4+1, 
        
        8+0,8+1,8+2,
        8+3,8+2,8+1, 

        12+0,12+1,12+2,
        12+3,12+2,12+1, 
        
        16+0,16+1,16+2,
        16+3,16+2,16+1, 

    };

    _indexCount = sizeof(indices) / sizeof(indices[0]);
    _vobj.create(GlobalData::app->data, *GlobalData::cmdBuffPool, (float*)strides, sizeof(strides), 0); 
    _vobj.createIndexBuff(GlobalData::app->data, *GlobalData::cmdBuffPool, indices, sizeof(indices), 0);
}

void Triangle::init(bool isDynamic) {
    BaseVertexData strides[] = {
        { {0.0, -0.5, 0.5},  {0.0, 0.0} },      
        { { 0.5, 0.5, 0.5}, {1.0, 0.0} },    
        { {-0.5, 0.5, 0.5}, {0.0, 1.0} },
    };

    ui32 indices[] = {
        0,1,2,
    };

    _indexCount = sizeof(indices) / sizeof(indices[0]);
    _vobj.create(GlobalData::app->data, *GlobalData::cmdBuffPool, (float*)strides, sizeof(strides), isDynamic); 
    _vobj.createIndexBuff(GlobalData::app->data, *GlobalData::cmdBuffPool, indices, sizeof(indices), isDynamic);     
}

void Mesh::clean() {
    vertices.clear();
    _vobj.dstr();
}


void Mesh::addVert(const std::vector<float>& v) {
    vertices.insert(vertices.end(), v.begin(), v.end());
}

void Mesh::addInd(const ui32 i) {
    indices.push_back(i);
}

void Mesh::add(const std::vector<float>* const v , const ui32 i) {
    if (v) addVert(*v);
    if (i != -1) addInd(i);
}

void Mesh::init(bool isDynamic) {
    _indexCount = indices.size();
    _vobj.create(GlobalData::app->data, *GlobalData::cmdBuffPool, (float*)vertices.data(), vertices.size() * sizeof(vertices[0]), isDynamic); 
    _vobj.createIndexBuff(GlobalData::app->data, *GlobalData::cmdBuffPool, indices.data(), indices.size() * sizeof(indices[0]), isDynamic);
}

void SubdivQuad::init(const ui32 sub, const float res, bool isDynamic, fvec2 uvoffset) {
    _subdiv = sub;
    fvec2 base = {-res, -res};
    fvec2 end  = {res,  res};
    float c = pow(2, sub - 1) + 1;
    fvec2 step = (end - base) * (1.0/c);
    for (ui32 i = 0; i < c+1; ++i) {
        std::vector<float> tmp(5);
           *(fvec3*)(&tmp[0]) = fvec3(base.x + i * step.x,base.y,0);
           *(fvec2*)(&tmp[3]) = fvec2(i * 1.0/c, 0) + uvoffset; 
            _data.addVert(tmp);
    }

    for (ui32 j = 0; j < c; ++j) {
        for (ui32 i = 0; i < c; ++i)  {
            std::vector<float> tmp(5);
            fvec3 l    = fvec3(base.x + i * step.x, base.y + j * step.y);
            //Add next row vertices

            if (i == 0) {
                *(fvec3*)(&tmp[0]) = l + fvec3(0, step.y, 0);
                *(fvec2*)(&tmp[3]) = fvec2(i * 1.0/c, (j+1) * 1.0/c) + uvoffset; 
                _data.addVert(tmp); //up
            }

            *(fvec3*)(&tmp[0]) = l + fvec3(step.x, step.y, 0);
            *(fvec2*)(&tmp[3]) = fvec2((i+1) * 1.0/c, (j+1) * 1.0/c) + uvoffset; 
            _data.addVert(tmp); //up right
            //Add next row indices
            int cc = c+1;
            _data.addInd(j*cc + i);
            _data.addInd(j*cc + i + 1);
            _data.addInd((j+1)*cc + i);
            _data.addInd((j+1)*cc + i);
            _data.addInd(j*cc + i + 1);
            _data.addInd((j+1)*cc + i + 1);
        }
    }
    _data.init(isDynamic);
}

void SubdivQuad::dstr() {
    _data.clean();
}

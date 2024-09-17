#include "shapes.h"
#include "shared.h"


void Shape::init() {}

void Shape::draw() {
        VkDeviceSize voff = 0;
        vkCmdBindVertexBuffers(GlobalData::cmdBuff->handle, 0, 1, &_vobj.buff.handle, &voff);
        vkCmdBindIndexBuffer(GlobalData::cmdBuff->handle, _vobj.indexBuff.handle, voff, VkIndexType::VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(GlobalData::cmdBuff->handle, _indexCount, 1, 0, 0, 0);
}

void Shape::destroy() {
    _vobj.dstr();
}

void Quad::init() {
    VertexData strides[] = {
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
    _vobj.create(GlobalData::app->data, *GlobalData::cmdBuffPool, (float*)strides, sizeof(strides), 0); 
    _vobj.createIndexBuff(GlobalData::app->data, *GlobalData::cmdBuffPool, indices, sizeof(indices), 0);
}


void Quad::setInitSize(const fvec2& size) {
     this->size = size;
}

void Cube::init() {
     VertexData strides[] = {
        { {-0.5, -0.5, 0.5},  {0.0, 0.0} },      
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

void Triangle::init() {
    VertexData strides[] = {
        { {0.0, -0.5, 0.5},  {0.0, 0.0} },      
        { { 0.5, 0.5, 0.5}, {1.0, 0.0} },    
        { {-0.5, 0.5, 0.5}, {0.0, 1.0} },
    };
    ui32 indices[] = {
        0,1,2,
    };

    _indexCount = sizeof(indices) / sizeof(indices[0]);
    _vobj.create(GlobalData::app->data, *GlobalData::cmdBuffPool, (float*)strides, sizeof(strides), 0); 
    _vobj.createIndexBuff(GlobalData::app->data, *GlobalData::cmdBuffPool, indices, sizeof(indices), 0);     
}

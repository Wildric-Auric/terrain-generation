#pragma once

#include "bcknd/vertex.h"

#define INHERIT_SHAPE_INIT \
    void init(bool isDynamic = 0) override;  \

#define INHERIT_SHAPE      \
    void init(bool isDynamic = 0) override;  \
    void draw() override;  

class Shape {
    public:
    VertexObject _vobj; 
    ui32         _indexCount = 0;
    virtual void init(bool isDynamic = 0);
    virtual void draw();
    virtual void destroy();
};

class Quad : public Shape {
    public:
    INHERIT_SHAPE_INIT
    fvec2 size = fvec2(1.0f, 1.0f);
    void setInitSize(const fvec2& size);
};

class Triangle: public Shape {
    public:
    INHERIT_SHAPE_INIT
};

class Cube : public Shape {
    public:
    INHERIT_SHAPE_INIT
};

class Mesh : public Shape {
    public:
    std::vector<float>      vertices;
    std::vector<ui32>       indices;
    void addVert(const std::vector<float>&);
    void addInd(const ui32);
    void add(const std::vector<float>* const, const ui32 i);
    void clean();
    INHERIT_SHAPE_INIT
};

class SubdivQuad {
    public:
    ui32 _subdiv = 5;
    Mesh _data;
    
    void init(const ui32 sub, const float res = 0.5f, bool isDynamic = 0, fvec2 uvoffset = {0.0f, 0.0f});
    void dstr();
};

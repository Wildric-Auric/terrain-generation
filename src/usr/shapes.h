#include "bcknd/vertex.h"

#define INHERIT_SHAPE_INIT      \
    void init() override;  \

#define INHERIT_SHAPE      \
    void init() override;  \
    void draw() override;  

class Shape {
    public:
    VertexObject _vobj; 
    ui32         _indexCount = 0;
    virtual void init();
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

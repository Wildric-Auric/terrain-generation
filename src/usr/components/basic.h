#pragma once
#include "bcknd/globals.h"

class Component {
    public:
    void* _owner = nullptr;
    inline Component(void* owner = nullptr) { _owner = owner;}
    inline ~Component() {};
};

class Transform : public Component {
public:
    fmat4 _model;

    fvec3 pos;
    fvec3 rot;
    fvec3 scale = {1.0f,1.0f,1.0f};
    fvec3 size  = {1.0f,1.0f,1.0f};

    inline Transform(void* owner) : Component(owner) {};

    inline fmat4& setModel() {
        _model = fmat4(1.0);
        ScaleMat(_model, scale * size);
        RotateMat(_model, rot.x, {1.0, 0.0,0.0});
        RotateMat(_model, rot.y, {.0, 1.0,0.0});
        RotateMat(_model, rot.z, {.0, 0.0,1.0});
        TranslateMat(_model, pos);
        return _model;
    };
    
    inline fmat4& getModel() {
        return _model;
    }

};

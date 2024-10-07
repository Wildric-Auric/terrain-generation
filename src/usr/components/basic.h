#pragma once

#include <string>

#include "bcknd/globals.h"


#define INHERIT_COMPONENT(componentToken) \
    inline componentToken(void* owner) : Component(owner) {_name = #componentToken;} \
    inline static const std::string name()                { return #componentToken;}

class Component {
    public:
    void*       _owner = nullptr;
    std::string _name   = ""; 

    
    inline Component(void* owner = nullptr) { _owner = owner;}
    inline const std::string& getType() { return _name;};
    inline ~Component() {};
};

class Transform : public Component {
public:
    INHERIT_COMPONENT(Transform)

    fmat4 _model;

    fvec3 pos;
    fvec3 rot;
    fvec3 scale = {1.0f,1.0f,1.0f};
    fvec3 size  = {1.0f,1.0f,1.0f};

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

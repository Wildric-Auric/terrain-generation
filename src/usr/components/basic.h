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
    fvec3 pos;
    fvec3 rot;
    fvec3 scale = {1.0f,1.0f,1.0f};
    fvec3 size  = {1.0f,1.0f,1.0f};
    inline Transform(void* owner) : Component(owner) {};
};

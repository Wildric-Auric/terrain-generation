#pragma once 
#include <unordered_map>
#include <list>

#include "obj/obj.h" 
#include "gfx.h"

class Scene {    
    public:
    std::unordered_map<GfxContext*, std::list<Obj>> objs;

    Obj& push(GfxContext*);
    void pop(GfxContext*); 

    GfxContext* push();
    void        popCtx(GfxContext*);

    void init();
    void update();
};

#include "scene.h"
#include "shared.h"


Obj& Scene::push(GfxContext* ctx) {
    auto iter = objs.find(ctx);
    iter->second.push_back({});
    return iter->second.back();
}

void Scene::pop(GfxContext* ctx) { 
    auto iter = objs.find(ctx);
    if (iter->second.empty()) return; iter->second.pop_back();
}

GfxContext* Scene::push() {
    GfxContext* ptr = new GfxContext();
    objs.emplace(ptr, std::list<Obj>());
    return ptr;
}

void    Scene::popCtx(GfxContext* ctx) {
    delete ctx;
    objs.erase(ctx);
}

void Scene::init() {
    for (auto iter = objs.begin(); iter != objs.end(); ++iter) {
        for (Obj& o : iter->second) {
            o.init();
        }
    }
}

void Scene::update() {
    for (auto iter = objs.begin(); iter != objs.end(); ++iter) {
        iter->first->bind(*GlobalData::cmdBuff);
        for (Obj& o : iter->second) {
            o.update();
        }
    }
}

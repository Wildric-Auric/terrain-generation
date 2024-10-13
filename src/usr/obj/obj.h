#pragma once

#include <unordered_map>
#include <string>

#include "../components/basic.h"

class Obj {
public: 

    void* initCbkData   = nullptr;
    void* updateCbkData = nullptr;

    void (*initProc)(Obj*)   = nullptr;
    void (*updateProc)(Obj*) = nullptr;
    std::unordered_map<std::string, Component*> comps;

    inline void setInitCbk(void (*proc)(Obj*), void* data = nullptr) {
        initProc = proc;
        initCbkData = data;
    }

    inline void setUpdateCkb(void (*proc)(Obj*), void* data = nullptr) {
        updateProc = proc;
        updateCbkData = data;

    }
    
    inline void init() {
        if (initProc == nullptr) return;
        initProc(this);
    }

    inline void update() {
        if (updateProc == nullptr) return;
        updateProc(this);
    }

    template<typename T>
    T* add() {
        auto iter = comps.find(T::name());
        if ( iter != comps.end()) 
            return (T*)&*iter->second;
        T* ptr = new T(this);
        comps.emplace(T::name(), (Component*)ptr);
        return ptr;
    }

    template<typename T> 
    T* get() {
        auto iter = comps.find(T::name());
        if ( iter != comps.end()) 
            return (T*)&*iter->second;
        return nullptr; 
    }

    template<typename T>
    void remove() {
        for (auto iter = comps.begin(); iter != comps.end(); ++iter) {
            if (iter->second->getType() != T::name()) 
                continue;
            delete iter->second;
            iter = comps.erase(iter);    
        }
    }
};

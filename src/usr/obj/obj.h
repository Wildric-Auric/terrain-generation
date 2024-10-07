#pragma once

#include <unordered_map>
#include <string>

#include "../components/basic.h"

class Obj {

    std::unordered_map<std::string, Component*> comps;

    template<typename T>
    T* Add() {
        auto iter = comps.find(T::name());
        if ( iter != comps.end()) 
            return &*iter->second;
        T* ptr = new T(this);
        comps.insert(T::name(), ptr);
        return ptr;
    }

    template<typename T>
    void Remove() {
        for (auto iter = comps.begin(); iter != comps.end(); ++iter) {
            if (iter->second->getType() != T::name()) 
                continue;
            delete iter->second;
            iter = comps.erase(iter);    
        }
    }
};

#pragma once

enum class MSAAvalue {
    x1  = 1,
    x2  = 2,
    x4  = 4,
    x8  = 8,
    x16 = 16,
    x32 = 32,
    x64 = 64
};
     
     
class GfxParams {   
    public: 
        float     anisotropy   =  0.0;
        MSAAvalue msaa         = MSAAvalue::x1;
        static GfxParams inst;
    private:
        GfxParams() = default;

};

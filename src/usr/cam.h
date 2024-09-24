#pragma once
#include "components/basic.h"

struct Rect {
    float left;
    float right;
    float buttom;
    float top;
};

class Cam {
    public: 
        //PerspectiveMat(Matrix4<float> &matrix, float degreeFovY, float aspect, float near, float far)
        //OrthorgraphicMat(Matrix4<float>& matrix, float left, float right, float buttom, float top, float near = 1.0f, float far = -1.0f) {

        void lookat(const fvec3& targetPosition);

        void setOrtho(const Rect& r, float near = 0.0f, float far = 1.0f);
        void setPerp(float degreeFovY, float aspect, float near = 0.0f, float far = 100.0f);

        void setPosition(const fvec3& newPos);
        void setRotation(const fvec3& newRot);
        
        void updateView(); 

        fmat4 _view; 
        fmat4 _proj;

        Transform trans = Transform(this);

};

        


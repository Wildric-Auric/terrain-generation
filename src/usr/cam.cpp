#include "cam.h"

//TODO::Update rotation and translation
void Cam::lookat(const fvec3& targetPosition) {
    _view = fmat4(1.0f); 
    LookAt(_view, trans.pos, targetPosition, {0.0f, 1.0f,0.f});
}

void Cam::setOrtho(const Rect& r, float near, float far) {
    _proj = fmat4(1.0f);
    OrthorgraphicMat(_proj, r.left, r.right, r.buttom, r.top);
}

void Cam::setPerp(float degreeFovY, float aspect, float near, float far) {
    _proj = fmat4(1.0f);
    PerspectiveMat(_proj, degreeFovY, aspect, near, far);
}

void Cam::setPosition(const fvec3& newPos) {
    trans.pos = newPos;
};

void Cam::setRotation(const fvec3& newRot) {
    trans.rot = newRot; 
};
 
void Cam::updateView() {
    _view = fmat4(1.0);

    RotateMat(_view, -trans.rot.x, {1.0, 0.0, 0.0});
    RotateMat(_view, -trans.rot.y, {0.0, 1.0, 0.0});
    RotateMat(_view, -trans.rot.z, {0.0, 0.0, 1.0});

    TranslateMat(_view, fvec3(-trans.pos.x, -trans.pos.y, -trans.pos.z));
};

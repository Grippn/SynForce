#include "camera.h"
#include "effect.h"

camera::camera(const frustum& f) 
    : frustum_(f)
{
}

void camera::setup(effect& fx)
{
    fx["gvCameraPositionWorld"] = frustum_.position_;
    fx.gmViewProjection = frustum_.view_ * frustum_.proj_;
    D3DXMATRIX inv;
    D3DXMatrixInverse(&inv, nullptr, &(frustum_.view_));
    fx["gmCameraInverseView"] = inv;
    D3DXMatrixInverse(&inv, nullptr, &(frustum_.proj_));
    fx["gmCameraInverseProjection"] = inv;
    D3DXMATRIX prod = frustum_.view_ * frustum_.proj_;
    D3DXMatrixInverse(&inv, nullptr, &prod);
    fx["gmCameraInverseViewProjection"] = inv;        
}

void camera::OnResizeDevice(int w, int h)
{
    frustum_.aspect_ = ((float) w) / h;
    frustum_.compute();
}
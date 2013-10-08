#pragma once

#include "common.h"

class frustum
{
public:
    frustum(D3DXVECTOR4 position, D3DXVECTOR4 look_at, D3DXVECTOR4 up, float fovy, float aspect, float znear, float zfar);
    void compute();
    D3DXVECTOR4 position_;
    D3DXVECTOR4 look_at_;
    D3DXVECTOR4 up_;
    float fovy_;
    float aspect_;
    float znear_;
    float zfar_;
    D3DXMATRIX view_;
    D3DXMATRIX proj_;
};
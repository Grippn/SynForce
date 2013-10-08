#pragma once

#include "common.h"
#include "com_ptr.h"
#include "event.h"

class font : public D3D9Events
{
public:
    com_ptr<ID3DXFont> mpFont;
    font();
    virtual void OnLostDevice();
    virtual void OnResetDevice();
private:
    font(const font&);
    font& operator=(const font&);
};
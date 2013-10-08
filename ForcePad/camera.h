#pragma once

#include "common.h"
#include "frustum.h"
#include "event.h"

class effect;

class camera : public D3D9Events
{
public:
    camera(const frustum& f);
    frustum frustum_;
    void setup(effect& fx);
    virtual void OnResizeDevice(int w, int h);
};
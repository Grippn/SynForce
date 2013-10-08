#pragma once

#include "common.h"
#include "effect.h"
#include "frustum.h"

class light
{
public:
    light(const frustum& f, IDirect3DTexture9* texture) ;
    void setup(effect& fx);

    frustum frustum_;
    IDirect3DTexture9* texture_;
};

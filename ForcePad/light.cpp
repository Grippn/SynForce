#include "light.h"
#include "effect.h"

light::light(const frustum& f, IDirect3DTexture9* texture) 
    : frustum_(f), texture_(texture)
{
}

void light::setup(effect& fx)
{
    fx.gvLightWorldDirection = frustum_.position_;
    fx.gmLightViewProjection = frustum_.view_ * frustum_.proj_;
    fx["gTexSpotlight"] = texture_;
}
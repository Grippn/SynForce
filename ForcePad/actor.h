#pragma once

#include <memory>

#include "common.h"

class effect;

class actor
{
public:
    actor();
    virtual ~actor() = 0;
    virtual void draw(effect& fx, D3DXMATRIX mat) const = 0;
    D3DXMATRIX local;
protected:
    actor(const actor&);
    actor& operator=(const actor&);
};

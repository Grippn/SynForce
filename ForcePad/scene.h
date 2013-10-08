#pragma once

#include <memory>
#include <sstream>
#include <vector>

#include "common.h"

class actor;
class camera;
class light;

class scene
{
public:
    scene();

    std::shared_ptr<actor> root;
    std::shared_ptr<camera> pCamera;
    std::vector<std::shared_ptr<light> > vpLight;
    D3DXVECTOR4 ambient;
    D3DXVECTOR4 diffuse;
    float time;
    mutable std::stringstream out;
};

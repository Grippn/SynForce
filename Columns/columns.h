#pragma once

#include <memory>
#include <sstream>

#include "application.h"

class troupe;
class mesh;
class forcepad;
class renderer;
class scene;

class columns : public application
{
public:
    columns();
    virtual void draw() const;
    virtual void update(double dt);
    
    std::shared_ptr<scene> m_scene;
    std::shared_ptr<renderer> m_renderer;

    // Scene graph participants
    std::shared_ptr<troupe> root;
    std::shared_ptr<mesh> surface;
    std::shared_ptr<mesh> ground;
    std::shared_ptr<mesh> finger[5];

    std::shared_ptr<forcepad> pForcePad;
    
    com_ptr<IDirect3DTexture9> pTextureSpotlight;

private:
    columns(const columns&);
    columns& operator=(const columns&);

};


#pragma once

#include <memory>

class scene;
class font;
class effect;
class mesh;

class renderer
{
public:
    renderer();
    virtual ~renderer();
    virtual void draw(const scene&) = 0;

    std::shared_ptr<font> pFont;
    std::shared_ptr<effect> pEffect;

    static std::shared_ptr<renderer> factory();
};

class render_target;
class depth_stencil;

class vsm_renderer : public renderer
{
public:
    vsm_renderer();
    virtual void draw(const scene&);

    std::shared_ptr<mesh> pScreenQuad;
    std::shared_ptr<render_target> pLightRenderTarget;
    std::shared_ptr<render_target> pBlurRenderTarget;
    std::shared_ptr<depth_stencil> pLightDepthStencil;
};

class mesh;

class deferred_renderer : public vsm_renderer
{
public:
    deferred_renderer();
    virtual void draw(const scene&);
    std::shared_ptr<render_target> pDepthRenderTarget;
    std::shared_ptr<render_target> pNormalRenderTarget;
    
};

class basic_renderer : public renderer
{
public:
    virtual void draw(const scene&);
};

class sv_renderer : public renderer
{
public:
    virtual void draw(const scene&);
};
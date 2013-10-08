#include "renderer.h"

#include "font.h"
#include "effect.h"
#include "drawable.h"
#include "mesh.h"
#include "application.h"
#include "scene.h"
#include "camera.h"
#include "light.h"
#include "utility.h"

using namespace std;

// -- renderer ----------------------------------------------------------------

renderer::renderer()
{
    pFont = make_shared<font>();
    pEffect = make_shared<effect>("standard.fx");
}

renderer::~renderer()
{
}

shared_ptr<renderer> renderer::factory()
{
    return make_shared<sv_renderer>();
    //return make_shared<vsm_renderer>();
    //return make_shared<deferred_renderer>();
}

// -- basic renderer ----------------------------------------------------------

void basic_renderer::draw(const scene& scn)
{
    (*pEffect)["gfTime"] = scn.time;
    // Set up the camera
    scn.pCamera->setup(*pEffect);
    

    // Clear them
    CHECK( application::device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_XRGB(0,64,128), 1.0f, 0) );
    // Draw ambient and populate the depth buffer
    pEffect->technique = "Ambient";
    pEffect->gvLightAmbient = scn.ambient;
    pEffect->draw(*scn.root);
    
    // For each light
    for (auto i = scn.vpLight.begin(); i != scn.vpLight.end(); ++i)
    {
        (*i)->setup(*pEffect); // Load the light parameters
        pEffect->gvLightDiffuse = scn.diffuse;
        // Use the light render target as the shadow map
        pEffect->technique = "IlluminatedNoShadow";
        pEffect->draw(*scn.root);
    }

    // Render text
    D3DVIEWPORT9 viewport;
    application::device->GetViewport(&viewport);
    RECT rect = { viewport.X + 1, viewport.Y, viewport.Width, viewport.Height };
    pFont->mpFont->DrawText(NULL, scn.out.str().c_str(), -1, &rect, DT_LEFT | DT_BOTTOM, D3DCOLOR_XRGB(0,0,0));
    RECT rect2 = { viewport.X, viewport.Y, viewport.Width, viewport.Height - 1};
    pFont->mpFont->DrawText(NULL, scn.out.str().c_str(), -1, &rect2, DT_LEFT | DT_BOTTOM, D3DCOLOR_XRGB(255,255,0));
}

// -- variance shadow map renderer --------------------------------------------

    const int N = 256;
vsm_renderer::vsm_renderer()
{
    pScreenQuad = mesh::screen_quad();
    pLightDepthStencil = make_shared<depth_stencil>(N, N, D3DFMT_D24S8, false);
    pLightRenderTarget = make_shared<render_target>(N, N, D3DFMT_A32B32G32R32F, false);
    pBlurRenderTarget = make_shared<render_target>(N, N, D3DFMT_A32B32G32R32F, false);
}

void vsm_renderer::draw(const scene& scn)
{
    timer t;
    (*pEffect)["gfTime"] = scn.time;

    // Set up the constants
    scn.pCamera->setup(*pEffect);
    //vpLight[0]->setup(*pEffect);

    // Take note of the initial buffers
    com_ptr<IDirect3DSurface9> swap_chain_render_target;
    CHECK( application::device->GetRenderTarget(0, swap_chain_render_target) );
    com_ptr<IDirect3DSurface9> swap_chain_depth_stencil_surface;
    CHECK( application::device->GetDepthStencilSurface(swap_chain_depth_stencil_surface) );
    // Clear them
    CHECK( application::device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,  D3DCOLOR_XRGB(0,64,128), 1.0f, 0) );
    // Draw ambient and populate the depth buffer
    pEffect->technique = "Ambient";
    pEffect->gvLightAmbient = scn.ambient;
    pEffect->draw(*scn.root);
    
    // For each light
    for (auto i = scn.vpLight.begin(); i != scn.vpLight.end(); ++i)
    {
        (*i)->setup(*pEffect); // Load the light parameters
        pEffect->gvLightDiffuse = scn.diffuse;
        // Render Z from the light's view
        application::device->SetRenderTarget(0, pLightRenderTarget->pSurface);
        application::device->SetDepthStencilSurface(pLightDepthStencil->pSurface);
        CHECK( application::device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_XRGB(255,0,255), 1.0f, 0) );
        pEffect->technique = "ShadowMap";
        pEffect->draw(*scn.root);
        // Blur the shadow buffer
        D3DXMATRIX local;
        application::device->SetRenderTarget(0, pBlurRenderTarget->pSurface);
        pEffect->technique = "Blur";
        (*pEffect)["gfTexN"] = N;
        pScreenQuad->pTexture = pLightRenderTarget->pTexture;
        pEffect->draw(*pScreenQuad);
        application::device->SetRenderTarget(0, pLightRenderTarget->pSurface);
        pScreenQuad->pTexture = pBlurRenderTarget->pTexture;
        pEffect->draw(*pScreenQuad);
        pScreenQuad->pTexture = nullptr;
        // Render to the swap chain buffers
        application::device->SetRenderTarget(0, swap_chain_render_target);
        application::device->SetDepthStencilSurface(swap_chain_depth_stencil_surface);
        // Use the light render target as the shadow map
        pEffect->gTexShadow = pLightRenderTarget->pTexture;
        pEffect->technique = "Illuminated";
        pEffect->draw(*scn.root);

    }
    /*
    {
        pEffect->technique = "Passthrough";
        pScreenQuad->pTexture = pLightRenderTarget->pTexture;
        pEffect->draw(*pScreenQuad);
    }
    pScreenQuad->pTexture = nullptr;
    */

    scn.out << "Render time " << t.elapsed() * 1000 << " ms" << endl;
   
    D3DVIEWPORT9 viewport;
    application::device->GetViewport(&viewport);
    // Render text
    RECT rect = { viewport.X + 1, viewport.Y, viewport.Width, viewport.Height };
    pFont->mpFont->DrawText(NULL, scn.out.str().c_str(), -1, &rect, DT_LEFT | DT_BOTTOM, D3DCOLOR_XRGB(0,0,0));
    RECT rect2 = { viewport.X, viewport.Y, viewport.Width, viewport.Height - 1};
    pFont->mpFont->DrawText(NULL, scn.out.str().c_str(), -1, &rect2, DT_LEFT | DT_BOTTOM, D3DCOLOR_XRGB(255,255,0));
    
}

// -- deferred renderer -------------------------------------------------------

deferred_renderer::deferred_renderer()
{
    D3DVIEWPORT9 viewport;
    application::device->GetViewport(&viewport);
    pDepthRenderTarget  = make_shared<render_target>(viewport.Width, viewport.Height, D3DFMT_R32F, true);
    pNormalRenderTarget = make_shared<render_target>(viewport.Width, viewport.Height, D3DFMT_A2R10G10B10, true);
}

void deferred_renderer::draw(const scene& scn)
{
    D3DVIEWPORT9 viewport;
    application::device->GetViewport(&viewport);

    timer t;
    (*pEffect)["gfTime"] = scn.time;

    // Set up the constants
    scn.pCamera->setup(*pEffect);

    // Take note of the initial buffers
    com_ptr<IDirect3DSurface9> swap_chain_render_target;
    CHECK( application::device->GetRenderTarget(0, swap_chain_render_target) );
    com_ptr<IDirect3DSurface9> swap_chain_depth_stencil_surface;
    CHECK( application::device->GetDepthStencilSurface(swap_chain_depth_stencil_surface) );
    // Clear them
    CHECK( application::device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,  D3DCOLOR_XRGB(0,64,128), 1.0f, 0) );
    // Install depth and normal buffers
    application::device->SetRenderTarget(0, pDepthRenderTarget->pSurface);
    application::device->SetRenderTarget(1, pNormalRenderTarget->pSurface);
    pEffect->technique = "WNormal";
    pEffect->draw(*scn.root);
    // Uninstall additional buffers
    application::device->SetRenderTarget(1, nullptr);
    // Render to the swap chain buffers
    
    for (auto i = scn.vpLight.begin(); i != scn.vpLight.end(); ++i)
    {
        (*i)->setup(*pEffect);
        // Render Z from the light's view
        application::device->SetRenderTarget(0, pLightRenderTarget->pSurface);
        application::device->SetDepthStencilSurface(pLightDepthStencil->pSurface);
        CHECK( application::device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_XRGB(255,0,255), 1.0f, 0) );
        pEffect->technique = "ShadowMap";
        pEffect->draw(*scn.root);
        // Blur the shadow buffer
        D3DXMATRIX local;
        application::device->SetRenderTarget(0, pBlurRenderTarget->pSurface);
        pEffect->technique = "Blur";
        (*pEffect)["gfTexN"] = N;
        pScreenQuad->pTexture = pLightRenderTarget->pTexture;
        pEffect->draw(*pScreenQuad);
        application::device->SetRenderTarget(0, pLightRenderTarget->pSurface);
        pScreenQuad->pTexture = pBlurRenderTarget->pTexture;
        pEffect->draw(*pScreenQuad);
        pScreenQuad->pTexture = nullptr;
    }
    // Render to the swap chain buffers
    application::device->SetRenderTarget(0, swap_chain_render_target);
    application::device->SetDepthStencilSurface(swap_chain_depth_stencil_surface);
    // Use the light render target as the shadow map
    pEffect->gTexShadow = pLightRenderTarget->pTexture;
    
    {
        pEffect->technique = "ReconstructWorld";
        (*pEffect)["gtWBuffer"] = pDepthRenderTarget->pTexture;
        (*pEffect)["gtNormal"] = pNormalRenderTarget->pTexture;
        (*pEffect)["gvScreenPixels"] = D3DXVECTOR4((float) viewport.Width, (float) viewport.Height, 0, 0);
        pEffect->draw(*pScreenQuad);
    }
    /*
    {
        pEffect->technique = "Passthrough";
        pScreenQuad->pTexture = pDepthRenderTarget->pTexture;
        pEffect->draw(*pScreenQuad);
    }
    */
    pScreenQuad->pTexture = nullptr;

    // Render text
    RECT rect = { viewport.X + 1, viewport.Y, viewport.Width, viewport.Height };
    pFont->mpFont->DrawText(NULL, scn.out.str().c_str(), -1, &rect, DT_LEFT | DT_BOTTOM, D3DCOLOR_XRGB(0,0,0));
    RECT rect2 = { viewport.X, viewport.Y, viewport.Width, viewport.Height - 1};
    pFont->mpFont->DrawText(NULL, scn.out.str().c_str(), -1, &rect2, DT_LEFT | DT_BOTTOM, D3DCOLOR_XRGB(255,255,0));

    
}

// -- shadow volumes renderer -------------------------------------------------

void sv_renderer::draw(const scene& scn)
{
    (*pEffect)["gfTime"] = scn.time;
    // Set up the camera
    scn.pCamera->setup(*pEffect);
    

    // Clear them
    CHECK( application::device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_XRGB(0,64,128), 1.0f, 0) );
    // Draw ambient and populate the depth buffer
    pEffect->technique = "Ambient";
    pEffect->gvLightAmbient = scn.ambient;
    pEffect->draw(*scn.root);
    
    // For each light
    for (auto i = scn.vpLight.begin(); i != scn.vpLight.end(); ++i)
    {
        // Clear stencil only, leave DB alone
        CHECK( application::device->Clear(0, NULL, D3DCLEAR_STENCIL, D3DCOLOR_XRGB(0,64,128), 1.0f, 0) );
        (*i)->setup(*pEffect); // Load the light parameters
        pEffect->gvLightDiffuse = scn.diffuse;
        // Render to the stencil buffer
        pEffect->technique = "ShadowVolume";
        mesh::draw_shadow_volumes = true;
        pEffect->draw(*scn.root);
        mesh::draw_shadow_volumes = false;

        // Use the light render target as the shadow map
        pEffect->technique = "IlluminatedStencil";
        pEffect->draw(*scn.root);
    }

    // Render text
    D3DVIEWPORT9 viewport;
    application::device->GetViewport(&viewport);
    RECT rect = { viewport.X + 1, viewport.Y, viewport.Width, viewport.Height };
    pFont->mpFont->DrawText(NULL, scn.out.str().c_str(), -1, &rect, DT_LEFT | DT_BOTTOM, D3DCOLOR_XRGB(0,0,0));
    RECT rect2 = { viewport.X, viewport.Y, viewport.Width, viewport.Height - 1};
    pFont->mpFont->DrawText(NULL, scn.out.str().c_str(), -1, &rect2, DT_LEFT | DT_BOTTOM, D3DCOLOR_XRGB(255,255,0));
}

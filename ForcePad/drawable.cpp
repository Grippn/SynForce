#include "application.h"
#include "drawable.h"

using namespace std;

// render_target

render_target::render_target(int width, int height, D3DFORMAT format, bool resizing)
{
    mResizing = resizing;
    mWidth = width;
    mHeight = height;
    mFormat = format;
    OnResetDevice();
}

void render_target::OnLostDevice()
{
    pSurface = nullptr;
    pTexture = nullptr;
}

void render_target::OnResetDevice()
{
    CHECK( D3DXCreateTexture(
        application::device, 
        mWidth, 
        mHeight, 
        D3DX_DEFAULT, 
        D3DUSAGE_RENDERTARGET | D3DUSAGE_AUTOGENMIPMAP, 
        mFormat, 
        D3DPOOL_DEFAULT, 
        pTexture) );
    CHECK( pTexture->GetSurfaceLevel(0, pSurface) );
}

void render_target::OnResizeDevice(int w, int h)
{
    if (mResizing)
    {
        mWidth = w;
        mHeight = h;
    }
}

// depth_stencil

depth_stencil::depth_stencil(int width, int height, D3DFORMAT format, bool resizing)
{
    mResizing = resizing;
    mWidth = width;
    mHeight = height;
    mFormat = format;
    OnResetDevice();
}

void depth_stencil::OnLostDevice()
{
    pSurface = nullptr;
}

void depth_stencil::OnResetDevice()
{
    CHECK( application::device->CreateDepthStencilSurface(
        mWidth, 
        mHeight, 
        mFormat, 
        D3DMULTISAMPLE_NONE,
        0,
        TRUE,
        pSurface,
        NULL) );
}

void depth_stencil:: OnResizeDevice(int w, int h)
{
    if (mResizing)
    {
        mWidth = w;
        mHeight = h;
    }
}


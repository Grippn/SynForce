#pragma once

#include "common.h"
#include "com_ptr.h"
#include "event.h"

class render_target : public D3D9Events
{
public:
    render_target(int width, int height, D3DFORMAT format, bool resizing);
    virtual void OnLostDevice();
    virtual void OnResetDevice();
    virtual void OnResizeDevice(int w, int h);

    bool mResizing;
    int mWidth;
    int mHeight;
    D3DFORMAT mFormat;
    
    com_ptr<IDirect3DTexture9> pTexture;
    com_ptr<IDirect3DSurface9> pSurface;
private:
    render_target(const render_target&);
    render_target& operator=(const render_target&);
};

class depth_stencil : public D3D9Events
{
public:
    depth_stencil(int width, int height, D3DFORMAT format, bool resizing);
    virtual void OnLostDevice();
    virtual void OnResetDevice();
    virtual void OnResizeDevice(int w, int h);

    bool mResizing;
    int mWidth;
    int mHeight;
    D3DFORMAT mFormat;
    
    com_ptr<IDirect3DSurface9> pSurface;
private:
    depth_stencil(const depth_stencil&);
    depth_stencil& operator=(const depth_stencil&);    
};
#pragma once

#include "common.h"
#include "event.h"
#include "com_ptr.h"

class actor;

class effect : public D3D9Events
{
public:
    explicit effect(const char* name);
    
    void draw(const actor& root);

    ID3DXEffect* operator->() { return mpEffect; }

    virtual void OnLostDevice();
    virtual void OnResetDevice();

    com_ptr<ID3DXEffect> mpEffect;

    class parameter_proxy
    {
    public:
        ID3DXEffect* p; // weak reference
        D3DXHANDLE h;
        void init(ID3DXEffect* q, const char* name);
        parameter_proxy& operator=(const D3DXMATRIX& m) { CHECK( p->SetMatrix(h, &m) ); return *this; }
        parameter_proxy& operator=(const D3DXVECTOR4& v) { CHECK( p->SetVector(h, &v) ); return *this; }
        parameter_proxy& operator=(IDirect3DTexture9* q) { CHECK( p->SetTexture(h, q) ); return *this; }
        parameter_proxy& operator=(float f) { CHECK( p->SetFloat(h, f) ); return *this; }
    };

    parameter_proxy operator[](const char*);

    parameter_proxy gmWorld;
    parameter_proxy gmWorldInverseTranspose;
    parameter_proxy gmViewProjection;
    parameter_proxy gmLightViewProjection;
    parameter_proxy gvLightWorldDirection;
    parameter_proxy gvLightAmbient;
    parameter_proxy gvLightDiffuse;
    parameter_proxy gvMaterial;
    parameter_proxy gTex;
    parameter_proxy gTexShadow;

    class technique_proxy
    {
    public:
        ID3DXEffect* p;
        technique_proxy& operator=(D3DXHANDLE h) { CHECK( p->SetTechnique(h) ); return *this; }
    };

    technique_proxy technique;
    
protected:
    effect(const effect&);
    effect& operator=(effect&);
};

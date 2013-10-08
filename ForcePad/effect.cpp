#include <exception>
#include <cassert>

#include "com_ptr.h"
#include "effect.h"
#include "application.h"
#include "actor.h"

using namespace std;

void effect::parameter_proxy::init(ID3DXEffect* q, const char* name)
{
    h = (p = q)->GetParameterByName(0, name);
    if (!h)
        xlog << "Could not find parameter: " << name << endl;
}

effect::parameter_proxy effect::operator[](const char* name)
{
    parameter_proxy p;
    p.init(mpEffect, name); 
    return p;
}

effect::effect(const char* name)
{
    com_ptr<ID3DXBuffer> errors;
    HRESULT hr = D3DXCreateEffectFromFile(application::device, name, NULL, NULL, D3DXSHADER_DEBUG, 0, mpEffect, errors);
    if (errors)
    {
        xlog << (char*) errors->GetBufferPointer() << endl;
        exit(-1);
        //throw exception((char*) errors->GetBufferPointer());
    }
    if (hr != D3D_OK)
    {
        xlog << "D3DXCreateEffectFromFile failed" << endl;
        exit(-1);
        //throw exception("D3DXCreateEffectFromFile failed");
    }

    technique.p = mpEffect;

#define INIT(X) X.init(mpEffect, #X)
    
    INIT(gmWorld);
    INIT(gmWorldInverseTranspose);
    INIT(gmViewProjection);
    INIT(gmLightViewProjection);
    INIT(gvLightWorldDirection);
    INIT(gvLightAmbient);
    INIT(gvLightDiffuse);
    INIT(gvMaterial);
    INIT(gTex);
    INIT(gTexShadow);

#undef INIT

}

void effect::OnLostDevice()
{
    mpEffect->OnLostDevice();
}

void effect::OnResetDevice()
{
    mpEffect->OnResetDevice();
}


void effect::draw(const actor& root)
{
    D3DXMATRIX world;
    D3DXMatrixIdentity(&world);
    UINT numPasses; 
    CHECK( mpEffect->Begin(&numPasses, 0) );
    for (UINT i = 0; i != numPasses; ++i)
    {
        CHECK( mpEffect->BeginPass(i) );
        root.draw(*this, world);        
        CHECK( mpEffect->EndPass() );
    }
    CHECK( mpEffect->End() );
}
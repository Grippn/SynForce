#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "camera.h"
#include "columns.h"
#include "forcepad.h"
#include "frustum.h"
#include "light.h"
#include "mesh.h"
#include "renderer.h"
#include "scene.h"
#include "troupe.h"
#include "utility.h"
#include "xlog.h"

using namespace std;

columns::columns()
{
    // Touchpad setup
    pForcePad = make_shared<forcepad>();
    if (!pForcePad->connected())
    {
        MessageBox(NULL, "ForcePad not found", "Error", MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }

    // Renderer selection
    m_renderer = renderer::factory();

    // Scene construction
    m_scene = make_shared<scene>();
    
    // Scene setup
    D3DXMATRIX A, B;
    root = make_shared<troupe>();

    {   // Touch surface
        ID3DXMesh* pXMesh = NULL;
        D3DXCreateBox(application::device, 1, 1, 1, &pXMesh, NULL);
        surface = make_shared<mesh>(pXMesh);
        surface->color(D3DXVECTOR4(0.5,0.5,0.5,1.0));
        D3DXMatrixScaling(&A, 1, ((float) pForcePad->YHiRim - pForcePad->YLoRim) / (pForcePad->XHiRim - pForcePad->XLoRim), 0.1f);
        D3DXMatrixTranslation(&B, 0,0,-0.5f);
        surface->local = B * A;
        root->insert(surface);
    }
    
    {   // Ground
        ID3DXMesh* pXMesh = NULL;
        D3DXCreateBox(application::device, 20, 20, 20, &pXMesh, NULL);
        ground = make_shared<mesh>(pXMesh);
        ground->color(D3DXVECTOR4(0.25f,0.25f,0.25f,1.0));
        D3DXMatrixTranslation(&B, 0, 0,-10.2f);
        ground->local = B;
        root->insert(ground);
    }
    
    // Fingers
    for (int i = 0; i != pForcePad->MAX_GROUP_SIZE; ++i)
    {
        ID3DXMesh* pXMesh = NULL;
        D3DXCreateCylinder(application::device, 0.067f, 0.067f, 1.0f, 32, 1, &pXMesh, NULL);
        finger[i] = make_shared<mesh>(pXMesh);
        D3DXMatrixTranslation(&B, 0, -1000, 0); // start the columns behind the camera
        finger[i]->local = B;
        root->insert(finger[i]);
    }
    finger[0]->color(D3DXVECTOR4(0,0,1,1));
    finger[1]->color(D3DXVECTOR4(0,1,0,1));
    finger[2]->color(D3DXVECTOR4(1,0,0,1));
    finger[3]->color(D3DXVECTOR4(1,1,0,1));
    finger[4]->color(D3DXVECTOR4(1,0,1,1));

    m_scene->root = root;

    CHECK( D3DXCreateTextureFromFile(application::device, "spotlight.png", pTextureSpotlight) );


     m_scene->pCamera = make_shared<camera>(
        frustum(
        D3DXVECTOR4( 0.0f, -2.0f, 2.0f, 1.0f), 
        D3DXVECTOR4( 0.0f,  0.0f, 0.0f, 1.0f), 
        D3DXVECTOR4( 0.0f,  0.0f, 1.0f,  0.0f), 
        D3DX_PI / 8, 
        ((float) width()) / height(), 
        1.0f, 
        100.0f));
    m_scene->ambient = D3DXVECTOR4(0.2f, 0.2f, 0.2f, 1.0f);
    m_scene->diffuse = D3DXVECTOR4(0.8f, 0.8f, 0.8f, 1.0f);
    m_scene->vpLight.push_back(make_shared<light>(
        frustum(
        D3DXVECTOR4(-3.0f,-2.0f, 4.0f, 1.0f),
        D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 1.0f),
        D3DXVECTOR4( 0.0f, 0.0f, 1.0f, 0.0f),
        D3DX_PI / 12, 
        ((float) width()) / height(), 
        1.0f, 
        100.0f),
        pTextureSpotlight));
    
}


void columns::update(double dt)
{
    swap(m_scene->out, stringstream()); // reset the stringstream
    for (int i = 0; i != pForcePad->MAX_GROUP_SIZE; ++i)
    {
        float x = ((float) pForcePad->X[i] - pForcePad->XLoRim) / (pForcePad->XHiRim - pForcePad->XLoRim) - 0.5f;
        float y = ((float) pForcePad->Y[i] - pForcePad->YLoRim) / (pForcePad->XHiRim - pForcePad->XLoRim) - 0.5f * ((float) pForcePad->YHiRim - pForcePad->YLoRim) / (pForcePad->XHiRim - pForcePad->XLoRim);
        float f = ((float) ((pForcePad->filteredF[i] > 1) ? pForcePad->filteredF[i] : 1)) / 256;
        x = -x;

        // Hide the unused finger behind the camera
        if (!(pForcePad->FingerState[i] & SF_FingerPresent))
            y = -1000; 

        // Set the location
        D3DXMATRIX A;
        D3DXMatrixTranslation(&A, x, y, 0.5f);
        D3DXMATRIX B;
        D3DXMatrixScaling(&B, 1, 1, f);
        finger[i]->local = A * B;
    }

    for (int j = 4; j != -1; --j)
        if (pForcePad->FingerState[j] & SF_FingerPresent)
            m_scene->out << "Finger " << j+1 << ": " << fixed << showpos << setprecision(2) << pForcePad->filteredF[j] / 16.0f << " Newtons" << noshowpos << endl;
    m_scene->out << "Corners " << 
        pForcePad->cornerForce[0] << ", " << 
        pForcePad->cornerForce[1] << ", " << 
        pForcePad->cornerForce[2] << ", " << 
        pForcePad->cornerForce[3];
    // m_scene->out << "Frame " << 1000 * dt << " ms" << endl;
}


void columns::draw() const
{
    m_renderer->draw(*m_scene);
}

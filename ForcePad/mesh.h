#pragma once

#include <cassert>

#include <vector>

#include "common.h"
#include "com_ptr.h"
#include "actor.h"

struct vertex
{
    D3DXVECTOR3 position;
    D3DXVECTOR3 normal;
    D3DXVECTOR2 texcoord;
    const static D3DVERTEXELEMENT9 declaration[];
};

class mesh : public actor
{
public:
    ID3DXMesh* m;
    D3DXVECTOR4 m_color;
    mesh();
    explicit mesh(ID3DXMesh* p);
    virtual ~mesh();
    virtual void draw(effect& fx, D3DXMATRIX mat) const;
    
    mesh& color(const D3DXVECTOR4& c);

    vertex* lock();
    void unlock();
    int numVertices();

    DWORD* face_lock();
    void face_unlock();
    int numFaces();

    IDirect3DTexture9* pTexture; // Must be a weak reference because it could be a losable render texture

    static std::shared_ptr<mesh> screen_quad();

    static bool draw_shadow_volumes;

protected:
    mesh(const mesh&);
    mesh& operator=(mesh&);
private:
    mutable ID3DXMesh* m_shadow;
    void compute_shadow_mesh() const;
    void update_shadow_mesh() const;
    mutable bool m_dirty_shadow;
};


void CreateSheet(int m, int n, D3DXVECTOR3 origin, D3DXVECTOR3 u, D3DXVECTOR3 v, ID3DXMesh** ppMesh);

void CreateBox(int m, int n, int o, D3DXVECTOR3 origin, D3DXVECTOR3 u, D3DXVECTOR3 v, D3DXVECTOR3 w, ID3DXMesh** pMesh);

void CreateIcosahedron(ID3DXMesh** ppMesh);

void TesselateMesh(ID3DXMesh** ppMeshIn);
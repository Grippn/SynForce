#include <iostream>
#include <algorithm>

#include "xlog.h"
#include "mesh.h"
#include "utility.h"
#include "application.h"
#include "effect.h"

using namespace std;

bool mesh::draw_shadow_volumes = false;

mesh::mesh() : m(nullptr), m_shadow(nullptr), m_color(D3DXVECTOR4(1,1,1,1)), m_dirty_shadow(true)
{
}

shared_ptr<mesh> mesh::screen_quad()
{
    ID3DXMesh* p;
    D3DXCreateMesh(2, 4, D3DXMESH_32BIT | D3DXMESH_MANAGED, vertex::declaration, application::device, &p);
    vertex* q;
    CHECK(p->LockVertexBuffer(0, (void**) &q) );
    q[0].position = D3DXVECTOR3(-1, -1,  0);
    q[1].position = D3DXVECTOR3(-1,  1,  0);
    q[2].position = D3DXVECTOR3( 1, -1,  0);
    q[3].position = D3DXVECTOR3( 1,  1,  0);
    q[0].texcoord = D3DXVECTOR2( 0,  1);
    q[1].texcoord = D3DXVECTOR2( 0,  0);
    q[2].texcoord = D3DXVECTOR2( 1,  1);
    q[3].texcoord = D3DXVECTOR2( 1,  0);
    q[0].normal = q[1].normal = q[2].normal = q[3].normal = D3DXVECTOR3(0, 0, -1);
    CHECK( p->UnlockVertexBuffer() );
    DWORD* i;
    CHECK( p->LockIndexBuffer(0, (void**) &i) );
    i[0] = 0;
    i[1] = 1;
    i[2] = 2;
    i[3] = 2;
    i[4] = 1;
    i[5] = 3;
    CHECK( p->UnlockIndexBuffer() );
    auto m = make_shared<mesh>();
    m->m = p;
    m->color(D3DXVECTOR4(1,1,1,1));
    return m;
}

IDirect3DTexture9* get_white()
{
    static com_ptr<IDirect3DTexture9> white;
    if (!white)
        D3DXCreateTextureFromFile(application::device, "white.png", white);
    return white;
}

mesh::mesh(ID3DXMesh* p) : m(p), m_shadow(nullptr), m_dirty_shadow(true)
{
    assert(p);
    p->CloneMesh(D3DXMESH_32BIT | D3DXMESH_MANAGED, vertex::declaration, application::device, &m);
    p->Release();

    vertex* q;
    CHECK( m->LockVertexBuffer(0, (void**) &q) );
    for (DWORD i = 0; i != m->GetNumVertices(); ++i)
    {
        q[i].texcoord.x = (q[i].position.x / 2 + 0.5f);
        q[i].texcoord.y = ((- q[i].position.y - q[i].position.z) / 2 - 0.45f);
    }
    CHECK( m->UnlockVertexBuffer() );

    pTexture = get_white();
    // Now we will leak this!
    m_color = D3DXVECTOR4(1,1,1,1);
}

mesh::~mesh() 
{
    if (m_shadow)
        m_shadow->Release();
    if (m)
        m->Release();
}

vertex* mesh::lock()
{
    vertex* p;
    m->LockVertexBuffer(0, (void**) &p);
    return p;
}
void mesh::unlock()
{
    m->UnlockVertexBuffer();

    //timer t;
    CHECK( D3DXComputeNormals(m, NULL) );
    //xlog << "D3DXComputeNormals took " << t.reset() * 1000 << " ms" << endl;
    m_dirty_shadow = true;
}

int mesh::numVertices()
{
    return m->GetNumVertices();
}

const D3DVERTEXELEMENT9 vertex::declaration[] =
{
    { 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
    { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
    D3DDECL_END()
};

void mesh::draw(effect& fx, D3DXMATRIX mat) const
{

    D3DXMATRIX world = local * mat;
    fx.gmWorld = world;
    fx.gvMaterial = m_color;

    D3DXMATRIX worldInverseTranspose;
    D3DXMatrixInverse(&worldInverseTranspose, NULL, &world);
    D3DXMatrixTranspose(&worldInverseTranspose, &worldInverseTranspose);
    fx.gmWorldInverseTranspose = worldInverseTranspose;
    
    fx.gTex = pTexture;

    CHECK( fx->CommitChanges() );

    if (!draw_shadow_volumes)
    {
        m->DrawSubset(0);
    }
    else
    {
        if (!m_shadow)
            compute_shadow_mesh();
        else if (m_dirty_shadow)
            update_shadow_mesh();
        m_shadow->DrawSubset(0);
    }

}

mesh& mesh::color(const D3DXVECTOR4& c)
{
    m_color = c;
    return *this;
}








void CreateSheet(int m, int n, D3DXVECTOR3 origin, D3DXVECTOR3 u, D3DXVECTOR3 v, ID3DXMesh** ppMesh)
{
    int numFaces = m * n * 2;
    int numVertices = (m + 1) * (n + 1);
    CHECK( D3DXCreateMesh(numFaces, numVertices, D3DXMESH_32BIT | D3DXMESH_MANAGED, vertex::declaration, application::device, ppMesh) );
    vertex* p;
    CHECK( (*ppMesh)->LockVertexBuffer(0, (void**) &p) );
    D3DXVECTOR3 w;
    D3DXVec3Cross(&w, &u, &v);
    D3DXVec3Normalize(&w, &w);
    for (int i = 0; i != (m + 1); ++i)
        for (int j = 0; j != (n + 1); ++j)
        {
            p[i * (n + 1) + j].position = origin + u * (float) i + v * (float) j;
            p[i * (n + 1) + j].normal = w;
        }
    CHECK( (*ppMesh)->UnlockVertexBuffer() );
    DWORD* q;
    CHECK( (*ppMesh)->LockIndexBuffer(0, (void**) &q) );
    int k = 0;
    for (int i = 0; i != m; ++i)
        for (int j = 0; j != n; ++j)
        {
            q[k++] = (i + 0) * (n + 1) + (j + 0);
            q[k++] = (i + 1) * (n + 1) + (j + 0);
            q[k++] = (i + 0) * (n + 1) + (j + 1);
            q[k++] = (i + 0) * (n + 1) + (j + 1);
            q[k++] = (i + 1) * (n + 1) + (j + 0);
            q[k++] = (i + 1) * (n + 1) + (j + 1);
        }
    CHECK( (*ppMesh)->UnlockIndexBuffer() );
}

void CreateBox(int m, int n, int o, D3DXVECTOR3 origin, D3DXVECTOR3 u, D3DXVECTOR3 v, D3DXVECTOR3 w, ID3DXMesh** pMesh)
{
    com_ptr<ID3DXMesh> sides[6];
    CreateSheet(m, n, origin + w * (float) o, u, v, sides[0]);
    CreateSheet(n, o, origin + u * (float) m, v, w, sides[1]);
    CreateSheet(o, m, origin + v * (float) n, w, u, sides[2]);
    CreateSheet(n, m, origin, v, u, sides[3]);
    CreateSheet(o, n, origin, w, v, sides[4]);
    CreateSheet(m, o, origin, u, w, sides[5]);
    CHECK( D3DXConcatenateMeshes((ID3DXMesh**) sides, 6, D3DXMESH_32BIT | D3DXMESH_MANAGED, NULL, NULL, vertex::declaration, application::device, pMesh) );
    int numFaces = (*pMesh)->GetNumFaces();
    DWORD* pAttr = nullptr;
    (*pMesh)->LockAttributeBuffer(0, &pAttr);
    for (int i = 0; i != numFaces; ++i)
        pAttr[i] = 0;
    (*pMesh)->UnlockAttributeBuffer();
}

void CreateIcosahedron(ID3DXMesh** ppMesh)
{
    const float psi = (1 + sqrt(5.0f)) / 2;
    
    int numFaces = 20;
    int numVertices = 12;
    D3DXCreateMesh(numFaces, numVertices, D3DXMESH_32BIT | D3DXMESH_MANAGED, vertex::declaration, application::device, ppMesh);

    vertex* p;
    (*ppMesh)->LockVertexBuffer(0, (void**) &p);
    p[ 0].position = D3DXVECTOR3(0, -1, -psi);
    p[ 1].position = D3DXVECTOR3(0, +1, -psi);
    p[ 2].position = D3DXVECTOR3(0, -1, +psi);
    p[ 3].position = D3DXVECTOR3(0, +1, +psi);
    p[ 4].position = D3DXVECTOR3(-1, -psi, 0);
    p[ 5].position = D3DXVECTOR3(+1, -psi, 0);
    p[ 6].position = D3DXVECTOR3(-1, +psi, 0);
    p[ 7].position = D3DXVECTOR3(+1, +psi, 0);
    p[ 8].position = D3DXVECTOR3(-psi, 0, -1);
    p[ 9].position = D3DXVECTOR3(-psi, 0, +1);
    p[10].position = D3DXVECTOR3(+psi, 0, -1);
    p[11].position = D3DXVECTOR3(+psi, 0, +1);
    
    for (int i = 0; i != numVertices; ++i)
        D3DXVec3Normalize(&p[i].normal, &p[i].position);

    DWORD* q;
    (*ppMesh)->LockIndexBuffer(0, (void**) &q);
    for (int i = 0; i != numVertices-2; ++i)
    {
        for (int j = i + 1; j != numVertices-1; ++j)
        {
            // Is i-j edge the right length?
            D3DXVECTOR3 delta_ij = p[j].position - p[i].position;
            if (D3DXVec3LengthSq(&delta_ij) < 4.1f)
            {
                for (int k = j + 1; k != numVertices; ++k)
                {
                    // Are j-k and k-i edges the right lengths?
                    D3DXVECTOR3 delta_jk = p[k].position - p[j].position;
                    D3DXVECTOR3 delta_ki = p[i].position - p[k].position;
                    if ((D3DXVec3LengthSq(&delta_jk) < 4.1f) &&
                        (D3DXVec3LengthSq(&delta_ki) < 4.1f))
                    {
                        // Emit the first point of the triangle
                        *(q++) = i;
                        // Check orientation
                        D3DXVECTOR3 normal;
                        D3DXVec3Cross(&normal, &delta_ij, &delta_jk);
                        if (D3DXVec3Dot(&normal, &p[i].position) > 0)
                        {   // Emit triangle
                            *(q++) = j;
                            *(q++) = k;
                        }
                        else
                        {   // Flip triangle
                            *(q++) = k;
                            *(q++) = j;
                        }
                    }
                }
            }
        }
    }
    (*ppMesh)->UnlockVertexBuffer();
    (*ppMesh)->UnlockIndexBuffer();

}

void TesselateMesh(ID3DXMesh** ppMeshIn)
{
    int numFacesIn = (*ppMeshIn)->GetNumFaces();
    int numFacesOut = numFacesIn * 4;
    int numVerticesOut = numFacesOut * 3;

    ID3DXMesh* pMeshOut;
    D3DXCreateMesh(numFacesOut, numVerticesOut, D3DXMESH_32BIT | D3DXMESH_MANAGED, vertex::declaration, application::device, &pMeshOut);

    DWORD* iIn;
    (*ppMeshIn)->LockIndexBuffer(0, (void**) &iIn);
    vertex* vIn;
    (*ppMeshIn)->LockVertexBuffer(0, (void**) &vIn);
    vertex* vOut;
    pMeshOut->LockVertexBuffer(0, (void**) &vOut);

    for (int i = 0; i != numFacesIn; ++i)
    {
        D3DXVECTOR3 a = vIn[iIn[i*3+0]].position;
        D3DXVECTOR3 b = vIn[iIn[i*3+1]].position;
        D3DXVECTOR3 c = vIn[iIn[i*3+2]].position;

        (vOut++)->position =  a;
        (vOut++)->position = (a + b) / 2;
        (vOut++)->position = (a + c) / 2;

        (vOut++)->position = (b + a) / 2;
        (vOut++)->position =  b;
        (vOut++)->position = (b + c) / 2;
        
        (vOut++)->position = (c + a) / 2;
        (vOut++)->position = (c + b) / 2;
        (vOut++)->position =  c;

        (vOut++)->position = (a + b) / 2;
        (vOut++)->position = (b + c) / 2;
        (vOut++)->position = (c + a) / 2;
    }

    pMeshOut->UnlockVertexBuffer();
    (*ppMeshIn)->UnlockVertexBuffer();
    (*ppMeshIn)->UnlockIndexBuffer();

    DWORD* iOut;
    pMeshOut->LockIndexBuffer(0, (void**) &iOut);
    for (int i = 0; i != numFacesOut * 3; ++i)
        iOut[i] = i;
    pMeshOut->UnlockIndexBuffer();

    (*ppMeshIn)->Release();
    *ppMeshIn = pMeshOut;
}



void mesh::compute_shadow_mesh() const
{
    xlog << "Computing shadow mesh" << endl;
    timer t;
    // Make a copy of the input mesh so we have a known format to deal with
    ID3DXMesh* pInputMesh = 0;
    m->CloneMesh(D3DXMESH_32BIT, vertex::declaration, application::device, &pInputMesh);
    
    // Get some quantities we will use a lot
    DWORD numFaces = pInputMesh->GetNumFaces();
    DWORD numVertices = pInputMesh->GetNumVertices();
    
    // Adjacent triangles
    DWORD* pAdjacency = new DWORD[numFaces * 3];
    pInputMesh->GenerateAdjacency(0.01f, pAdjacency);
    // Identify equivalent vertices
    DWORD* pPRep = new DWORD[numVertices];
    pInputMesh->ConvertAdjacencyToPointReps(pAdjacency, pPRep);
    
    // Count the edges we will emit so we can correctly size the output mesh
    DWORD numEdges = 0;
    DWORD numHalfEdges = 0;
    for (DWORD i = 0; i != numFaces; ++i)
    {
        for (DWORD j = 0; j != 3; ++j)
        {
            DWORD k = pAdjacency[i * 3 + j]; // the other triangle
            if (k >= numFaces)
            {
                ++numHalfEdges; // This is potentially an error
            }
            if ((k > i) && (k < numFaces))
            {
                // we have found the lower-numbered face's side of an edge
                ++numEdges;
            }
        }
    }
    
    DWORD outNumVertices = numFaces * 3;
    DWORD outNumFaces = numEdges * 2;

    ID3DXMesh* pOutputMesh = NULL;
    D3DXCreateMesh(outNumFaces, outNumVertices, D3DXMESH_32BIT | D3DXMESH_MANAGED, vertex::declaration, application::device, &pOutputMesh);

    vertex* pInputVertices = NULL;
    DWORD* pInputIndices = NULL;
    vertex* pOutputVertices = NULL;
    DWORD* pOutputIndices = NULL;

    pOutputMesh->LockIndexBuffer(0, (void**) &pOutputIndices);
    pOutputMesh->LockVertexBuffer(0, (void**) &pOutputVertices);
    pInputMesh->LockIndexBuffer(0, (void**) &pInputIndices);
    pInputMesh->LockVertexBuffer(0, (void**) &pInputVertices);
    
    for (DWORD i = 0; i != numFaces; ++i)
    {
        // For each face, copy over the representative positions
        for (DWORD j = 0; j != 3; ++j)
            pOutputVertices[i * 3 + j].position = pInputVertices[pPRep[pInputIndices[i * 3 + j]]].position;
        // Compute the normal for that triangle
        D3DXVECTOR3 normal;
        D3DXVECTOR3 delta_01 = pOutputVertices[i * 3 + 1].position - pOutputVertices[i * 3 + 0].position;
        D3DXVECTOR3 delta_12 = pOutputVertices[i * 3 + 2].position - pOutputVertices[i * 3 + 1].position;
        D3DXVec3Cross(&normal, 
            &delta_01,
            &delta_12);
        for (DWORD j = 0; j != 3; ++j)
            pOutputVertices[i * 3 + j].normal = normal;
    }

    // Decode the edges from the adjacency information
    DWORD k = 0;
    for (DWORD i = 0; i != numFaces; ++i)
    {
        for (DWORD j = 0; j != 3; ++j)
        {
            DWORD i2 = pAdjacency[i * 3 + j]; // the other triangle
            if ((i2 > i) && (i2 < numFaces))
            {
                // Now find the matching edge in the other triangle
                for (DWORD j2 = 0; j2 != 3; ++j2)
                {
                    if (pAdjacency[i2 * 3 + j2] == i) // This is called exactly once in the j2 loop, or the adjacency information is not self-consistent
                    {
                        // The edges appear in the triangles in reverse order
                        pOutputIndices[k++] = i  * 3 + (j + 1) % 3;
                        pOutputIndices[k++] = i  * 3 + j;
                        pOutputIndices[k++] = i2 * 3 + j2;

                        pOutputIndices[k++] = i2 * 3 + j2;
                        pOutputIndices[k++] = i  * 3 + j;
                        pOutputIndices[k++] = i2 * 3 + (j2 + 1) % 3;
                    }
                }
            }
        }
    }

    pInputMesh->UnlockVertexBuffer();
    pInputMesh->UnlockIndexBuffer();
    pOutputMesh->UnlockVertexBuffer();
    pOutputMesh->UnlockIndexBuffer();

    
    pInputMesh->Release(); // Release the copy of the input mesh
    
    m_shadow = pOutputMesh;
    m_dirty_shadow = false;

    delete[] pPRep;
    delete[] pAdjacency;

    xlog << "compute_shadow_mesh: " << t.elapsed() * 1000 << " ms" << endl;
}

void mesh::update_shadow_mesh() const
{
    xlog << "Updating shadow mesh" << endl;
    // Called after deformation to copy positions and compute new normals
    vertex* pInputVertices = NULL;
        DWORD* pInputIndices = NULL;
        vertex* pOutputVertices = NULL;
        
        m_shadow->LockVertexBuffer(0, (void**) &pOutputVertices);
        m->LockIndexBuffer(0, (void**) &pInputIndices);
        m->LockVertexBuffer(0, (void**) &pInputVertices);
    
        int numFaces = m->GetNumFaces();
        for (int i = 0; i != numFaces; ++i)
        {
            // For each face, copy over the representative positions
            for (int j = 0; j != 3; ++j)
                pOutputVertices[i * 3 + j].position = pInputVertices[pInputIndices[i * 3 + j]].position;
            // Compute the normal for that triangle
            D3DXVECTOR3 normal;
            D3DXVECTOR3 delta_01 = pOutputVertices[i * 3 + 1].position - pOutputVertices[i * 3 + 0].position;
            D3DXVECTOR3 delta_12 = pOutputVertices[i * 3 + 2].position - pOutputVertices[i * 3 + 1].position;
            D3DXVec3Cross(&normal, &delta_01, &delta_12);
            for (int j = 0; j != 3; ++j)
                pOutputVertices[i * 3 + j].normal = normal;
        }
        m->UnlockVertexBuffer();
        m->UnlockIndexBuffer();
        m_shadow->UnlockVertexBuffer();

        m_dirty_shadow = false;

}
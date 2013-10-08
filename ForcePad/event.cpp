#include <cassert>

#include "event.h"

using namespace std;

set<D3D9Events*>& D3D9Events::registry()
{
    static set<D3D9Events*> r;
    return r;
}

D3D9Events::D3D9Events()
{
    auto p = registry().insert(this);
    assert(p.second); // assert that the pointer was not already stored
}

D3D9Events::~D3D9Events()
{
    size_t n = registry().erase(this);
    assert(n); // assert that the pointer was stored
}

void D3D9Events::OnLostDevice() 
{
}

void D3D9Events::OnResetDevice() 
{
}

void D3D9Events::OnResizeDevice(int /* w */, int /* h */) 
{
}

void D3D9Events::NotifyLostDevice()
{
    for (auto i = registry().begin(); i != registry().end(); ++i)
    {
        assert(*i);
        (*i)->OnLostDevice();
    }
}

void D3D9Events::NotifyResetDevice()
{
    for (auto i = registry().begin(); i != registry().end(); ++i)
    {
        assert(*i);
        (*i)->OnResetDevice();
    }
}

void D3D9Events::NotifyResizeDevice(int w, int h)
{
    for (auto i = registry().begin(); i != registry().end(); ++i)
    {
        assert(*i);
        (*i)->OnResizeDevice(w, h);
    }
}


#pragma once

#include <set>
#include "common.h"


class D3D9Events
{
public:
    D3D9Events();
    virtual ~D3D9Events();
    virtual void OnLostDevice();
    virtual void OnResetDevice();
    virtual void OnResizeDevice(int w, int h);
    static void NotifyLostDevice();
    static void NotifyResetDevice();
    static void NotifyResizeDevice(int w, int h);
protected:
    D3D9Events(const D3D9Events&);
    D3D9Events& operator=(const D3D9Events&);
private:
    static std::set<D3D9Events*>& registry();
};

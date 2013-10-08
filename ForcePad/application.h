#pragma once

#include <cstdio>
#include <exception>
#include <string>

#include "common.h"

#include "com_ptr.h"

class application
{
public:
    application();                      // Initialize the application
    virtual ~application();             // Shut down the application
    int operator()();                   // Run the application
    virtual void draw() const = 0;      // User hooks
    virtual void update(double dt) = 0;
    int width() const;                  // Accessors
    int height() const;

static com_ptr<IDirect3DDevice9> device;
static com_ptr<IDirect3D9> api;

private:
    class implementation;
    implementation* pimpl;
    application(const application&);    
    application& operator=(const application&);
};


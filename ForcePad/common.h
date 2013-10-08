#ifndef COMMON_H
#define COMMON_H

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#if defined(_DEBUG) & !defined(D3D_DEBUG_INFO)
#define D3D_DEBUG_INFO
#endif

#include <D3D9.h>
#include <D3DX9.h>

#pragma comment(lib, "D3D9.lib" )

#if defined(_DEBUG)
#pragma comment(lib, "d3dx9d.lib")
#else
#pragma comment(lib, "d3dx9.lib")
#endif

#include <DxErr.h>
#pragma comment(lib, "dxerr.lib" )

#pragma comment(lib, "SynCOM.lib")

// const double pi = 3.1415926535897932384626433832795;

#if defined(_DEBUG) | 1
#define CHECK(X) check_impl(__FILE__, __LINE__, X, #X);
HRESULT check_impl(char* file, int line, HRESULT hr, char* expr);
#else
#define CHECK(X) X
#endif


#include "xlog.h"


#endif // COMMON_H

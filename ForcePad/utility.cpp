#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "common.h"
#include "utility.h"
#include "xlog.h"

using namespace std;

std::ostream& operator<<(std::ostream& os, const D3DXVECTOR3& x)
{
    os << "[" << x.x << ", " << x.y << ", " << x.z << "]";
    return os;
}

ostream& operator<<(ostream& a, const D3DXMATRIX& b)
{
    float* c = (float*) &b;
    for (int i = 0; i != 16; ++i)
        a << c[i] << " ";
    return a;
}


char* loadfile(const char* filename)
{
    FILE* handle;
    int count, read_count;
    char* buffer;
    // handle = fopen(filename, "rt");
    fopen_s(&handle, filename, "rt");
    if (!handle)
    {
        perror(filename);
        return 0;
    }
    fseek(handle, 0, SEEK_END);
    count = ftell(handle);
    rewind(handle);
    buffer = (char*) malloc(count + 1);
    read_count = fread(buffer, 1, count, handle);
    buffer[read_count] = 0;
    fclose(handle);
    return buffer;
}

HRESULT check_impl(char* file, int line, HRESULT hr, char* expr)
{
    if (hr != D3D_OK)
    {
        xlog << file << ":" << line << ": " << hex << hr << " = " << expr << endl;
        // DXTrace(file, line, hr, expr, TRUE);
        if (FAILED(hr))
            abort();
    }
    return hr;
}

timer::timer()
{
    QueryPerformanceCounter(&t0);
}

double timer::elapsed() const
{
    LARGE_INTEGER t1;
    QueryPerformanceCounter(&t1);
    return (t1.QuadPart - t0.QuadPart) / frequency();
}

double timer::reset()
{
    LARGE_INTEGER t1 = t0;
    QueryPerformanceCounter(&t0);
    return (t0.QuadPart - t1.QuadPart) / frequency();
}

struct QPFWrapper
{
    double f;
    QPFWrapper()
    {
        LARGE_INTEGER i;
        QueryPerformanceFrequency(&i);
        f = (double) i.QuadPart;
    }
};

double timer::frequency()
{
    static QPFWrapper qpf;
    return qpf.f;
}

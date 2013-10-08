#ifndef UTILITY_H
#define UTILITY_H

#include "common.h"

char* loadfile(const char* filename);

// double timer();

class timer
{
public:
    timer();
    double elapsed() const;
    double reset();
    static double frequency();
private:
    LARGE_INTEGER t0;
};


std::ostream& operator<<(std::ostream& os, const D3DXVECTOR3& x);
#endif // UTILITY_H
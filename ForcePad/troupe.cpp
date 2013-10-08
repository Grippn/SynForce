#include <cassert>

#include "application.h"
#include "troupe.h"

using namespace std;

troupe::troupe()
{
}

troupe::~troupe()
{
}


void troupe::insert(const shared_ptr<actor>& p)
{
    assert(p);
    actors.push_back(p);
}

void troupe::draw(effect& fx, D3DXMATRIX mat) const
{
    D3DXMATRIX mat2 = local * mat;
    for (auto p = actors.begin(); p != actors.end(); ++p)
        (*p)->draw(fx, mat2);
}

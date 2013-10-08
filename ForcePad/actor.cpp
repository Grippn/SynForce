#include <cassert>
#include "application.h"
#include "actor.h"
#include "effect.h"

using namespace std;

actor::actor()
{
    D3DXMatrixIdentity(&local);
}

actor::~actor()
{
}




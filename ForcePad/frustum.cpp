#include "frustum.h"

frustum::frustum(D3DXVECTOR4 position, D3DXVECTOR4 look_at, D3DXVECTOR4 up, float fovy, float aspect, float znear, float zfar) 
    : position_(position), look_at_(look_at), up_(up), fovy_(fovy), aspect_(aspect), znear_(znear), zfar_(zfar)
{
    compute();
}

void frustum::compute()
{
    D3DXMatrixLookAtLH( &view_, (D3DXVECTOR3*) &position_, (D3DXVECTOR3*) &look_at_, (D3DXVECTOR3*) &up_ );
    D3DXMatrixPerspectiveFovLH( &proj_, fovy_, aspect_, znear_, zfar_ );
}

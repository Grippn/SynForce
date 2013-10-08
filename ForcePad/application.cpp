#include <iostream>

#include "application.h"
#include "utility.h"
#include "event.h"

com_ptr<IDirect3D9> application::api;
com_ptr<IDirect3DDevice9> application::device;

class application::implementation
{
public:
    static LRESULT CALLBACK window_procedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    explicit implementation(application*);
    ~implementation();
    int operator()();
    void draw() const;
    void update(double dt);
    int width() const;
    int height() const;

    bool isDeviceLost();

private:
    application* parent;
    HINSTANCE hInstance;
    char* szClassName; 
    HCURSOR hCursor;
    HWND ghWnd;
    
    int width_;
    int height_;
    implementation(const implementation&);
    implementation& operator=(const implementation&);

    D3DPRESENT_PARAMETERS d3dpp;
    bool paused;

};

bool application::implementation::isDeviceLost()
{
    HRESULT hr = application::device->TestCooperativeLevel();
    if (hr == D3DERR_DEVICELOST)
    {
        Sleep(20);
        return true;
    }
    else if (hr == D3DERR_DRIVERINTERNALERROR)
    {
        MessageBox(0, "Internal driver error.", 0, 0);
        PostQuitMessage(0);
        return true;
    }
    else if (hr == D3DERR_DEVICENOTRESET)
    {
        D3D9Events::NotifyLostDevice();
        // No resize involved
        CHECK( application::device->Reset(&d3dpp) ); // Will fail if all resources not released
        D3D9Events::NotifyResetDevice();
    }
    // Not lost (anymore)
    return false;
}

application::application()
{
    pimpl = new implementation(this);
}

application::~application()
{
    delete pimpl;
}

int application::operator()()
{
    return (*pimpl)();
}

int application::width() const
{
    return pimpl->width();
}

int application::height() const
{
    return pimpl->height();
}

using namespace std;
#define ensure(A) ensure_(A, #A);

void ensure_(bool eval, const char* str)
{
    if (!eval)
    {
        xlog << "FALSE: " << str << endl;
    }
}

application::implementation::implementation(application* p)
{
    paused = true;
    parent = p;
    width_ = 640;
    height_ = 480;
    szClassName =  "Synaptics ForcePad";
    hInstance = GetModuleHandle(NULL);
    hCursor = LoadCursor(NULL, IDC_CROSS);
    WNDCLASSEX wcx = { 
        sizeof(WNDCLASSEX),                     // cbSize
        CS_HREDRAW | CS_OWNDC | CS_VREDRAW,     // style
        window_procedure,                       // lpfnWndProc
        0,                                      // cbClsExtra
        sizeof(implementation*),                // cbWndExtra
        hInstance,                              // hInstance
        LoadIcon(NULL, IDI_APPLICATION),        // hIcon
        hCursor,                                // hCursor
        NULL,                                   // hbrBackground
        NULL,                                   // lpszMenuName
        szClassName,                            // lpszClassName
        NULL                                    // hIconSm
    };
    RegisterClassEx(&wcx);
    ghWnd = CreateWindowEx(
        WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,     // Extended style
        szClassName,                            // Class name
        szClassName,                            // Window name
        WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,    // Style
        100,                                   // X
        100,                                    // Y
        width_,                                 // Width
        height_,                                // Height
        NULL,                                   // Parent window
        NULL,                                   // Menu
        hInstance,                              // Instance
        this                                    // WM_CREATE parameter
        );
    SetWindowLongPtr(ghWnd, GWLP_USERDATA, (LONG_PTR) this);
    
    ensure( application::api = Direct3DCreate9(D3D_SDK_VERSION) );
    
    D3DDISPLAYMODE displayMode;
    CHECK( application::api->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode) );
    CHECK( application::api->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, displayMode.Format, displayMode.Format, true) );
    
    // Check usability of surface formats
    XSHOW( application::api->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, displayMode.Format,
                                         D3DUSAGE_DEPTHSTENCIL,
                                         D3DRTYPE_SURFACE,
                                         D3DFMT_D24S8) );
    XSHOW( application::api->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, displayMode.Format,
                                         D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_RENDERTARGET,
                                         D3DRTYPE_TEXTURE,
                                         displayMode.Format) );
    XSHOW( application::api->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, displayMode.Format,
                                         D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_RENDERTARGET,
                                         D3DRTYPE_TEXTURE,
                                         D3DFMT_A2R10G10B10) );
    XSHOW( application::api->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, displayMode.Format,
                                         D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_RENDERTARGET,
                                         D3DRTYPE_TEXTURE,
                                         D3DFMT_R32F) );
    XSHOW( application::api->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, displayMode.Format,
                                         D3DUSAGE_RENDERTARGET,
                                         D3DRTYPE_SURFACE,
                                         D3DFMT_G32R32F) );
    XSHOW( application::api->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, displayMode.Format,
                                         D3DUSAGE_RENDERTARGET,
                                         D3DRTYPE_TEXTURE,
                                         D3DFMT_G32R32F) );
    XSHOW( application::api->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, displayMode.Format,
                                         D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_RENDERTARGET,
                                         D3DRTYPE_TEXTURE,
                                         D3DFMT_G32R32F) );


    // Check compatibility between render target and depth formats
    XSHOW( application::api->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 
        D3DFMT_X8R8G8B8, 
        D3DFMT_D24S8) );
    XSHOW( application::api->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, displayMode.Format, 
        displayMode.Format, 
        D3DFMT_D24S8) ); 
    XSHOW (application::api->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, displayMode.Format, 
        D3DFMT_G32R32F, 
        D3DFMT_D24S8) );
    XSHOW (application::api->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, displayMode.Format, 
        D3DFMT_R32F, 
        D3DFMT_D24S8) );
    XSHOW (application::api->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, displayMode.Format, 
        D3DFMT_A2R10G10B10, 
        D3DFMT_D24S8) );

    

    D3DCAPS9 caps;
    CHECK( application::api->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps) );
    xlog << "Vertex shader vs_" << ((caps.VertexShaderVersion >> 8) & 0xFF) << "_" << (caps.VertexShaderVersion & 0xFF) << endl;
    xlog << "Pixel  shader ps_" << ((caps.PixelShaderVersion >> 8) & 0xFF) << "_" << (caps.PixelShaderVersion & 0xFF) << endl;
    xlog << "D3DDEVCAPS_HWTRANSFORMANDLIGHT " << !!(caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) << endl;
    xlog << "D3DPRESENT_INTERVAL_ONE " << !!(caps.PresentationIntervals & D3DPRESENT_INTERVAL_ONE) << endl;
    xlog << "D3DPRESENT_INTERVAL_IMMEDIATE " << !!(caps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE) << endl;
    xlog << "D3DCAPS2_CANAUTOGENMIPMAP " << !!(caps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP) << endl;

    ensure(!!(caps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP)); 

    DWORD vertexProcessing = (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) 
        ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    //UINT presentationInterval = (caps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE)
    //    ? D3DPRESENT_INTERVAL_IMMEDIATE : D3DPRESENT_INTERVAL_DEFAULT;
    UINT presentationInterval = D3DPRESENT_INTERVAL_ONE;
     
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed   = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    d3dpp.PresentationInterval = presentationInterval;
    CHECK(
        application::api->CreateDevice(
        D3DADAPTER_DEFAULT, 
        D3DDEVTYPE_HAL, 
        ghWnd, 
        vertexProcessing, 
        &d3dpp,
        application::device)
        );
}

int application::implementation::operator()()
{
    timer t;
    MSG msg;
    ShowWindow(ghWnd, SW_SHOW);
    SetForegroundWindow(ghWnd);
    SetFocus(ghWnd);

    for(;;)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                return (int) msg.wParam;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if (!paused)
            {
                if (!isDeviceLost())
                {
                    update(t.reset());
                    draw();
                }
            }
            else
            {
                Sleep(20);
            }
        }
        Sleep(0);
    }
}

LRESULT CALLBACK application::implementation::window_procedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    implementation* self = (implementation*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
    static bool minOrMaxed = false;
    switch(uMsg)
    {
    case WM_ACTIVATE:
        //self->paused = (wParam == WA_INACTIVE);
        self->paused = false;
        return 0;
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
        {
            self->paused = true;
            minOrMaxed = true;
        }
        else if (wParam == SIZE_MAXIMIZED)
        {
            self->paused = false;
            minOrMaxed = true;
            self->d3dpp.BackBufferWidth = LOWORD(lParam);
            self->d3dpp.BackBufferHeight = HIWORD(lParam);
            D3D9Events::NotifyLostDevice();
            D3D9Events::NotifyResizeDevice(self->d3dpp.BackBufferWidth, self->d3dpp.BackBufferHeight);
            CHECK( application::device->Reset(&self->d3dpp) );
            D3D9Events::NotifyResetDevice();
        }
        else if (wParam == SIZE_RESTORED)
        {
            self->paused = false;
            if (minOrMaxed)
            {
                self->d3dpp.BackBufferWidth = LOWORD(lParam);
                self->d3dpp.BackBufferHeight = HIWORD(lParam);
                D3D9Events::NotifyLostDevice();
                D3D9Events::NotifyResizeDevice(self->d3dpp.BackBufferWidth, self->d3dpp.BackBufferHeight);
                CHECK( application::device->Reset(&self->d3dpp) );
                D3D9Events::NotifyResetDevice();
            }
            minOrMaxed = false;
        }
        self->draw();
        return 0;
    case WM_EXITSIZEMOVE:
        // resize the back buffer
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        self->d3dpp.BackBufferWidth = clientRect.right;
        self->d3dpp.BackBufferHeight = clientRect.bottom;
        D3D9Events::NotifyLostDevice();
        D3D9Events::NotifyResizeDevice(self->d3dpp.BackBufferWidth, self->d3dpp.BackBufferHeight);
        CHECK( application::device->Reset(&self->d3dpp) );
        D3D9Events::NotifyResetDevice();
        return 0;
    case WM_PAINT:
        self->draw();
        break;
    //case WM_MOVE:
    //    self->draw();
    //    return 0;
    case WM_CLOSE:
        DestroyWindow(hWnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_KEYUP:
        if (wParam == VK_ESCAPE)
            PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int application::implementation::width() const
{
    return d3dpp.BackBufferWidth;
}

int application::implementation::height() const
{
    return d3dpp.BackBufferHeight;
}

void application::implementation::draw() const
{
    CHECK( application::device->BeginScene() );
    parent->draw();
    CHECK( application::device->EndScene() );
    CHECK( application::device->Present(NULL, NULL, NULL, NULL) );
}

void application::implementation::update(double dt)
{
    parent->update(dt);
}

application::implementation::~implementation()
{
    DestroyWindow(ghWnd);
    UnregisterClass(szClassName, hInstance);
}


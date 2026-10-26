// [Blur1PCDX11] Native Win32 D3D11 game device and DXGI swapchain.
#include "stdafx.h"
#include "Display/Device/D3D11/PCD3D11GameDevice.h"
#include "Display/Device/D3D11/D3D11Formats.h"
#include <system/debug/Blur2PCAutomation.h>
#include "Display/Render/Render.h"
#include "Display/Utils/Screenshot.h"
#include <d3d11sdklayers.h>
#include <d3d11_4.h>
#include <dxgi1_5.h>
#include <vector>
#include <string>
#include "Display/Device/D3D11/D3D11HDROutput.h"
#include <DirectXPackedVector.h> // [Blur2PC] recorder colour conversion
#include <algorithm>
#include <cmath>
// [Blur1PCDX11] Supplied by fx, without introducing a display header dependency.
namespace FX { const char* NativeFX11CurrentPass(); }
namespace Display { namespace D3D11 {
namespace
{
ID3D11Device* g_ExceptionDevice = nullptr;
// [Blur2PC][HDR] Existing debug-variable UI/persistence supplies live calibration.
Debug::Bool g_HDROutputEnable("Graphics.Render.HDR.Display.Enable", true, Debug::Persist());
Debug::Float g_HDROutputPeak("Graphics.Render.HDR.Display.PeakNits", 0.0f, Debug::Persist());
Debug::Float g_HDROutputWhite("Graphics.Render.HDR.Display.PaperWhiteNits", 0.0f, Debug::Persist());

// [Blur1PCDX11] The D3D11 debug layer raises 0x87D on the corruption it finds and the
// message text only exists inside its queue, so log it before the crash handler runs.
LONG CALLBACK LogNativeException(EXCEPTION_POINTERS* info)
{
    if (!g_ExceptionDevice || !info || !info->ExceptionRecord) return EXCEPTION_CONTINUE_SEARCH;
    // [Blur1PCDX11] Report distinct failures; one repeating shader pair must
    // not consume the diagnostic budget before the race starts rendering.
    static std::vector<std::string> reported;
    ID3D11DeviceContext* context = nullptr;
    g_ExceptionDevice->GetImmediateContext(&context);
    ID3D11VertexShader* vertex = nullptr;
    ID3D11PixelShader* pixel = nullptr;
    if (context) { context->VSGetShader(&vertex, nullptr, nullptr); context->PSGetShader(&pixel, nullptr, nullptr); }
    ID3D11InfoQueue* queue = nullptr;
    if (SUCCEEDED(g_ExceptionDevice->QueryInterface(__uuidof(ID3D11InfoQueue), reinterpret_cast<void**>(&queue))) && queue)
    {
        const UINT64 count = queue->GetNumStoredMessages();
        for (UINT64 cursor = 0; cursor < count && reported.size() < 40; ++cursor)
        {
            SIZE_T size = 0;
            queue->GetMessage(cursor, nullptr, &size);
            std::vector<U8> bytes(size);
            D3D11_MESSAGE* message = reinterpret_cast<D3D11_MESSAGE*>(bytes.data());
            if (SUCCEEDED(queue->GetMessage(cursor, message, &size)))
            {
                char prefix[80] = {};
                sprintf_s(prefix, "%p/%p/%u/", vertex, pixel, static_cast<unsigned>(message->ID));
                const std::string key = std::string(prefix) + message->pDescription;
                bool duplicate = false;
                for (const auto& previous : reported) duplicate |= previous == key;
                if (duplicate) continue;
                Blur2PCAutomation::LogF("DX11 queued sev=%u id=%d VS=%p PS=%p: %s",
                    static_cast<unsigned>(message->Severity), static_cast<int>(message->ID),
                    vertex, pixel, message->pDescription);
                // [Blur1PCDX11] Identify material permutations behind runtime binding failures.
                Blur2PCAutomation::LogF("DX11 queued current FX: %s", FX::NativeFX11CurrentPass());
                reported.push_back(key);
            }
        }
        queue->ClearStoredMessages();
        queue->Release();
    }
    if (context) { if (vertex) vertex->Release(); if (pixel) pixel->Release(); context->Release(); }
    return EXCEPTION_CONTINUE_SEARCH;
}
}
C_PCD3D11GameDevice::C_PCD3D11GameDevice() : m_Lost(false),m_Valid(false),m_pSwapChain(nullptr),
    m_HDROutput(nullptr),
    m_BeginVerticesUsage(0),m_MaxBeginVerticesUsage(0),m_UserBeginVerticesUsage(0),
    m_UserMaxBeginVerticesUsage(0),m_BeginVerticesLocked(false)
{
#ifndef CFG_GOLD
    m_BeginVerticesStart=nullptr;m_BeginVerticesSize=0;
#endif
    SetDepthFlip(true,false);
}
C_PCD3D11GameDevice::~C_PCD3D11GameDevice() { delete m_HDROutput; }
void C_PCD3D11GameDevice::BindHDROutputConstants()
{
    if (!m_HDROutput) return;
    ID3D11DeviceContext* context = nullptr; m_pd3dDevice->GetImmediateContext(&context);
    m_HDROutput->Bind(context); context->Release();
}
namespace {
// [Blur1PCDX11] D3D9 presents a windowed swap chain with PRESENT_INTERVAL_IMMEDIATE
// at any rate. A DXGI blt-model (DISCARD) windowed swap chain is composed by DWM
// once per display refresh, which caps the game at the monitor rate (measured:
// exactly 144 fps at both 1280x720 and 1920x1080 on a 144 Hz display, where
// PC D3D9 runs at 540-570). The flip model with tearing support restores
// unthrottled immediate presentation; the blt model remains the fallback.
bool g_legacySwapChain = false;
bool TearingSupported()
{
    IDXGIFactory5* factory = nullptr;
    if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory5), reinterpret_cast<void**>(&factory))) || !factory)
        return false;
    BOOL allowed = FALSE;
    const bool supported = SUCCEEDED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
        &allowed, sizeof(allowed))) && allowed;
    factory->Release();
    return supported;
}
// [Blur1PCDX11] Full screen changes the display mode the way D3D9 exclusive mode does, and the
// swap chain stays a DXGI windowed chain in a popup covering the monitor. DXGI full screen cannot
// reproduce the D3D9 mode: FindClosestMatchingMode/ResizeTarget turn an unspecified scaling into
// the STRETCHED variant of a mode whose aspect differs from the panel's, which on the measured
// NVIDIA/3840x2160 output kept the desktop mode and stretched a 3840x1080 buffer to 16:9.
// D3D9 (and ChangeDisplaySettingsEx) set 3840x1080 with the driver's default scaling,
// DISPLAYCONFIG_SCALING_ASPECTRATIOCENTEREDMAX there (probes in _work/uw_probe).
struct S_FullscreenMode
{
    HWND m_Window;
    WNDPROC m_WindowProc;
    char m_Device[CCHDEVICENAME];
    DEVMODEA m_Mode;
    bool m_Wanted;
    bool m_Applied;
};
S_FullscreenMode g_FullscreenMode = {};
void RestoreDesktopMode()
{
    if (!g_FullscreenMode.m_Applied) return;
    ChangeDisplaySettingsExA(g_FullscreenMode.m_Device, nullptr, nullptr, 0, nullptr);
    g_FullscreenMode.m_Applied = false;
}
void ApplyFullscreenMode()
{
    if (!g_FullscreenMode.m_Wanted || g_FullscreenMode.m_Applied) return;
    const LONG result = ChangeDisplaySettingsExA(g_FullscreenMode.m_Device, &g_FullscreenMode.m_Mode, nullptr,
        CDS_FULLSCREEN, nullptr);
    g_FullscreenMode.m_Applied = result == DISP_CHANGE_SUCCESSFUL;
    MONITORINFO info = {sizeof(info)};
    GetMonitorInfoA(MonitorFromWindow(g_FullscreenMode.m_Window, MONITOR_DEFAULTTONEAREST), &info);
    SetWindowPos(g_FullscreenMode.m_Window, HWND_TOP, info.rcMonitor.left, info.rcMonitor.top,
        g_FullscreenMode.m_Mode.dmPelsWidth, g_FullscreenMode.m_Mode.dmPelsHeight, SWP_NOACTIVATE | SWP_NOCOPYBITS);
    Blur2PCAutomation::LogF("DX11 full screen mode %s %ux%u@%u result %d", g_FullscreenMode.m_Device,
        (unsigned)g_FullscreenMode.m_Mode.dmPelsWidth, (unsigned)g_FullscreenMode.m_Mode.dmPelsHeight,
        (unsigned)g_FullscreenMode.m_Mode.dmDisplayFrequency, (int)result);
}
// D3D9 exclusive mode returns the desktop mode and minimises the window when the application loses
// activation, and sets the mode again when it is reactivated.
LRESULT CALLBACK FullscreenWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    const WNDPROC next = g_FullscreenMode.m_WindowProc;
    if (message == WM_ACTIVATEAPP && g_FullscreenMode.m_Wanted)
    {
        if (wParam) { if (IsIconic(window)) ShowWindow(window, SW_RESTORE); ApplyFullscreenMode(); }
        else if (g_FullscreenMode.m_Applied) { RestoreDesktopMode(); ShowWindow(window, SW_MINIMIZE); }
    }
    return CallWindowProcA(next, window, message, wParam, lParam);
}
void SetFullscreenMode(HWND window, bool fullscreen, U32 width, U32 height, U32 refreshRate)
{
    if (!g_FullscreenMode.m_WindowProc)
    {
        g_FullscreenMode.m_Window = window;
        g_FullscreenMode.m_WindowProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(window, GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(FullscreenWindowProc)));
    }
    if (!fullscreen) { g_FullscreenMode.m_Wanted = false; RestoreDesktopMode(); return; }
    if (!g_FullscreenMode.m_Applied)
    {
        MONITORINFOEXA info = {};
        info.cbSize = sizeof(info);
        GetMonitorInfoA(MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST), &info);
        memcpy(g_FullscreenMode.m_Device, info.szDevice, sizeof(g_FullscreenMode.m_Device));
    }
    DEVMODEA& mode = g_FullscreenMode.m_Mode;
    ZeroMemory(&mode, sizeof(mode));
    mode.dmSize = sizeof(mode);
    mode.dmPelsWidth = width;
    mode.dmPelsHeight = height;
    mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
    if (refreshRate) { mode.dmDisplayFrequency = refreshRate; mode.dmFields |= DM_DISPLAYFREQUENCY; }
    g_FullscreenMode.m_Wanted = true;
    g_FullscreenMode.m_Applied = false;
    ApplyFullscreenMode();
}
void ReleaseFullscreenMode()
{
    g_FullscreenMode.m_Wanted = false;
    RestoreDesktopMode();
    if (g_FullscreenMode.m_WindowProc && IsWindow(g_FullscreenMode.m_Window))
        SetWindowLongPtrA(g_FullscreenMode.m_Window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_FullscreenMode.m_WindowProc));
    g_FullscreenMode.m_WindowProc = nullptr;
}
}
void C_PCD3D11GameDevice::SetPresentParamsD3D11()
{
    ZeroMemory(&m_d3dPresentParams,sizeof(m_d3dPresentParams));
    m_d3dPresentParams.BufferDesc.Width=m_DeviceParams.m_Width;
    m_d3dPresentParams.BufferDesc.Height=m_DeviceParams.m_Height;
    m_d3dPresentParams.BufferDesc.Format=DXGI_FORMAT_B8G8R8A8_UNORM;
    if (m_HDROutput && m_HDROutput->FloatSwapChain()) // [Blur2PC][HDR]
        m_d3dPresentParams.BufferDesc.Format=DXGI_FORMAT_R16G16B16A16_FLOAT;
    m_d3dPresentParams.SampleDesc.Count=1;
    m_d3dPresentParams.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;
    m_d3dPresentParams.BufferCount=1;
    m_d3dPresentParams.OutputWindow=m_DeviceParams.m_hWnd;
    m_d3dPresentParams.Windowed=TRUE;    // full screen is a display mode and a covering popup (SetFullscreenMode)
    m_d3dPresentParams.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
    if(!g_legacySwapChain)
    {
        static const bool tearing=TearingSupported();
        m_d3dPresentParams.BufferCount=2;
        m_d3dPresentParams.SwapEffect=DXGI_SWAP_EFFECT_FLIP_DISCARD;
        m_d3dPresentParams.Flags=tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    }
}
bool C_PCD3D11GameDevice::CreateD3D11()
{
    // [Blur1PCDX11] The swap chain is always created windowed; full screen sets the display mode
    // first (SetFullscreenMode). The blt model remains the fallback when flip creation fails, and
    // is kept across device resets because ResizeBuffers cannot change the presentation model.
    SetFullscreenMode(m_DeviceParams.m_hWnd, !m_DeviceParams.m_Windowed, m_DeviceParams.m_Width,
        m_DeviceParams.m_Height, m_DeviceParams.m_RefreshRate);
    delete m_HDROutput; m_HDROutput = new C_HDROutput(m_DeviceParams.m_hWnd);
    g_legacySwapChain = false;
    SetPresentParamsD3D11();
    char debugSetting[2] = {};
    UINT flags=D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    if (GetEnvironmentVariableA("BLUR_DX11_DEBUG", debugSetting, sizeof(debugSetting)) &&
        debugSetting[0] == '1') flags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL features[]={D3D_FEATURE_LEVEL_11_1,D3D_FEATURE_LEVEL_11_0,D3D_FEATURE_LEVEL_10_1,D3D_FEATURE_LEVEL_10_0};
    D3D_FEATURE_LEVEL selected={};ID3D11DeviceContext* context=nullptr;
    char warpSetting[2] = {};
    const bool forceWarp = GetEnvironmentVariableA("BLUR_DX11_WARP", warpSetting, sizeof(warpSetting)) &&
        warpSetting[0] == '1';
    D3D_DRIVER_TYPE driver=(m_DeviceParams.m_UseREF || forceWarp) ? D3D_DRIVER_TYPE_WARP : D3D_DRIVER_TYPE_HARDWARE;
    HRESULT hr=D3D11CreateDeviceAndSwapChain(nullptr,driver,nullptr,flags,features,4,D3D11_SDK_VERSION,
        &m_d3dPresentParams,&m_pSwapChain,&m_pd3dDevice,&selected,&context);
    if (FAILED(hr) && !g_legacySwapChain)
    {
        Blur2PCAutomation::LogF("DX11 flip-model swap chain failed %08x; using the blt model", static_cast<unsigned>(hr));
        g_legacySwapChain = true;
        m_HDROutput->FallBack(); // [Blur2PC][HDR] Preserve SDR on older DXGI implementations.
        SetPresentParamsD3D11();
        hr=D3D11CreateDeviceAndSwapChain(nullptr,driver,nullptr,flags,features,4,D3D11_SDK_VERSION,
            &m_d3dPresentParams,&m_pSwapChain,&m_pd3dDevice,&selected,&context);
    }
    if (hr == DXGI_ERROR_SDK_COMPONENT_MISSING)
    {
        flags &= ~D3D11_CREATE_DEVICE_DEBUG;
        hr=D3D11CreateDeviceAndSwapChain(nullptr,driver,nullptr,flags,features,4,D3D11_SDK_VERSION,
            &m_d3dPresentParams,&m_pSwapChain,&m_pd3dDevice,&selected,&context);
    }
    if(hr==E_INVALIDARG)
        hr=D3D11CreateDeviceAndSwapChain(nullptr,driver,nullptr,flags,features+1,3,D3D11_SDK_VERSION,
            &m_d3dPresentParams,&m_pSwapChain,&m_pd3dDevice,&selected,&context);
    if(FAILED(hr) && driver==D3D_DRIVER_TYPE_HARDWARE)
        hr=D3D11CreateDeviceAndSwapChain(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,flags,features+1,3,D3D11_SDK_VERSION,
            &m_d3dPresentParams,&m_pSwapChain,&m_pd3dDevice,&selected,&context);
    // [Blur1PCDX11] The engine uploads textures and buffers from its loading thread while the
    // main thread renders; D3D9 serialised that internally and D3D11 needs the same guarantee.
    if (m_pd3dDevice)
    {
        ID3D11DeviceContext* immediate = nullptr;
        m_pd3dDevice->GetImmediateContext(&immediate);
        if (immediate)
        {
            ID3D11Multithread* multithread = nullptr;
            if (SUCCEEDED(immediate->QueryInterface(__uuidof(ID3D11Multithread),
                reinterpret_cast<void**>(&multithread))) && multithread)
            {
                multithread->SetMultithreadProtected(TRUE);
                multithread->Release();
            }
            immediate->Release();
        }
    }
    if(context)context->Release();
    if(FAILED(hr)) {LOGERROR(g_Log,"D3D11CreateDeviceAndSwapChain failed 0x%08x\n",hr);return false;}
    Blur2PCAutomation::LogF("DX11 device created debug=%u warp=%u feature=%x", (flags & D3D11_CREATE_DEVICE_DEBUG) != 0,
        driver == D3D_DRIVER_TYPE_WARP, static_cast<unsigned>(selected));
    g_ExceptionDevice = m_pd3dDevice;
    AddVectoredExceptionHandler(1, LogNativeException);
    if (flags & D3D11_CREATE_DEVICE_DEBUG)
    {
        // [Blur1PCDX11] Keep errors and corruption; warnings such as an unbound render
        // target flood the queue and each one raises under a debugger's default settings.
        ID3D11InfoQueue* queue = nullptr;
        if (SUCCEEDED(m_pd3dDevice->QueryInterface(__uuidof(ID3D11InfoQueue), reinterpret_cast<void**>(&queue))) && queue)
        {
            D3D11_MESSAGE_SEVERITY denied[] = {D3D11_MESSAGE_SEVERITY_INFO,
                D3D11_MESSAGE_SEVERITY_MESSAGE, D3D11_MESSAGE_SEVERITY_WARNING};
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumSeverities = 3;
            filter.DenyList.pSeverityList = denied;
            queue->AddStorageFilterEntries(&filter);
            queue->Release();
        }
    }
    m_pd3dObject=m_DeviceParams.m_pd3dObject;
    if(m_pd3dObject)m_pd3dObject->AddRef();
    return true;
}
bool C_PCD3D11GameDevice::CreateDevice()
{
    if(!CreateD3D11())return false;
    Display2::Device(&m_Display2);
    Display2::C_DeviceParams params;params.m_pd3dDevice=m_pd3dDevice;params.m_pd3dObject=m_pd3dObject;
    m_Display2.SetParams(params);
    if(!m_Display2.Create() || !m_Display2.CreateVolatile())return false;
    C_BackBuffer* back=new C_BackBuffer();
    back->m_Width=m_DeviceParams.m_Width;back->m_Height=m_DeviceParams.m_Height;
    back->m_ColourFormat=m_HDROutput->FloatSwapChain() ? E_Format_A16B16G16R16F : m_DeviceParams.m_ColourFormat;
    back->m_DepthStencilFormat=m_DeviceParams.m_DepthStencilFormat;
    back->m_Multisample=E_Multisample_None;
    HRESULT outputResult = m_HDROutput->CreateTargets(m_pd3dDevice, m_pSwapChain,
        reinterpret_cast<ID3D11Texture2D**>(&back->m_ColourSurface.m_pd3dSurface));
    if (FAILED(outputResult) && m_HDROutput->FloatSwapChain())
    {
        Blur2PCAutomation::LogF("HDR output unavailable %08x; falling back to SDR", (unsigned)outputResult);
        m_HDROutput->FallBack(); SetPresentParamsD3D11();
        outputResult = m_pSwapChain->ResizeBuffers(m_d3dPresentParams.BufferCount, back->m_Width, back->m_Height,
            m_d3dPresentParams.BufferDesc.Format, m_d3dPresentParams.Flags);
        if (SUCCEEDED(outputResult)) outputResult = m_HDROutput->CreateTargets(m_pd3dDevice, m_pSwapChain,
            reinterpret_cast<ID3D11Texture2D**>(&back->m_ColourSurface.m_pd3dSurface));
        back->m_ColourFormat = m_DeviceParams.m_ColourFormat;
    }
    if(FAILED(outputResult)) {delete back;return false;}
    m_BackBuffers.PushBack(back);
    SetAspectRatio((F32)back->m_Width/(F32)back->m_Height);    // [Blur1PCDX11] as PCD3D9Device (T254)
    if(!m_Display2.CreateDepthStencilSurface(back->m_Width,back->m_Height,E_Usage_DepthStencil,
        static_cast<Display2::E_SurfaceFormat>(back->m_DepthStencilFormat),Display2::E_Multisample_None,nullptr,
        &back->m_DepthStencilSurface))return false;
    InitialiseBuffers();
    if(!C_D3D11GameDevice::CreateDevice())return false;
    m_ShaderProfile=E_ShaderProfile_3_0;
    m_VertexShaderProfile=E_VertexShaderProfile_3_0;
    m_PixelShaderProfile=E_PixelShaderProfile_3_0;
    if(!m_DynamicVertexBuffer.Create(m_DeviceParams.m_BeginVerticesSize))return false;
    m_MaxBeginVerticesUsage=m_UserMaxBeginVerticesUsage=
        m_DeviceParams.m_BeginVerticesSize-m_DeviceParams.m_FreeBeginVerticesSize;
    SetColourDepthStencilSurface();InvalidateState();DefaultStates();ResizeWindows();
    m_Valid=true;return true;
}
// [Blur1PCDX11] The counterpart of PCD3D9Device::CreateDeviceVolatile/DestroyDeviceVolatile. The
// application applies the configured resolution, window mode and multisampling through ResetDevice
// (DestroyVolatile, then CreateVolatile with the new parameters); D3D9 resets its device there. Without
// this the swap chain, back buffer and window targets kept their creation-time size.
bool C_PCD3D11GameDevice::CreateDeviceVolatile()
{
    if(!m_pSwapChain || !m_BackBuffers.Size())return false;
    C_BackBuffer* back=m_BackBuffers[0];
    if(back->m_ColourSurface.m_pd3dSurface)return true;    // created by CreateDevice, not yet destroyed
    SetPresentParamsD3D11();
    // D3D9 Reset switches the display mode with the back buffer.
    SetFullscreenMode(m_DeviceParams.m_hWnd,!m_DeviceParams.m_Windowed,m_DeviceParams.m_Width,
        m_DeviceParams.m_Height,m_DeviceParams.m_RefreshRate);
    HRESULT hr=m_pSwapChain->ResizeBuffers(m_d3dPresentParams.BufferCount,m_DeviceParams.m_Width,m_DeviceParams.m_Height,
        m_d3dPresentParams.BufferDesc.Format,m_d3dPresentParams.Flags);
    if(FAILED(hr)){Blur2PCAutomation::LogF("DX11 ResizeBuffers %ux%u failed %08x",m_DeviceParams.m_Width,m_DeviceParams.m_Height,(unsigned)hr);return false;}
    back->m_Width=m_DeviceParams.m_Width;back->m_Height=m_DeviceParams.m_Height;
    back->m_ColourFormat=m_HDROutput->FloatSwapChain() ? E_Format_A16B16G16R16F : m_DeviceParams.m_ColourFormat;
    back->m_DepthStencilFormat=m_DeviceParams.m_DepthStencilFormat;
    back->m_Multisample=E_Multisample_None;
    SetAspectRatio((F32)back->m_Width/(F32)back->m_Height);    // [Blur1PCDX11] as PCD3D9Device (T254)
    if(FAILED(m_HDROutput->CreateTargets(m_pd3dDevice, m_pSwapChain,
        reinterpret_cast<ID3D11Texture2D**>(&back->m_ColourSurface.m_pd3dSurface))))return false;
    if(!m_Display2.CreateDepthStencilSurface(back->m_Width,back->m_Height,E_Usage_DepthStencil,
        static_cast<Display2::E_SurfaceFormat>(back->m_DepthStencilFormat),Display2::E_Multisample_None,nullptr,
        &back->m_DepthStencilSurface))return false;
    Blur2PCAutomation::LogF("DX11 back buffer reset %ux%u windowed=%d windows %ux%u multisample=%d",
        back->m_Width,back->m_Height,(int)m_DeviceParams.m_Windowed,m_DeviceParams.m_WindowBufferWidth,
        m_DeviceParams.m_WindowBufferHeight,(int)m_DeviceParams.m_WindowBufferMultisample);
    SetColourDepthStencilSurface();InvalidateState();DefaultStates();ResizeWindows();
    m_Valid=true;m_Lost=false;return true;
}
void C_PCD3D11GameDevice::DestroyDeviceVolatile()
{
    DestroyWindows();
    // ResizeBuffers requires every reference to the swap chain's buffer to be released.
    ID3D11DeviceContext* context=nullptr;
    if(m_pd3dDevice)m_pd3dDevice->GetImmediateContext(&context);
    if(context){context->OMSetRenderTargets(0,nullptr,nullptr);context->Flush();context->Release();}
    if (m_HDROutput) m_HDROutput->ReleaseTargets(); // [Blur2PC][HDR]
    for(S32 i=0;i<m_BackBuffers.Size();++i)
    {m_Display2.DestroySurface(&m_BackBuffers[i]->m_DepthStencilSurface);
     m_Display2.DestroySurface(&m_BackBuffers[i]->m_ColourSurface);}
}
void C_PCD3D11GameDevice::DestroyDevice()
{
    if (m_HDROutput) { delete m_HDROutput; m_HDROutput = nullptr; }
    ClearWindows();m_DynamicVertexBuffer.Destroy();
    for(S32 i=0;i<m_BackBuffers.Size();++i)
    {m_Display2.DestroySurface(&m_BackBuffers[i]->m_DepthStencilSurface);
     m_Display2.DestroySurface(&m_BackBuffers[i]->m_ColourSurface);delete m_BackBuffers[i];}
    m_BackBuffers.Clear();m_Display2.Destroy();
    if(m_pSwapChain){m_pSwapChain->Release();m_pSwapChain=nullptr;}
    if(m_pd3dDevice){m_pd3dDevice->Release();m_pd3dDevice=nullptr;}
    if(m_pd3dObject){m_pd3dObject->Release();m_pd3dObject=nullptr;}
    ReleaseFullscreenMode();
}
extern unsigned g_dx11TraceFrame; // [Blur1PCDX11] Temporary, defined in D3D11GameDeviceNative.cpp.
// [Blur2PC] Video recorder for the automation `record` command (system/debug/Blur2PCAutomation.h),
// the native counterpart of the D3D9 device's: consecutive back buffers are copied to a ring of
// staging textures, read back two frames later (the CPU never waits on the copy), reduced to half
// size, converted to 8-bit sRGB (scRGB back buffers use the screenshot's paper-white mapping) and
// streamed to ffmpeg by a writer thread as <prefix>.mp4. No per-frame image files (AGENTS.md rule 12).
namespace
{
	struct C_D3D11Recorder
	{
		enum { k_Ring = 3 };
		struct Frame { Frame* next; unsigned char* data; };

		ID3D11Texture2D*	staging[k_Ring];
		ID3D11Texture2D*	resolve;
		DXGI_FORMAT			format;
		UINT				srcWidth, srcHeight, width, height;
		bool				scRGB;
		float				paperWhite;
		int					total, copied, read;
		HANDLE				process, pipe, thread, wake;
		CRITICAL_SECTION	lock;
		Frame*				head;
		Frame*				tail;
		bool				closing;
		FILE*				times;

		static DWORD WINAPI Writer(LPVOID p)
		{
			C_D3D11Recorder& r = *(C_D3D11Recorder*)p;
			const DWORD bytes = r.width * r.height * 4;
			for (;;)
			{
				EnterCriticalSection(&r.lock);
				Frame* f = r.head;
				if (f) { r.head = f->next; if (!r.head) r.tail = NULL; }
				const bool closing = r.closing;
				LeaveCriticalSection(&r.lock);
				if (!f)
				{
					if (closing) break;
					WaitForSingleObject(r.wake, 100);
					continue;
				}
				DWORD written = 0;
				if (r.pipe) WriteFile(r.pipe, f->data, bytes, &written, NULL);
				free(f->data);
				delete f;
			}
			if (r.pipe) CloseHandle(r.pipe);
			r.pipe = NULL;
			DWORD code = (DWORD)-1;
			if (r.process)
			{
				WaitForSingleObject(r.process, 120000);
				GetExitCodeProcess(r.process, &code);
				CloseHandle(r.process);
				r.process = NULL;
			}
			Blur2PCAutomation::LogF("recording written, ffmpeg exit code %d", (int)code);
			Blur2PCAutomation::RecordingDone();
			return 0;
		}

		bool Start(ID3D11Device* device, ID3D11Texture2D* back, bool floatScRGB, float paperWhiteScale, const char* prefix, int frames)
		{
			memset(this, 0, sizeof(*this));
			D3D11_TEXTURE2D_DESC desc = {};
			back->GetDesc(&desc);
			format = desc.Format;
			if (format == DXGI_FORMAT_B8G8R8A8_TYPELESS) format = DXGI_FORMAT_B8G8R8A8_UNORM;
			if (format == DXGI_FORMAT_R8G8B8A8_TYPELESS) format = DXGI_FORMAT_R8G8B8A8_UNORM;
			if (format != DXGI_FORMAT_B8G8R8A8_UNORM && format != DXGI_FORMAT_B8G8R8A8_UNORM_SRGB &&
				format != DXGI_FORMAT_R8G8B8A8_UNORM && format != DXGI_FORMAT_R8G8B8A8_UNORM_SRGB &&
				format != DXGI_FORMAT_R16G16B16A16_FLOAT && format != DXGI_FORMAT_R10G10B10A2_UNORM)
			{
				Blur2PCAutomation::LogF("record: unsupported back buffer format %u", (unsigned)desc.Format);
				return false;
			}
			scRGB = floatScRGB && format == DXGI_FORMAT_R16G16B16A16_FLOAT;
			paperWhite = paperWhiteScale > 0.0f ? paperWhiteScale : 1.0f;
			srcWidth = desc.Width;
			srcHeight = desc.Height;
			width = (desc.Width / 2) & ~1u;
			height = (desc.Height / 2) & ~1u;
			D3D11_TEXTURE2D_DESC s = desc;
			s.SampleDesc.Count = 1; s.SampleDesc.Quality = 0;
			s.MipLevels = 1; s.ArraySize = 1; s.MiscFlags = 0;
			if (desc.SampleDesc.Count > 1)
			{
				D3D11_TEXTURE2D_DESC r = s;
				r.Usage = D3D11_USAGE_DEFAULT; r.BindFlags = 0; r.CPUAccessFlags = 0;
				if (FAILED(device->CreateTexture2D(&r, NULL, &resolve)))
					return false;
			}
			s.Usage = D3D11_USAGE_STAGING; s.BindFlags = 0; s.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			for (int i = 0; i < k_Ring; ++i)
				if (FAILED(device->CreateTexture2D(&s, NULL, &staging[i])))
					return false;

			const char* fpsText = getenv("BLUR2PC_FRAMELIMIT");
			const int fps = (fpsText && atoi(fpsText) > 0) ? atoi(fpsText) : 60;
			const char* exe = getenv("BLUR2PC_FFMPEG");
			char cmd[2048];
			_snprintf_s(cmd, _TRUNCATE, "\"%s\" -hide_banner -loglevel error -y -f rawvideo -pixel_format bgr0 -video_size %ux%u -framerate %d -i - -c:v libx264 -preset veryfast -crf 20 -pix_fmt yuv420p \"%s.mp4\"",
				exe ? exe : "ffmpeg.exe", width, height, fps, prefix);
			SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
			HANDLE readEnd = NULL;
			if (!CreatePipe(&readEnd, &pipe, &sa, 8 * 1024 * 1024))
				return false;
			SetHandleInformation(pipe, HANDLE_FLAG_INHERIT, 0);
			STARTUPINFOA si = { sizeof(si) };
			si.dwFlags = STARTF_USESTDHANDLES;
			si.hStdInput = readEnd;
			si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
			si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
			PROCESS_INFORMATION pi;
			const BOOL ok = CreateProcessA(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
			CloseHandle(readEnd);
			if (!ok)
			{
				Blur2PCAutomation::LogF("record: cannot start ffmpeg (%lu): %s", GetLastError(), cmd);
				CloseHandle(pipe);
				pipe = NULL;
				return false;
			}
			CloseHandle(pi.hThread);
			process = pi.hProcess;
			char timesPath[MAX_PATH];
			_snprintf_s(timesPath, _TRUNCATE, "%s_times.txt", prefix);
			fopen_s(&times, timesPath, "w");
			InitializeCriticalSection(&lock);
			wake = CreateEvent(NULL, FALSE, FALSE, NULL);
			total = frames;
			thread = CreateThread(NULL, 0, Writer, this, 0, NULL);
			Blur2PCAutomation::LogF("record: %d frames, %ux%u at %d fps, into %s.mp4", frames, width, height, fps, prefix);
			return true;
		}

		static U8 ToByte(float v) { return static_cast<U8>((v <= 0.0f ? 0.0f : (v >= 1.0f ? 1.0f : v)) * 255.0f + 0.5f); }

		void ReadBack(ID3D11DeviceContext* context)
		{
			const int slot = read % k_Ring;
			++read;
			D3D11_MAPPED_SUBRESOURCE mapped = {};
			if (FAILED(context->Map(staging[slot], 0, D3D11_MAP_READ, 0, &mapped)))
				return;
			Frame* f = new Frame;
			f->next = NULL;
			f->data = (unsigned char*)malloc(width * height * 4);
			const bool rgba = format == DXGI_FORMAT_R8G8B8A8_UNORM || format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			for (UINT y = 0; y < height; ++y)
			{
				const U8* row = static_cast<const U8*>(mapped.pData) + (y * 2) * mapped.RowPitch;
				U8* out = f->data + y * width * 4;
				for (UINT x = 0; x < width; ++x, out += 4)
				{
					if (format == DXGI_FORMAT_R16G16B16A16_FLOAT)
					{
						const U16* fp = reinterpret_cast<const U16*>(row) + (x * 2) * 4;
						for (int c = 0; c < 3; ++c)
						{
							float v = DirectX::PackedVector::XMConvertHalfToFloat(fp[c]);
							if (scRGB) { v = (std::max)(0.0f, v / paperWhite); v = v <= 0.0031308f ? 12.92f * v : 1.055f * powf(v, 1.0f / 2.4f) - 0.055f; }
							out[2 - c] = ToByte(v);
						}
					}
					else if (format == DXGI_FORMAT_R10G10B10A2_UNORM)
					{
						const U32 p = *reinterpret_cast<const U32*>(row + (x * 2) * 4);
						out[2] = static_cast<U8>(((p >> 0) & 1023) >> 2);
						out[1] = static_cast<U8>(((p >> 10) & 1023) >> 2);
						out[0] = static_cast<U8>(((p >> 20) & 1023) >> 2);
					}
					else
					{
						const U8* p = row + (x * 2) * 4;
						out[0] = p[rgba ? 2 : 0]; out[1] = p[1]; out[2] = p[rgba ? 0 : 2];
					}
					out[3] = 255;
				}
			}
			context->Unmap(staging[slot], 0);
			EnterCriticalSection(&lock);
			if (tail) tail->next = f; else head = f;
			tail = f;
			LeaveCriticalSection(&lock);
			SetEvent(wake);
		}

		// Returns false once every frame has been handed to the writer.
		bool Frame_(ID3D11DeviceContext* context, ID3D11Texture2D* back)
		{
			if (copied < total)
			{
				if (copied - read >= k_Ring - 1)
					ReadBack(context);
				if (resolve)
				{
					context->ResolveSubresource(resolve, 0, back, 0, format);
					context->CopyResource(staging[copied % k_Ring], resolve);
				}
				else
					context->CopyResource(staging[copied % k_Ring], back);
				if (times)
					fprintf(times, "%d %.4f\n", copied, Blur2PCAutomation::Now());
				++copied;
				return true;
			}
			while (read < copied)
				ReadBack(context);
			for (int i = 0; i < k_Ring; ++i)
			{
				if (staging[i]) staging[i]->Release();
				staging[i] = NULL;
			}
			if (resolve) resolve->Release();
			resolve = NULL;
			if (times) fclose(times);
			times = NULL;
			EnterCriticalSection(&lock);
			closing = true;
			LeaveCriticalSection(&lock);
			SetEvent(wake);
			CloseHandle(thread);	// the writer finishes on its own and reports through RecordingDone
			return false;
		}
	};

	void RecordD3D11Frame(ID3D11Device* device, IDXGISwapChain* swapChain, bool floatScRGB, float paperWhiteScale)
	{
		static C_D3D11Recorder* s_pRecorder = NULL;
		static bool s_busy = false;
		if (!s_busy && !Blur2PCAutomation::IsActive())
			return;
		ID3D11Texture2D* back = NULL;
		if (!s_busy)
		{
			int frames = 0;
			const char* pPrefix = Blur2PCAutomation::TakeRecordRequest(&frames);
			if (!pPrefix)
				return;
			if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back))))
			{
				Blur2PCAutomation::RecordingDone();
				return;
			}
			s_pRecorder = new C_D3D11Recorder;	// one per recording, never freed: its writer thread may outlive it
			if (!s_pRecorder->Start(device, back, floatScRGB, paperWhiteScale, pPrefix, frames < 1 ? 1 : frames))
			{
				Blur2PCAutomation::LogF("record: could not start");
				Blur2PCAutomation::RecordingDone();
				back->Release();
				return;
			}
			s_busy = true;
		}
		else if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back))))
			return;
		ID3D11DeviceContext* context = NULL;
		device->GetImmediateContext(&context);
		if (context)
		{
			if (!s_pRecorder->Frame_(context, back))
				s_busy = false;
			context->Release();
		}
		back->Release();
	}
}

bool C_PCD3D11GameDevice::Present()
{
    if(!m_pSwapChain)return false;
    if (m_HDROutput) m_HDROutput->Compose(m_pd3dDevice); // [Blur2PC][HDR]
    {
        ID3D11InfoQueue* queue = nullptr;
        if (SUCCEEDED(m_pd3dDevice->QueryInterface(__uuidof(ID3D11InfoQueue),
            reinterpret_cast<void**>(&queue))))
        {
            static UINT64 nextMessage = 0;
            static unsigned messagesLogged = 0;
            const UINT64 count = queue->GetNumStoredMessages();
            for (; nextMessage < count && messagesLogged < 25; ++nextMessage)
            {
                SIZE_T size = 0;
                queue->GetMessage(nextMessage, nullptr, &size);
                std::vector<U8> bytes(size);
                D3D11_MESSAGE* message = reinterpret_cast<D3D11_MESSAGE*>(bytes.data());
                if (SUCCEEDED(queue->GetMessage(nextMessage, message, &size)) &&
                    message->Severity <= D3D11_MESSAGE_SEVERITY_WARNING)
                {
                    Blur2PCAutomation::LogF("DX11 validation %u: %s",
                        static_cast<unsigned>(message->Severity), message->pDescription);
                    ++messagesLogged;
                }
            }
            queue->Release();
        }
    }
    if (const char* shot = Blur2PCAutomation::TakeShotRequest())
    {
        ID3D11Texture2D* back = nullptr;
        const HRESULT shotResult = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
            reinterpret_cast<void**>(&back));
        if (SUCCEEDED(shotResult))
        {
            TakeD3D11BackBufferScreenshot(shot, back, m_HDROutput && m_HDROutput->FloatSwapChain(),
                m_HDROutput ? m_HDROutput->PaperWhiteScale() : 1.0f);
            back->Release();
        }
		else Blur2PCAutomation::LogF("DX11 shot GetBuffer failed %08x", static_cast<unsigned>(shotResult));
		if (m_Windows.Size() > 0)
		{
			ID3D11Texture2D* windowTexture = m_Windows[0]->m_ColourRenderTexture.GetTexture().GetGPUTexture2D();
			if (windowTexture)
			{
				std::string windowShot(shot);
				windowShot += ".window.tga";
				TakeD3D11BackBufferScreenshot(windowShot.c_str(), windowTexture);
			}
		}
    }
    // [Blur2PC] scripted recording (see RecordD3D11Frame)
    RecordD3D11Frame(m_pd3dDevice, m_pSwapChain, m_HDROutput && m_HDROutput->FloatSwapChain(),
        m_HDROutput ? m_HDROutput->PaperWhiteScale() : 1.0f);
    {
        // [Blur1PCDX11] Temporary: trace every draw of one frame after BLUR_DX11_TRACE_AT seconds.
        static int traceAt = -1;
        if (traceAt < 0)
        {
            char value[16] = {};
            traceAt = GetEnvironmentVariableA("BLUR_DX11_TRACE_AT", value, sizeof(value)) ? atoi(value) : 0;
        }
        ID3D11InfoQueue* traceQueue = nullptr;
        m_pd3dDevice->QueryInterface(__uuidof(ID3D11InfoQueue), reinterpret_cast<void**>(&traceQueue));
        if (g_dx11TraceFrame)
        {
            g_dx11TraceFrame = 0;
            if (traceQueue)
            {
                const UINT64 stored = traceQueue->GetNumStoredMessages();
                for (UINT64 i = 0; i < stored && i < 80; ++i)
                {
                    SIZE_T size = 0;
                    traceQueue->GetMessage(i, nullptr, &size);
                    std::vector<U8> bytes(size);
                    D3D11_MESSAGE* message = reinterpret_cast<D3D11_MESSAGE*>(bytes.data());
                    if (SUCCEEDED(traceQueue->GetMessage(i, message, &size)))
                        Blur2PCAutomation::LogF("DX11 trace message %u: %s", static_cast<unsigned>(message->Severity), message->pDescription);
                }
                traceQueue->PopStorageFilter();
                traceQueue->ClearStoredMessages();
            }
            Blur2PCAutomation::LogF("DX11 trace end");
        }
        else if (traceAt > 0 && Blur2PCAutomation::Now() > traceAt)
        {
            traceAt = 0; g_dx11TraceFrame = 1;
            if (traceQueue) { traceQueue->ClearStoredMessages(); traceQueue->PushEmptyStorageFilter(); traceQueue->SetMessageCountLimit(100000); }
            Blur2PCAutomation::LogF("DX11 trace begin queue=%p", traceQueue);
        }
        if (traceQueue) traceQueue->Release();
    }
    // [Blur1PCDX11] PCD3D9Device presents with D3D9PresentInterval(m_DeviceParams.m_PresentInterval).
    const E_PresentInterval interval=m_DeviceParams.m_PresentInterval;
    const UINT syncInterval=interval==E_PresentInterval_Immediate ? 0 :
        interval==E_PresentInterval_Two ? 2 : interval==E_PresentInterval_Three ? 3 : 1;
    UINT presentFlags=0;
    if(syncInterval==0 && (m_d3dPresentParams.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING))
    {
        // Tearing presents are valid only while not in exclusive fullscreen.
        BOOL fullscreen=FALSE;m_pSwapChain->GetFullscreenState(&fullscreen,nullptr);
        if(!fullscreen)presentFlags=DXGI_PRESENT_ALLOW_TEARING;
    }
    HRESULT hr=m_pSwapChain->Present(syncInterval,presentFlags);
    if (FAILED(hr))
    {
        static unsigned presentFailures = 0;
        if (presentFailures++ < 4)
            Blur2PCAutomation::LogF("DX11 Present failed %08x, removed reason %08x",
                static_cast<unsigned>(hr), static_cast<unsigned>(m_pd3dDevice->GetDeviceRemovedReason()));
    }
    m_Valid=SUCCEEDED(hr);m_Lost=hr==DXGI_ERROR_DEVICE_REMOVED || hr==DXGI_ERROR_DEVICE_RESET;
    m_DynamicVertexBuffer.Swap();m_BeginVerticesUsage=m_UserBeginVerticesUsage=0;
    if (m_HDROutput)
    {
        m_HDROutput->Calibrate(g_HDROutputEnable, g_HDROutputPeak, g_HDROutputWhite);
        m_HDROutput->Refresh(m_pd3dDevice); // elapsed-time capability refresh
    }
    return !m_Lost && m_Valid;
}
bool C_PCD3D11GameDevice::GetLost(){return m_Lost;}
bool C_PCD3D11GameDevice::GetValid(){return m_Valid;}
bool C_PCD3D11GameDevice::CanReset(){return !m_Lost;}
bool C_PCD3D11GameDevice::CanRender(){return m_Valid && !m_Lost;}
const bool C_PCD3D11GameDevice::BeginVerticesLocked() const{return m_BeginVerticesLocked;}
void* C_PCD3D11GameDevice::BeginVertices(E_PrimitiveType type,U32 count,U32 stride,C_VertexDeclaration* decl,bool checkUsage)
{
    if(m_BeginVerticesLocked)return nullptr;
    U32 size=count*stride;
    if(checkUsage && (m_BeginVerticesUsage+size>m_MaxBeginVerticesUsage ||
       m_UserBeginVerticesUsage+size>m_UserMaxBeginVerticesUsage))return nullptr;
    void* result=m_DynamicVertexBuffer.Begin(type,count,stride,decl);
    if(result){m_BeginVerticesLocked=true;m_BeginVerticesUsage+=size;m_UserBeginVerticesUsage+=size;
#ifndef CFG_GOLD
        m_BeginVerticesStart=static_cast<U8*>(result);m_BeginVerticesSize=size;
#endif
    }
    return result;
}
void C_PCD3D11GameDevice::EndVertices(void* vertex)
{
    if(!m_BeginVerticesLocked)return;
    m_DynamicVertexBuffer.End();
    SetVertexBuffer(0,m_DynamicVertexBuffer.GetVertexBuffer(),
        m_DynamicVertexBuffer.GetOffset()-m_DynamicVertexBuffer.GetVertexSize(),m_DynamicVertexBuffer.GetVertexStride());
    SetVertexDeclaration(m_DynamicVertexBuffer.GetVertexDeclaration());
    DrawPrimitive(m_DynamicVertexBuffer.GetPrimitiveType(),0,m_DynamicVertexBuffer.GetVertexCount());
    m_BeginVerticesLocked=false;
}
void C_PCD3D11GameDevice::FakeEndVertices(void* vertex)
{if(!m_BeginVerticesLocked)return;m_DynamicVertexBuffer.End();m_BeginVerticesLocked=false;}
void C_PCD3D11GameDevice::SetMaxBeginVerticesUsage(U32 size){m_MaxBeginVerticesUsage=size;}
void C_PCD3D11GameDevice::SetUserMaxBeginVerticesUsage(F32 scale)
{m_UserMaxBeginVerticesUsage=static_cast<U32>(m_MaxBeginVerticesUsage*scale);}
U32 C_PCD3D11GameDevice::GetFreeResourceMemory(){return 0;}
bool C_PCD3D11GameDevice::AllocMemory(E_ResourceType type,U32 size,U8** address,S32 id){return true;}
void C_PCD3D11GameDevice::FreeMemory(E_ResourceType type,U32 size,U8* address,S32 id){}
bool C_PCD3D11GameDevice::SupportForRenderTarget(E_ResourceType type,E_Format format)
{C_FormatLayout layout={};return GetFormatLayout(static_cast<D3DFORMAT>(format),&layout);}

bool C_PCD3D11GameDevice::CreateRenderTexture2D(U32 width,U32 height,U32 levels,U32 flags,E_Format format,
    E_Multisample samples,const C_SurfaceParams* params,C_RenderTexture2D* target,U8* address,U32 addressSize)
{
    if(!target || target->IsCreated() || !width || !height || !levels)return false;
    flags|=E_Usage_Colour|E_Usage_Video;
    if(samples!=E_Multisample_None)flags&=~E_Usage_ReadWrite;
    target->m_Flags=flags;target->m_Multisample=samples;
    const U32 buffers=flags&E_Usage_ReadWrite ? 2 : 1;
    for(U32 index=0;index<buffers;++index)
    {
        C_Texture& texture=target->m_Texture[index];
        if(!CreateTexture2D(width,height,levels,flags,format,&texture,address,addressSize))
        {DestroyRenderTexture2D(target);return false;}
        if(samples!=E_Multisample_None)
        {
            target->m_Surfaces[index].Resize(1);
            if(!CreateColourSurface(width,height,flags,format,samples,params,&target->m_Surfaces[index][0]))
            {DestroyRenderTexture2D(target);return false;}
            target->m_ResolveSurfaces[index].Resize(1);
            target->m_ResolveSurfaces[index][0].m_pd3dSurface=texture.GetGPUTexture2D();
            texture.GetGPUTexture2D()->AddRef();
        }
        else
        {
            target->m_Surfaces[index].Resize(levels);
            for(U32 level=0;level<levels;++level)
            {target->m_Surfaces[index][level].m_pd3dSurface=texture.GetGPUTexture2D();
             target->m_Surfaces[index][level].m_Subresource=level;
             texture.GetGPUTexture2D()->AddRef();}
        }
    }
    target->m_SurfaceIndex=0;target->m_TextureIndex=buffers==2 ? 1 : 0;
    return true;
}
void C_PCD3D11GameDevice::DestroyRenderTexture2D(C_RenderTexture2D* target)
{
    if(!target)return;
    for(U32 index=0;index<2;++index)
    {
        for(S32 level=0;level<target->m_Surfaces[index].Size();++level)
            m_Display2.DestroySurface(&target->m_Surfaces[index][level]);
        target->m_Surfaces[index].Clear();
        for(S32 level=0;level<target->m_ResolveSurfaces[index].Size();++level)
            m_Display2.DestroySurface(&target->m_ResolveSurfaces[index][level]);
        target->m_ResolveSurfaces[index].Clear();
        DestroyTexture2D(&target->m_Texture[index]);
    }
    target->m_Flags=0;target->m_Multisample=0;
    target->m_SurfaceIndex=target->m_TextureIndex=0;
}
void C_RenderTexture2D::Resolve(U32 level) const
{
    if(m_Flags&E_Usage_ReadWrite)
    {m_SurfaceIndex=(m_SurfaceIndex+1)&1;m_TextureIndex=(m_TextureIndex+1)&1;}
    if(m_Multisample==E_Multisample_None || !m_Surfaces[0].Size() || !m_ResolveSurfaces[0].Size())return;
    ID3D11Texture2D* source=m_Surfaces[0][0].m_pd3dSurface;
    ID3D11Texture2D* dest=m_ResolveSurfaces[0][0].m_pd3dSurface;
    if(!source || !dest || source==dest)return;
    ID3D11Device* device=nullptr;source->GetDevice(&device);if(!device)return;
    ID3D11DeviceContext* context=nullptr;device->GetImmediateContext(&context);
    D3D11_TEXTURE2D_DESC desc={};source->GetDesc(&desc);
    // [Blur1PCDX11] Colour targets use typeless storage (per-sampler sRGB views); a resolve needs a typed
    // format, and D3D9's StretchRect resolve works on the linear values.
    if(context){context->ResolveSubresource(dest,level,source,0,GetLinearFormat(desc.Format));Display2::D3D11ResourceWritten(dest,context);context->Release();} // [Blur1PCDX11] mirror
    device->Release();
}

void C_PCD3D11GameDevice::ClearWindows()
{ DestroyWindows();for(S32 i=0;i<m_Windows.Size();++i)delete m_Windows[i];m_Windows.Clear(); }
void C_PCD3D11GameDevice::ResizeWindows()
{
    DestroyWindows();
    for(S32 i=0;i<m_Windows.Size();++i)
        m_Windows[i]->Resize(m_DeviceParams.m_WindowBufferWidth,m_DeviceParams.m_WindowBufferHeight,
            m_DeviceParams.m_WindowBufferMultisample,m_DeviceParams.m_ScreenWidth,m_DeviceParams.m_ScreenHeight,
            m_DeviceParams.m_WindowBufferColourFormat,m_DeviceParams.m_WindowBufferDepthStencilFormat);
    CreateWindows();
}
bool C_PCD3D11GameDevice::CreateWindows()
{
    for(S32 i=0;i<m_Windows.Size();++i)
    {
        C_Window* window=m_Windows[i];window->m_Multisample=m_DeviceParams.m_WindowBufferMultisample;
        if (m_HDROutput && m_HDROutput->FloatSwapChain()) window->m_ColourFormat = E_Format_A16B16G16R16F;
        if(!CreateRenderTexture2D(window->m_Width,window->m_Height,1,E_Usage_Colour,
            window->m_ColourFormat,window->m_Multisample,nullptr,&window->m_ColourRenderTexture))return false;
        if(!CreateDepthStencilSurface(window->m_Width,window->m_Height,E_Usage_DepthStencil,
            window->m_DepthStencilFormat,window->m_Multisample,nullptr,&window->m_DepthStencilSurface))return false;
        if(!CreateRenderTexture2D(window->m_Width,window->m_Height,1,E_Usage_Colour,
            E_Format_A8R8G8B8,window->m_Multisample,nullptr,&window->m_MaskRenderTexture))return false;
        window->m_ClearDepthStencil=true;window->m_MaskEnable=true;
    }
    return true;
}
void C_PCD3D11GameDevice::DestroyWindows()
{
    for(S32 i=0;i<m_Windows.Size();++i)
    {
        C_Window* window=m_Windows[i];
        if(window->m_ColourRenderTexture.IsCreated())DestroyRenderTexture2D(&window->m_ColourRenderTexture);
        if(window->m_MaskRenderTexture.IsCreated())DestroyRenderTexture2D(&window->m_MaskRenderTexture);
        DestroyDepthStencilSurface(&window->m_DepthStencilSurface);
    }
}
void C_PCD3D11GameDevice::SetWindow(S32 index,bool aa)
{
    if(index>=0 && index<m_Windows.Size())
    {
        C_Window* window=m_Windows[index];
        SetColourDepthStencilSurface(&window->m_ColourRenderTexture.GetSurface(),&window->m_DepthStencilSurface);
        const U32 flags=window->m_ClearDepthStencil ? E_Clear_Colour|E_Clear_Depth|E_Clear_Stencil : E_Clear_Colour;
        Clear(flags,Maths::RGBA(0,0,0,255));
    }
    m_CurrentWindow=index;
}
void C_PCD3D11GameDevice::EndWindow(){m_CurrentWindow=-1;}
void C_PCD3D11GameDevice::RestoreWindow(S32 index,U32 colourWrite)
{
    if(index<0 || index>=m_Windows.Size())return;
    m_CurrentWindow=index;C_Window* window=m_Windows[index];
    window->m_ColourRenderTexture.Resolve();
    SetBlendMode(E_BlendMode_Solid);SetTextureGamma(0,E_Bool_False);
    ID3D11DeviceContext* context = nullptr;
    m_pd3dDevice->GetImmediateContext(&context);
    ID3D11RenderTargetView* target = nullptr;
    if (context) context->OMGetRenderTargets(1, &target, nullptr);
    if (target) target->Release();
    if (context) context->Release();
    static bool quadProbed = false;
    char probe[2] = {};
    if (!quadProbed && Blur2PCAutomation::Now() > 5.0 &&
        GetEnvironmentVariableA("BLUR_DX11_TEST_QUAD", probe, sizeof(probe)) && probe[0] == '1')
    {
        quadProbed = true;
        SetColourDepthStencilSurface();
        Quad::DrawFullScreenColour(Maths::C_RGBA_F32(1.0f, 0.0f, 1.0f, 1.0f));
        ID3D11DeviceContext* traceContext = nullptr;
        m_pd3dDevice->GetImmediateContext(&traceContext);
        if (traceContext)
        {
            ID3D11VertexShader* vertex = nullptr;
            ID3D11PixelShader* pixel = nullptr;
            ID3D11InputLayout* layout = nullptr;
            ID3D11RenderTargetView* colour = nullptr;
            ID3D11Buffer* buffer = nullptr;
            ID3D11BlendState* blend = nullptr;
            ID3D11DepthStencilState* depth = nullptr;
            UINT stride = 0, offset = 0, sampleMask = 0, stencilRef = 0;
            float blendFactor[4] = {};
            D3D11_VIEWPORT viewport = {};
            UINT viewportCount = 1;
            traceContext->VSGetShader(&vertex, nullptr, nullptr);
            traceContext->PSGetShader(&pixel, nullptr, nullptr);
            traceContext->IAGetInputLayout(&layout);
            traceContext->OMGetRenderTargets(1, &colour, nullptr);
            traceContext->IAGetVertexBuffers(0, 1, &buffer, &stride, &offset);
            traceContext->OMGetBlendState(&blend, blendFactor, &sampleMask);
            traceContext->OMGetDepthStencilState(&depth, &stencilRef);
            traceContext->RSGetViewports(&viewportCount, &viewport);
            D3D11_BLEND_DESC blendDesc = {};
            D3D11_DEPTH_STENCIL_DESC depthDesc = {};
            if (blend) blend->GetDesc(&blendDesc);
            if (depth) depth->GetDesc(&depthDesc);
            Blur2PCAutomation::LogF("DX11 restore quad VS=%p PS=%p IA=%p RTV=%p VB=%p stride=%u offset=%u VP=%.0fx%.0f depth=%u blend=%u src=%u dst=%u write=%u",
                vertex, pixel, layout, colour, buffer, stride, offset,
                viewport.Width, viewport.Height, depthDesc.DepthEnable,
                blendDesc.RenderTarget[0].BlendEnable,
                blendDesc.RenderTarget[0].SrcBlend,
                blendDesc.RenderTarget[0].DestBlend,
                blendDesc.RenderTarget[0].RenderTargetWriteMask);
            if (vertex) vertex->Release(); if (pixel) pixel->Release();
            if (layout) layout->Release(); if (colour) colour->Release();
            if (buffer) buffer->Release(); if (blend) blend->Release();
            if (depth) depth->Release(); traceContext->Release();
        }
        CreateDirectoryA("dx11_trace", nullptr); // [Blur2PC] opt-in still beside the game, not a developer path
        TakeD3D11BackBufferScreenshot("dx11_trace\\quadcolour.tga",
            m_BackBuffers[0]->m_ColourSurface.m_pd3dSurface);
    }
    // [Blur1PCDX11] Retail branches on the engine's current colour surface, not on what is bound:
    // onto the back buffer the window is a textured quad in the current viewport, onto any other
    // surface it is a copy (D3D9: StretchRect).
    {
        static unsigned mismatch = 0;
        const C_Surface* backSurface = &GetBackBuffer().m_ColourSurface;
        if (m_RenderTarget.m_pColourSurface != backSurface && mismatch++ < 4)
            Blur2PCAutomation::LogF("DX11 RestoreWindow surface mismatch engine=%p (tex=%p) back=%p (tex=%p) windows=%d first=%p",
                m_RenderTarget.m_pColourSurface,
                m_RenderTarget.m_pColourSurface ? m_RenderTarget.m_pColourSurface->m_pd3dSurface : nullptr,
                backSurface, backSurface->m_pd3dSurface, m_Windows.Size(),
                m_Windows.Size() > 0 ? &m_Windows[0]->m_ColourRenderTexture.GetSurface() : nullptr);
    }
    if (m_RenderTarget.m_pColourSurface == &GetBackBuffer().m_ColourSurface)
    {
        static unsigned quadPaths = 0;
        if (quadPaths++ % 60 == 0)
            Blur2PCAutomation::LogF("DX11 RestoreWindow quad paths=%u t=%.1f", quadPaths, Blur2PCAutomation::Now());
        Quad::DrawFullScreen(&window->m_ColourRenderTexture.GetTexture());
        return;
    }
    ID3D11Texture2D* source = window->m_ColourRenderTexture.GetTexture().GetGPUTexture2D();
    ID3D11Texture2D* destination = m_RenderTarget.m_pColourSurface ?
        m_RenderTarget.m_pColourSurface->m_pd3dSurface : nullptr;
    {
        static unsigned copyPaths = 0;
        if (copyPaths++ % 60 == 0)
            Blur2PCAutomation::LogF("DX11 RestoreWindow copy paths=%u source=%p destination=%p t=%.1f", copyPaths, source, destination, Blur2PCAutomation::Now());
    }
    if (!source || !destination || source == destination) return;
    D3D11_TEXTURE2D_DESC sourceDesc = {}, destinationDesc = {};
    source->GetDesc(&sourceDesc);
    destination->GetDesc(&destinationDesc);
    ID3D11DeviceContext* copyContext = nullptr;
    m_pd3dDevice->GetImmediateContext(&copyContext);
    if (!copyContext) return;
    if (sourceDesc.Width == destinationDesc.Width && sourceDesc.Height == destinationDesc.Height &&
        sourceDesc.Format == destinationDesc.Format)
    {
        copyContext->CopyResource(destination, source);
        Display2::D3D11ResourceWritten(destination, copyContext); // [Blur1PCDX11]
    }
    else
        Quad::DrawFullScreen(&window->m_ColourRenderTexture.GetTexture());
    copyContext->Release();
    {
        static unsigned reported = 0;
        if (reported < 4 && Blur2PCAutomation::Now() > 30.0)
        {
            ReportTextureStats("restore-source", source, reported);
            ReportTextureStats("restore-destination", destination, reported);
            ++reported;
        }
    }
}
void C_PCD3D11GameDevice::RenderWindowMask()
{
    if(m_CurrentWindow<0 || m_CurrentWindow>=m_Windows.Size())return;
    C_Window* window=m_Windows[m_CurrentWindow];window->m_ColourRenderTexture.Resolve();
    if(!window->m_MaskEnable)return;
    SetColourSurface(0,&window->m_MaskRenderTexture.GetSurface());
    Clear(E_Clear_Colour,Maths::RGBA(0,0,0,0));SetBlendMode(E_BlendMode_Solid);
    C_StencilState stencil;stencil.m_StencilEnable=E_Bool_True;
    stencil.m_StencilFunc=E_CmpFunc_Equal;stencil.m_StencilRef=0xff;
    stencil.m_StencilMask=0xff;stencil.m_StencilWriteMask=0;
    SetStencilState(stencil);SetDepthEnable(E_Bool_False);
    SetDepthWriteEnable(E_Bool_False);SetCullMode(E_CullMode_None);
    Quad::DrawVertexColour(Maths::RGBA(255,255,255,255));
    stencil.m_StencilEnable=E_Bool_False;SetStencilState(stencil);
    SetCullMode(E_CullMode_Default);SetDepthWriteEnable(E_Bool_True);SetDepthEnable(E_Bool_True);
    window->m_MaskRenderTexture.Resolve();
    SetColourSurface(0,&window->m_ColourRenderTexture.GetSurface());
}
} }


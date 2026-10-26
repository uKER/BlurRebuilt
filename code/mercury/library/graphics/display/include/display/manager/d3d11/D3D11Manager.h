#ifndef DISPLAY_MANAGER_D3D11_MANAGER_H
#define DISPLAY_MANAGER_D3D11_MANAGER_H

// [Blur1PCDX11] Native DXGI adapter and video mode enumeration.
#include <dxgi.h>
#include <Core/Containers.h>
#include <Core/Containers/Genesis_Containers/Array.h>
#include "Display/Manager/ManagerBase.h"

namespace Display { namespace D3D11 {

struct C_Multisample
{
    bool m_Windowed, m_FullScreen;
    E_Format m_ColourFormat, m_DepthStencilFormat;
    E_Multisample m_Multisample;
};

class C_D3D11Manager : public C_ManagerBase
{
public:
    C_D3D11Manager();
    ~C_D3D11Manager();
    bool IsInitialised();
    bool Initialise();
    void Shutdown();
    S32 GetMultisampleCount() const;
    const C_Multisample& GetMultisample(S32 index) const;
    S32 GetVideoModeCount() const;
    const C_VideoMode& GetVideoMode(S32 index);
    S32 FindMultisample(E_Multisample multisample) const;
    S32 FindVideoMode(U32 width, U32 height, E_Format format);
    bool GetVideoMode(C_VideoMode* mode);
    bool GetDesktopVideoMode(C_VideoMode* mode);
    IDXGIFactory1* GetFactory() const { return m_Factory; }
private:
    IDXGIFactory1* m_Factory = nullptr;
    Ut::Vector<C_Multisample> m_MultisampleList;
    Ut::Vector<C_VideoMode> m_VideoModeList;
};

} }
#endif

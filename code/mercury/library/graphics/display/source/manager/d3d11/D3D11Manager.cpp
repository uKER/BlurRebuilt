// [Blur1PCDX11] Native DXGI mode enumeration.
#include "stdafx.h"
#include "Display/Manager/D3D11/D3D11Manager.h"
#include <d3d11.h>
#include <vector>
namespace Display { namespace D3D11 {
C_D3D11Manager::C_D3D11Manager() {}
C_D3D11Manager::~C_D3D11Manager() { Shutdown(); }
bool C_D3D11Manager::IsInitialised() { return m_Factory != nullptr; }
bool C_D3D11Manager::Initialise()
{
    if (m_Factory) return true;
    if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&m_Factory)))) return false;
    IDXGIAdapter1* adapter = nullptr;
    if (SUCCEEDED(m_Factory->EnumAdapters1(0,&adapter)))
    {
        IDXGIOutput* output = nullptr;
        if (SUCCEEDED(adapter->EnumOutputs(0,&output)))
        {
            UINT count = 0;
            output->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM,0,&count,nullptr);
            std::vector<DXGI_MODE_DESC> modes(count);
            if (count && SUCCEEDED(output->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM,0,&count,modes.data())))
                for (const auto& native : modes)
                {
                    if (native.Width < 640 || native.Height < 480 || FindVideoMode(native.Width,native.Height,E_Format_A8R8G8B8)>=0) continue;
                    C_VideoMode mode = {};
                    mode.m_Width=native.Width;mode.m_Height=native.Height;mode.m_Format=E_Format_A8R8G8B8;
                    mode.m_RefreshRate=native.RefreshRate.Denominator ? native.RefreshRate.Numerator/native.RefreshRate.Denominator : 0;
                    mode.m_WideScreen=false;mode.m_HiDef=false;m_VideoModeList.PushBack(mode);
                }
            output->Release();
        }
        ID3D11Device* device = nullptr; ID3D11DeviceContext* context = nullptr; D3D_FEATURE_LEVEL feature;
        if (SUCCEEDED(D3D11CreateDevice(adapter,D3D_DRIVER_TYPE_UNKNOWN,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,&feature,&context)))
        {
            const E_Multisample samples[] = {E_Multisample_None,E_Multisample_2,E_Multisample_4,E_Multisample_8,E_Multisample_16};
            for (auto sample : samples)
            {
                UINT count=sample==E_Multisample_None ? 1 : static_cast<UINT>(sample), colour=0,depth=0;
                device->CheckMultisampleQualityLevels(DXGI_FORMAT_B8G8R8A8_UNORM,count,&colour);
                device->CheckMultisampleQualityLevels(DXGI_FORMAT_D24_UNORM_S8_UINT,count,&depth);
                if(colour && depth) { C_Multisample item={true,true,E_Format_X8R8G8B8,E_Format_D24S8,sample};m_MultisampleList.PushBack(item); }
            }
            context->Release();device->Release();
        }
        adapter->Release();
    }
    return true;
}
void C_D3D11Manager::Shutdown()
{ m_MultisampleList.Destroy();m_VideoModeList.Destroy();if(m_Factory){m_Factory->Release();m_Factory=nullptr;} }
S32 C_D3D11Manager::GetMultisampleCount() const {return m_MultisampleList.Size();}
const C_Multisample& C_D3D11Manager::GetMultisample(S32 i) const {return m_MultisampleList[i];}
S32 C_D3D11Manager::GetVideoModeCount() const {return m_VideoModeList.Size();}
const C_VideoMode& C_D3D11Manager::GetVideoMode(S32 i) {return m_VideoModeList[i];}
S32 C_D3D11Manager::FindMultisample(E_Multisample value) const
{for(S32 i=0;i<m_MultisampleList.Size();++i)if(m_MultisampleList[i].m_Multisample==value)return i;return -1;}
S32 C_D3D11Manager::FindVideoMode(U32 w,U32 h,E_Format f)
{for(S32 i=0;i<m_VideoModeList.Size();++i)if(m_VideoModeList[i].m_Width==w && m_VideoModeList[i].m_Height==h && m_VideoModeList[i].m_Format==f)return i;return -1;}
bool C_D3D11Manager::GetVideoMode(C_VideoMode* mode) {return GetDesktopVideoMode(mode);}
bool C_D3D11Manager::GetDesktopVideoMode(C_VideoMode* mode)
{
    if(!mode)return false;DEVMODEA native={};native.dmSize=sizeof(native);
    if(!EnumDisplaySettingsA(nullptr,ENUM_CURRENT_SETTINGS,&native))return false;
    mode->m_Width=native.dmPelsWidth;mode->m_Height=native.dmPelsHeight;
    mode->m_Format=E_Format_A8R8G8B8;mode->m_RefreshRate=native.dmDisplayFrequency;
    mode->m_WideScreen=false;mode->m_HiDef=false;return true;
}
} }


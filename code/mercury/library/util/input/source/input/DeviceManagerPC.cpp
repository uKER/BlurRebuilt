#include "stdafx.h"

#include <input\DeviceManagerPC.h>

namespace Input
{

#define MAX_USER_INDEX_COUNT 1

// C_DeviceManagerPC

C_DeviceManagerPC::C_DeviceManagerPC( const C_DeviceManagerCreateParam& param )
	: C_DeviceManager( param, MAX_USER_INDEX_COUNT )
	, m_pDirectInput(NULL)
	, m_hWnd(param.m_hWnd)
{
    HRESULT hr;

    hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8A , (VOID**)&m_pDirectInput, NULL );
        
    if(FAILED(hr))
    {
        LOGWARN(g_Log, "Could not create DirectInput Object\n");
    }    
}

C_DeviceManagerPC::~C_DeviceManagerPC()
{
    m_pDirectInput->Release();
    m_pDirectInput = NULL;
}

void C_DeviceManagerPC::PlatformUpdate()
{
}

C_DeviceManager* CreateDeviceManager( const C_DeviceManagerCreateParam& param )
{
	return (C_DeviceManager*)new C_DeviceManagerPC(param);
}

} // namespace Input


#include "stdafx.h"

#include <input\DeviceManagerPS3.h>

namespace Input
{

#define MAX_USER_INDEX_COUNT 4
#define MAX_PAD 4

// C_DeviceManagerPS3

C_DeviceManagerPS3::C_DeviceManagerPS3( const C_DeviceManagerCreateParam& param )
	: C_DeviceManager( param, MAX_USER_INDEX_COUNT )
{
	memset(&m_PadInfo, 0x00, sizeof(m_PadInfo));

	if (cellPadInit(MAX_PAD))
	{
		LOGERROR(g_Log, "C_DeviceManagerPS3: Error Initialising Pad\n");
	}
}

C_DeviceManagerPS3::~C_DeviceManagerPS3()
{
}

void C_DeviceManagerPS3::PlatformUpdate()
{
	int32_t result = 0;
   
#if CELL_SDK_VERSION >= 0x250000
	result = cellPadGetInfo2(&m_PadInfo);
#else
	result = cellPadGetInfo(&m_PadInfo);
#endif
	if(result) 
	{
		LOGERROR(g_Log, "C_DeviceManagerPS3: cellUsbPadGetPadInfo Error %d\n", result);
		return;
	}
}

C_DeviceManager* CreateDeviceManager( const C_DeviceManagerCreateParam& param )
{
	return (C_DeviceManager*)new C_DeviceManagerPS3(param);
}

} // namespace Input


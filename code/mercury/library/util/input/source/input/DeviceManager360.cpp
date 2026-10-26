#include "stdafx.h"

#include <input\DeviceManager360.h>

namespace Input
{

#define MAX_USER_INDEX_COUNT 4

// C_DeviceManager360

C_DeviceManager360::C_DeviceManager360( const C_DeviceManagerCreateParam& param )
	: C_DeviceManager( param, MAX_USER_INDEX_COUNT )
{
}

C_DeviceManager360::~C_DeviceManager360()
{
}

void C_DeviceManager360::PlatformUpdate()
{
}

C_DeviceManager* CreateDeviceManager( const C_DeviceManagerCreateParam& param )
{
	return (C_DeviceManager*)new C_DeviceManager360(param);
}

} // namespace Input


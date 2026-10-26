#ifndef INPUT_I_DEVICEMANAGER360_H
#define INPUT_I_DEVICEMANAGER360_H

#ifdef _MSC_VER
#pragma once
#endif

#include <input\Input.h>
#include <input\DeviceManager.h>

namespace Input
{

// 360 implementation of C_DeviceManager
class C_DeviceManager360 : public C_DeviceManager
{
public:	
	C_DeviceManager360( const C_DeviceManagerCreateParam& param );
	~C_DeviceManager360();

	// Inherited from C_DeviceManager
	void PlatformUpdate();
};

} // namespace Input

#endif


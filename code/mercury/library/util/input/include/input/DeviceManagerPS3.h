#ifndef INPUT_I_DEVICEMANAGERPS3_H
#define INPUT_I_DEVICEMANAGERPS3_H

#ifdef _MSC_VER
#pragma once
#endif

#include <input\Input.h>
#include <input\DeviceManager.h>

#include <cell/pad.h>    

namespace Input
{

class C_PadPS3;

// PS3 implementation of a C_DeviceManager.
class C_DeviceManagerPS3 : public C_DeviceManager
{
friend class C_PadPS3;
public:	
	C_DeviceManagerPS3( const C_DeviceManagerCreateParam& param );
	~C_DeviceManagerPS3();

	// Inherited from C_DeviceManager
	void PlatformUpdate();

private:
#if CELL_SDK_VERSION >= 0x250000
	CellPadInfo2 m_PadInfo; // connection padinfo buffer
#else
	CellPadInfo m_PadInfo; // connection padinfo buffer
#endif
};

} // namespace Input

#endif


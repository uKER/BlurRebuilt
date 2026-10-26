#ifndef INPUT_I_DEVICEMANAGER360_H
#define INPUT_I_DEVICEMANAGER360_H

#ifdef _MSC_VER
#pragma once
#endif

#include <input\Input.h>
#include <input\DeviceManager.h>

typedef IDirectInput8           _IDirectInput;

namespace Input
{

// PC implementation of a C_DeviceManager.
class C_DeviceManagerPC : public C_DeviceManager
{
public:	
	C_DeviceManagerPC( const C_DeviceManagerCreateParam& param );
	~C_DeviceManagerPC();

	// Inherited from C_DeviceManager
	void PlatformUpdate();

	_IDirectInput* GetDirectInput() { return m_pDirectInput; }
	HWND GetWindowHandle() { return m_hWnd; }
public:
    HWND m_hWnd;
    _IDirectInput* m_pDirectInput;
};

} // namespace Input

#endif


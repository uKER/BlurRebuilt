#ifndef DISPLAY_MANAGER_MANAGER_H
#define DISPLAY_MANAGER_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceDefines.h"

#if defined(DEVICE_PCD3D11) // [Blur1PCDX11] Native DXGI manager.
#include "Display/Manager/D3D11/D3D11Manager.h"
#elif defined(DEVICE_PS3GCM)
#include "Display/Manager/GCM/GCMManager.h"
#elif defined(DEVICE_PCGL)
#include "Display/Manager/GL/GLManager.h"
#elif defined(DEVICE_PS3GL)
#include "Display/Manager/GL/GLManager.h"
#else
#pragma message("Unsupported device define when compiling Manager.h")
#endif

namespace Display
{

//! C_Manager

#if defined(DEVICE_PCD3D11)
class C_Manager : public D3D11::C_D3D11Manager
#elif defined(DEVICE_PS3GCM)
class C_Manager : public GCM::C_GCMManager
#elif defined(DEVICE_PCGL)
class C_Manager : public GL::C_GLManager
#elif defined(DEVICE_PS3GL)
class C_Manager : public GL::C_GLManager
#endif
{
public:

	C_Manager();
	~C_Manager();

	void Destroy();

	C_Device* CreateDevice(C_DeviceParams& deviceParams);
	bool CreateDevice(C_Device* pDevice, C_DeviceParams& deviceParams);
	void DestroyDevice(C_Device* pDevice);

	bool CreateDeviceIndirect(C_Device* pDevice, C_DeviceParams& deviceParams);
	void DestroyDeviceIndirect(C_Device* pDevice);

private:

	Ut::UtSet<void*> m_DeviceList;
};

extern C_Manager* g_pManager;

void Manager(C_Manager* pManager);

INLINE C_Manager* Manager()
{
	assert(g_pManager);
	return g_pManager;
}

INLINE const bool ManagerExists()
{
	return g_pManager != NULL;
}

}; // namespace Display

#endif // DISPLAY_MANAGER_MANAGER_H

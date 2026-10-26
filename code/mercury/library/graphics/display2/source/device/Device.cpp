#include "StdAfx.h"
#include "Display2/Device/Device.h"

namespace Display2
{

bool CreateDeviceManager(C_DeviceManager* pDeviceManager, C_DeviceManagerParams& deviceManagerParams)
{
	assert(pDeviceManager);

	if(pDeviceManager->IsCreated())
		return false;

	if(!pDeviceManager->Create(deviceManagerParams))
	{
		pDeviceManager->Destroy();
		return false;
	}

	return true;
}

void DestroyDeviceManager(C_DeviceManager* pDeviceManager)
{
	assert(pDeviceManager);

	if(!pDeviceManager->IsCreated())
		return;

	pDeviceManager->Destroy();
}

void SetupDefaultDeviceParams(C_DeviceManager* pDeviceManager, C_DeviceParams& deviceParams)
{
	assert(pDeviceManager);

	if(!pDeviceManager->IsCreated())
		return;

	pDeviceManager->SetupDefaultDeviceParams(deviceParams);
}

bool CreateDevice(C_DeviceManager* pDeviceManager, C_Device* pDevice, C_DeviceParams& deviceParams)
{
	//MEMORY_TRACKER_PUSH("device");

	assert(pDeviceManager && pDevice);

	if(!pDeviceManager->IsCreated() || pDevice->IsCreated())
	{
		//MEMORY_TRACKER_POP("device");

		return false;
	}

	pDevice->SetParams(deviceParams);

	if(!pDevice->Create())
	{
		pDevice->Destroy();

		//MEMORY_TRACKER_POP("device");

		return false;
	}

	if(!pDevice->CreateVolatile())
	{
		pDevice->DestroyVolatile();
		pDevice->Destroy();
		
		//MEMORY_TRACKER_POP("device");

		return false;
	}

	//MEMORY_TRACKER_POP("device");

	if(pDevice != g_pDevice)
		g_pDevice = g_pDevice;

	return true;
}

void DestroyDevice(C_DeviceManager* pDeviceManager, C_Device* pDevice)
{
	assert(pDeviceManager && pDevice);

	if(!pDeviceManager->IsCreated() || !pDevice->IsCreated())
		return;

	pDevice->DestroyVolatile();
	pDevice->Destroy();

	if(pDevice == g_pDevice)
		g_pDevice = NULL;
}

void* g_pDevice = NULL;

void Device(C_Device* pDevice)
{
	g_pDevice = pDevice;
}

} // namespace Display2

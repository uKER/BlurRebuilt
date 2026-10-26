#include "stdafx.h"
#include "Display/Manager/Manager.h"
#include "Display/Buffer/VertexDeclarations.h"
#include "Display/Shader/VertexShaders.h"
#include "Display/Shader/PixelShaders.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

namespace Display
{

C_Manager::C_Manager()
{
	BIZ_ASSERT(!g_pManager);
	g_pManager = this;
}

C_Manager::~C_Manager()
{
	BIZ_ASSERT(m_DeviceList.Size() == 0);
	
	BIZ_ASSERT(g_pManager == this);
	g_pManager = NULL;
}

C_Device* C_Manager::CreateDevice(C_DeviceParams& deviceParams)
{
	C_Device* pDevice = new C_Device();

	if(!CreateDeviceIndirect(pDevice, deviceParams))
		SAFE_DELETE_PTR(pDevice);

	return pDevice;
}

bool C_Manager::CreateDevice(C_Device* pDevice, C_DeviceParams& deviceParams)
{
	return CreateDeviceIndirect(pDevice, deviceParams);
}

void C_Manager::DestroyDevice(C_Device* pDevice)
{
	DestroyDeviceIndirect(pDevice);

	SAFE_DELETE_PTR(pDevice);
}

bool C_Manager::CreateDeviceIndirect(C_Device* pDevice, C_DeviceParams& deviceParams)
{
	if(!pDevice)
		return false;

	RESOURCE_MEMORY_ID("Device");

	
	if(!pDevice->Create(deviceParams))
		return false;

	CreateVertexDeclarations();
	CreateVertexShaders();
	CreatePixelShaders();

	m_DeviceList.Insert(pDevice);

	return true;
}

void C_Manager::DestroyDeviceIndirect(C_Device* pDevice)
{
	if(!pDevice)
		return;
	
	DestroyVertexShaders();
	DestroyPixelShaders();

	pDevice->DestroyVolatile();
	pDevice->Destroy();

	m_DeviceList.Erase(pDevice);
}

void C_Manager::Destroy()
{
	DestroyVertexShaders();
	DestroyPixelShaders();

	Ut::UtSet<void*>::Iterator it = m_DeviceList.Begin();

	while(it != m_DeviceList.End())
	{
		void *_pDevice = *it;
		C_Device* pDevice = (C_Device*)_pDevice;
		it++;

		pDevice->DestroyVolatile();
		pDevice->Destroy();

		SAFE_DELETE_PTR(pDevice);
	}

	m_DeviceList.Clear();
}

C_Manager* g_pManager = NULL;

void Manager(C_Manager* pManager)
{
	g_pManager = pManager;
}

}; // namespace Display

#include "stdafx.h"

#include <input\DeviceManager.h>
#include <input\Pad360.h>
#include <input\PadPS3.h>
#include <input\KeyboardPC.h>
#include <input\MousePC.h>
#ifndef CFG_GOLD
#include <input\PadVirtual.h>
#endif //CFG_GOLD

namespace Input
{

C_DeviceManager* g_pDeviceManager = NULL;

// C_DeviceManager

C_DeviceManager::C_DeviceManager( const C_DeviceManagerCreateParam& param, const S32 userIndexCount )
	: m_UserIndexCount( userIndexCount )
	, m_ActivityUpdate( 0 ) // [Blur2PC]
{
	assert( g_pDeviceManager == NULL );
	g_pDeviceManager = this;
	m_DeviceActivity.Resize(userIndexCount); // [Blur2PC]
}

C_DeviceManager::~C_DeviceManager()
{
	assert((m_DeviceList.Size() == 0) && "Device list not empty");
	
	assert(g_pDeviceManager != NULL);
	g_pDeviceManager = NULL;
}
	
C_DeviceManager* C_DeviceManager::GetInstance()
{
	return g_pDeviceManager;
}

void C_DeviceManager::Update()
{
	PlatformUpdate();

	const S32 deviceCount = m_DeviceList.Size();
	++m_ActivityUpdate; // [Blur2PC]
	for(S32 i=0; i<deviceCount; i++)
	{
		m_DeviceList[i]->Update();
		// [Blur2PC] Sample once per input update, never per rendered frame. If both
		// families act in this sample, keyboard/mouse wins regardless of device order.
		// Retain activity per user; the UI supplies its current navigation eligibility.
#ifdef PLATFORM_PC
		C_Device* device = m_DeviceList[i];
		if (device->HasNewActivity())
		{
			S_DeviceActivity& activity = m_DeviceActivity[device->GetUserIndex()];
			if (activity.m_Update != m_ActivityUpdate || !activity.m_pDevice ||
				device->GetDeviceType() < activity.m_pDevice->GetDeviceType())
			{
				activity.m_pDevice = device;
				activity.m_Update = m_ActivityUpdate;
			}
		}
#endif
	}
}

// [Blur2PC] A connected but idle controller must not select controller prompts.
E_Device C_DeviceManager::GetLastActiveDeviceType( const U32 userMask ) const
{
	const S_DeviceActivity* latest = NULL;
	for (S32 i = 0; i < m_DeviceActivity.Size() && i < 32; ++i)
	{
		const S_DeviceActivity& activity = m_DeviceActivity[i];
		if ((userMask & (1u << i)) && activity.m_pDevice &&
			(!latest || activity.m_Update > latest->m_Update ||
			(activity.m_Update == latest->m_Update &&
			 activity.m_pDevice->GetDeviceType() < latest->m_pDevice->GetDeviceType())))
			latest = &activity;
	}
	if (!latest || !latest->m_pDevice->IsConnected())
		return E_Device_Keyboard;
	return (E_Device)latest->m_pDevice->GetDeviceType();
}

S32 C_DeviceManager::GetUserIndexCount()
{
	return m_UserIndexCount;
}

void C_DeviceManager::CreateDevice( const S32 userIndex, const E_Device deviceType, C_Device** ppDevice )
{
	assert(( userIndex >= 0 ) && ( userIndex < m_UserIndexCount));

	C_Device* pNewDevice = NULL;
	switch(deviceType)
	{
		default:
		break;

		#if defined(PLATFORM_PC)
		case E_Device_Pad360:
			pNewDevice = (C_Device*)new C_Pad360( this, userIndex );
		break;
		#ifndef CFG_GOLD
		case E_Device_PadVirtual360:
			pNewDevice = (C_Device*)new C_PadVirtual360( this, userIndex );
		break;
		#endif //CFG_GOLD
		#endif

		#if defined(PLATFORM_PS3)
		case E_Device_PadPS3:
			pNewDevice = (C_Device*)new C_PadPS3( this, userIndex );
		break;
		#ifndef CFG_GOLD
		case E_Device_PadVirtualPS3:
			pNewDevice = (C_Device*)new C_PadVirtualPS3( this, userIndex );
		break;
		#endif //CFG_GOLD
		#endif

		#if defined(PLATFORM_PC)
		case E_Device_Keyboard:
			pNewDevice = (C_Device*)new C_KeyboardPC( this, userIndex );
		break;
		
		case E_Device_Mouse:
			pNewDevice = new C_MousePC( this, userIndex );
		break;
		#endif
	}

	*ppDevice = pNewDevice;

	if(!pNewDevice)
	{
		LOGWARN(g_Log, "Input::C_DeviceManager Device not created\n");
		return;
	}

	// add pointer to list of devices
	// [Blur2PC] Track the slots actually created: PC's legacy user count is one,
	// while the game's framework creates devices for all four local players.
	if (userIndex >= m_DeviceActivity.Size()) m_DeviceActivity.Resize(userIndex + 1);
	m_DeviceList.PushBack(pNewDevice);
}

void C_DeviceManager::RemoveDevice( C_Device* pDevice )
{
	// [Blur2PC] Forget activity before the physical device is destroyed.
	S_DeviceActivity& activity = m_DeviceActivity[pDevice->GetUserIndex()];
	if (activity.m_pDevice == pDevice) activity = S_DeviceActivity();
	// remove pointer from list of devices
	const S32 deviceCount = m_DeviceList.Size();
	for(S32 i=0; i<deviceCount; i++)
	{
		if(m_DeviceList[i] == pDevice)
		{
			m_DeviceList.EraseByIndex(i);
			break;
		}
	}
}

C_Device* C_DeviceManager::GetDevice( const S32 deviceNo ) const
{
	return m_DeviceList[deviceNo];
}

void C_DeviceManager::EnableAllDevices()
{
	const S32 deviceCount = m_DeviceList.Size();
	for(S32 i=0; i<deviceCount; i++)
	{
		m_DeviceList[i]->Enable();
	}
}

void C_DeviceManager::DisableAllDevices()
{
	const S32 deviceCount = m_DeviceList.Size();
	for(S32 i=0; i<deviceCount; i++)
	{
		m_DeviceList[i]->Disable();
	}
}

void DestroyDeviceManager( C_DeviceManager* pDeviceManger )
{
	C_DeviceManager* pDeviceManager = GetDeviceManager();
	assert(pDeviceManager);
	SAFE_DELETE_PTR(pDeviceManager); 
}

C_DeviceManager* GetDeviceManager()
{
	return C_DeviceManager::GetInstance();
}

} // namespace Input



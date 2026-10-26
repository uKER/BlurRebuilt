#ifndef INPUT_I_DEVICEMANAGER_H
#define INPUT_I_DEVICEMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <input\Input.h>

namespace Input
{

//! @addtogroup Input
//! @{

class C_Device;

//! Used to select which type of input device to create in the C_DeviceManager.
enum E_Device
{	
	E_Device_Keyboard = 0,	
	E_Device_Mouse,		  	
	E_Device_Pad360,		
	E_Device_PadPS3,
	#ifndef CFG_GOLD
	E_Device_PadVirtual360,
	E_Device_PadVirtualPS3,
	#endif //CFG_GOLD
	E_Device_Count			
};

//! Information required to create the device manager
class C_DeviceManagerCreateParam
{
public:
#ifdef PLATFORM_PC
	HWND m_hWnd;
#endif
};

//! Manages input devices.
class C_DeviceManager
{
friend class C_Device;

public:
	//! Don't call this constructor, use the Input::CreateDeviceManager function.
	C_DeviceManager( const C_DeviceManagerCreateParam& param, const S32 userIndexCount );

	virtual ~C_DeviceManager();

	//! This class is not a singleton although only one instance should be created at once. 
	//! GetInstance can be used to return the currently created instance of the class. 
	//! If the class has not been created GetInstance will return NULL.
	static C_DeviceManager* GetInstance();

	//! Update all devices that are currently available
	void Update();
						
	//! Returns the number of possible signed in users on the target
	S32 GetUserIndexCount();

	//! Create a class for reading a device. This class will always be created. Use the IsConnected() method to determine if the device is connected.
	void CreateDevice( const S32 userIndex, const E_Device deviceType, C_Device** ppDevice );


	//! Enable all input devices. When a device is Disabled, all button and axis queries will return 0. Devices are Enabled by default.
	void EnableAllDevices();

	//! Disable all input devices. When a device is Disabled, all button and axis queries will return 0. Devices are Enabled by default.
	void DisableAllDevices();

	//! Returns the current number of created devices
	const S32 GetDeviceCount() const;

	//! Get device
	C_Device* GetDevice( const S32 deviceNo ) const;

	//! [Blur2PC] Most recent physical input from eligible users; keyboard if none or disconnected.
	E_Device GetLastActiveDeviceType( const U32 userMask = ~0u ) const;

private:
	virtual void PlatformUpdate() = 0;

	// called by device, use the device's dtor to delete a device
	void RemoveDevice( C_Device* pDevice );

private:
	const S32 m_UserIndexCount;

	Ut::Vector<C_Device*> m_DeviceList;
	// [Blur2PC] Keep each user's history so ignored players cannot overwrite menu-owner activity.
	struct S_DeviceActivity
	{
		S_DeviceActivity() : m_pDevice(NULL), m_Update(0) {}
		C_Device* m_pDevice;
		U64 m_Update;
	};
	Ut::Vector<S_DeviceActivity> m_DeviceActivity;
	U64 m_ActivityUpdate;
};


//! CreateDeviceManager. Creates the device manager. Only one device manager can be created at once.
C_DeviceManager* CreateDeviceManager( const C_DeviceManagerCreateParam& param );

//! Destroys the device manager
void DestroyDeviceManager( C_DeviceManager* pDeviceManger );

//! Gets the current device manager
C_DeviceManager* GetDeviceManager();

//! @}

} // namespace Input

#endif


#ifndef INPUT_DEVICEBASE_H
#define INPUT_DEVICEBASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <input\Input.h>
#include <input\DeviceManager.h>

namespace Input
{

//! @addtogroup Input
//! @{

class C_DeviceManager;

//! Base class interface to a physical input device. Specific types of physical input device are inherited from this.
class C_Device
{
friend class C_DeviceManager;

public:
	C_Device(	C_DeviceManager* pDeviceManager, 
				const S32 userIndex, 
				const S32 buttonCount, 
				const S32 axisCount, 
				const S32 rumbleMotorCount );

	virtual ~C_Device();

	#ifndef CFG_GOLD
	bool m_Connected;
	#endif //CFG_GOLD

	//! Index of the signed-in player associated with the device.
	const S32 GetUserIndex() const;

	//! [Blur2PC] The device's type (E_Device), -1 if the device class sets none. Retail Blur PC's device
	//! constructors store it at +0x34 (keyboard 0x008451a0, mouse 0x00845c50, pad 0x00845e30, each given its
	//! type by C_DeviceManager::CreateDevice 0x00845aa0); C_ButtonInputFromDevice reports it.
	const S32 GetDeviceType() const { return m_DeviceType; }

	//! Returns true if this type of device is currently connected.
	const bool IsConnected() const;

	//! Returns the current number of buttons controls available for this device.
	const S32 GetButtonCount() const;

	//! Returns a value between 0.0f and 1.0f indicating how much a button is pressed.
	const F32 GetButtonValue( const S32 index ) const;

	//! Returns the current number of axis controls available for this device.
	const S32 GetAxisCount() const;

	//! Returns axis position value between -1.0f and 1.0f (centre is 0.0f).
	const F32 GetAxisSignedValue( const S32 index ) const;

	//! Enable the device. When a device is Disabled, all button and axis queries will return 0. Devices are Enabled by default.
	void Enable();

	//! Disable the device. When a device is Disabled, all button and axis queries will return 0. Devices are Enabled by default.
	void Disable();

	//! Returns true if this type of device is currently enabled.
	const bool IsEnabled() const;

	//! Returns the number of rumble motors on the device. Returns 0 if rumble not supported on this device.
	const U32 GetRumbleMotorCount() const;

	//! Enables rumble functionality on the device.
	void EnableRumble();

	//! Disables rumble functionality on the device.
	void DisableRumble();

	//! Returns if rumble functionality currently enabled.
	const bool IsRumbleEnabled() const;

	void SetRumbleMotorSpeed(const S32 motorIndex, const F32 motorSpeed);

private:
	// [Blur2PC] Physical activity for prompt selection, independent of game bindings.
	bool HasNewActivity();
	Ut::Vector<F32> m_PreviousButtonValues;
	Ut::Vector<F32> m_PreviousAxisValues;

	//! Called by the DeviceManager to read the device. This fills out m_ButtonValues, m_AxisValues.
	virtual void Update() = 0;

protected:
	//! Used by Update, sets all button and axis values to 0.
	void ResetValues();

	//! Used by Update, sets all rumble motor speeds to 0.
	void ResetRumbleMotorSpeeds();

protected:
	const S32 m_UserIndex;

	C_DeviceManager* m_pDeviceManager;


	#ifdef CFG_GOLD
	bool m_Connected;
	#endif //CFG_GOLD

	bool m_Enabled;

	bool m_RumbleEnabled;

	Ut::Vector<F32>	m_ButtonValues;

	Ut::Vector<F32>	m_AxisValues;

	Ut::Vector<F32>	m_RumbleMotorSpeeds;

	S32 m_DeviceType;	// [Blur2PC] E_Device, set by the derived constructor
};

//! @}

} // namespace Input

#endif

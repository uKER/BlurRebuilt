#ifndef INPUT_PADPS3_H
#define INPUT_PADPS3_H

#ifdef _MSC_VER
#pragma once
#endif

#include <input\Input.h>
#include <input\Device.h>

namespace Input
{

//! @addtogroup Input
//! @{

//! This class is for a PS3 SixAxis controller.
class C_PadPS3 : public C_Device
{
public:
	//! The available ps3 pad buttons
	enum E_Button
	{
		E_Button_Cross = 0,
		E_Button_Circle,
		E_Button_Square,
		E_Button_Triangle,

		E_Button_DPadUp,
		E_Button_DPadDown,
		E_Button_DPadLeft,
		E_Button_DPadRight,

		E_Button_Start,
		E_Button_Select,

		E_Button_L1,
		E_Button_R1,

		E_Button_L2,
		E_Button_R2,

		E_Button_L3,
		E_Button_R3,

		E_Button_Count
	};
	
	//! The available ps3 pad axis
	enum E_Axis
	{
		E_Axis_LeftStick_X = 0,
		E_Axis_LeftStick_Y,
		E_Axis_RightStick_X,
		E_Axis_RightStick_Y,

		E_Axis_Sensor_X,
		E_Axis_Sensor_Y,
		E_Axis_Sensor_Z,
		E_Axis_Sensor_G,

		E_Axis_Count
	};

	//! The available ps3 pad rumble motors
	enum E_RumbleMotor
	{
		E_RumbleMotor_LowFrequency = 0,
		E_RumbleMotor_HighFrequency,

		E_RumbleMotor_Count
	};

public:
	//! Create a C_PadPS3 using the C_DeviceManager class
	C_PadPS3( C_DeviceManager* pDeviceManager, const S32 userIndex );
	~C_PadPS3();

	//! Sets the deadzone on the Thumbstick Axes
	void SetStickDeadzone( const F32 deadzone );

	//! Inherited from C_Device
	void Update();

	void DumpDeviceState();

private:
	void UpdateValues();
	void UpdateRumble();

private:
	bool m_SupportsPressure;
	bool m_SupportsSensor;

	F32 m_StickDeadzone;
};

//! @}

} // namespace Input

#endif

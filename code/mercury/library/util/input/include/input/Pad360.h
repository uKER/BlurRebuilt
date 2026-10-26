#ifndef INPUT_PAD360_H
#define INPUT_PAD360_H

#ifdef _MSC_VER
#pragma once
#endif

#include <input\Input.h>
#include <input\Device.h>

namespace Input
{

//! @addtogroup Input
//! @{

//! This class is for an Xbox 360 pad connected to 360 or PC.
class C_Pad360 : public C_Device
{
public:
	//! The available 360 pad buttons
	enum E_Button
	{
		E_Button_A = 0,				
		E_Button_B,					
		E_Button_X,					
		E_Button_Y,					

		E_Button_DPadUp,			
		E_Button_DPadDown,			
		E_Button_DPadLeft,			
		E_Button_DPadRight,			

		E_Button_Start,				
		E_Button_Back,				
									
		E_Button_LeftShoulder,		
		E_Button_RightShoulder,		

		E_Button_LeftTrigger,		
		E_Button_RightTrigger,		

		E_Button_LeftThumb,			
		E_Button_RightThumb,		

		E_Button_Count
	};
	
	//! The available 360 pad axis
	enum E_Axis
	{
		E_Axis_LeftStick_X = 0,		
		E_Axis_LeftStick_Y,			
		E_Axis_RightStick_X,		
		E_Axis_RightStick_Y,		

		E_Axis_Count
	};

	//! The available 360 pad rumble motor
	enum E_RumbleMotor
	{
		E_RumbleMotor_LowFrequency = 0,	
		E_RumbleMotor_HighFrequency,	

		E_RumbleMotor_Count
	};

public:
	//! Create a C_Pad360 using the C_DeviceManager class
	C_Pad360( C_DeviceManager* pDeviceManager, const S32 userIndex );
	~C_Pad360();

	//! Sets the deadzone on the Thumbstick Axes
	void SetStickDeadzone( const F32 deadzone );

	//! Inherited from C_Device
	void Update();

	void DumpDeviceState();

private:
	void UpdateRumble();

private:
	F32 m_StickDeadzone;
};

//! @}

} // namespace Input

#endif

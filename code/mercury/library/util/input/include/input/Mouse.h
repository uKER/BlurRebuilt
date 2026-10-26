#ifndef INPUT_MOUSE_H
#define INPUT_MOUSE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <input\Input.h>
#include <input\Device.h>

namespace Input
{

//! @addtogroup Input
//! @{

//! Base class for mouse devices
class C_Mouse : public C_Device
{
public:
	//! The available mouse buttons
	enum E_Button
	{
		E_Button_Mouse1 = 0,				
		E_Button_Mouse2,					
		E_Button_Mouse3,					
		E_Button_Mouse4,					
		E_Button_Mouse5,					

		E_Button_WheelUp,					
		E_Button_WheelDown,					

		E_Button_Count,						

		E_Button_Left = E_Button_Mouse1,	
		E_Button_Right = E_Button_Mouse2,	
		E_Button_Middle = E_Button_Mouse3	
	};
	
	//! The available mouse axis
	enum E_Axis
	{
		E_Axis_DeltaX = 0,	
		E_Axis_DeltaY,		

		E_Axis_XPos,		
		E_Axis_YPos,		

		E_Axis_Wheel,		

		E_Axis_Count		
	};

	enum E_RumbleMotor
	{
		E_RumbleMotor_Count = 0
	};

public:
	C_Mouse( C_DeviceManager* pDeviceManager, const S32 userIndex )
		: C_Device(pDeviceManager, userIndex, E_Button_Count, E_Axis_Count, E_RumbleMotor_Count)
	{
	}

	virtual ~C_Mouse()
	{
	}


	//! Inherited from C_Device
	virtual void Update() = 0;
};

//! @}

} // namespace Input

#endif

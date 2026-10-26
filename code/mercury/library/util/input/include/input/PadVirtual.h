#ifndef PAD_VIRTUAL_H
#define PAD_VIRTUAL_H

#ifdef _MSC_VER
#pragma once
#endif

#include <input\Input.h>
#include <input\Device.h>

namespace Input
{

class C_VirtualPadState
{
public:
	C_VirtualPadState(S16, S16);
	void SetButtonValue(S16 button, F32 value);
	F32 GetButtonValue(S16 button);
	void SetAxisValue(S16 axis, F32 value);
	F32 GetAxisValue(S16 axis);
	void Reset();

private:
	S16 m_NoOfButtons;
	S16 m_NoOfAxis;
	Ut::Vector<F32>	m_ButtonValues;
	Ut::Vector<F32>	m_AxisValues;
};

#if defined(PLATFORM_PC)
class C_PadVirtual360 : public C_Device
{
public:
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
	
	enum E_Axis
	{
		E_Axis_LeftStick_X = 0,		
		E_Axis_LeftStick_Y,			
		E_Axis_RightStick_X,		
		E_Axis_RightStick_Y,		

		E_Axis_Count
	};

	enum E_RumbleMotor
	{
		E_RumbleMotor_LowFrequency = 0,	
		E_RumbleMotor_HighFrequency,	

		E_RumbleMotor_Count
	};

public:
		C_PadVirtual360( C_DeviceManager* pDeviceManager, const S32 userIndex );
	~C_PadVirtual360();

	void SetStickDeadzone( const F32 deadzone );

	void Update();

	void DumpDeviceState();

	void SetState(C_VirtualPadState* state);

private:
	void UpdateValues();
	void UpdateRumble();
	C_VirtualPadState* m_PadState;

private:
	bool m_SupportsPressure;
	bool m_SupportsSensor;

	F32 m_StickDeadzone;
};
#endif

#if defined(PLATFORM_PS3)
class C_PadVirtualPS3 : public C_Device
{
public:
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

	enum E_RumbleMotor
	{
		E_RumbleMotor_LowFrequency = 0,
		E_RumbleMotor_HighFrequency,

		E_RumbleMotor_Count
	};

public:
		C_PadVirtualPS3( C_DeviceManager* pDeviceManager, const S32 userIndex );
	~C_PadVirtualPS3();

	void SetStickDeadzone( const F32 deadzone );

	void Update();

	void DumpDeviceState();

	void SetState(C_VirtualPadState* state);

private:
	void UpdateValues();
	void UpdateRumble();
	C_VirtualPadState* m_PadState;

private:
	bool m_SupportsPressure;
	bool m_SupportsSensor;

	F32 m_StickDeadzone;
};
#endif

class C_VirtualPadHandler
{
private:
	static C_VirtualPadState* m_VirtualPadState;
	static C_Device* m_VirtualPads[4];

public:
	static C_Device*  GetVirtualPad(S16);
	static C_VirtualPadState*  GetVirtualPadState();
	static void DisconnectVirtualPads();
};

}

#endif

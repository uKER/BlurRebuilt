#include "stdafx.h"

#include <input\Pad360.h>

namespace Input
{

F32 ConvertPadAxisValue(short value)
{
	return ((F32)value - 0.5f) / 32767.5f;
}

C_Pad360::C_Pad360( C_DeviceManager* pDeviceManager, const S32 userIndex )
	: C_Device( pDeviceManager, userIndex, E_Button_Count, E_Axis_Count, E_RumbleMotor_Count )
	, m_StickDeadzone(0.25f)
{
	m_DeviceType = E_Device_Pad360;	// [Blur2PC] retail 0x00845e30
}

C_Pad360::~C_Pad360()
{
}

void C_Pad360::SetStickDeadzone( const F32 deadzone )
{
	m_StickDeadzone = deadzone;
}

void C_Pad360::Update()
{
	XINPUT_STATE xInputState;

	ZeroMemory(&xInputState, sizeof(XINPUT_STATE));

	U32 status = XInputGetState(m_UserIndex, &xInputState);

	if(status == ERROR_SUCCESS)
	{
		m_Connected = true;
	}
	else
	{
		m_Connected = false;
	}

	if(!m_Enabled || !m_Connected)
	{
		ResetValues();		
	}
	else
	{
		// set button states from XInput state
		m_ButtonValues[E_Button_A] = (xInputState.Gamepad.wButtons & XINPUT_GAMEPAD_A) ? 1.0f : 0.0f;
		m_ButtonValues[E_Button_B] = (xInputState.Gamepad.wButtons & XINPUT_GAMEPAD_B) ? 1.0f : 0.0f;
		m_ButtonValues[E_Button_X] = (xInputState.Gamepad.wButtons & XINPUT_GAMEPAD_X) ? 1.0f : 0.0f;
		m_ButtonValues[E_Button_Y] = (xInputState.Gamepad.wButtons & XINPUT_GAMEPAD_Y) ? 1.0f : 0.0f;

		m_ButtonValues[E_Button_DPadUp] = (xInputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) ? 1.0f : 0.0f;
		m_ButtonValues[E_Button_DPadDown] = (xInputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? 1.0f : 0.0f;
		m_ButtonValues[E_Button_DPadLeft] = (xInputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? 1.0f : 0.0f;
		m_ButtonValues[E_Button_DPadRight] = (xInputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? 1.0f : 0.0f;

		m_ButtonValues[E_Button_Start] = (xInputState.Gamepad.wButtons & XINPUT_GAMEPAD_START) ? 1.0f : 0.0f;
		m_ButtonValues[E_Button_Back] = (xInputState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ? 1.0f : 0.0f;

		m_ButtonValues[E_Button_LeftShoulder] = (xInputState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 1.0f : 0.0f;
		m_ButtonValues[E_Button_RightShoulder] = (xInputState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? 1.0f : 0.0f;

		m_ButtonValues[E_Button_LeftTrigger] = xInputState.Gamepad.bLeftTrigger / 255.0f;
		m_ButtonValues[E_Button_RightTrigger] = xInputState.Gamepad.bRightTrigger / 255.0f;

		m_ButtonValues[E_Button_LeftThumb] = (xInputState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? 1.0f : 0.0f;
		m_ButtonValues[E_Button_RightThumb] = (xInputState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? 1.0f : 0.0f;

		// set axis states from XInput state
		m_AxisValues[E_Axis_LeftStick_X] = ConvertPadAxisValue( xInputState.Gamepad.sThumbLX );
		m_AxisValues[E_Axis_LeftStick_Y] = ConvertPadAxisValue( xInputState.Gamepad.sThumbLY );
		m_AxisValues[E_Axis_RightStick_X] = ConvertPadAxisValue( xInputState.Gamepad.sThumbRX );
		m_AxisValues[E_Axis_RightStick_Y] = ConvertPadAxisValue( xInputState.Gamepad.sThumbRY );

		ApplyCircularDeadzone(m_AxisValues[E_Axis_LeftStick_X], m_AxisValues[E_Axis_LeftStick_Y], m_StickDeadzone);
		ApplyCircularDeadzone(m_AxisValues[E_Axis_RightStick_X], m_AxisValues[E_Axis_RightStick_Y], m_StickDeadzone);
	}

	UpdateRumble();
}

void C_Pad360::UpdateRumble()
{
	// update device rumble
	XINPUT_VIBRATION xInputVibration;

	if(m_RumbleEnabled && m_Connected)
	{
		xInputVibration.wLeftMotorSpeed = (WORD)(m_RumbleMotorSpeeds[E_RumbleMotor_LowFrequency] * 65535.0f);
		xInputVibration.wRightMotorSpeed = (WORD)(m_RumbleMotorSpeeds[E_RumbleMotor_HighFrequency] * 65535.0f);
	}
	else
	{
		xInputVibration.wLeftMotorSpeed = 0;
		xInputVibration.wRightMotorSpeed = 0;
	}

	XInputSetState(m_UserIndex, &xInputVibration);	// what do we do if this fails?
}

void C_Pad360::DumpDeviceState()
{
	#ifndef CFG_GOLD
	const char buttonNames[Input::C_Pad360::E_Button_Count][16] = 
	{
		"A",			
		"B",					
		"X",					
		"Y",					

		"DPadUp",
		"DPadDown",			
		"DPadLeft",			
		"DPadRight",			

		"Start",				
		"Back",				
						
		"LeftShoulder",		
		"RightShoulder",		

		"LeftTrigger",		
		"RightTrigger",		

		"LeftThumb",			
		"RightThumb",
	};
	
	const char axisNames[Input::C_Pad360::E_Axis_Count][16] = 
	{
		"LeftStick_X",
		"LeftStick_Y",				
		"RightStick_X",				
		"RightStick_Y"				
	};

	LOGINFO( g_Log, "\n");		
	LOGINFO( g_Log, "\n");		

	if(IsConnected())
	{
		LOGINFO( g_Log, "CONNECTED\n");		
	}
	else
	{
		LOGINFO( g_Log, "NOT CONNECTED\n");		
	}

	for(S32 i=0; i<Input::C_Pad360::E_Button_Count; i++)
	{
		F32 buttonValue = GetButtonValue( i );

		LOGINFO( g_Log, "Button[%s] : %f\n", buttonNames[i], buttonValue);		
	}

	for(S32 i=0; i<Input::C_Pad360::E_Axis_Count; i++)
	{
		F32 axisValue = GetAxisSignedValue( i );

		LOGINFO( g_Log, "Axis[%s] : %f\n", axisNames[i], axisValue);		
	}
	#endif
}


} // namespace Input

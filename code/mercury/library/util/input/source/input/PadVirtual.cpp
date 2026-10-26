#include "stdafx.h"

#include <input\PadVirtual.h>
#include <input\DeviceManager.h>
#include <input\PadPS3.h>
#include <input\Pad360.h>

#ifndef CFG_GOLD
namespace Input
{

C_Device* C_VirtualPadHandler::m_VirtualPads[4];
C_VirtualPadState* C_VirtualPadHandler::m_VirtualPadState;

C_Device* C_VirtualPadHandler::GetVirtualPad(S16 pad)
{
	if(m_VirtualPads[pad] == 0)
	{
		#if defined(PLATFORM_PC)
			m_VirtualPads[pad] = new C_PadVirtual360(GetDeviceManager(),0);
			m_VirtualPadState = new C_VirtualPadState(m_VirtualPads[pad]->GetButtonCount(), m_VirtualPads[pad]->GetAxisCount());
			GetDeviceManager()->CreateDevice(pad, E_Device_PadVirtual360, &m_VirtualPads[0]);
		#endif

		#if defined(PLATFORM_PS3)
			m_VirtualPads[pad] = new C_PadVirtualPS3(GetDeviceManager(),0);
			m_VirtualPadState = new C_VirtualPadState(m_VirtualPads[pad]->GetButtonCount(), m_VirtualPads[pad]->GetAxisCount());
			GetDeviceManager()->CreateDevice(pad, E_Device_PadVirtualPS3, &m_VirtualPads[0]);
		#endif
	}
	return m_VirtualPads[pad];
}

C_VirtualPadState* C_VirtualPadHandler::GetVirtualPadState()
{
	return m_VirtualPadState;
}

void C_VirtualPadHandler::DisconnectVirtualPads()
{
	for(int i = 0; i < 4; i++)
	{
		if(m_VirtualPads[i] != 0)
		{
			m_VirtualPads[i]->Disable();
			delete m_VirtualPads[i];
			m_VirtualPads[i] = 0;
		}
	}
}

C_VirtualPadState::C_VirtualPadState(S16 noOfButtons, S16 noOfAxis)
{
	m_NoOfButtons = noOfButtons;
	m_NoOfAxis = noOfAxis;
	m_ButtonValues.SetCapacity(noOfButtons);
	m_AxisValues.SetCapacity(noOfAxis);

	for(S16 b = 0; b < m_NoOfButtons; b++)
	{
		m_ButtonValues.PushBack(0.0f);
	}

	for(S16 a = 0; a < m_NoOfAxis; a++)
	{
		m_AxisValues.PushBack(0.0f);
	}

	Reset();
}
	
void C_VirtualPadState::SetButtonValue(S16 button, F32 value)
{
	m_ButtonValues[button] = value;
}
	
F32 C_VirtualPadState::GetButtonValue(S16 button)
{
	return m_ButtonValues[button];
}

void C_VirtualPadState::SetAxisValue(S16 axis, F32 value)
{
	m_AxisValues[axis] = value;
}
	
F32 C_VirtualPadState::GetAxisValue(S16 axis)
{
	return m_AxisValues[axis];
}

void C_VirtualPadState::Reset()
{
	for(S16 b = 0; b < m_NoOfButtons; b++)
	{
		m_ButtonValues[b] = 0;
	}

	for(S16 a = 0; a < m_NoOfAxis; a++)
	{
		m_AxisValues[a] = 0;
	}
}

#if defined(PLATFORM_PC)
void C_PadVirtual360::SetState(C_VirtualPadState* state)
{
	m_PadState = state;
}

C_PadVirtual360::C_PadVirtual360( C_DeviceManager* pDeviceManager, const S32 userIndex )
: C_Device( pDeviceManager, userIndex, E_Button_Count, 
				  E_Axis_Count, E_RumbleMotor_Count )
	, m_SupportsPressure(false)
	, m_SupportsSensor(false)
	, m_StickDeadzone(0.2f)
{
	m_DeviceType = E_Device_PadVirtual360;	// [Blur2PC] see C_Device::GetDeviceType
}

C_PadVirtual360::~C_PadVirtual360()
{
}

void C_PadVirtual360::SetStickDeadzone( const F32 deadzone )
{
	m_StickDeadzone = deadzone;
}

void C_PadVirtual360::Update()
{
	GetDeviceManager()->GetDevice(0)->m_Connected = true;
	UpdateValues();
	//UpdateRumble();
}

void C_PadVirtual360::UpdateValues()
{
	m_Connected = true;

	C_VirtualPadState* state = C_VirtualPadHandler::GetVirtualPadState();

	m_ButtonValues[E_Button_DPadUp] = state->GetButtonValue(E_Button_DPadUp);
	m_ButtonValues[E_Button_DPadDown] = state->GetButtonValue(E_Button_DPadDown);
	m_ButtonValues[E_Button_DPadLeft] = state->GetButtonValue(E_Button_DPadLeft);
	m_ButtonValues[E_Button_DPadRight] = state->GetButtonValue(E_Button_DPadRight);

	m_ButtonValues[E_Button_A] = state->GetButtonValue(E_Button_A);
	m_ButtonValues[E_Button_X] = state->GetButtonValue(E_Button_X);
	m_ButtonValues[E_Button_Y] = state->GetButtonValue(E_Button_Y);
	m_ButtonValues[E_Button_B] = state->GetButtonValue(E_Button_B);

	m_ButtonValues[E_Button_LeftTrigger] = state->GetButtonValue(E_Button_LeftTrigger) / 255.0f;
	m_ButtonValues[E_Button_RightTrigger] = state->GetButtonValue(E_Button_RightTrigger) / 255.0f;

	m_ButtonValues[E_Button_LeftShoulder] = state->GetButtonValue(E_Button_LeftShoulder);
	m_ButtonValues[E_Button_RightShoulder] = state->GetButtonValue(E_Button_RightShoulder);

	m_ButtonValues[E_Button_LeftThumb] = state->GetButtonValue(E_Button_LeftThumb);
	m_ButtonValues[E_Button_RightThumb] = state->GetButtonValue(E_Button_RightThumb);

	m_ButtonValues[E_Button_Start] = state->GetButtonValue(E_Button_Start);
	m_ButtonValues[E_Button_Back] = state->GetButtonValue(E_Button_Back);

	m_AxisValues[E_Axis_LeftStick_X] = state->GetAxisValue(E_Axis_LeftStick_X);
	m_AxisValues[E_Axis_LeftStick_Y] = state->GetAxisValue(E_Axis_LeftStick_Y);
	m_AxisValues[E_Axis_RightStick_X] = state->GetAxisValue(E_Axis_RightStick_X);
	m_AxisValues[E_Axis_RightStick_Y] = state->GetAxisValue(E_Axis_RightStick_Y);

	ApplyCircularDeadzone(m_AxisValues[E_Axis_LeftStick_X], m_AxisValues[E_Axis_LeftStick_Y], m_StickDeadzone);
	ApplyCircularDeadzone(m_AxisValues[E_Axis_RightStick_X], m_AxisValues[E_Axis_RightStick_Y], m_StickDeadzone);

	if(m_ButtonValues[E_Button_RightTrigger] > 0)
	{

	}
}

void C_PadVirtual360::UpdateRumble()
{
	//No rumble support added
}

void C_PadVirtual360::DumpDeviceState()
{
	const char buttonNames[C_PadVirtual360::E_Button_Count][16] = 
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
	
	const char axisNames[C_PadVirtual360::E_Axis_Count][16] = 
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

	for(S32 i=0; i<C_PadVirtual360::E_Button_Count; i++)
	{
		F32 buttonValue = GetButtonValue( i );

		LOGINFO( g_Log, "Button[%s] : %f\n", buttonNames[i], buttonValue);		
	}

	for(S32 i=0; i<C_PadVirtual360::E_Axis_Count; i++)
	{
		F32 axisValue = GetAxisSignedValue( i );

		LOGINFO( g_Log, "Axis[%s] : %f\n", axisNames[i], axisValue);		
	}

}
#endif

#if defined(PLATFORM_PS3)
C_PadVirtualPS3::C_PadVirtualPS3( C_DeviceManager* pDeviceManager, const S32 userIndex )
: C_Device( pDeviceManager, userIndex, E_Button_Count, 
				  E_Axis_Count, E_RumbleMotor_Count )
	, m_SupportsPressure(false)
	, m_SupportsSensor(false)
	, m_StickDeadzone(0.2f)
{
}

C_PadVirtualPS3::~C_PadVirtualPS3()
{
}

void C_PadVirtualPS3::SetState(C_VirtualPadState* state)
{
	m_PadState = state;
}

void C_PadVirtualPS3::SetStickDeadzone( const F32 deadzone )
{
	m_StickDeadzone = deadzone;
}

void C_PadVirtualPS3::Update()
{
	GetDeviceManager()->GetDevice(0)->m_Connected = true;
	UpdateValues();
	//UpdateRumble();
}

void C_PadVirtualPS3::UpdateValues()
{
	C_VirtualPadState* state = C_VirtualPadHandler::GetVirtualPadState();

	m_Connected = true; 

	m_ButtonValues[E_Button_DPadUp] = state->GetButtonValue(E_Button_DPadUp);
	m_ButtonValues[E_Button_DPadDown] = state->GetButtonValue(E_Button_DPadDown);
	m_ButtonValues[E_Button_DPadLeft] = state->GetButtonValue(E_Button_DPadLeft);
	m_ButtonValues[E_Button_DPadRight] = state->GetButtonValue(E_Button_DPadRight);

	m_ButtonValues[E_Button_Cross] = state->GetButtonValue(E_Button_Cross);
	m_ButtonValues[E_Button_Circle] = state->GetButtonValue(E_Button_Circle);
	m_ButtonValues[E_Button_Square] = state->GetButtonValue(E_Button_Square);
	m_ButtonValues[E_Button_Triangle] = state->GetButtonValue(E_Button_Triangle);

	m_ButtonValues[E_Button_L1] = state->GetButtonValue(E_Button_L1);
	m_ButtonValues[E_Button_R1] = state->GetButtonValue(E_Button_R1);

	m_ButtonValues[E_Button_L2] = state->GetButtonValue(E_Button_L2) / 255.0f;
	m_ButtonValues[E_Button_R2] = state->GetButtonValue(E_Button_R2) / 255.0f;

	m_ButtonValues[E_Button_L3] = state->GetButtonValue(E_Button_L3);
	m_ButtonValues[E_Button_R3] = state->GetButtonValue(E_Button_R3);

	m_ButtonValues[E_Button_Start] = state->GetButtonValue(E_Button_Start);
	m_ButtonValues[E_Button_Select] = state->GetButtonValue(E_Button_Select);

	m_AxisValues[E_Axis_LeftStick_X] = state->GetAxisValue(E_Axis_LeftStick_X);
	m_AxisValues[E_Axis_LeftStick_Y] = state->GetAxisValue(E_Axis_LeftStick_Y);
	m_AxisValues[E_Axis_RightStick_X] = state->GetAxisValue(E_Axis_RightStick_X);
	m_AxisValues[E_Axis_RightStick_Y] = state->GetAxisValue(E_Axis_RightStick_Y);

	ApplyCircularDeadzone(m_AxisValues[E_Axis_LeftStick_X], m_AxisValues[E_Axis_LeftStick_Y], m_StickDeadzone);
	ApplyCircularDeadzone(m_AxisValues[E_Axis_RightStick_X], m_AxisValues[E_Axis_RightStick_Y], m_StickDeadzone);


	m_AxisValues[E_Axis_Sensor_X] = 0;
	m_AxisValues[E_Axis_Sensor_Y] = 0;
	m_AxisValues[E_Axis_Sensor_Z] = 0;
	m_AxisValues[E_Axis_Sensor_G] = 0;
}

void C_PadVirtualPS3::UpdateRumble()
{
	//No rumble support added
}

void C_PadVirtualPS3::DumpDeviceState()
{
	const char buttonNames[C_PadVirtualPS3::E_Button_Count][16] = 
	{
		"Cross",
		"Circle",
		"Square",
		"Triangle",
		 
		"DPadUp",
		"DPadDown",
		"DPadLeft",
		"DPadRight",
		 
		"Start",
		"Select",
		 
		"L1",
		"R1",
		 
		"L2",
		"R2",
		 
		"L3",
		"R3"
	};
	
	const char axisNames[C_PadVirtualPS3::E_Axis_Count][16] = 
	{
		"LeftStick_X",
		"LeftStick_Y",				
		"RightStick_X",				
		"RightStick_Y",				

		"Sensor_X",				
		"Sensor_Y",				
		"Sensor_Z",				
		"Sensor_G"
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

	for(S32 i=0; i<C_PadVirtualPS3::E_Button_Count; i++)
	{
		F32 buttonValue = GetButtonValue( i );

		LOGINFO( g_Log, "Button[%s] : %f\n", buttonNames[i], buttonValue);		
	}

	for(S32 i=0; i<C_PadVirtualPS3::E_Axis_Count; i++)
	{
		F32 axisValue = GetAxisSignedValue( i );

		LOGINFO( g_Log, "Axis[%s] : %f\n", axisNames[i], axisValue);		
	}
}
#endif

} //End of namespace Input
#endif
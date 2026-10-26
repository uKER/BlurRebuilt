#include "stdafx.h"

#include <input\PadPS3.h>
#include <input\DeviceManagerPS3.h>
#include <cell/pad.h>      

namespace Input
{

C_PadPS3::C_PadPS3( C_DeviceManager* pDeviceManager, const S32 userIndex )
	: C_Device( pDeviceManager, userIndex, E_Button_Count, E_Axis_Count, E_RumbleMotor_Count )
	, m_SupportsPressure(false)
	, m_SupportsSensor(false)
	, m_StickDeadzone(0.2f)
{
}

C_PadPS3::~C_PadPS3()
{
}

void C_PadPS3::SetStickDeadzone( const F32 deadzone )
{
	m_StickDeadzone = deadzone;
}

void C_PadPS3::Update()
{
	bool wasConnected = m_Connected;

	C_DeviceManagerPS3* pDeviceManagerPS3 = (C_DeviceManagerPS3*)m_pDeviceManager;

#if CELL_SDK_VERSION >= 0x250000
	if ((pDeviceManagerPS3->m_PadInfo.port_status[m_UserIndex] & 1) == 0) 
#else
	if (pDeviceManagerPS3->m_PadInfo.status[m_UserIndex] == 0) 
#endif
	{
		m_Connected = false;
	}
	else
	{
		m_Connected = true;
	}

	// Gamepad has been connected
	if (m_Connected && !wasConnected) 
	{
		// It is a new connection
#if CELL_SDK_VERSION >= 0x250000
		LOGINFO (g_Log, "New gamepad %d is connected\n", m_UserIndex );
#else
		LOGINFO (g_Log, "New gamepad %d is connected: VENDOR_ID=%d PRODUCT_ID=%d\n",
					m_UserIndex, pDeviceManagerPS3->m_PadInfo.vendor_id[m_UserIndex], pDeviceManagerPS3->m_PadInfo.product_id[m_UserIndex]);
#endif

		// check if device has pressure sensitivity support
#if CELL_SDK_VERSION >= 0x250000
		uint32_t padMode = pDeviceManagerPS3->m_PadInfo.port_setting[m_UserIndex];

		if(pDeviceManagerPS3->m_PadInfo.device_capability[m_UserIndex] & CELL_PAD_CAPABILITY_PRESS_MODE)
		{
			padMode |= CELL_PAD_SETTING_PRESS_ON;
			m_SupportsPressure = true;
		}
		else
			m_SupportsPressure = false;
		
		if(pDeviceManagerPS3->m_PadInfo.device_capability[m_UserIndex] & CELL_PAD_CAPABILITY_SENSOR_MODE)
		{
			padMode |= CELL_PAD_SETTING_SENSOR_ON;
			m_SupportsSensor = true;
		}		
		else
			m_SupportsSensor = false;

		cellPadSetPortSetting(m_UserIndex, padMode); // TODO: handle error codes
#else
		if(cellPadInfoPressMode(m_UserIndex) == CELL_PAD_INFO_SUPPORTED_PRESS_MODE)
		{
			cellPadSetPressMode(m_UserIndex, CELL_PAD_PRESS_MODE_ON); // TODO: handle error codes
			m_SupportsPressure = true;
		}
		else
			m_SupportsPressure = false;

		if( cellPadInfoSensorMode(m_UserIndex) == CELL_PAD_INFO_SUPPORTED_SENSOR_MODE)
		{
			cellPadSetSensorMode(m_UserIndex, CELL_PAD_SENSOR_MODE_ON); // TODO: handle error codes
			m_SupportsSensor = true;
		}		
		else
			m_SupportsSensor = false;
#endif
	}

	if(!m_Enabled || !m_Connected)
	{
		ResetValues();		
	}
	else
	{
		UpdateValues();
	}

	UpdateRumble();
}

const F32 ConvertPadAxisValue( const U8 value )
{
	return ((float)value / 255.0f) * 2.0f - 1.0f;
}

void C_PadPS3::UpdateValues()
{
	CellPadData padData;

	U32 result = cellPadGetData(m_UserIndex, &padData);
	if((result != 0) || (padData.len == 0))
	{
		bool systemHudActive = false;
#if CELL_SDK_VERSION >= 0x250000
		CellPadInfo2 info;
		if (cellPadGetInfo2(&info) == CELL_PAD_OK)
			systemHudActive = (info.system_info & CELL_PAD_INFO_INTERCEPTED) ? true : false;
#else
		CellPadInfo info;
		if (cellPadGetInfo(&info) == CELL_PAD_OK)
			systemHudActive = (info.info & CELL_PAD_INFO_INTERCEPTED) ? true : false;
#endif

		if(/*!m_Enabled || !m_Connected ||*/ systemHudActive)
			ResetValues();

		// no new values from device, use previous values
		return;
	}

	if(m_SupportsPressure)
	{
		m_ButtonValues[E_Button_DPadUp] = (F32)padData.button[CELL_PAD_BTN_OFFSET_PRESS_UP] * (1.0f / 255.0f);
		m_ButtonValues[E_Button_DPadDown] = (F32)padData.button[CELL_PAD_BTN_OFFSET_PRESS_DOWN] * (1.0f / 255.0f);
		m_ButtonValues[E_Button_DPadLeft] = (F32)padData.button[CELL_PAD_BTN_OFFSET_PRESS_LEFT] * (1.0f / 255.0f);
		m_ButtonValues[E_Button_DPadRight] = (F32)padData.button[CELL_PAD_BTN_OFFSET_PRESS_RIGHT] * (1.0f / 255.0f);

		m_ButtonValues[E_Button_Cross] = (F32)padData.button[CELL_PAD_BTN_OFFSET_PRESS_CROSS] * (1.0f / 255.0f);
		m_ButtonValues[E_Button_Circle] = (F32)padData.button[CELL_PAD_BTN_OFFSET_PRESS_CIRCLE] * (1.0f / 255.0f);
		m_ButtonValues[E_Button_Square] = (F32)padData.button[CELL_PAD_BTN_OFFSET_PRESS_SQUARE] * (1.0f / 255.0f);
		m_ButtonValues[E_Button_Triangle] = (F32)padData.button[CELL_PAD_BTN_OFFSET_PRESS_TRIANGLE] * (1.0f / 255.0f);

		m_ButtonValues[E_Button_L1] = (F32)padData.button[CELL_PAD_BTN_OFFSET_PRESS_L1] * (1.0f / 255.0f);
		m_ButtonValues[E_Button_R1] = (F32)padData.button[CELL_PAD_BTN_OFFSET_PRESS_R1] * (1.0f / 255.0f);

		m_ButtonValues[E_Button_L2] = (F32)padData.button[CELL_PAD_BTN_OFFSET_PRESS_L2] * (1.0f / 255.0f);
		m_ButtonValues[E_Button_R2] = (F32)padData.button[CELL_PAD_BTN_OFFSET_PRESS_R2] * (1.0f / 255.0f);
	}
	else
	{
		m_ButtonValues[E_Button_DPadUp] = (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_UP) ? 1.0f : 0.0f;
		m_ButtonValues[E_Button_DPadDown] = (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_DOWN) ? 1.0f : 0.0f;
		m_ButtonValues[E_Button_DPadLeft] = (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_LEFT) ? 1.0f : 0.0f;
		m_ButtonValues[E_Button_DPadRight] = (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_RIGHT) ? 1.0f : 0.0f;

		m_ButtonValues[E_Button_Cross] = (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CROSS) ? 1.0f : 0.0f;
		m_ButtonValues[E_Button_Circle] = (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CIRCLE) ? 1.0f : 0.0f;
		m_ButtonValues[E_Button_Square] = (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_SQUARE) ? 1.0f : 0.0f;
		m_ButtonValues[E_Button_Triangle] = (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_TRIANGLE) ? 1.0f : 0.0f;

		m_ButtonValues[E_Button_L1] = (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L1) ? 1.0f : 0.0f;
		m_ButtonValues[E_Button_R1] = (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R1) ? 1.0f : 0.0f;

		m_ButtonValues[E_Button_L2] = (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L2) ? 1.0f : 0.0f;
		m_ButtonValues[E_Button_R2] = (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R2) ? 1.0f : 0.0f;
	}	

	m_ButtonValues[E_Button_L3] = (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_L3) ? 1.0f : 0.0f;
	m_ButtonValues[E_Button_R3] = (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_R3) ? 1.0f : 0.0f;

	m_ButtonValues[E_Button_Start] = (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_START) ? 1.0f : 0.0f;
	m_ButtonValues[E_Button_Select] = (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_SELECT) ? 1.0f : 0.0f;
	
	// set axis states from cell pad state
	m_AxisValues[E_Axis_LeftStick_X] = ConvertPadAxisValue( padData.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_X] );
	m_AxisValues[E_Axis_LeftStick_Y] = -ConvertPadAxisValue( padData.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_Y] );
	m_AxisValues[E_Axis_RightStick_X] = ConvertPadAxisValue( padData.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_X] );
	m_AxisValues[E_Axis_RightStick_Y] = -ConvertPadAxisValue( padData.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_Y] );

	ApplyCircularDeadzone(m_AxisValues[E_Axis_LeftStick_X], m_AxisValues[E_Axis_LeftStick_Y], m_StickDeadzone);
	ApplyCircularDeadzone(m_AxisValues[E_Axis_RightStick_X], m_AxisValues[E_Axis_RightStick_Y], m_StickDeadzone);

	if(m_SupportsSensor)
	{
		m_AxisValues[E_Axis_Sensor_X] = ((F32)padData.button[CELL_PAD_BTN_OFFSET_SENSOR_X] / (F32)0x03FF) * 2.0f - 1.0f;
		m_AxisValues[E_Axis_Sensor_Y] = ((F32)padData.button[CELL_PAD_BTN_OFFSET_SENSOR_Y] / (F32)0x03FF) * 2.0f - 1.0f;
		m_AxisValues[E_Axis_Sensor_Z] = ((F32)padData.button[CELL_PAD_BTN_OFFSET_SENSOR_Z] / (F32)0x03FF) * 2.0f - 1.0f;
		m_AxisValues[E_Axis_Sensor_G] = ((F32)padData.button[CELL_PAD_BTN_OFFSET_SENSOR_G] / (F32)0x03FF) * 2.0f - 1.0f;
	}
	else
	{
		m_AxisValues[E_Axis_Sensor_X] = 0;
		m_AxisValues[E_Axis_Sensor_Y] = 0;
		m_AxisValues[E_Axis_Sensor_Z] = 0;
		m_AxisValues[E_Axis_Sensor_G] = 0;
	}
}

void C_PadPS3::UpdateRumble()
{
#if CELL_SDK_VERSION >= 0x250000
	C_DeviceManagerPS3* pDeviceManagerPS3 = (C_DeviceManagerPS3*)m_pDeviceManager;

	bool isActuator = pDeviceManagerPS3->m_PadInfo.device_capability[m_UserIndex] & CELL_PAD_CAPABILITY_ACTUATOR;
#else
	// update device rumble
	static CellPadCapabilityInfo capa_info; // TODO:Why is this static?
	cellPadGetCapabilityInfo(m_UserIndex, &capa_info);

	bool isActuator = CELL_PAD_IS_ACTUATOR(&capa_info);
#endif

	if (isActuator)
	{		
		CellPadActParam actuatorParameters;
		memset(&actuatorParameters, 0, sizeof(actuatorParameters)); // need to zero out for this to work in release.

		if(m_RumbleEnabled && m_Connected)
		{
			actuatorParameters.motor[0] = (m_RumbleMotorSpeeds[E_RumbleMotor_HighFrequency] > 0.05f )? 1 : 0;
			U32 lowSpeedVal = (U32)(m_RumbleMotorSpeeds[E_RumbleMotor_LowFrequency] * 255.0f * 4.0f);
			if(lowSpeedVal > 255)
				lowSpeedVal = 255;
			actuatorParameters.motor[1] = lowSpeedVal;
		}
		else
		{
			actuatorParameters.motor[0] = 0;
			actuatorParameters.motor[1] = 0;
		}

		cellPadSetActDirect( m_UserIndex, &actuatorParameters );
	}
}

void C_PadPS3::DumpDeviceState()
{
	#ifndef CFG_GOLD
	const char buttonNames[Input::C_PadPS3::E_Button_Count][16] = 
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
	
	const char axisNames[Input::C_PadPS3::E_Axis_Count][16] = 
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

	LOGINFO( Input::g_Log, "\n");		
	LOGINFO( Input::g_Log, "\n");		

	if(IsConnected())
	{
		LOGINFO( Input::g_Log, "CONNECTED\n");		
	}
	else
	{
		LOGINFO( Input::g_Log, "NOT CONNECTED\n");		
	}

	for(S32 i=0; i<Input::C_PadPS3::E_Button_Count; i++)
	{
		F32 buttonValue = GetButtonValue( i );

		LOGINFO( Input::g_Log, "Button[%s] : %f\n", buttonNames[i], buttonValue);		
	}

	for(S32 i=0; i<Input::C_PadPS3::E_Axis_Count; i++)
	{
		F32 axisValue = GetAxisSignedValue( i );

		LOGINFO( Input::g_Log, "Axis[%s] : %f\n", axisNames[i], axisValue);		
	}
	#endif
}

} // namespace Input

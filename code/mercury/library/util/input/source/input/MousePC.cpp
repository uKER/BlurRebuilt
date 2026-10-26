#include "stdafx.h"

#include <input\MousePC.h>
#include <system\Debug\Variables.h>
#include <system/time/time.h>

namespace Input
{

Debug::Float g_debugMouseSensitivity("Mouse.Sensitivity", 0.25f, Debug::Range(0.0f, 1.f));

C_MousePC::C_MousePC( C_DeviceManager* pDeviceManager, const S32 userIndex )
	: C_Mouse( pDeviceManager, userIndex )
{
	m_DeviceType = E_Device_Mouse;	// [Blur2PC] retail 0x00845c50
	for(U32 i=0; i<E_Axis_Count; i++)
	{
		m_AccumAxisState[i] = 0.0f;
	}
	
	m_LastUpdateTime = Time::GetPerformanceCounter();
}

C_MousePC::~C_MousePC()
{
}

void C_MousePC::Update()
{
	Time::C_Time timeNow = Time::GetPerformanceCounter();
	F32 deltaTime = (F32)(timeNow - m_LastUpdateTime) / (F32)Time::GetPerformanceFrequency();
	if (deltaTime < 0.001f)
		deltaTime = 0.001f;

	m_LastUpdateTime = timeNow;
	
	for(U32 i=0; i<E_Axis_Count; i++)
	{
		m_AxisValues[i] = m_AccumAxisState[i] / deltaTime * g_debugMouseSensitivity * 0.01f;
		m_AccumAxisState[i] = 0.0f;
	}
}

void C_MousePC::SetButtonValue( const U32 index, const F32 value )
{
	m_ButtonValues[index] = value;
}

void C_MousePC::SetAxisValue( const U32 index, const F32 value )
{
	m_AccumAxisState[index] += value;
}

} // namespace Input

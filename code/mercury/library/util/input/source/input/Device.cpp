#include "stdafx.h"

#include <input\Device.h>

namespace Input
{

C_Device::C_Device(	C_DeviceManager* pDeviceManager, 
							const S32 userIndex, 
							const S32 buttonCount, 
							const S32 axisCount, 
							const S32 rumbleMotorCount )
:	m_pDeviceManager(pDeviceManager),
	m_UserIndex(userIndex),
	m_Connected(false),
	m_Enabled(true),
	m_RumbleEnabled(false),
	m_DeviceType(-1)	// [Blur2PC]
{
	m_ButtonValues.Resize(buttonCount);
	m_AxisValues.Resize(axisCount);
	// [Blur2PC] Keep the last sample so held inputs and releases cannot steal prompts.
	m_PreviousButtonValues.Resize(buttonCount);
	m_PreviousAxisValues.Resize(axisCount);
	for (S32 i = 0; i < buttonCount; ++i) m_PreviousButtonValues[i] = 0.0f;
	for (S32 i = 0; i < axisCount; ++i) m_PreviousAxisValues[i] = 0.0f;

	m_RumbleMotorSpeeds.Resize(rumbleMotorCount);

	ResetValues();
	ResetRumbleMotorSpeeds();
}

C_Device::~C_Device()
{
	m_pDeviceManager->RemoveDevice(this);
}

// [Blur2PC] Intentional PC extension (T256), not recovered retail behavior.
bool C_Device::HasNewActivity()
{
	bool active = false;
	for (S32 i = 0; i < m_ButtonValues.Size(); ++i)
	{
		// XInput's trigger threshold; digital buttons use the same edge test.
		const F32 threshold = 30.0f / 255.0f;
		active |= m_ButtonValues[i] > threshold && m_PreviousButtonValues[i] <= threshold;
		m_PreviousButtonValues[i] = m_ButtonValues[i];
	}
	for (S32 i = 0; i < m_AxisValues.Size(); ++i)
	{
		if (m_DeviceType == E_Device_Mouse)
			active |= m_AxisValues[i] != 0.0f; // relative motion / wheel
		else
			// Sticks are already dead-zoned. Require deliberate displacement as well.
			active |= (m_AxisValues[i] > 0.25f && m_PreviousAxisValues[i] <= 0.25f) ||
				(m_AxisValues[i] < -0.25f && m_PreviousAxisValues[i] >= -0.25f);
		m_PreviousAxisValues[i] = m_AxisValues[i];
	}
	// The window-message mouse has no connection state (unlike DirectInput/XInput).
	return active && (m_Connected || m_DeviceType == E_Device_Mouse) && m_Enabled;
}

const S32 C_Device::GetUserIndex() const
{
	return m_UserIndex;
}

const bool C_Device::IsConnected() const
{
	return m_Connected;
}

const S32 C_Device::GetButtonCount() const
{
	return m_ButtonValues.Size();
}

const F32 C_Device::GetButtonValue( const S32 index ) const
{
	return m_ButtonValues[index];
}

const S32 C_Device::GetAxisCount() const
{
	return m_AxisValues.Size();
}

const F32 C_Device::GetAxisSignedValue( const S32 index ) const
{
	return m_AxisValues[index];
}

void C_Device::Enable()
{
	m_Enabled = false;
}

void C_Device::Disable()
{
	m_Enabled = true;
}

const bool C_Device::IsEnabled() const
{
	return m_Enabled;
}

void C_Device::ResetValues()
{
	const S32 buttonCount = m_ButtonValues.Size();
	for(S32 i=0; i<buttonCount; i++)
	{
		m_ButtonValues[i] = 0;
	}

	const S32 axisCount = m_AxisValues.Size();
	for(S32 i=0; i<axisCount; i++)
	{
		m_AxisValues[i] = 0;
	}
}

void C_Device::ResetRumbleMotorSpeeds()
{
	const S32 rumbleMotorCount = m_RumbleMotorSpeeds.Size();
	for(S32 i=0; i<rumbleMotorCount; i++)
	{
		m_RumbleMotorSpeeds[i] = 0;
	}
}

const U32 C_Device::GetRumbleMotorCount() const
{
	return m_RumbleMotorSpeeds.Size();
}

void C_Device::EnableRumble()
{
	m_RumbleEnabled = true;
}

void C_Device::DisableRumble()
{
	m_RumbleEnabled = false;
}

const bool C_Device::IsRumbleEnabled() const
{
	return m_RumbleEnabled;
}

void C_Device::SetRumbleMotorSpeed(const S32 motorIndex, const F32 motorSpeed)
{
	m_RumbleMotorSpeeds[motorIndex] = motorSpeed;
}

} // namespace Input

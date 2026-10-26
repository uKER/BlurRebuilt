#ifndef INPUT_MOUSEPC_H
#define INPUT_MOUSEPC_H

#ifdef _MSC_VER
#pragma once
#endif

#include <input\Input.h>
#include <system\time\Time.h>
#include <input\Device.h>
#include <input\Mouse.h>

namespace Input
{

//! @addtogroup Input
//! @{

//! The PC Implementation of a C_Mouse. This class does not actually read the device as this would be done in the windows message loop. Instead the current values can be set directly using the SetButtonValue and SetAxisValue methods.
class C_MousePC : public C_Mouse
{
public:
	//! Create a C_MousePC using the C_DeviceManager class
	C_MousePC( C_DeviceManager* pDeviceManager, const S32 userIndex );
	~C_MousePC();

	//! Inherited from C_Device
	void Update();

	void SetButtonValue( const U32 index, const F32 value );
	void SetAxisValue( const U32 index, const F32 value );

private:
	F32						m_AccumAxisState[E_Axis_Count];

	F32						m_UpdateTimestep;
	bool					m_FirstUpdate;
	Time::C_Time			m_LastUpdateTime;
};

//! @}

} // namespace Input

#endif

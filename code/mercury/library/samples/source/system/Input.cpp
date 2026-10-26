#include "StdAfx.h"
#include "Samples/System/Input.h"

namespace Samples
{

C_Input::C_Input()
{
	Reset();
}

C_Input::~C_Input()
{
}

void C_Input::Reset()
{
	memset(m_ButtonState, 0, sizeof(float) * E_Button_Count);
	memset(m_Button, 0, sizeof(bool) * E_Button_Count);
	memset(m_Key, 0, sizeof(bool) * E_Key_Count);
	memset(m_Axis, 0, sizeof(F32) * E_Axis_Count);
}

void C_Input::SetButtonState(const E_Button button, float state)
{
	assert(button < E_Button_Count);
	m_ButtonState[button] = state;
}

float C_Input::GetButtonState(const E_Button button) const
{
	assert(button < E_Button_Count);
	return m_ButtonState[button];
}

bool C_Input::IsButtonUp(const E_Button button) const
{
	assert(button < E_Button_Count);
	return !m_Button[button];
}

bool C_Input::IsButtonDn(const E_Button button) const
{
	assert(button < E_Button_Count);
	return m_Button[button];
}

void C_Input::ButtonUp(const E_Button button)
{
	assert(button < E_Button_Count);
	m_Button[button] = false;
}

void C_Input::ButtonDn(const E_Button button)
{
	assert(button < E_Button_Count);
	m_Button[button] = true;
}

bool C_Input::IsKeyUp(const U32 key) const
{
	return !m_Key[key];
}

bool C_Input::IsKeyDn(const U32 key) const
{
	return m_Key[key];
}

void C_Input::KeyUp(const U32 key)
{
	m_Key[key] = false;
}

void C_Input::KeyDn(const U32 key)
{
	m_Key[key] = true;
}

void C_Input::AxisValue( const E_Axis axis, const F32 fValue )
{
	m_Axis[ axis ] = fValue;
}

F32 C_Input::AxisValue( const E_Axis axis ) const
{
	return m_Axis[ axis ];
}

}; // namespace Samples

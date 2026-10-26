#ifndef SAMPLES_INPUT_H
#define SAMPLES_INPUT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Samples
{

enum E_Button
{
	E_Button_LtMouse = 0,
	E_Button_RtMouse,
	E_Button_A,
	E_Button_B,
	E_Button_X,
	E_Button_Y,
	E_Button_Start,
	E_Button_Back,
	E_Button_LtShoulder,
	E_Button_RtShoulder,
	E_Button_LtTrigger,
	E_Button_RtTrigger,
	E_Button_LtThumb,
	E_Button_RtThumb,
	E_Button_LtPad,
	E_Button_RtPad,
	E_Button_UpPad,
	E_Button_DnPad,
	E_Button_Count,
};

enum E_Key
{
	E_Key_Count = 256,
};

enum E_Axis
{
	E_Axis_LX = 0,
	E_Axis_LY,
	E_Axis_RX,
	E_Axis_RY,

	E_Axis_Count,
};

class C_Input
{
public:

	C_Input();
	~C_Input();

	void Reset();

	void SetButtonState(const E_Button button, float state);
	float GetButtonState(const E_Button button) const;

	bool IsButtonUp(const E_Button button) const;
	bool IsButtonDn(const E_Button button) const;

	void ButtonUp(const E_Button button);
	void ButtonDn(const E_Button button);

	bool IsKeyUp(const U32 key) const;
	bool IsKeyDn(const U32 key) const;

	void KeyUp(const U32 key);
	void KeyDn(const U32 key);

	void	AxisValue( const E_Axis axis, const F32 fValue );
	F32		AxisValue( const E_Axis axis ) const;

private:

	float m_ButtonState[E_Button_Count];
	bool  m_Button[E_Button_Count];
	bool  m_Key[E_Key_Count];
	F32   m_Axis[E_Axis_Count];
};

}; // namespace Samples

#endif // SAMPLES_INPUT_H

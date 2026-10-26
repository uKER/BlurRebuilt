#ifndef GAMEPAD_H
#define GAMEPAD_H

#ifdef _MSC_VER
#pragma once
#endif

#include "btl/containers/vector.h"

class GamePad
{
public:
	enum Button
	{
		Button_A,
		Button_B,
		Button_X,
		Button_Y,
		Button_Start,
		Button_Back,
		Button_LeftShoulder,
		Button_RightShoulder,
		Button_LeftThumb,
		Button_RightThumb,
		Button_DPadUp,
		Button_DPadDown,
		Button_DPadLeft,
		Button_DPadRight,
		Button_LeftTrigger,
		Button_RightTrigger,

		Button_Count,
	};

	enum Axis
	{
		Axis_LX,
		Axis_LY,
		Axis_RX,
		Axis_RY,

		Axis_Count
	};

	GamePad();

	void Clear();

	bool GetDigitalPressed(Button button) const {return m_padData.digitalPressed[button];}
	bool GetDigitalHeld(Button button) const {return m_padData.digitalHeld[button];}
	float GetAnalogue(Button button) const {return m_padData.analogue[button];}
	float GetAxis(Axis axis) const {return m_padData.thumbsticks[axis];}

	void ReadControls();

private:
	struct PadData
	{
		Btl::Vector<bool, Btl::VectorParameters_Array<Button_Count> > digitalHeld;
		Btl::Vector<bool, Btl::VectorParameters_Array<Button_Count> > digitalPressed;
		Btl::Vector<float, Btl::VectorParameters_Array<Button_Count> > analogue;
		Btl::Vector<float, Btl::VectorParameters_Array<Axis_Count> > thumbsticks;

		void Clear() {memset(this, 0, sizeof(this));} 
	};

	PadData m_padData;
};

#endif // SAMPLES_INPUT_H

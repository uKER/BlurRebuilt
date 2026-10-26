#include "StdAfx.h"
#include "gamepad.h"

GamePad::GamePad()
{
	Clear();
}

void GamePad::Clear()
{
	m_padData.Clear();
}

float ConvertThumbstickValue(S32 input, S32 deadzone)
{
	if (Maths::Abs(input) < deadzone)
		return 0.f;

	S32 corrected = input - deadzone * Maths::Sign(input);

	return (float) corrected / ((corrected > 0 ? 32767.f : 32768.f) - deadzone);
}

void GamePad::ReadControls()
{
#ifdef PLATFORM_XENON
	XINPUT_STATE inputState;
	XInputGetState(0, &inputState);

	PadData previous = m_padData;

	m_padData.digitalHeld[Button_A] = (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
	m_padData.digitalHeld[Button_B] = (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;
	m_padData.digitalHeld[Button_X] = (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0;
	m_padData.digitalHeld[Button_Y] = (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;
	m_padData.digitalHeld[Button_Start] = (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
	m_padData.digitalHeld[Button_Back] = (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0;
	m_padData.digitalHeld[Button_LeftShoulder] = (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
	m_padData.digitalHeld[Button_RightShoulder] = (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
	m_padData.digitalHeld[Button_LeftThumb] = (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
	m_padData.digitalHeld[Button_RightThumb] = (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
	m_padData.digitalHeld[Button_DPadUp] = (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
	m_padData.digitalHeld[Button_DPadDown] = (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
	m_padData.digitalHeld[Button_DPadLeft] = (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
	m_padData.digitalHeld[Button_DPadRight] = (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
	m_padData.digitalHeld[Button_LeftTrigger] = inputState.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
	m_padData.digitalHeld[Button_RightTrigger] = inputState.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;

	for (int i = 0; i < Button_Count; i++)
		m_padData.digitalPressed[i] = m_padData.digitalHeld[i] && !previous.digitalHeld[i];

	for (int i = 0; i < Button_Count; i++)
		m_padData.analogue[i] = m_padData.digitalHeld[i];

	m_padData.analogue[Button_LeftTrigger] = (float) inputState.Gamepad.bLeftTrigger / 255.f;
	m_padData.analogue[Button_RightTrigger] = (float) inputState.Gamepad.bRightTrigger / 255.f;

	m_padData.thumbsticks[Axis_LX] = ConvertThumbstickValue(inputState.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );
	m_padData.thumbsticks[Axis_LY] = ConvertThumbstickValue(inputState.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );
	m_padData.thumbsticks[Axis_RX] = ConvertThumbstickValue(inputState.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );
	m_padData.thumbsticks[Axis_RY] = ConvertThumbstickValue(inputState.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );
#endif
}


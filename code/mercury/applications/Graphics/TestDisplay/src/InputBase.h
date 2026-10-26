#ifndef INPUT_BASE_H
#define INPUT_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

enum E_InputValue
{
	E_InputValue_Quit,
	E_InputValue_Pause,
	E_InputValue_Unpause,
	E_InputValue_CameraSlow,
	E_InputValue_CameraFast,
	E_InputValue_CameraSpeed,
	E_InputValue_CameraMoveX,
	E_InputValue_CameraMoveY,
	E_InputValue_CameraMoveZ,
	E_InputValue_CameraLookX,
	E_InputValue_CameraLookY,
};

struct C_Input
{
	virtual F32 GetValue(const E_InputValue eValue) = 0;
};

struct C_InputManagerBase
{
	bool Create();
	bool Destroy();
	bool Update();
};

#endif // INPUT_BASE_H

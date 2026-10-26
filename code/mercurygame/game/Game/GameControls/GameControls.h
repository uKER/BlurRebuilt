#ifndef GAME_GAME_CONTROLS_H
#define GAME_GAME_CONTROLS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Input/Input.h>
#include <Input/GameInput.h>
#include <framework/application/UserControls.h>

namespace Game
{

//! E_Button

enum E_Button
{
	E_Button_Throttle = Framework::E_Button_Count,	
	E_Button_Brake,
	E_Button_Handbrake,
	E_Button_GearUp,
	E_Button_GearDown,
	
	E_Button_Jump,

	E_Button_Count,
};

//! E_Axis

enum E_Axis
{
	E_Axis_Steering = Framework::E_Axis_Count,	

	E_Axis_MoveX,
	E_Axis_MoveY,
	E_Axis_CameraLookX,
	E_Axis_CameraLookY,

	E_Axis_Count,
};

void SetupUserControls( Input::C_GameInput* pGameInput, Input::C_Device* pKeyboard, Input::C_Device* pMouse, Input::C_Device* pPad );

} // namespace Game

#endif // GAME_GAME_CONTROLS_H

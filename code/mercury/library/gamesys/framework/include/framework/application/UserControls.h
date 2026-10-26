#ifndef FRAMEWORK_USER_CONTROLS_H
#define FRAMEWORK_USER_CONTROLS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Input/Input.h>
#include <Input/GameInput.h>
#include <Input/GameInputMapping.h>

namespace Cam2
{
	class CamInput;
};

namespace Framework
{

class C_UserInputDevices;

void SetupApplicationUserControls(Input::C_GameInput* pGameInput, Input::C_Device* pKeyboard, Input::C_Device* pMouse, Input::C_Device* pPad);

Cam2::CamInput GetFlyCameraControls(S32 padNumber, bool extraControls = true);

// combine a button input with an existing one
void AddButtonInput( Input::C_GameInput* pGameInput, const U32 buttonId, Input::C_ButtonInput* pNewButtonInput );

// combine an axis input with an existing one
void AddAxisInput( Input::C_GameInput* pGameInput, const U32 axisId, Input::C_AxisInput* pNewAxisInput );

// button combiner helpers
Input::C_ButtonInput* CreateButtonCombiner( Input::C_ButtonInputCombiner::E_Operation op, Input::C_ButtonInput* pButtonA, Input::C_ButtonInput* pButtonB );
Input::C_ButtonInput* CreateDeviceButtonCombiner(  Input::C_Device* pDevice, Input::C_ButtonInputCombiner::E_Operation op, S32 buttonIdA, S32 buttonIdB);

//! E_Button

enum E_Button
{
	// these are only really meant for debug, in game you should define your own specific buttons so they can be remapped
	E_Button_Debug_ButtonA = 0,	
	E_Button_Debug_ButtonB,		
	E_Button_Debug_ButtonX,		
	E_Button_Debug_ButtonY,		
	E_Button_Debug_ButtonLB,
	E_Button_Debug_ButtonRB,

	E_Button_Debug_ButtonLeftTrigger,		
	E_Button_Debug_ButtonRightTrigger,		

	E_Button_Debug_ButtonStart,
	E_Button_Debug_ButtonBack,

	E_Button_Debug_DPadUp,
	E_Button_Debug_DPadDown,
	E_Button_Debug_DPadLeft,
	E_Button_Debug_DPadRight,

	E_Button_Debug_ButtonLeftStickClick,
	E_Button_Debug_ButtonRightStickClick,

	E_Button_MenuUp,
	E_Button_MenuDown,
	E_Button_MenuLeft,
	E_Button_MenuRight,
	E_Button_MenuCancel,
	E_Button_MenuConfirm,
	E_Button_ChangeToFlyCamera,
	E_Button_CameraMenu,
	E_Button_ChangeCamera,
	E_Button_FlySpeedSlow,
	E_Button_FlySpeedFast,
	E_Button_FlyKeyframe,
	E_Button_DebugMenuActivate,
	E_Button_Freeze,
	E_Button_HudSubscriberScrollUp,
	E_Button_HudSubscriberScrollDown,
	E_Button_Count,
};

//! E_Axis

enum E_Axis
{
	// these are only really meant for debug, in game you should define your own specific axis so they can be remapped
	E_Axis_Debug_LeftStick_X = 0,	
	E_Axis_Debug_LeftStick_Y,		
	E_Axis_Debug_RightStick_X,		
	E_Axis_Debug_RightStick_Y,		

	E_Axis_FlyMoveX,
	E_Axis_FlyMoveZ,
	E_Axis_FlyRotateX,
	E_Axis_FlyRotateY,
	E_Axis_FlyRoll,
	E_Axis_FlyZoom,	
	E_Axis_Count,
};

}; // namespace Framework

#endif // FRAMEWORK_USER_CONTROLS_H

#include "stdafx.h"

#include <Input/Device.h>
#include <Input/DeviceManager.h>
#include <Input/GameInputMapping.h>

#if defined(PLATFORM_PC) || defined(PLATFORM_XENON)
#include <Input/Pad360.h>
#endif

#if defined(PLATFORM_PS3)
#include <Input/PadPS3.h>
#endif

#if defined(PLATFORM_PC)
#include <Input/KeyboardPC.h>
#include <Input/Mouse.h>
#endif

#include <Game/GameControls/GameControls.h>

namespace Game
{

void SetupUserControls( Input::C_GameInput* pGameInput, Input::C_Device* pKeyboard, Input::C_Device* pMouse, Input::C_Device* pPad )
{
	assert(pGameInput);

	#if defined(PLATFORM_PS3)
	
	pGameInput->SetButtonInput(E_Button_Throttle,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_R2));


	pGameInput->SetButtonInput(E_Button_Throttle,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_R2));
	pGameInput->SetButtonInput(E_Button_Brake,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_L2));
	pGameInput->SetButtonInput(E_Button_Handbrake,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_Cross));
	pGameInput->SetButtonInput(E_Button_GearUp,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_Circle));
	pGameInput->SetButtonInput(E_Button_GearDown,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_Square));

	pGameInput->SetButtonInput(E_Button_Jump,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_Cross));

	pGameInput->SetAxisInput(E_Axis_Steering,		new Input::C_AxisInputFromDevice(pPad, 	Input::C_PadPS3::E_Axis_LeftStick_X));

	pGameInput->SetAxisInput(E_Axis_MoveX,			new Input::C_AxisInputFromDevice(pPad, 	Input::C_PadPS3::E_Axis_LeftStick_X));
	pGameInput->SetAxisInput(E_Axis_MoveY,			new Input::C_AxisInputFromDevice(pPad, 	Input::C_PadPS3::E_Axis_LeftStick_Y));
	pGameInput->SetAxisInput(E_Axis_CameraLookX,	new Input::C_AxisInputFromDevice(pPad, 	Input::C_PadPS3::E_Axis_RightStick_X));
	pGameInput->SetAxisInput(E_Axis_CameraLookY,	new Input::C_AxisInputFromDevice(pPad, 	Input::C_PadPS3::E_Axis_RightStick_Y));

	#elif defined(PLATFORM_PC) || defined(PLATFORM_XENON)

	pGameInput->SetButtonInput(E_Button_Throttle,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_RightTrigger));
	pGameInput->SetButtonInput(E_Button_Brake,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_LeftTrigger));
	pGameInput->SetButtonInput(E_Button_Handbrake,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_A));
	pGameInput->SetButtonInput(E_Button_GearUp,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_B));
	pGameInput->SetButtonInput(E_Button_GearDown,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_X));

	pGameInput->SetButtonInput(E_Button_Jump,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_A));

	pGameInput->SetAxisInput(E_Axis_Steering,		new Input::C_AxisInputFromDevice(pPad, 	Input::C_Pad360::E_Axis_LeftStick_X));

	pGameInput->SetAxisInput(E_Axis_MoveX,			new Input::C_AxisInputFromDevice(pPad, 	Input::C_Pad360::E_Axis_LeftStick_X));
	pGameInput->SetAxisInput(E_Axis_MoveY,			new Input::C_AxisInputFromDevice(pPad, 	Input::C_Pad360::E_Axis_LeftStick_Y));
	pGameInput->SetAxisInput(E_Axis_CameraLookX,	new Input::C_AxisInputFromDevice(pPad, 	Input::C_Pad360::E_Axis_RightStick_X));
	pGameInput->SetAxisInput(E_Axis_CameraLookY,	new Input::C_AxisInputFromDevice(pPad, 	Input::C_Pad360::E_Axis_RightStick_Y));

	#endif
	
	#if defined(PLATFORM_PC)
	Input::C_ButtonInput* pSteerLeftButton =					new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_A);
	Input::C_ButtonInput* pSteerRightButton =					new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_D);
	Framework::AddAxisInput( pGameInput, E_Axis_Steering,		new Input::C_AxisInputFromButtons(pSteerLeftButton,			pSteerRightButton));

	Framework::AddButtonInput( pGameInput, E_Button_Throttle,	new Input::C_ButtonInputFromDevice(pKeyboard,			Input::C_Keyboard::E_Button_W));
	Framework::AddButtonInput( pGameInput, E_Button_Brake,		new Input::C_ButtonInputFromDevice(pKeyboard,			Input::C_Keyboard::E_Button_S));

	#endif

}

} // namespace Game

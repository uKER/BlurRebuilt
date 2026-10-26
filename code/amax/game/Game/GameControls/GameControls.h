#ifndef GAME_GAME_CONTROLS_H
#define GAME_GAME_CONTROLS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Input/Input.h>
#include <Input/GameInput.h>
#include <framework/application/UserControls.h>

#if defined(PLATFORM_PC)
namespace UI { class C_ControlConfig; }	// [Blur2PC]
#endif

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
	E_Button_ChangeView,
	E_Button_ResetVehicle,

	E_Button_UsePickup,
	E_Button_DiscardPickup,
	E_Button_SwitchPickup,
	E_Button_SwitchPickupLeft,
	E_Button_SwitchPickupAlt,
	E_Button_UseShield,
	E_Button_UseSecondTierPickup,

	E_Button_Auto_Shuffle_Perks,
	E_Button_Convert_To_Health,
	E_Button_Clone_Selected_Perk,
	E_Button_Fullhouse,

	E_Button_UsePickupAlt,
	E_Button_PhotoRoll,
	E_Button_PhotoZoom,
	E_Button_PhotoRollReset,
	E_Button_PhotoZoomReset,

	E_Button_FireSlot1,
	E_Button_FireSlot2,
	E_Button_FireSlot3,

	// [Blur2PC] retail Blur PC (Blur.exe 1.2) has three more game buttons here, values 58-60, bound by
	// the keyboard layouts (SetupAmaxUserControls 0x00415a30) and read by the vehicle camera (0x00413b20)
	// and the pickup logic (0x0047be80). Names are ours (the profile's key fields call them RearView,
	// PowerUpFwd and PowerUpBwd).
	E_Button_RearView,
	E_Button_UsePickupForward,
	E_Button_UsePickupBackward,

	E_Button_Count,
};

enum E_UiButton
{
	E_UiButton_Pad_Up = E_Button_Count,
	E_UiButton_Pad_Down,
	E_UiButton_Pad_Left,
	E_UiButton_Pad_Right,
	E_UiButton_Confirm,
	E_UiButton_Cancel,
	E_UiButton_Option1,
	E_UiButton_Option2,
	E_UiButton_LeftTrigger,
	E_UiButton_RightTrigger,
	E_UiButton_LeftShoulder,
	E_UiButton_RightShoulder,
	E_UiButton_LeftThumb,
	E_UiButton_RightThumb,
	E_UiButton_Start,
	E_UiButton_Back,

	E_UiButton_PadOrAxis_Up,  
	E_UiButton_PadOrAxis_Down,	
	E_UiButton_PadOrAxis_Left,
	E_UiButton_PadOrAxis_Right,

	E_UiButton_Count,
};

enum E_Axis
{
	E_Axis_Steering = Framework::E_Axis_Count,	
	E_Axis_Pitch,	
	E_Axis_PhotoMoveUp,
	E_Axis_DigitalSteering,

	// [Blur2PC] retail Blur PC's photo camera (C_PhotoCamera2::GetControls, 0x0040ef50) reads four game axes
	// of its own (retail values 13, 15, 16, 17) in place of the framework fly axes; SetupAmaxUserControls
	// binds them to A/D, S/W and the arrow keys plus the pad sticks. Names are ours.
	E_Axis_PhotoMoveX,
	E_Axis_PhotoMoveZ,
	E_Axis_PhotoRotateX,
	E_Axis_PhotoRotateY,

	E_Axis_Count,
};

enum E_UiAxis
{
	E_UiAxis_LeftStick_UD = E_Axis_Count,
	E_UiAxis_LeftStick_LR,

	E_UiAxis_RightStick_UD,
	E_UiAxis_RightStick_LR,

	E_UiAxis_Count
};	

void SetupAmaxUserControls( Input::C_GameInput* pGameInput, Input::C_Device* pKeyboard, Input::C_Device* pMouse, Input::C_Device* pPad );

#if defined(PLATFORM_PC)
// [Blur2PC] retail Blur PC's SetupAmaxUserControls (Blur.exe 1.2, 0x00415a30): keyboard layout 0 or 1 (the
// profile's choice in Options > Keyboard), split-screen key tables, the pad mapping of the profile's controller
// configuration (NULL: no pad game buttons, as at boot) and the split-screen column of the menu keys.
void SetupAmaxUserControlsPC( Input::C_GameInput* pGameInput, Input::C_Device* pPad, Input::C_Device* pKeyboard, Input::C_Device* pMouse,
							  S32 keyboardLayout, bool splitScreen, const UI::C_ControlConfig* pPadConfig, S32 menuKeyColumn );
#endif

} // namespace Game

#endif // GAME_GAME_CONTROLS_H

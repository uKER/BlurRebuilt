#ifndef INPUT_H
#define INPUT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core\Core.h>

//! @defgroup Input Input Module
//! The input module handles low level reading of input devices and allows the configuring and reading of game controls. 
//!
//! \par Overview
//! The Input library is split into two sections, managing and reading the physical devices and managing the game controls and mapping
//! inputs to the game controls.
//! The Input library is contained in the ::Input namespace.
//!
//! \par Physical Devices:
//! The base class Input::C_Device provides the interface for reading physical devices. Each device has a number
//! of axes and buttons. Specific implementations for specific devices such as Input::C_Pad360 and Input::C_PadPS3 are derived
//! from Input::C_Device. The creation of these specific physical devices is performed through the Input::C_DeviceManager class.
//!
//! \par Game Controls - Virtual Controller:
//! The derived Input::C_Device classes should rarely be read directly by the game. Controls may need to be remapped
//! and we need to support different controllers on different platforms. The C_GameInput class implements a virtual game controller 
//! which is a game oriented input device where the buttons and axis are game defined actions such as 'accelerate' or 'steering' 
//! The classes derived from Input::C_ButtonInput and Input::C_AxisInput provide the mechanism to map physical device axes or buttons
//! to axes and buttons in the Input::C_GameInput class.
//!
//! \par Sample code:
//! \code
//!
//!#include <input\DeviceManager.h>
//!#include <input\GameInput.h>
//!#include <input\GameInputMapping.h>
//!#include <input\Pad360.h>
//!
//!Input::C_DeviceManager* g_pDeviceManager;
//!Input::C_GameInputManager* g_pGameInputManager;
//!Input::C_Device* g_pPad360;
//!
//!enum E_GameButton
//!{
//!	E_GameButton_Accelerate = 0,
//!	E_GameButton_Count
//!};
//!
//!enum E_GameAxis
//!{
//!	E_GameAxis_Steer = 0,
//!	E_GameAxis_AxisFromButtons,
//!	E_GameAxis_Count
//!};
//!
//!void SetupControlMappings( S32 userIndex );
//!
//!void ControlSystemInit()
//!{
//!	// Create the device manager
//!	Input::C_DeviceManagerCreateParam param;
//!	g_pDeviceManager = Input::CreateDeviceManager( param ); 
//!
//!	// Create the 360 Pad 0 Device
//!	g_pDeviceManager->CreateDevice( 0, Input::E_Device_Pad360, &g_pPad360 );
//!
//!	// Create the game input manager
//!	g_pGameInputManager = new Input::C_GameInputManager( 1, E_GameButton_Count, E_GameAxis_Count );
//!
//!	// Create control mappings
//!	SetupControlMappings( 0 );
//!}
//!
//!void SetupControlMappings( S32 userIndex )
//!{
//!	// Set steering to come from left stick
//!	g_pGameInputManager->GetGameInput(userIndex)->SetAxisInput( E_GameAxis_Steer, new Input::C_AxisInputFromDevice( g_pPad360, Input::C_Pad360::E_Axis_LeftStick_X ) );
//!
//!	// Set accelerate to come from right trigger and A
//!	Input::C_ButtonInputCombiner* pButtonCombiner = new Input::C_ButtonInputCombiner();
//!	pButtonCombiner->AddButtonInput( new Input::C_ButtonInputFromDevice( g_pPad360, Input::C_Pad360::E_Button_X ) );
//!	pButtonCombiner->AddButtonInput( new Input::C_ButtonInputFromDevice( g_pPad360, Input::C_Pad360::E_Button_RightTrigger ) );
//!	g_pGameInputManager->GetGameInput(userIndex)->SetButtonInput( E_GameButton_Accelerate, pButtonCombiner );
//!
//!	// Setup axis using X and B buttons
//!	Input::C_ButtonInput* pButtonNegative = new Input::C_ButtonInputFromDevice( g_pPad360, Input::C_Pad360::E_Button_X );
//!	Input::C_ButtonInput* pButtonPositive = new Input::C_ButtonInputFromDevice( g_pPad360, Input::C_Pad360::E_Button_B );
//!	Input::C_AxisInputFromButtons* pAxisFromButtons = new Input::C_AxisInputFromButtons( pButtonNegative, pButtonPositive );
//!	g_pGameInputManager->GetGameInput(userIndex)->SetAxisInput( E_GameAxis_AxisFromButtons, pAxisFromButtons );
//!}
//!
//!void ControlSystemUpdate()
//!{
//!	Input::C_DeviceManager::GetInstance()->Update();
//!	Input::C_GameInputManager::GetInstance()->Update();
//!
//!	// Read device directly
//!	F32 buttonValue = 0.0f;
//!	
//!	buttonValue = g_pPad360->GetButtonValue( Input::C_Pad360::E_Button_A );
//!	LOGINFO( Input::g_Log, "Button = %f\n\n", buttonValue);
//!
//!	// Read game control
//!	const F32 accelerate = Input::GetGameInput( 0 )->GetButtonValue( E_GameButton_Accelerate );
//!	LOGINFO( Input::g_Log, "accelerate = %f\n", accelerate);
//!
//!	// Read game control
//!	const F32 testAxis = Input::GetGameInput( 0 )->GetAxisSignedValue( E_GameAxis_AxisFromButtons );
//!	LOGINFO( Input::g_Log, "AxisFromButtons = %f\n", testAxis);
//!}
//!
//!void ControlSystemClose()
//!{
//!	// Destory the game input manager
//!	delete g_pGameInputManager;
//!
//!	// Destroy the 360 Pad 0 Device
//!	delete g_pPad360;
//!
//!	// Destroy the device manager
//!	delete g_pDeviceManager;
//!}
//!
//!
//! \endcode

//!
//! @addtogroup Input
//! @{

//! The Input namespace contains the code for the Input module.
namespace Input
{

extern Debug::Logger &g_Log;

void ApplyCircularDeadzone(F32& x, F32& y, const F32 deadzone);

class C_Device;
class C_DeviceManager;
class C_GameInputManager;

} // namespace Input

//! @}

#endif


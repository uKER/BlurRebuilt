#include "FrameworkPch.h"
#include "Framework/Application/UserControls.h"

#include <System/Memory/MemoryTracker.h>

#include <Input/Device.h>
#include <Input/DeviceManager.h>
#include <Input/GameInputMapping.h>

#if defined(PLATFORM_PC)
#include <Input/Pad360.h>
#endif

#if defined(PLATFORM_PS3)
#include <Input/PadPS3.h>
#endif

#if defined(PLATFORM_PC)
#include <Input/KeyboardPC.h>
#include <Input/Mouse.h>
#endif

#include <camera2.h>

namespace Framework
{

static Debug::Bool FlyCamera_SwapSticks("Camera2.FlyCamera.SwapSticks", false, Debug::Persist());

// combine a button input with an existing one
void AddButtonInput( Input::C_GameInput* pGameInput, const U32 buttonId, Input::C_ButtonInput* pNewButtonInput )
{
	// [Blur2PC] a button with no binding yet takes the new input directly (the retail PC keyboard map
	// leaves some UI buttons without a key; combining with a NULL input asserted and crashed)
	if ( pGameInput->GetButtonInput(buttonId) == NULL )
	{
		pGameInput->SetButtonInput(buttonId, pNewButtonInput);
		return;
	}
	Input::C_ButtonInputCombiner* pButtonCombiner = new Input::C_ButtonInputCombiner();
	pButtonCombiner->AddButtonInput( pGameInput->GetButtonInput(buttonId) );
	pButtonCombiner->AddButtonInput( pNewButtonInput );
	pGameInput->SetButtonInput(buttonId, pButtonCombiner);
}

// combine an axis input with an existing one
void AddAxisInput( Input::C_GameInput* pGameInput, const U32 axisId, Input::C_AxisInput* pNewAxisInput )
{
	// [Blur2PC] Retail PC's split-screen setup leaves keyboard photo axes empty on
	// controller-only slots. The first pad axis becomes the binding directly.
	if ( pGameInput->GetAxisInput(axisId) == NULL )
	{
		pGameInput->SetAxisInput(axisId, pNewAxisInput);
		return;
	}
	Input::C_AxisInputCombiner* pAxisCombiner = new Input::C_AxisInputCombiner();
	pAxisCombiner->AddAxisInput( pGameInput->GetAxisInput(axisId) );
	pAxisCombiner->AddAxisInput( pNewAxisInput );
	pGameInput->SetAxisInput(axisId, pAxisCombiner);
}

Input::C_ButtonInput* CreateButtonCombiner( Input::C_ButtonInputCombiner::E_Operation op, Input::C_ButtonInput* pButtonA, Input::C_ButtonInput* pButtonB )
{
	Input::C_ButtonInputCombiner* pButtonCombiner = new Input::C_ButtonInputCombiner( op );
	pButtonCombiner->AddButtonInput(pButtonA);
	pButtonCombiner->AddButtonInput(pButtonB);
	return pButtonCombiner;
}

Input::C_ButtonInput* CreateDeviceButtonCombiner(  Input::C_Device* pDevice, Input::C_ButtonInputCombiner::E_Operation op, S32 buttonIdA, S32 buttonIdB)
{
	return CreateButtonCombiner( op, 
		new Input::C_ButtonInputFromDevice(pDevice, buttonIdA), 
		new Input::C_ButtonInputFromDevice(pDevice, buttonIdB));
}

void SetupApplicationUserControls(Input::C_GameInput* pGameInput, Input::C_Device* pKeyboard, Input::C_Device* pMouse, Input::C_Device* pPad)
{
	assert(pGameInput);

	#if defined(PLATFORM_PS3)

	pGameInput->SetButtonInput(E_Button_Debug_ButtonA,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_Cross));
	pGameInput->SetButtonInput(E_Button_Debug_ButtonB,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_Circle));
	pGameInput->SetButtonInput(E_Button_Debug_ButtonX,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_Square));
	pGameInput->SetButtonInput(E_Button_Debug_ButtonY,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_Triangle));	
	pGameInput->SetButtonInput(E_Button_Debug_ButtonLB,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_L1));
	pGameInput->SetButtonInput(E_Button_Debug_ButtonRB,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_R1));
	pGameInput->SetButtonInput(E_Button_Debug_ButtonLeftTrigger,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_L2));	
	pGameInput->SetButtonInput(E_Button_Debug_ButtonRightTrigger,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_R2));	

	pGameInput->SetButtonInput(E_Button_Debug_ButtonStart,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_Start));	
	pGameInput->SetButtonInput(E_Button_Debug_ButtonBack,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_Select));	

	pGameInput->SetButtonInput(E_Button_Debug_DPadUp,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_DPadUp));
	pGameInput->SetButtonInput(E_Button_Debug_DPadDown,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_DPadDown));
	pGameInput->SetButtonInput(E_Button_Debug_DPadLeft,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_DPadLeft));
	pGameInput->SetButtonInput(E_Button_Debug_DPadRight,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_DPadRight));

	pGameInput->SetButtonInput(E_Button_Debug_ButtonLeftStickClick,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_L3));	
	pGameInput->SetButtonInput(E_Button_Debug_ButtonRightStickClick,new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_R3));	

	pGameInput->SetAxisInput(E_Axis_Debug_LeftStick_X,	new Input::C_AxisInputFromDevice(pPad, 	Input::C_PadPS3::E_Axis_LeftStick_X));
	pGameInput->SetAxisInput(E_Axis_Debug_LeftStick_Y,	new Input::C_AxisInputFromDevice(pPad, 	Input::C_PadPS3::E_Axis_LeftStick_Y));
	pGameInput->SetAxisInput(E_Axis_Debug_RightStick_X,	new Input::C_AxisInputFromDevice(pPad, 	Input::C_PadPS3::E_Axis_RightStick_X));
	pGameInput->SetAxisInput(E_Axis_Debug_RightStick_Y,	new Input::C_AxisInputFromDevice(pPad, 	Input::C_PadPS3::E_Axis_RightStick_Y));


	Input::C_AxisInput* pMenuAxisY = new Input::C_AxisInputFromDevice( pPad, Input::C_PadPS3::E_Axis_LeftStick_Y);
	Input::C_AxisInput* pMenuAxisX = new Input::C_AxisInputFromDevice( pPad, Input::C_PadPS3::E_Axis_LeftStick_X);
	pGameInput->SetButtonInput(E_Button_MenuUp, CreateButtonCombiner(Input::C_ButtonInputCombiner::E_Operation_Max, 
			new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_DPadUp),
			new Input::C_ButtonInputFromAxis( pMenuAxisY, false, 0.3f ) ) );
	pGameInput->SetButtonInput(E_Button_MenuDown, CreateButtonCombiner(Input::C_ButtonInputCombiner::E_Operation_Max, 
			new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_DPadDown),
			new Input::C_ButtonInputFromAxis( pMenuAxisY, true, 0.3f ) ) );
	pGameInput->SetButtonInput(E_Button_MenuRight, CreateButtonCombiner(Input::C_ButtonInputCombiner::E_Operation_Max, 
			new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_DPadRight),
			new Input::C_ButtonInputFromAxis( pMenuAxisX, false, 0.3f ) ) );
	pGameInput->SetButtonInput(E_Button_MenuLeft, CreateButtonCombiner(Input::C_ButtonInputCombiner::E_Operation_Max, 
			new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_DPadLeft),
			new Input::C_ButtonInputFromAxis( pMenuAxisX, true, 0.3f ) ) );

	//pGameInput->SetButtonInput(E_Button_MenuUp,			new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_DPadUp));
	//pGameInput->SetButtonInput(E_Button_MenuDown,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_DPadDown));
	//pGameInput->SetButtonInput(E_Button_MenuRight,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_DPadRight));
	//pGameInput->SetButtonInput(E_Button_MenuLeft,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_DPadLeft));
	pGameInput->SetButtonInput(E_Button_MenuCancel,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_Circle));
	pGameInput->SetButtonInput(E_Button_MenuConfirm,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_Cross));
	
	pGameInput->SetButtonInput(E_Button_DebugMenuActivate, CreateDeviceButtonCombiner( pPad, Input::C_ButtonInputCombiner::E_Operation_Min, Input::C_PadPS3::E_Button_Start, Input::C_PadPS3::E_Button_Select ) );

	pGameInput->SetButtonInput(E_Button_ChangeToFlyCamera,
		CreateDeviceButtonCombiner( pPad, Input::C_ButtonInputCombiner::E_Operation_Min,
		Input::C_PadPS3::E_Button_Select, Input::C_PadPS3::E_Button_L1 )
	);

	pGameInput->SetButtonInput(E_Button_CameraMenu,
		CreateDeviceButtonCombiner( pPad, Input::C_ButtonInputCombiner::E_Operation_Min,
		Input::C_PadPS3::E_Button_Select, Input::C_PadPS3::E_Button_R1 )
	);

	pGameInput->SetButtonInput(E_Button_Freeze,
		CreateDeviceButtonCombiner( pPad, Input::C_ButtonInputCombiner::E_Operation_Min,
		Input::C_PadPS3::E_Button_Select, Input::C_PadPS3::E_Button_Square )
	);
	

	{
		Input::C_AxisInput* pMenuAxisY = new Input::C_AxisInputFromDevice( pPad, Input::C_PadPS3::E_Axis_LeftStick_Y);

		Input::C_ButtonInput* pButtonUp = CreateButtonCombiner( Input::C_ButtonInputCombiner::E_Operation_Max, 
																new Input::C_ButtonInputFromAxis( pMenuAxisY, false, 0.3f ), 
																new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_DPadUp));

		Input::C_ButtonInput* pButtonDown = CreateButtonCombiner( Input::C_ButtonInputCombiner::E_Operation_Max, 
																new Input::C_ButtonInputFromAxis( pMenuAxisY, true, 0.3f ), 
																new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_DPadDown));

		Input::C_ButtonInput* pButtonShift = new Input::C_ButtonInputFromDevice( pPad, Input::C_PadPS3::E_Button_Select );
		
		pGameInput->SetButtonInput(E_Button_HudSubscriberScrollUp,
			CreateButtonCombiner( Input::C_ButtonInputCombiner::E_Operation_Min, pButtonShift, pButtonUp ));

		pGameInput->SetButtonInput(E_Button_HudSubscriberScrollDown,
			CreateButtonCombiner( Input::C_ButtonInputCombiner::E_Operation_Min, pButtonShift, pButtonDown ));
	}

	//Input::C_ButtonInput* pDebugMenuStart =					new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_Start);
	//Input::C_ButtonInput* pDebugMenuSelect =					new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_Select);
	//Input::C_ButtonInputCombiner* pDebugMenuButton =			new Input::C_ButtonInputCombiner(Input::C_ButtonInputCombiner::E_Operation_Min);
	//pDebugMenuButton->AddButtonInput(pDebugMenuStart);
	//pDebugMenuButton->AddButtonInput(pDebugMenuSelect);
	//pGameInput->SetButtonInput(E_Button_DebugMenuActivate,		pDebugMenuButton);

	pGameInput->SetButtonInput(E_Button_ChangeCamera,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_L1));
	pGameInput->SetButtonInput(E_Button_FlySpeedSlow,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_L2));
	pGameInput->SetButtonInput(E_Button_FlySpeedFast,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_R2));
	pGameInput->SetButtonInput(E_Button_FlyKeyframe,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_Triangle));

	pGameInput->SetAxisInput(E_Axis_FlyMoveX,			new Input::C_AxisInputFromDevice(pPad, 	Input::C_PadPS3::E_Axis_LeftStick_X));
	pGameInput->SetAxisInput(E_Axis_FlyMoveZ,			new Input::C_AxisInputFromDevice(pPad, 	Input::C_PadPS3::E_Axis_LeftStick_Y));
	pGameInput->SetAxisInput(E_Axis_FlyRotateX,			new Input::C_AxisInputFromDevice(pPad, 	Input::C_PadPS3::E_Axis_RightStick_X));
	pGameInput->SetAxisInput(E_Axis_FlyRotateY,			new Input::C_AxisInputFromDevice(pPad, 	Input::C_PadPS3::E_Axis_RightStick_Y));

	Input::C_AxisInput* pFlyRotateAxis =				new Input::C_AxisInputFromDevice(pPad, 	Input::C_PadPS3::E_Axis_LeftStick_X);  
	Input::C_ButtonInput* pFlyRotateShift =				new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_Square);
	pGameInput->SetAxisInput(E_Axis_FlyRoll,			new Input::C_AxisInputShifted( pFlyRotateAxis, pFlyRotateShift) );
	Input::C_AxisInput* pFlyZoomAxis =					new Input::C_AxisInputFromButtons(	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_DPadDown),
																							new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_DPadUp) );
	Input::C_ButtonInput* pFlyZoomShift =				new Input::C_ButtonInputFromDevice(pPad, 	Input::C_PadPS3::E_Button_Square);
	pGameInput->SetAxisInput(E_Axis_FlyZoom,			new Input::C_AxisInputShifted( pFlyZoomAxis, pFlyZoomShift ));
	
	#elif defined(PLATFORM_PC)
	pGameInput->SetButtonInput(E_Button_Debug_ButtonA,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_A));
	pGameInput->SetButtonInput(E_Button_Debug_ButtonB,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_B));
	pGameInput->SetButtonInput(E_Button_Debug_ButtonX,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_X));
	pGameInput->SetButtonInput(E_Button_Debug_ButtonY,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_Y));
	pGameInput->SetButtonInput(E_Button_Debug_ButtonLB,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_LeftShoulder));
	pGameInput->SetButtonInput(E_Button_Debug_ButtonRB,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_RightShoulder));

	pGameInput->SetButtonInput(E_Button_Debug_ButtonLeftTrigger,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_LeftTrigger));
	pGameInput->SetButtonInput(E_Button_Debug_ButtonRightTrigger,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_RightTrigger));

	pGameInput->SetButtonInput(E_Button_Debug_ButtonStart,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_Start));
	pGameInput->SetButtonInput(E_Button_Debug_ButtonBack,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_Back));

	pGameInput->SetButtonInput(E_Button_Debug_DPadUp,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadUp));
	pGameInput->SetButtonInput(E_Button_Debug_DPadDown,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadDown));
	pGameInput->SetButtonInput(E_Button_Debug_DPadLeft,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadLeft));
	pGameInput->SetButtonInput(E_Button_Debug_DPadRight,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadRight));

	pGameInput->SetButtonInput(E_Button_Debug_ButtonLeftStickClick,	new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_LeftThumb));	
	pGameInput->SetButtonInput(E_Button_Debug_ButtonRightStickClick,new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_RightThumb));	

	pGameInput->SetAxisInput(E_Axis_Debug_LeftStick_X,	new Input::C_AxisInputFromDevice(pPad, 	Input::C_Pad360::E_Axis_LeftStick_X));
	pGameInput->SetAxisInput(E_Axis_Debug_LeftStick_Y,	new Input::C_AxisInputFromDevice(pPad, 	Input::C_Pad360::E_Axis_LeftStick_Y));
	pGameInput->SetAxisInput(E_Axis_Debug_RightStick_X,	new Input::C_AxisInputFromDevice(pPad, 	Input::C_Pad360::E_Axis_RightStick_X));
	pGameInput->SetAxisInput(E_Axis_Debug_RightStick_Y,	new Input::C_AxisInputFromDevice(pPad, 	Input::C_Pad360::E_Axis_RightStick_Y));

	Input::C_AxisInput* pMenuAxisY = new Input::C_AxisInputFromDevice( pPad, Input::C_Pad360::E_Axis_LeftStick_Y);
	Input::C_AxisInput* pMenuAxisX = new Input::C_AxisInputFromDevice( pPad, Input::C_Pad360::E_Axis_LeftStick_X);
	pGameInput->SetButtonInput(E_Button_MenuUp, CreateButtonCombiner(Input::C_ButtonInputCombiner::E_Operation_Max, 
			new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadUp),
			new Input::C_ButtonInputFromAxis( pMenuAxisY, false, 0.3f ) ) );
	pGameInput->SetButtonInput(E_Button_MenuDown, CreateButtonCombiner(Input::C_ButtonInputCombiner::E_Operation_Max, 
			new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadDown),
			new Input::C_ButtonInputFromAxis( pMenuAxisY, true, 0.3f ) ) );
	pGameInput->SetButtonInput(E_Button_MenuRight, CreateButtonCombiner(Input::C_ButtonInputCombiner::E_Operation_Max, 
			new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadRight),
			new Input::C_ButtonInputFromAxis( pMenuAxisX, false, 0.3f ) ) );
	pGameInput->SetButtonInput(E_Button_MenuLeft, CreateButtonCombiner(Input::C_ButtonInputCombiner::E_Operation_Max, 
			new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadLeft),
			new Input::C_ButtonInputFromAxis( pMenuAxisX, true, 0.3f ) ) );

	//pGameInput->SetButtonInput(E_Button_MenuUp,			new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadUp));
	//pGameInput->SetButtonInput(E_Button_MenuDown,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadDown));
	//pGameInput->SetButtonInput(E_Button_MenuRight,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadRight));
	//pGameInput->SetButtonInput(E_Button_MenuLeft,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadLeft));
	pGameInput->SetButtonInput(E_Button_MenuCancel,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_B));
	pGameInput->SetButtonInput(E_Button_MenuConfirm,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_A));

	pGameInput->SetButtonInput(E_Button_ChangeToFlyCamera,
		CreateDeviceButtonCombiner( pPad, Input::C_ButtonInputCombiner::E_Operation_Min,
		Input::C_Pad360::E_Button_Back, Input::C_Pad360::E_Button_LeftShoulder )
	);

	pGameInput->SetButtonInput(E_Button_CameraMenu,
		CreateDeviceButtonCombiner( pPad, Input::C_ButtonInputCombiner::E_Operation_Min,
		Input::C_Pad360::E_Button_Back, Input::C_Pad360::E_Button_RightShoulder )
	);

	pGameInput->SetButtonInput(E_Button_Freeze,
		CreateDeviceButtonCombiner( pPad, Input::C_ButtonInputCombiner::E_Operation_Min,
		Input::C_Pad360::E_Button_Back, Input::C_Pad360::E_Button_X )
	);


	{
		Input::C_AxisInput* pMenuAxisY = new Input::C_AxisInputFromDevice( pPad, Input::C_Pad360::E_Axis_LeftStick_Y);

		Input::C_ButtonInput* pButtonUp = CreateButtonCombiner( Input::C_ButtonInputCombiner::E_Operation_Max, 
																new Input::C_ButtonInputFromAxis( pMenuAxisY, false, 0.3f ), 
																new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadUp));

		Input::C_ButtonInput* pButtonDown = CreateButtonCombiner( Input::C_ButtonInputCombiner::E_Operation_Max, 
																new Input::C_ButtonInputFromAxis( pMenuAxisY, true, 0.3f ), 
																new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadDown));

		Input::C_ButtonInput* pButtonShift = new Input::C_ButtonInputFromDevice( pPad, Input::C_Pad360::E_Button_Back );
		
		pGameInput->SetButtonInput(E_Button_HudSubscriberScrollUp,
			CreateButtonCombiner( Input::C_ButtonInputCombiner::E_Operation_Min, pButtonShift, pButtonUp ));

		pGameInput->SetButtonInput(E_Button_HudSubscriberScrollDown,
			CreateButtonCombiner( Input::C_ButtonInputCombiner::E_Operation_Min, pButtonShift, pButtonDown ));
	}

	pGameInput->SetButtonInput(E_Button_ChangeCamera,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_LeftShoulder));
	pGameInput->SetButtonInput(E_Button_FlySpeedSlow,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_LeftTrigger));
	pGameInput->SetButtonInput(E_Button_FlySpeedFast,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_RightTrigger));
	pGameInput->SetButtonInput(E_Button_FlyKeyframe,	new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_Y));
	pGameInput->SetAxisInput(E_Axis_FlyMoveX,			new Input::C_AxisInputFromDevice(pPad, 	Input::C_Pad360::E_Axis_LeftStick_X));
	pGameInput->SetAxisInput(E_Axis_FlyMoveZ,			new Input::C_AxisInputFromDevice(pPad, 	Input::C_Pad360::E_Axis_LeftStick_Y));
	pGameInput->SetAxisInput(E_Axis_FlyRotateX,			new Input::C_AxisInputFromDevice(pPad, 	Input::C_Pad360::E_Axis_RightStick_X));
	pGameInput->SetAxisInput(E_Axis_FlyRotateY,			new Input::C_AxisInputFromDevice(pPad, 	Input::C_Pad360::E_Axis_RightStick_Y));
	
	//Input::C_ButtonInput* pDebugMenuStart =					new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_Start);
	//Input::C_ButtonInput* pDebugMenuSelect =					new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_Back);
	//Input::C_ButtonInputCombiner* pDebugMenuButton =			new Input::C_ButtonInputCombiner(Input::C_ButtonInputCombiner::E_Operation_Min);
	//pDebugMenuButton->AddButtonInput(pDebugMenuStart);
	//pDebugMenuButton->AddButtonInput(pDebugMenuSelect);
	//pGameInput->SetButtonInput(E_Button_DebugMenuActivate,		pDebugMenuButton);
	
	pGameInput->SetButtonInput(E_Button_DebugMenuActivate, CreateDeviceButtonCombiner( pPad, Input::C_ButtonInputCombiner::E_Operation_Min, Input::C_Pad360::E_Button_Start, Input::C_Pad360::E_Button_Back ) );
	
	Input::C_AxisInput* pFlyRotateAxis =						new Input::C_AxisInputFromDevice(pPad, 	Input::C_Pad360::E_Axis_LeftStick_X);  
	Input::C_ButtonInput* pFlyRotateShift =						new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_X);
	pGameInput->SetAxisInput(E_Axis_FlyRoll,				new Input::C_AxisInputShifted( pFlyRotateAxis, pFlyRotateShift) );
	
	Input::C_ButtonInputFromDevice* pFlyZoomAxisDown =			new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadDown);
	Input::C_ButtonInputFromDevice* pFlyZoomAxisUp =			new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadUp);
	Input::C_AxisInput* pFlyZoomAxis =							new Input::C_AxisInputFromButtons(pFlyZoomAxisDown, pFlyZoomAxisUp);
	Input::C_ButtonInput* pFlyZoomShift =						new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_X);
	pGameInput->SetAxisInput(E_Axis_FlyZoom,				new Input::C_AxisInputShifted( pFlyZoomAxis, pFlyZoomShift ));
	#endif
	
	#if defined(PLATFORM_PC)
	AddButtonInput( pGameInput, E_Button_MenuUp,			new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_UpArrow ) );
	AddButtonInput( pGameInput, E_Button_MenuDown,			new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_DownArrow ) );
	AddButtonInput( pGameInput, E_Button_MenuLeft, 			new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_LeftArrow ) );
	AddButtonInput( pGameInput, E_Button_MenuRight, 		new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_RightArrow ) );
	AddButtonInput( pGameInput, E_Button_MenuCancel, 		new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_Backspace ) );
	AddButtonInput( pGameInput, E_Button_MenuConfirm, 		new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_Return ) );
	AddButtonInput( pGameInput, E_Button_DebugMenuActivate,	new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_F11 ) );
	AddButtonInput( pGameInput, E_Button_Freeze,			new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_F10 ) );
	AddButtonInput( pGameInput, E_Button_CameraMenu,		new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_F9 ) );
	AddButtonInput( pGameInput, E_Button_FlySpeedFast,		new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_LShift ) );
	AddButtonInput( pGameInput, E_Button_FlySpeedSlow,		new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_LControl ) );

	AddButtonInput( pGameInput, E_Button_Debug_ButtonA,		new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_F1 ) );
	AddButtonInput( pGameInput, E_Button_Debug_ButtonB,		new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_F2 ) );
	AddButtonInput( pGameInput, E_Button_Debug_ButtonX,		new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_F3 ) );
	AddButtonInput( pGameInput, E_Button_Debug_ButtonY,		new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_F4 ) );
	AddButtonInput( pGameInput, E_Button_Debug_ButtonLB,	new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_F7 ) );
	AddButtonInput( pGameInput, E_Button_Debug_ButtonRB,	new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_F8 ) );
	AddButtonInput( pGameInput, E_Button_Debug_ButtonLeftTrigger,		new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_F5 ) );
	AddButtonInput( pGameInput, E_Button_Debug_ButtonRightTrigger,		new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_F6 ) );

	Input::C_ButtonInput* pFlyMoveLeftInput =					new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_A);
	Input::C_ButtonInput* pFlyMoveRightInput =					new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_D);
	Input::C_ButtonInput* pFlyMoveUpGameInput =					new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_S); // Reversed?
	Input::C_ButtonInput* pFlyMoveDownInput =					new Input::C_ButtonInputFromDevice(pKeyboard, 	Input::C_Keyboard::E_Button_W); // Reversed?

	AddAxisInput( pGameInput, E_Axis_FlyMoveX, 		new Input::C_AxisInputFromButtons(pFlyMoveLeftInput,			pFlyMoveRightInput) );
	AddAxisInput( pGameInput, E_Axis_FlyMoveZ,		new Input::C_AxisInputFromButtons(pFlyMoveUpGameInput,			pFlyMoveDownInput));
	AddButtonInput( pGameInput, E_Button_FlySpeedFast,		new Input::C_ButtonInputFromDevice(pKeyboard,			Input::C_Keyboard::E_Button_LShift));
	AddAxisInput( pGameInput, E_Axis_FlyRotateX, 		new Input::C_AxisInputFromDevice(pMouse, 			Input::C_Mouse::E_Axis_DeltaX));
	AddAxisInput( pGameInput, E_Axis_FlyRotateY,		new Input::C_AxisInputFromDevice(pMouse, 			Input::C_Mouse::E_Axis_DeltaY));
	#endif
}

Cam2::CamInput GetFlyCameraControls(S32 padNumber, bool extraControls)
{
	Input::C_GameInput* gameInput = Input::GetGameInput(padNumber);
	Cam2::CamInput cinp;

	
	cinp.pad = padNumber;
	
	if (!FlyCamera_SwapSticks)
	{
		// Left stick move, right stick rotate
		cinp.val[Cam2::E_Input_FlyMoveX]	= gameInput->GetAxisSignedValue(E_Axis_FlyMoveX);
		cinp.val[Cam2::E_Input_FlyMoveZ]	= gameInput->GetAxisSignedValue(E_Axis_FlyMoveZ);
		cinp.val[Cam2::E_Input_FlyRotateX]	= gameInput->GetAxisSignedValue(E_Axis_FlyRotateX);
		cinp.val[Cam2::E_Input_FlyRotateY]	= gameInput->GetAxisSignedValue(E_Axis_FlyRotateY);
	}
	else
	{
		// Right stick move, left stick rotate
		cinp.val[Cam2::E_Input_FlyMoveX]	= gameInput->GetAxisSignedValue(E_Axis_FlyRotateX);
		cinp.val[Cam2::E_Input_FlyMoveZ]	= gameInput->GetAxisSignedValue(E_Axis_FlyRotateY);
		cinp.val[Cam2::E_Input_FlyRotateX]	= gameInput->GetAxisSignedValue(E_Axis_FlyMoveX);
		cinp.val[Cam2::E_Input_FlyRotateY]	= gameInput->GetAxisSignedValue(E_Axis_FlyMoveZ);
	}
	
	if (extraControls)
	{
		cinp.val[Cam2::E_Input_FlyRoll]		= gameInput->GetAxisSignedValue(E_Axis_FlyRoll);
		cinp.val[Cam2::E_Input_FlyZoom]		= gameInput->GetAxisSignedValue(E_Axis_FlyZoom);
		cinp.val[Cam2::E_Input_FlySlow]		= gameInput->GetButtonValue(E_Button_FlySpeedSlow);
		cinp.val[Cam2::E_Input_FlyFast]		= gameInput->GetButtonValue(E_Button_FlySpeedFast);
		cinp.val[Cam2::E_Input_Keyframe]	= gameInput->GetButtonValue(E_Button_FlyKeyframe);
	}
	
	return cinp;
}

}; // namespace Framework

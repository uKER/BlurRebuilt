
#include "stdafx.h"
#include "CameraSwitcher.h"
#include "CameraSystem.h"
#include <framework/application/UserControls.h>

// Camera Switcher allows you to switch camera, either from DebugView or by using Back+LB / LB key
// combinations

DEBUG_ENUM(MGCam::E_DebugCam)
	DE_OPTION(MGCam::E_DebugCam_GameFeed,          "GameFeed")
	DE_OPTION(MGCam::E_DebugCam_AnimCamera,        "AnimCamera")
	DE_OPTION(MGCam::E_DebugCam_ChaseCamera2,      "ChaseCamera2")
	DE_OPTION(MGCam::E_DebugCam_GravGun,           "GravGun")
	DE_OPTION(MGCam::E_DebugCam_FlyCamera,         "FlyCamera")
	DE_OPTION(MGCam::E_DebugCam_FixedCamera,       "FixedCamera")
	DE_OPTION(MGCam::E_DebugCam_RelativeFlyCamera, "RelativeFlyCamera")
	DE_OPTION(MGCam::E_DebugCam_RelativeCamera,    "RelativeCamera")
	DE_OPTION(MGCam::E_DebugCam_ScriptCamera,      "ScriptCamera")
	DE_OPTION(MGCam::E_DebugCam_SimpleChaseCamera, "SimpleChaseCamera")
	DE_OPTION(MGCam::E_DebugCam_TvCamera,          "TvCamera")
END_DEBUG_ENUM

namespace MGCam
{
static Debug::Enum<E_DebugCam> CameraChoice("Camera2.CameraChoice", E_DebugCam_GameFeed);
static S32 LastChoice = 0;

C_CameraSwitcher& Switcher()
{
	static C_CameraSwitcher s_instance;
	return s_instance;
}

C_CameraSwitcher::C_CameraSwitcher()
{
	m_PreviousCamera = E_DebugCam_GameFeed;
	m_DebugPad = 0;
}

void C_CameraSwitcher::Init()
{
	// Build camera list:
	Reset();
	AddGroup();
	AddOption(E_DebugCam_GameFeed, "GameFeed");
	AddOption(E_DebugCam_GravGun, "GravGun");
	AddGroup();
	AddOption(E_DebugCam_FlyCamera, "Fly");
	AddOption(E_DebugCam_FixedCamera, "Fixed");
	AddOption(E_DebugCam_RelativeFlyCamera, "RelativeFly");
	AddOption(E_DebugCam_RelativeCamera, "Relative");
	AddGroup();
	AddOption(E_DebugCam_AnimCamera, "Anim");
	AddOption(E_DebugCam_ScriptCamera, "Script");
	AddGroup();
	AddOption(E_DebugCam_TvCamera, "TvCamera");

	SyncCameraChoice();
}

void C_CameraSwitcher::Exit()
{
	Reset();
}

void C_CameraSwitcher::Update(F32 period)
{
	S32 max = Input::GetGameInputManager()->GetUserIndexCount();

	Input::C_GameInput* input = 0;
	for (S32 i = 0; i < max; i++)
	{
		input = Input::GetGameInput(i);
		
		if (input->GetButtonPressedDebounced(Framework::E_Button_CameraMenu) ||
			input->GetButtonPressedDebounced(Framework::E_Button_ChangeToFlyCamera))
		{
			// This pad is being used as a debug pad now:
			m_DebugPad = i;
		}
	}

	input = Input::GetGameInput(m_DebugPad);
	if (input)
	{
		m_Key[Cam2::E_SwitchKey_Up]    = input->GetButtonPressedDebounced(Framework::E_Button_MenuUp);
		m_Key[Cam2::E_SwitchKey_Down]  = input->GetButtonPressedDebounced(Framework::E_Button_MenuDown);
		m_Key[Cam2::E_SwitchKey_Left]  = input->GetButtonPressedDebounced(Framework::E_Button_MenuLeft);
		m_Key[Cam2::E_SwitchKey_Right] = input->GetButtonPressedDebounced(Framework::E_Button_MenuRight);
		m_Key[Cam2::E_SwitchKey_ShowMenu] = input->GetButtonPressedDebounced(Framework::E_Button_CameraMenu);
		m_Key[Cam2::E_SwitchKey_Confirm] = input->GetButtonPressedDebounced(Framework::E_Button_MenuConfirm);
	}
		
	if (input->GetButtonPressedDebounced(Framework::E_Button_ChangeToFlyCamera))
	{
		if (CameraChoice != E_DebugCam_FlyCamera)
		{
			m_PreviousCamera = CameraChoice;
			CameraChoice = E_DebugCam_FlyCamera;
		}
		else
			CameraChoice = m_PreviousCamera;
		
		ShowAndFade();
	}

	m_Choice = CameraChoice;
	Cam2::C_CameraSwitch::Update(period);
	if (CameraChoice != m_Choice)
		CameraChoice = (E_DebugCam)m_Choice;

	if (CameraChoice != LastChoice)
		SetType((E_DebugCam)CameraChoice);
}

void C_CameraSwitcher::SetType(E_DebugCam type)
{
	LastChoice = CameraChoice = type;
	m_Choice = type;

	System().DebugFeedShow(type);
	SyncCameraChoice();
}

} // End of namespace

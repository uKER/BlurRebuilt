
#include "stdafx.h"
#include "CameraSwitcher.h"
#include "CameraSystem.h"
#include <framework/application/UserControls.h>
#include "VehicleFeed.h"
#include <GameApp.h>

namespace Framework { extern Debug::Bool g_DebugKeyCombos; }

// Camera Switcher allows you to switch camera, either from DebugView or by using Back+LB / LB key
// combinations

DEBUG_ENUM(AmaxCam::E_DebugCam)
	DE_OPTION(AmaxCam::E_DebugCam_GameFeed,          "GameFeed")
	DE_OPTION(AmaxCam::E_DebugCam_AnimCamera,        "AnimCamera")
	DE_OPTION(AmaxCam::E_DebugCam_ChaseCamera2,      "ChaseCamera2")
	DE_OPTION(AmaxCam::E_DebugCam_FatalCamera,       "FatalCamera")
	DE_OPTION(AmaxCam::E_DebugCam_FixedCamera,       "FixedCamera")
	DE_OPTION(AmaxCam::E_DebugCam_FlyCamera,         "FlyCamera")
	DE_OPTION(AmaxCam::E_DebugCam_FutureFeed,        "FutureFeed")
	DE_OPTION(AmaxCam::E_DebugCam_GravGun,           "GravGun")
	DE_OPTION(AmaxCam::E_DebugCam_OrbitCamera,       "OrbitCamera")
	DE_OPTION(AmaxCam::E_DebugCam_RelativeFlyCamera, "RelativeFlyCamera")
	DE_OPTION(AmaxCam::E_DebugCam_RelativeCamera,    "RelativeCamera")
	DE_OPTION(AmaxCam::E_DebugCam_ScriptCamera,      "ScriptCamera")
	DE_OPTION(AmaxCam::E_DebugCam_SimpleChaseCamera, "SimpleChaseCamera")
	DE_OPTION(AmaxCam::E_DebugCam_TvCamera,          "TvCamera")
	DE_OPTION(AmaxCam::E_DebugCam_TvFeed,            "TvFeed")
	DE_OPTION(AmaxCam::E_DebugCam_VehicleFeed,       "VehicleFeed")
END_DEBUG_ENUM

namespace AmaxCam
{
static Debug::Int g_DebugViewport("Camera2.DebugViewport", 0, Debug::Range(0, 3));
static Debug::Enum<E_DebugCam> g_CameraChoice("Camera2.CameraChoice", E_DebugCam_GameFeed);
static Debug::Int g_GameViewport("Camera2.GameViewport", 0, Debug::Range(0, 3));
static S32 g_LastGameViewport = 0, g_LastChoice = 0;

C_CameraSwitcher& Switcher()
{
	static C_CameraSwitcher s_instance;
	return s_instance;
}

C_CameraSwitcher::C_CameraSwitcher()
{
	m_DebugPad = 0;
}

void C_CameraSwitcher::Init()
{
	// Build camera list:
	Reset();
	AddGroup();
	AddOption(E_DebugCam_GameFeed, "GameFeed");
	AddOption(E_DebugCam_GravGun, "GravGun");
	AddOption(E_DebugCam_VehicleFeed, "VehicleFeed");
	AddGroup();
	AddOption(E_DebugCam_FlyCamera, "Fly");
	AddOption(E_DebugCam_FixedCamera, "Fixed");
	AddOption(E_DebugCam_RelativeFlyCamera, "RelativeFly");
	AddOption(E_DebugCam_RelativeCamera, "Relative");
	AddGroup();
	AddOption(E_DebugCam_ChaseCamera2, "Chase2");
	AddOption(E_DebugCam_ScriptCamera, "Script");
	AddOption(E_DebugCam_TvCamera, "Tv");
	AddOption(E_DebugCam_TvFeed, "TvFeed");

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
	if (Framework::g_DebugKeyCombos)
	{
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
	}
	
	
	memset(m_Key, 0, sizeof(m_Key));

	input = Input::GetGameInput(m_DebugPad);

	if (Framework::g_DebugKeyCombos && input)
	{
		m_Key[Cam2::E_SwitchKey_Up]    = input->GetButtonPressedDebounced(Framework::E_Button_MenuUp);
		m_Key[Cam2::E_SwitchKey_Down]  = input->GetButtonPressedDebounced(Framework::E_Button_MenuDown);
		m_Key[Cam2::E_SwitchKey_Left]  = input->GetButtonPressedDebounced(Framework::E_Button_MenuLeft);
		m_Key[Cam2::E_SwitchKey_Right] = input->GetButtonPressedDebounced(Framework::E_Button_MenuRight);
		m_Key[Cam2::E_SwitchKey_ShowMenu] = input->GetButtonPressedDebounced(Framework::E_Button_CameraMenu);
		m_Key[Cam2::E_SwitchKey_Confirm] = input->GetButtonPressedDebounced(Framework::E_Button_MenuConfirm);
		
		if (input->GetButtonPressedDebounced(Framework::E_Button_ChangeToFlyCamera))
		{
			if (g_CameraChoice == E_DebugCam_FlyCamera)
				g_CameraChoice = E_DebugCam_GameFeed;
			else
				g_CameraChoice = E_DebugCam_FlyCamera;
			
			ShowAndFade();
		}
	}
	
	m_Choice = g_CameraChoice;
	Cam2::C_CameraSwitch::Update(period);
	if (g_CameraChoice != m_Choice)
		g_CameraChoice = (E_DebugCam)m_Choice;

	if (g_CameraChoice != g_LastChoice || g_GameViewport != g_LastGameViewport)
		SetType((E_DebugCam)g_CameraChoice);
}

void C_CameraSwitcher::SetType(E_DebugCam type)
{
	g_LastChoice = g_CameraChoice = type;
	g_LastGameViewport = g_GameViewport;
	
	m_Choice = type;

	CameraSystem().DebugFeedShow(g_DebugViewport, type, g_GameViewport);
	SyncCameraChoice();
}

S32 C_CameraSwitcher::DebugViewport() const
{
	return g_DebugViewport;
}

} // End of namespace

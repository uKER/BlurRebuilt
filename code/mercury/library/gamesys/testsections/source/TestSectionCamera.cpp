#include "stdafx.h"
#include "TestSections/TestSectionCamera.h"
#include <Render/Sky/SkyDome.h>
#include <Render/Render/GridRender.h>
#include <Framework/Application/UserControls.h>
#include <Framework/Application/ApplicationBase.h>
#include <camera2.h>
#include <camera2/debug/DebugTools.h>
#include <camera2/stock/ChaseCamera2.h>
#include <camera2/stock/AnimCamera.h>
#include <camera2/stock/TvFeed.h>
#include <camera2/shake/ShakeWorld.h>
#include "BoxWorld.h"
#include "BoxCar.h"
#include <system/archive/btl.h>

namespace Framework
{
enum E_CameraOption
{
	E_GameFeed = 1,
	E_TvFeed = 2,
	E_AnimCamera = Cam2::E_AnimCamera,
	E_ChaseCamera2 = Cam2::E_ChaseCamera2,
	E_FixedCamera = Cam2::E_FixedCamera,
	E_FlyCamera = Cam2::E_FlyCamera,
	E_RelativeCamera = Cam2::E_RelativeCamera,
	E_RelativeFlyCamera = Cam2::E_RelativeFlyCamera,
	E_ScriptCamera = Cam2::E_ScriptCamera,
	E_SimpleChaseCamera = Cam2::E_SimpleChaseCamera,
	E_TvCamera = Cam2::E_TvCamera,
};
}

DEBUG_ENUM(Framework::E_CameraOption)
	DE_OPTION(Framework::E_GameFeed,       "GameFeed")
	DE_OPTION(Framework::E_AnimCamera,     "AnimCamera")
	DE_OPTION(Framework::E_ChaseCamera2,   "ChaseCamera2")
	DE_OPTION(Framework::E_FlyCamera,      "FlyCamera")
	DE_OPTION(Framework::E_FixedCamera,    "FixedCamera")
	DE_OPTION(Framework::E_RelativeCamera, "RelativeCamera")
	DE_OPTION(Framework::E_RelativeFlyCamera,"RelativeFlyCamera")
	DE_OPTION(Framework::E_ScriptCamera,     "ScriptCamera")
	DE_OPTION(Framework::E_SimpleChaseCamera,"SimpleChaseCamera")
	DE_OPTION(Framework::E_TvCamera,       "TvCamera")
	DE_OPTION(Framework::E_TvFeed,         "TvFeed")
END_DEBUG_ENUM

namespace Framework
{
using namespace Maths;
using namespace Util;

static Debug::Enum<E_CameraOption> CameraChoice("Camera2.CameraChoice", E_GameFeed);
static S32 LastChoice = 0;
static Debug::Float LookAngle("Camera2.LookAngle", 0.f, Debug::Range( -180.f, 180.f));
static Cam2::C_Feed GameFeed;
static Cam2::C_ChaseCamera2 ChaseCamera;

C_TestSectionCamera::C_TestSectionCamera(const C_String& name) : C_TestSection(name)
{
	// Debug tools (network comms etc...)
	m_pDebugTools = new Cam2::C_DebugTools;
	assert(m_pDebugTools);
	m_pDebugTools->Init();

	m_pSkyDome = new Render::Sky::C_SkyDome();

	// Build camera list:
	m_camList.Clear();
	m_camList.PushBack();
	m_camList.Back().PushBack(E_GameFeed);
	m_camList.PushBack();
	m_camList.Back().PushBack(E_FlyCamera);
	m_camList.Back().PushBack(E_FixedCamera);
	m_camList.Back().PushBack(E_RelativeFlyCamera);
	m_camList.Back().PushBack(E_RelativeCamera);
	m_camList.PushBack();
	m_camList.Back().PushBack(E_TvFeed);
	m_camList.Back().PushBack(E_AnimCamera);
	m_camGroup = m_camSel = 0;
	GameFeed.Show(&ChaseCamera);

}

C_TestSectionCamera::~C_TestSectionCamera()
{
	SAFE_DELETE_PTR(m_pSkyDome);
	SAFE_DELETE_PTR(m_pDebugTools);
}

void C_TestSectionCamera::OnUpdate()
{
	assert(m_pDebugTools);
	F32 period = Application().GetTimer().GetFrameTime();

	GetSplitscreenView(0).m_ViewSetup.m_ViewSetup3D.m_FarClip = 2000.f;

	g_BoxWorld.Update(period);
	// Get player's car:
	C_BoxCar* car = g_BoxWorld.GetCar(0);
	assert(car);
	Cam2::C_CamObj obj(car->m_position, car->m_orient);

	// Update debug tools:
	Cam2::C_DebugFeed& debugFeed = GetSplitscreenView(0).DebugFeed();
	m_pDebugTools->SetObject(obj);
	m_pDebugTools->SetLocalCamera(debugFeed.Live());
	m_pDebugTools->Update(period);

	if (m_pDebugTools->Notify() == Cam2::E_NetNotify_SetPos)
	{
		debugFeed.SetDebugType(E_FlyCamera);
		debugFeed.CopyFrom(m_pDebugTools->RemoteCamera());
	}

	CycleCameras();

	// Activate controls if we aren't in a debug camera which uses input:
	car->m_activeControls = !debugFeed.UsesInput();
	
	ChaseCamera.SetObject(obj);
	ChaseCamera.SetLookAround(DegToRad((F32)LookAngle));
	ChaseCamera.Update(period);
	debugFeed.SetObject(obj);
}

void C_TestSectionCamera::CycleCameras()
{
	if (m_camList.Size() <= 0)
		return;

	Input::C_GameInput *input = Input::GetGameInput(0);
	bool group = input->GetButtonPressedDebounced(E_Button_ChangeToFlyCamera);
	bool sub = input->GetButtonPressedDebounced(E_Button_ChangeCamera);
	if (group)
		sub = false;

	if (group)
	{
		m_camGroup++; // Change camera group
		m_camSel = 0;
	}

	if (sub)
		m_camSel++; // Change camera selection

	// Wrap around group:
	if (m_camGroup >= m_camList.Size())
		m_camGroup = 0;

	Ut::Vector<S32>& subList = m_camList[m_camGroup];
	if (subList.Size() <= 0)
		return;
	
	// Wrap around group:
	if (m_camSel >= subList.Size())
		m_camSel = 0;

	if (group || sub)
		CameraChoice = (E_CameraOption)subList[m_camSel];

	Cam2::C_DebugFeed& debugFeed = GetSplitscreenView(0).DebugFeed();
	if (LastChoice != CameraChoice)
	{
		switch (CameraChoice)
		{
			case E_GameFeed:
				debugFeed.ShowFeed(&GameFeed);
			break;
			case E_TvFeed:
				debugFeed.ShowFeed(m_pDebugTools->TvFeed());
			break;
			case E_AnimCamera:
				debugFeed.ShowFeed(m_pDebugTools->AnimCamera());
			break;
			default:
				debugFeed.SetDebugType(CameraChoice);
			break;
		}
		
		LastChoice = CameraChoice;
	}
}

void C_TestSectionCamera::OnRender3DViewWithAA(const S32 viewIndex)
{
	assert(m_pSkyDome);
	assert(m_pDebugTools);

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, RGBA(0, 75, 150, 225));

	m_pSkyDome->Render();
	
	m_pDebugTools->DebugRender();
	g_BoxWorld.Render();
	GameFeed.Live().DebugRender(RGBA(0xffffff00), true);

	Render::DrawGrid();

	Cam2::I_Feed& feed = GetSplitscreenView(0).DebugFeed();
	C_String string = C_String("Cam: ") + feed.InfoString();
	Display::FontRender()->Draw(string.Chars(), 64.f, 704.f, 20.f, 24.f, RGBA(255, 255, 255));
}

}; // namespace Framework

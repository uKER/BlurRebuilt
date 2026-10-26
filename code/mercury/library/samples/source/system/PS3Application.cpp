#include "StdAfx.h"
#include "Samples/System/PS3Application.h"
#include <cell/sysmodule.h>
#include <system/Artery/Artery.h>
#include <netex/net.h>
#include <sys/synchronization.h>
#include <sys/ppu_thread.h>
#include <sys/poll.h>
#include <netex/libnetctl.h>
#include <system/Debug/Log/DebugLog.h>
#include <System/Debug/Log/Subscribers/DebugViewSubscriber.h>
#include <system/Perf/ProfilerBackend.h>
#include <system/memory/memorytracker.h>

//int PreStaticInit()
//{
//	int ret = cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
//	if(ret) 
//	{
//		printf("cellSysmoduleLoadModule() error 0x%x !\n", ret);
//		sys_process_exit(1);
//	}
//	return 0;
//}

void GameLoadDebugVariables()
{
}

void GameSaveDebugVariables()
{
}

void LinkToDebugView(const char* name)
{
	cellSysmoduleLoadModule(CELL_SYSMODULE_NET);

	sys_net_initialize_network();
	cellNetCtlInit();

	ArteryStartup(true, name);

	Debug::NetworkStart();

	Debug::GetRootLogger().AddSubscriber( new Debug::DebugViewSubscriber() );

	Memory::InitialiseNetworkConnection();

	Perf::InitialiseNetworkConnection();
}

namespace Samples
{

C_PS3Application::C_PS3Application() :
	m_Quit(false)
{
	m_DeviceParams.m_SystemMemorySize	= 1024*1024*45;
	m_DeviceParams.m_MainMemorySize		= 1024*1024*10;
	m_DeviceParams.m_DeviceMemorySize	= 1024*1024*50;
	m_DeviceParams.m_UniqueBackBuffer	= true;

	Sys::C_FileSystem::Instance().SetWorkingDirectory("/app_home/Image");

	m_Name = "MercuryTestAppPS3";

	m_Pad.Initialise();
}

C_PS3Application::~C_PS3Application()
{
}

bool C_PS3Application::Create()
{
	LinkToDebugView(m_Name.Chars());

	if (C_Application::Create())
	{
		Display::CreateTexture2DFromFile("Textures/ps3.dds", &m_LogoTexture);
		
		return true;
	}
	else
	{
		return false;
	}
}

void C_PS3Application::Update()
{
	m_Pad.Update();

	// [Jan] pad reading code for PS3
	if (m_Pad.IsFirstDn(0, 2, CELL_PAD_CTRL_LEFT))		
		m_Input.ButtonDn(E_Button_LtPad);	
	else if(!m_Pad.NextDn(0, 2, CELL_PAD_CTRL_LEFT))
		m_Input.ButtonUp(E_Button_LtPad);

	if (m_Pad.IsFirstDn(0, 3, CELL_PAD_CTRL_CIRCLE))	
		m_Input.ButtonDn(E_Button_B);		
	else if (!m_Pad.NextDn(0, 3, CELL_PAD_CTRL_CIRCLE))		
		m_Input.ButtonUp(E_Button_B);
	if (m_Pad.IsFirstDn(0, 3, CELL_PAD_CTRL_CROSS))	
		m_Input.ButtonDn(E_Button_A);		
	else if (!m_Pad.NextDn(0, 3, CELL_PAD_CTRL_CROSS))		
		m_Input.ButtonUp(E_Button_A);
	if (m_Pad.IsFirstDn(0, 3, CELL_PAD_CTRL_TRIANGLE))	
		m_Input.ButtonDn(E_Button_Y);		
	else if (!m_Pad.NextDn(0, 3, CELL_PAD_CTRL_TRIANGLE))		
		m_Input.ButtonUp(E_Button_Y);
	if (m_Pad.IsFirstDn(0, 3, CELL_PAD_CTRL_SQUARE))	
		m_Input.ButtonDn(E_Button_X);		
	else if (!m_Pad.NextDn(0, 3, CELL_PAD_CTRL_SQUARE))	
		m_Input.ButtonUp(E_Button_X);
	// TODO: read all other buttons as well

	if (m_Pad.NextDn(0, 3, CELL_PAD_CTRL_R2))
		m_GameViews[0].m_Camera.m_Speed = 4.0f;
	else
	if (m_Pad.NextDn(0, 3, CELL_PAD_CTRL_L2))
		m_GameViews[0].m_Camera.m_Speed = 0.25f;
	else
		m_GameViews[0].m_Camera.m_Speed = 1.0f;

	if (Display::MenuRenderExists() && Display::MenuRender()->GetShow())
	{
		if (m_Pad.IsFirstDn(0, 2, CELL_PAD_CTRL_LEFT))
			Display::MenuRender()->GetCurrItem()->Dec();
		if (m_Pad.IsFirstDn(0, 2, CELL_PAD_CTRL_RIGHT))
			Display::MenuRender()->GetCurrItem()->Inc();
		if (m_Pad.IsFirstDn(0, 2, CELL_PAD_CTRL_UP))
			Display::MenuRender()->GetCurrItem()->Up();
		if (m_Pad.IsFirstDn(0, 2, CELL_PAD_CTRL_DOWN))
			Display::MenuRender()->GetCurrItem()->Dn();
		if (m_Pad.IsFirstDn(0, 3, CELL_PAD_CTRL_CIRCLE))
			Display::MenuRender()->GetCurrItem()->Lt();
		if (m_Pad.IsFirstDn(0, 3, CELL_PAD_CTRL_CROSS))
			Display::MenuRender()->GetCurrItem()->Rt();

		F32 t = 0.2f;
		{
			F32 v = (F32)((char)(m_Pad.m_Prev[0].button[4] - 0x80)) / 128.f;
			if (v < -t || v > t)
				Display::MenuRender()->IncX(v * 4.f);
		}
		{
			F32 v = (F32)((char)(m_Pad.m_Prev[0].button[5] - 0x80)) / 128.f;
			if (v < -t || v > t)
				Display::MenuRender()->IncY(v * 4.f);
		}
	}
	else
	{
		if (m_Pad.PrevUp(0, 2, CELL_PAD_CTRL_LEFT) && m_Pad.NextDn(0, 2, CELL_PAD_CTRL_LEFT) && m_pSection->InSection())
			m_pSection->ExitSection();
		if (m_Pad.PrevUp(0, 2, CELL_PAD_CTRL_RIGHT) && m_Pad.NextDn(0, 2, CELL_PAD_CTRL_RIGHT) && !m_pSection->InSection())
			m_pSection->EnterSection();
		if (m_Pad.PrevUp(0, 2, CELL_PAD_CTRL_UP) && m_Pad.NextDn(0, 2, CELL_PAD_CTRL_UP) && !m_pSection->InSection())
			m_pSection->PrevSection();
		if (m_Pad.PrevUp(0, 2, CELL_PAD_CTRL_DOWN) && m_Pad.NextDn(0, 2, CELL_PAD_CTRL_DOWN) && !m_pSection->InSection())
			m_pSection->NextSection();

		if (m_pSection->InSection())
		{
			F32 t = 0.2f;
			{
				F32 v = (F32)((char)(m_Pad.m_Prev[0].button[6] - 0x80)) / 128.f;
				if (v > 0.f && v > t)
					m_GameViews[0].m_Camera.EnableAction(E_CameraAction_Lt, -v);
				if (v < 0.f && v < -t)
					m_GameViews[0].m_Camera.EnableAction(E_CameraAction_Rt, v);
				m_Input.AxisValue(E_Axis_LX, ((Maths::Abs(v) > t) ? v : 0.f));
			}
			{
				F32 v = (F32)((char)(m_Pad.m_Prev[0].button[7] - 0x80)) / 128.f;
				if (v > 0.f && v > t)
					m_GameViews[0].m_Camera.EnableAction(E_CameraAction_Bw, v);
				if (v < 0.f && v < -t)
					m_GameViews[0].m_Camera.EnableAction(E_CameraAction_Fw, -v);
				m_Input.AxisValue(E_Axis_LY, ((Maths::Abs(v) > t) ? -v : 0.f));
			}
			{
				F32 v = (F32)((char)(m_Pad.m_Prev[0].button[4] - 0x80)) / 128.f;
				if (v < -t || v > t)
					m_GameViews[0].m_Camera.EnableAction(E_CameraAction_Ax, v * -0.25f);
				m_Input.AxisValue(E_Axis_RX, ((Maths::Abs(v) > t) ? v : 0.f));
			}
			{
				F32 v = (F32)((char)(m_Pad.m_Prev[0].button[5] - 0x80)) / 128.f;
				if (v < -t || v > t)
					m_GameViews[0].m_Camera.EnableAction(E_CameraAction_Az, v * 0.25f);
				m_Input.AxisValue(E_Axis_RY, ((Maths::Abs(v) > t) ? v : 0.f));
			}
		}
	}

	if (m_Pad.IsFirstDn(0, 2, CELL_PAD_CTRL_START))
	{
		Display::MenuRender()->SetShow(!Display::MenuRender()->GetShow());
	}

	if (m_Pad.IsFirstDn(0, 2, CELL_PAD_CTRL_SELECT)) // [LloydW] GCM HUD uses this button!
		m_Timer.TogglePaused();

	if (m_Pad.IsFirstDn(0, 3, CELL_PAD_CTRL_TRIANGLE))
		if (m_GameViewCount > 0)
			m_GameViews[0].m_Camera.Save();

	C_Application::Update();
	
	m_GameViews[0].m_Camera.Reset();
}

void C_PS3Application::Run()
{
	while (!m_Quit)
	{
		UpdateMemoryManager();

		Debug::NetworkUpdate();

		CPU_TIMING(0, Maths::RGBA(255, 255, 255, 255), "CPU");
		
		Update();
		Render();

		#ifdef USE_CPU_TIMINGS
		Perf::C_CPUTimingStats::UpdateStats();
		#endif

		#ifdef USE_GPU_TIMINGS
		Display::C_GPUTimingStats::UpdateStats();
		#endif

		if(m_EnterSection)
		{
			m_pSection->EnterSection();
			m_EnterSection = false;
		}
	}

	Destroy();
}

}; // namespace Samples

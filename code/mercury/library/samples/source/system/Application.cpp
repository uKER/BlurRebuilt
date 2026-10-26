#include "StdAfx.h"
#include "Samples/System/Application.h"
#include "Samples/System/Config.h"
#include "Display/Utils/Screenshot.h"
#include "Display/Memory/MemorySystem.h"
#include "Display/Memory/MemoryGraphics.h"
#include "Display/Device/DeviceDebug.h"
#if defined(PLATFORM_PS3)
#include "System/Memory.h"
#endif
#include "System/Debug/Log/AutoConfigure.h"
#include <system/Perf/ProfilerBackend.h>

Debug::Int		g_debugPresentInterval								("Render.PresentInterval",							0, Debug::Range(0, S32(Display::E_PresentInterval_Count) - 1));
Debug::Bool		g_debugDumpSystemMemoryUsage						("Memory.DumpSystemMemoryUsage",					false);
Debug::Bool		g_debugDumpGraphicsMemoryUsage						("Memory.DumpGraphicsMemoryUsage",					false);
Debug::Bool		g_debugDumpSystemMemoryUsageToFile					("Memory.DumpSystemMemoryUsageToFile",				false);
Debug::Bool		g_debugDumpGraphicsMemoryUsageToFile				("Memory.DumpGraphicsMemoryUsageToFile",			false);

DEBUG_ENUM(Samples::E_ViewSetup)
	DE_OPTION(Samples::E_ViewSetup_1,	"E_ViewSetup_1")
	DE_OPTION(Samples::E_ViewSetup_2h,	"E_ViewSetup_2h")
	DE_OPTION(Samples::E_ViewSetup_2v,	"E_ViewSetup_2v")
	DE_OPTION(Samples::E_ViewSetup_3h,	"E_ViewSetup_3h")
	DE_OPTION(Samples::E_ViewSetup_3v,	"E_ViewSetup_3v")
	DE_OPTION(Samples::E_ViewSetup_4,	"E_ViewSetup_4")
END_DEBUG_ENUM

Debug::Enum<Samples::E_ViewSetup> g_debugViewSetup("Samples.ViewSetup", Samples::E_ViewSetup_1);

enum E_Resolution
{
	E_Resolution_Disabled = 0,
};
DEBUG_ENUM(E_Resolution)
	DE_OPTION(E_Resolution_Disabled,	"Disabled")
END_DEBUG_ENUM
Debug::Enum<E_Resolution> g_debugResolution("Samples.Resolution", E_Resolution_Disabled);

#ifndef PLATFORM_PC
extern Debug::Bool g_debugDisplayUpdateResolution;
#endif

#if defined(PLATFORM_PS3)
void AssertOutOfMemory(void)
{
	#if defined(PLATFORM_PS3)
	Memory::Tracker::Instance().DumpMemoryRaw("/app_home/OutOfMemory");
	#endif

	Display::C_MemoryManagerSystem::OutOfMemory();
}
#endif

Samples::C_Application* g_pApp = NULL;

namespace Samples
{

Debug::Bool g_debugDumpMemoryUsage			("Render.DumpMemoryUsage",			false);
Debug::Bool g_debugReloadableTextures		("Render.ReloadableTextures",		false);
Debug::Bool g_debugTakeScreenshot			("Render.TakeScreenshot",			false);
Debug::Bool g_debugGradWindowBufferColour	("Render.GradWindowBufferColour",	false);
Debug::Bool g_debugGradWindowBufferDepth	("Render.GradWindowBufferDepth",	false);
Debug::Bool g_debugGradBackBufferColour		("Render.GradBackBufferColour",		false);
Debug::Bool g_debugGradBackBufferDepth		("Render.GradBackBufferDepth",		false);

#ifdef USER_LLOYDW
Debug::Bool g_debugReloadableShaders		("Render.ReloadableShaders", 		false);
#else
Debug::Bool g_debugReloadableShaders		("Render.ReloadableShaders", 		false);
#endif

C_Application::C_Application() :
	m_EnterSection(false),
	m_GameViewCount(0)
{
	Display::C_Manager::CreateErrors();

	MEMORY_TRACKER_PUSH("mercury.samples.application");
	m_pDisplay		= new Display::C_Manager();
	MEMORY_TRACKER_POP("mercury.samples.application");
	m_pDevice		= NULL;
	m_pDebugRender	= NULL;

	Debug::AutoConfigureDebugLog();

	m_Cycle			= 0.0f;
	m_Resolution	= 0;

	g_pApp = this;
}

C_Application::~C_Application()
{
	assert(!m_pDevice && "The application must be destroyed before it is deleted");
	delete m_pDisplay;

	Display::C_Manager::DestroyErrors();
}

struct C_Resolution
{
};

bool C_Application::Create()
{
	assert(m_pDisplay);

	if(!m_pDisplay->Initialise())
		return false;

	Display::C_VideoMode videoMode;
	if(!m_pDisplay->GetVideoMode(&videoMode))
		return false;

	#if defined(PLATFORM_PS3)
	m_DeviceParams.m_HiDef = videoMode.m_HiDef;
	m_DeviceParams.m_WideScreen = videoMode.m_WideScreen;
	#endif

	#if defined(PLATFORM_PS3)
	Display::E_Multisample windowMultisample = Display::E_Multisample_2;
	#else
	Display::E_Multisample windowMultisample = Display::E_Multisample_None;
	#endif

	#if defined(PLATFORM_PS3)
	if(videoMode.m_Height == 480 || videoMode.m_Height == 576 || videoMode.m_Height == 720)
	{
		m_DeviceParams.Setup(
			videoMode.m_Width, videoMode.m_Height,
			1280, 720, Display::E_Multisample_None,
			1024, 720, windowMultisample);

		m_DeviceParams.m_UniqueBackBuffer = videoMode.m_Height == 720 ? false : true;
	}
	else
	{
		m_DeviceParams.Setup(
			videoMode.m_Width, videoMode.m_Height,
			videoMode.m_Width, videoMode.m_Height, Display::E_Multisample_None,
			videoMode.m_Width, videoMode.m_Height, windowMultisample);

		m_DeviceParams.m_UniqueBackBuffer = true;
	}
	#else
	{
		m_DeviceParams.Setup(
			1280, 720,
			1280, 720, Display::E_Multisample_None,
			1280, 720, windowMultisample);
	}
	#endif

	// TODO: Query res etc.

	m_pDevice = new Display::C_Device();
	
	AddWindows();

	if(!Display::Manager()->CreateDevice(m_pDevice, m_DeviceParams))
	{
		SAFE_DELETE_PTR(m_pDevice);
		return false;
	}

	m_pDebugRender	= new Display::C_DebugRender();
	m_pSection		= new C_SectionManager();

	m_pSection->SetRendererType(Display::E_RendererType_Persistent);

	RESOURCE_MEMORY_ID("Samples");

	if(!MyCreate())
		return false;

	if(!m_pDevice->CreateRenderersVolatile())
		return false;

	if(!m_pDevice->CreateRenderers())
		return false;

	AddViews();

	#ifndef PLATFORM_PC
	SetupResolutions(
		m_DeviceParams.m_WindowBufferWidth,
		m_DeviceParams.m_WindowBufferHeight,
		m_DeviceParams.m_WindowBufferColourFormat,
		m_DeviceParams.m_WindowBufferMultisample,
		Display::E_Format_Tiled);
	#endif

	return true;
}

void C_Application::Destroy()
{
	assert(m_pDisplay);

	if(m_pDevice)
	{
		m_pDevice->DestroyRenderersVolatile();
		m_pDevice->DestroyRenderers();

		MyDestroy();

		SAFE_DELETE_PTR(m_pDebugRender);
		SAFE_DELETE_PTR(m_pSection);

		m_pDisplay->Destroy();

		//lint -save -e423
		m_pDevice = NULL;
		//line -restore
	}

	m_pDisplay->Shutdown();
}

void C_Application::AddSection(C_SectionLauncherBase* pSection)
{
	assert(pSection);
	assert(m_pSection);
	m_pSection->AddSection(pSection);
}

void C_Application::Update()
{
	UpdateMemoryManager();

	PERF_EVENT("Update");
	CPU_TIMING(0, Maths::RGBA(0, 0, 255, 255), "CPU.Update");

	MyUpdate();
	m_Timer.Update();

	m_Cycle += m_Timer.GetFrameTime();
	if(m_Cycle > 1.0f)
	{
		m_Cycle = 0.0f;
		
		if(g_debugDumpMemoryUsage)
			Memory::Tracker::Instance().DumpNodeStats(false, true);
	}

	m_HudView.Update(m_Timer.GetFrameTime());
	for(S32 i = 0; i < m_GameViewCount; i++)
	{
		m_GameViews[i].Update(m_Timer.GetFrameTime());
	}

	m_pSection->UpdateSection(m_Input, m_Timer);

	if(g_debugReloadableShaders)
	{

		Display::RefreshVertexShaders();
		Display::RefreshPixelShaders();
	}

	if(g_debugReloadableTextures)
	{
	}
}

void C_Application::Render()
{
	PERF_EVENT("Render");
	GPU_TIMING_BEGIN_FRAME();
	GPU_TIMING(0, Maths::RGBA(255, 255, 255, 255), "GPU");

	// Render the frame
	{
		CPU_TIMING(0, Maths::RGBA(0, 255, 0, 255), "CPU.Render");
		GPU_TIMING(0, Maths::RGBA(0, 255, 0, 255), "GPU.Render");

		ResizeWindows();

		if(m_pDevice->BeginScene())
		{
			SetupViews();

			m_pSection->PreRenderSection(m_Input, m_Timer);

			m_pDevice->SetColourDepthStencilSurface();
			m_pDevice->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 0, 0, 0));
			m_pDevice->SetPresentInterval((Display::E_PresentInterval)((S32)g_debugPresentInterval));
			
			RenderStage1();
			RenderStage2();
			RenderUI();
			
			m_pSection->PostRenderSection(m_Input, m_Timer);

			m_pDevice->SetView(NULL, false);

			m_pDevice->EndScene();
		}
	}

	#if !defined(CFG_GOLD) && defined(PLATFORM_PC)
	if(g_debugGradBackBufferColour)
	{
		const Display::C_BackBuffer& backBuffer = m_pDevice->GetBackBuffer();
		const Util::C_String fileName = "Screenshots/BackBufferColour.tga";
		Display::TakeScreenshot(fileName.Chars(), backBuffer.GetColourSurface());
		g_debugGradBackBufferColour = false;
	}
	if(g_debugGradBackBufferDepth)
	{
		const Display::C_BackBuffer& backBuffer = m_pDevice->GetBackBuffer();
		const Util::C_String fileName = "Screenshots/BackBufferDepth.tga";
		Display::TakeScreenshot(fileName.Chars(), backBuffer.GetDepthStencilSurface());
		g_debugGradBackBufferDepth = false;
	}
	#endif

	// Present the frame
	{
		CPU_TIMING(0, Maths::RGBA(255, 0, 0, 255), "CPU.Present");
		GPU_TIMING(0, Maths::RGBA(255, 0, 0, 255), "GPU.Present");

		m_pDevice->Present();
	}
	
	GPU_TIMING_END_FRAME();
	Perf::Profiler::Instance().EndFrame();
}

void C_Application::RenderStage1()
{
	PERF_EVENT("RenderStage1");
	assert(m_pDevice);

	for(S32 i=m_GameViewCount-1; i>=0; i--)
	{
		PERF_EVENT("RenderGameView");

		C_View& gameView = m_GameViews[i];

		#if !defined(PLATFORM_PC)
		m_pDevice->SetWindow(i, true);
		m_pDevice->BeginWindow();
		#endif

		#if defined(PLATFORM_PC)
		m_pDevice->SetupViewIndex(gameView.m_ViewIndex, gameView.m_ViewSetup);
		#else
		Display::C_Window* pWindow = m_pDevice->GetWindow(i);
		const Maths::C_Rect windowCoords = gameView.m_ViewSetup.m_WindowCoords;
		gameView.m_ViewSetup.m_WindowCoords = Maths::C_Rect(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(1024.0f, 768.0f));
		m_pDevice->SetupViewIndex(gameView.m_ViewIndex, gameView.m_ViewSetup, pWindow->m_Width, pWindow->m_Height);
		gameView.m_ViewSetup.m_WindowCoords = windowCoords;
		#endif

		m_pDevice->SetViewIndex(gameView.m_ViewIndex);

		RenderGameViewWithAA();

		#if defined(PLATFORM_PC)
		RenderGameViewWithoutAA();
		#else
		m_pDevice->EndWindow();
		#endif
	}

	#if !defined(PLATFORM_PC)
	m_pDevice->SetColourDepthStencilSurface();
	#endif
}

void C_Application::RenderStage2()
{
	PERF_EVENT("RenderStage2");
	assert(m_pDevice);
	
	for(S32 i=m_GameViewCount-1; i>=0; i--)
	{
		PERF_EVENT("RenderGameView");

		const C_View& gameView = m_GameViews[i];

		m_pDevice->SetupViewIndex(gameView.m_ViewIndex, gameView.m_ViewSetup);
		m_pDevice->SetViewIndex(gameView.m_ViewIndex);

		BeginRenderSplitscreenWithoutAA();

		#if defined(PLATFORM_PC)
		const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();
		const Display::C_RenderTarget& renderTarget = Display::Device()->GetRenderTarget();
		IDirect3DSurface9* pd3dSrcSurface = backBuffer.GetColourSurface()->m_pd3dSurface;
		IDirect3DSurface9* pd3dDstSurface = renderTarget.GetColourSurface()->m_pd3dSurface;
		if(pd3dSrcSurface != pd3dDstSurface)
		{
			const HRESULT hr = Display::Device()->GetD3DDevice()->StretchRect(
				pd3dSrcSurface,
				NULL,
				pd3dDstSurface,
				NULL,
				D3DTEXF_LINEAR);
			assert(SUCCEEDED(hr));
		}
		#else
		m_pDevice->RestoreWindow(i);

		RenderGameViewWithoutAA();
		#endif

		EndRenderSplitscreenWithoutAA();
	}

	#if !defined(PLATFORM_PC)
	m_pDevice->SetColourDepthStencilSurface();
	#endif
}

void C_Application::RenderUI()
{
	PERF_EVENT("RenderUI");
	assert(m_pDevice);

	m_pDevice->SetupViewIndex(m_HudView.m_ViewIndex, m_HudView.m_ViewSetup);
	m_pDevice->SetViewIndex(m_HudView.m_ViewIndex);

	RenderHudView();
}

void C_Application::RenderGameViewWithAA()
{
	assert(m_pSection);
	m_pSection->GameViewWithAARenderSection(m_Input, m_Timer);
}

void C_Application::RenderGameViewWithoutAA()
{
	assert(m_pSection);
	m_pSection->GameViewWithoutAARenderSection(m_Input, m_Timer);
}

void C_Application::RenderHudView()
{
	assert(m_pDebugRender);
	if(m_EnterSection)
	{
		Display::BasicRender()->DrawColoured2DRect(0.f, 0.f, 1024.f, 768.f, Maths::RGBA(0, 0, 0, 255));
		Display::FontRender()->Draw("Entering section...", 50.f, 50.f, 30.f, 30.f, Display::GREEN);
	}
	else
	{
		assert(m_pSection);
		m_pSection->HudViewRenderSection(m_Input, m_Timer);
	}

	m_pDebugRender->Draw();

	if(m_LogoTexture.IsCreated())
	{
		const F32 w = 64.0f;
		const F32 h = 32.0f;
		Display::BasicRender()->DrawTextured2DRect(1024.0f - w - 40.0f, 40.0f, w, h, Maths::RGBA(255, 255, 255, 128), &m_LogoTexture);
	}
}

void C_Application::BeginRenderSplitscreenWithoutAA()
{
	assert(m_pSection);
	m_pSection->BeginRenderSplitscreenWithoutAA();
}

void C_Application::EndRenderSplitscreenWithoutAA()
{
	assert(m_pSection);
	m_pSection->EndRenderSplitscreenWithoutAA();
}

void C_Application::SetupViews()
{
	for(S32 i=0; i<m_GameViewCount; i++)
		SetupView(&m_GameViews[i]);
	SetupView(&m_HudView);
}

void C_Application::SetupView(C_View* pView)
{
	assert(m_pDevice);
	Display::C_View* pDisplayView = m_pDevice->GetView(pView->m_ViewIndex);
	assert(pDisplayView);

	Maths::C_Rect rect = pView->m_ViewSetup.GetDeviceCoords();
	pView->m_ViewSetup.m_WindowCoords.SetBounds(rect.Max() * Maths::HVec2(pView->m_WindowCoords.x1, pView->m_WindowCoords.y1), rect.Max() * Maths::HVec2(pView->m_WindowCoords.x2, pView->m_WindowCoords.y2));

	pDisplayView->m_ViewSettings3D.m_ViewMatrixPrev	= pDisplayView->m_ViewSettings3D.m_ViewMatrix;
	pDisplayView->m_ViewSettings3D.m_ProjMatrixPrev	= pDisplayView->m_ViewSettings3D.m_ProjMatrix;

	pDisplayView->m_ViewSettings3D.m_CameraToWorldPrev	= pDisplayView->m_ViewSettings3D.m_CameraToWorld;
	pDisplayView->m_ViewSettings3D.m_CameraToDevicePrev	= pDisplayView->m_ViewSettings3D.m_CameraToDevice;
	pDisplayView->m_ViewSettings3D.m_WorldToCameraPrev	= pDisplayView->m_ViewSettings3D.m_WorldToCamera;
	pDisplayView->m_ViewSettings3D.m_WorldToDevicePrev	= pDisplayView->m_ViewSettings3D.m_WorldToDevice;
}

bool C_Application::MyCreate()
{
	return true;
}

void C_Application::MyDestroy()
{
}

void C_Application::MyUpdate()
{
}

void C_Application::AddViews()
{
	assert(m_pDevice);
	m_GameViewCount = 0;

	switch(g_debugViewSetup)
	{
	case E_ViewSetup_1:
		m_GameViews[m_GameViewCount++].Initialise(Display::C_Rect(0.0f, 0.0f, 1.0f, 1.0f));
		break;
	case E_ViewSetup_2h:
		m_GameViews[m_GameViewCount++].Initialise(Display::C_Rect(0.0f, 0.0f, 1.0f, 0.5f));
		m_GameViews[m_GameViewCount++].Initialise(Display::C_Rect(0.0f, 0.5f, 1.0f, 1.0f));
		break;
	case E_ViewSetup_2v:
		m_GameViews[m_GameViewCount++].Initialise(Display::C_Rect(0.0f, 0.0f, 0.5f, 1.0f));
		m_GameViews[m_GameViewCount++].Initialise(Display::C_Rect(0.5f, 0.0f, 1.0f, 1.0f));
		break;
	case E_ViewSetup_3h:
		m_GameViews[m_GameViewCount++].Initialise(Display::C_Rect(0.0f, 0.0f, 1.0f, 0.5f));
		m_GameViews[m_GameViewCount++].Initialise(Display::C_Rect(0.0f, 0.5f, 0.5f, 1.0f));
		m_GameViews[m_GameViewCount++].Initialise(Display::C_Rect(0.5f, 0.5f, 1.0f, 1.0f));
		break;
	case E_ViewSetup_3v:
		m_GameViews[m_GameViewCount++].Initialise(Display::C_Rect(0.0f, 0.0f, 0.5f, 1.0f));
		m_GameViews[m_GameViewCount++].Initialise(Display::C_Rect(0.5f, 0.0f, 1.0f, 0.5f));
		m_GameViews[m_GameViewCount++].Initialise(Display::C_Rect(0.5f, 0.5f, 1.0f, 1.0f));
		break;
	case E_ViewSetup_4:
		m_GameViews[m_GameViewCount++].Initialise(Display::C_Rect(0.0f, 0.0f, 0.5f, 0.5f));
		m_GameViews[m_GameViewCount++].Initialise(Display::C_Rect(0.5f, 0.0f, 1.0f, 0.5f));
		m_GameViews[m_GameViewCount++].Initialise(Display::C_Rect(0.0f, 0.5f, 0.5f, 1.0f));
		m_GameViews[m_GameViewCount++].Initialise(Display::C_Rect(0.5f, 0.5f, 1.0f, 1.0f));
		break;
	default:
		assert(0 && "Unsupported view setup");
		break;
	}

	for(S32 i=0; i<m_GameViewCount; i++)
	{
		m_GameViews[i].m_ViewIndex = m_pDevice->AddView(Display::E_ViewType_2D | Display::E_ViewType_3D, i);
	}

	m_HudView.Initialise(Display::C_Rect(0.0f, 0.0f, 1.0f, 1.0f));
	m_HudView.m_ViewIndex = m_pDevice->AddView(Display::E_ViewType_2D, -1);
}

bool C_Application::ReadConfig(const Util::C_String& fileName)
{
	C_Config config;
	if(config.Open(fileName))
	{
		char buffer[256];
		while(!config.m_Stream.eof())	
		{
			config.m_Stream.getline(buffer, sizeof(buffer));
			ReadConfigLine(buffer);
		}
		return true;
	}
	else
	{
		return false;
	}
}

void C_Application::ReadConfigLine(char* pLine)
{
	assert(pLine);

	U32 strLen = strlen(pLine);

	if(strLen == 0)
		return;

	U32 i;

	for(i=0; i<strLen; i++)
	{
		if(pLine[i] == '=')
			break;
	}

	if(i>=strLen)
		return;

	char name[128];
	char item[128];

	strncpy_s(name, sizeof(name), &pLine[0], i);
	strncpy_s(item, sizeof(item), &pLine[i+1], strLen-i);

	ReadConfigItem(name, item);
}

void C_Application::ReadConfigItem(char* pName, char* pItem)
{
	for(Debug::Iterator it; it; ++it)
	{
		Debug::VarBase* pVarBase = it;
		const char* pVarName = pVarBase->Name();
		if(strcmp(pName, pVarName) == 0)
		{
			switch(pVarBase->Type())
			{
				case Debug::kVtBool:
				{
					Debug::Bool* pVar = (Debug::Bool*)pVarBase;
					if(strcmp(pItem, "false") == 0)
						pVar->SetValue(false);
					else
						pVar->SetValue(true);
				}
				break;

				case Debug::kVtInt:
				{
					Debug::Int* pVar = (Debug::Int*)pVarBase;
					const S32 value = atoi(pItem);
					pVar->SetValue(value);
				}
				break;

				case Debug::kVtFloat:
				{
					Debug::Float* pVar = (Debug::Float*)pVarBase;
					const F32 value = (F32)atof(pItem);
					pVar->SetValue(value);
				}
				break;

				case Debug::kVtEnum:
				{
					Debug::EnumBase* pVar = (Debug::EnumBase*)pVarBase;
					pVar->SetValueByName(pItem);
				}
				break;

				case Debug::kVtString:
				{
					Debug::String* pVar = (Debug::String*)pVarBase;
					pVar->SetValue(pItem);
				}
				break;
			}
		}
		//LOGINFO(g_Log, "%s\n", pVarName);
	}
}

void C_Application::AddWindow(const Maths::C_Rect& windowCoords)
{
	#ifndef PLATFORM_PC
	const S32 windowIndex = m_pDevice->AddWindow(windowCoords);
	Display::C_Window* pWindow = m_pDevice->GetWindow(windowIndex);
	pWindow->Resize(
		m_DeviceParams.m_WindowBufferWidth,
		m_DeviceParams.m_WindowBufferHeight,
		m_DeviceParams.m_WindowBufferMultisample,
		m_DeviceParams.m_WindowBufferColourFormat,
		m_DeviceParams.m_WindowBufferDepthStencilFormat);
	#endif
}

void C_Application::AddWindows()
{
	#ifndef PLATFORM_PC
	m_pDevice->ClearWindows();
	#endif

	switch(g_debugViewSetup)
	{
	case E_ViewSetup_1:
		AddWindow(Maths::C_Rect(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(1.0f, 1.0f)));
		break;
	case E_ViewSetup_2h:
		AddWindow(Maths::C_Rect(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(1.0f, 0.5f)));
		AddWindow(Maths::C_Rect(Maths::HVec2(0.0f, 0.5f), Maths::HVec2(1.0f, 1.0f)));
		break;
	case E_ViewSetup_2v:
		AddWindow(Maths::C_Rect(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(0.5f, 1.0f)));
		AddWindow(Maths::C_Rect(Maths::HVec2(0.5f, 0.0f), Maths::HVec2(1.0f, 1.0f)));
		break;
	case E_ViewSetup_3h:
		AddWindow(Maths::C_Rect(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(1.0f, 0.5f)));
		AddWindow(Maths::C_Rect(Maths::HVec2(0.0f, 0.5f), Maths::HVec2(0.5f, 1.0f)));
		AddWindow(Maths::C_Rect(Maths::HVec2(0.5f, 0.5f), Maths::HVec2(1.0f, 1.0f)));
		break;
	case E_ViewSetup_3v:
		AddWindow(Maths::C_Rect(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(0.5f, 1.0f)));
		AddWindow(Maths::C_Rect(Maths::HVec2(0.5f, 0.0f), Maths::HVec2(1.0f, 0.5f)));
		AddWindow(Maths::C_Rect(Maths::HVec2(0.5f, 0.5f), Maths::HVec2(1.0f, 1.0f)));
		break;
	case E_ViewSetup_4:
		AddWindow(Maths::C_Rect(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(0.5f, 0.5f)));
		AddWindow(Maths::C_Rect(Maths::HVec2(0.5f, 0.0f), Maths::HVec2(1.0f, 0.5f)));
		AddWindow(Maths::C_Rect(Maths::HVec2(0.0f, 0.5f), Maths::HVec2(0.5f, 1.0f)));
		AddWindow(Maths::C_Rect(Maths::HVec2(0.5f, 0.5f), Maths::HVec2(1.0f, 1.0f)));
		break;
	default:
		assert(0 && "Unsupported view setup");
		break;
	}
}

void C_Application::ResizeWindows()
{
	#ifndef PLATFORM_PC
	#ifdef PLATFORM_PS3
	if((g_debugResolution != 0 && g_debugResolution != m_Resolution) || g_debugDisplayUpdateResolution)
	#else
	if(g_debugResolution != 0 && g_debugResolution != m_Resolution)
	#endif
	{
		Display::C_Resolution* pResolution = Display::g_Resolutions.GetResolution(g_debugResolution);
		if(pResolution)
		{
			m_pDevice->m_DeviceParams.m_WindowBufferWidth			= pResolution->m_Width;
			m_pDevice->m_DeviceParams.m_WindowBufferHeight			= pResolution->m_Height;
			m_pDevice->m_DeviceParams.m_WindowBufferMultisample		= pResolution->m_Multisample;


			#if defined(PLATFORM_PS3)
			m_pDevice->BlockUntilIdle();
			#endif

			m_pDevice->ResizeWindows();
		}
		
		m_Resolution = g_debugResolution;

		#ifdef PLATFORM_PS3
		if(g_debugDisplayUpdateResolution)
			g_debugDisplayUpdateResolution = false;
		#endif
	}
	#endif
}

void C_Application::UpdateMemoryManager()
{
	Memory::Tracker::Instance().Update();

	if(g_debugDumpSystemMemoryUsage)
	{
		Display::C_MemoryManagerSystem::DumpUsage();

		g_debugDumpSystemMemoryUsage = false;
	}

	if(g_debugDumpGraphicsMemoryUsage)
	{
		Display::C_MemoryManagerGraphics::DumpUsage(true, false);

		g_debugDumpGraphicsMemoryUsage = false;
	}
	
	if(g_debugDumpSystemMemoryUsageToFile)
	{
		#if defined(PLATFORM_PS3)
		Util::C_String filePath = "\\app_home\\User\\";
		#else
		Util::C_String filePath = "";
		#endif

		filePath += "OutOfMemorySystem";

		Display::C_MemoryManagerSystem::DumpUsage(filePath);

		g_debugDumpSystemMemoryUsageToFile = false;
	}

	if(g_debugDumpGraphicsMemoryUsageToFile)
	{
		#if defined(PLATFORM_PS3)
		Util::C_String filePath = "\\app_home\\User\\";
		#else
		Util::C_String filePath = "";
		#endif

		filePath += "OutOfMemoryGraphics";

		Display::C_MemoryManagerGraphics::DumpUsage(filePath, true, false);

		g_debugDumpGraphicsMemoryUsageToFile = false;
	}
}

void C_Application::SetupResolutions(const U32 width, const U32 height, const Display::E_Format format, const Display::E_Multisample multisample, const U32 flags)
{
	#if 1
	Display::g_Resolutions.SetMaxResolution(1280, 720, Display::E_Format_A8R8G8B8, Display::E_Multisample_2, flags);
	#else
	Display::g_Resolutions.SetMaxResolution(width, height, format, multisample, flags);
	#endif
	
	Display::g_Resolutions.ResetResolutions();

	Display::g_Resolutions.AddResolution(960, 720, Display::E_Format_A8R8G8B8, Display::E_Multisample_None,		flags);
	Display::g_Resolutions.AddResolution(960, 720, Display::E_Format_A8R8G8B8, Display::E_Multisample_2,		flags);
	Display::g_Resolutions.AddResolution(960, 720, Display::E_Format_A8R8G8B8, Display::E_Multisample_4,		flags);

	Display::g_Resolutions.AddResolution(1024, 720, Display::E_Format_A8R8G8B8, Display::E_Multisample_None,	flags);
	Display::g_Resolutions.AddResolution(1024, 720, Display::E_Format_A8R8G8B8, Display::E_Multisample_2,		flags);
	Display::g_Resolutions.AddResolution(1024, 720, Display::E_Format_A8R8G8B8, Display::E_Multisample_4,		flags);

	Display::g_Resolutions.AddResolution(1280, 720, Display::E_Format_A8R8G8B8, Display::E_Multisample_None,	flags);
	Display::g_Resolutions.AddResolution(1280, 720, Display::E_Format_A8R8G8B8, Display::E_Multisample_2,		flags);
	Display::g_Resolutions.AddResolution(1280, 720, Display::E_Format_A8R8G8B8, Display::E_Multisample_4,		flags);
	
	Display::g_Resolutions.SetResolutions(&g_debugResolution);

	g_debugResolution.SetValue(E_Resolution_Disabled);
}

bool C_Application::Resize()
{
	return true;
}

C_Application* GetApp()
{
	return g_pApp;
}

void SetApp(C_Application* pApp)
{
	g_pApp = pApp;
}

}; // namespace Samples

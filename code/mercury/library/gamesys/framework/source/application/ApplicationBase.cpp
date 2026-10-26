#include "FrameworkPch.h"
#include <system/debug/Blur2PCAutomation.h>
#include "Framework/Application/ApplicationBase.h"
#include "Framework/Application/CaptureService.h"
#include "Framework/Application/CoreSystems.h"
#include "Framework/Application/PerformanceLogging.h"
#include "Framework/Application/UserControls.h"
#include "Framework/Application/TimingColours.h"
#include "Framework/Layer/Layer.h"
#include "Framework/Debug/MemoryDumpers.h"

#include <Display/Device/DeviceMarkers.h>

#include <System/Memory/MemoryTracker.h>

#include <Input/DeviceManager.h>
#include <Input/Input.h>
#include <Input/Device.h>

#include <Camera2.h>
#include <Camera2/Capture/CaptureService.h>

#include <Render/Debug/DebugRender.h>
#include <Render/Debug/ImmediateDebugRender.h>
#include <System/perf/ProfilerBackend.h>

#include <Display/Device/DeviceDebug.h>
#include <Display/Utils/Screenshot.h>

#include <Render/Font/Font.h>
#include <Render/Utils/Globals.h>

#include <Resource/Resource.h>

#if TRACK_DISPLAY2_RESOURCES
#include <Display2/Utils/Utils.h>
#endif

#if PLATFORM_PS3
#include "display/buffer/gcm/GCMDeferredCommandBufferManager.h"
#include <system/scheduler.h>
#endif

#include <Display/Device/DeviceMarkers.h>

static Debug::Logger &g_log = Debug::GetLogger("framework.application");

DEBUG_ENUM(Framework::E_ViewSetup)
	DE_OPTION(Framework::E_ViewSetup_1,	"E_ViewSetup_1")
	DE_OPTION(Framework::E_ViewSetup_2h,	"E_ViewSetup_2h")
	DE_OPTION(Framework::E_ViewSetup_2v,	"E_ViewSetup_2v")
	DE_OPTION(Framework::E_ViewSetup_3,		"E_ViewSetup_3")
	DE_OPTION(Framework::E_ViewSetup_3h,	"E_ViewSetup_3h")
	DE_OPTION(Framework::E_ViewSetup_3v,	"E_ViewSetup_3v")
	DE_OPTION(Framework::E_ViewSetup_4,	"E_ViewSetup_4")
END_DEBUG_ENUM

Debug::Enum<Framework::E_ViewSetup> g_debugViewSetup("Graphics.Render.ViewSetup", Framework::E_ViewSetup_1);

enum E_Resolution
{
	E_Resolution_Disabled = 0,
};

DEBUG_ENUM(E_Resolution)
	DE_OPTION(E_Resolution_Disabled,	"Disabled")
END_DEBUG_ENUM

Debug::Enum<E_Resolution>	g_debugResolution		("Graphics.Render.Resolution.Resolution", E_Resolution_Disabled);
Debug::Bool					g_debugChangeResolution	("Graphics.Render.Resolution.ChangeResolution", false);

#if PLATFORM_PS3 && !CFG_GOLD
#define DUMP_OUT_OF_MEMORY 1
#endif

#if DUMP_OUT_OF_MEMORY
bool g_DumpOutOfMemory = false;
#endif

// hack
void CheckForGraphicsMemoryStomping()
{
	CHECK_FOR_GRAPHICS_MEMORY_STOMPING();
}

namespace Framework
{

bool g_bShowFps = false;

#if TRACK_DISPLAY2_RESOURCES
Debug::Bool		g_debugReloadableShaders			("Graphics.Render.ReloadableShaders",				false);
#endif

Debug::Int		g_debugPresentInterval				("Graphics.Render.PresentInterval",					1, Debug::Range(0, (S32)Display::E_PresentInterval_Count - 1), Debug::Persist());
Debug::Bool		g_debugShowFps						("Graphics.Render.ShowFps",							false);
Debug::Bool 	g_debugGrabWindowBufferColour		("Graphics.Render.GrabWindowBufferColour",			false);
Debug::Bool 	g_debugGrabWindowBufferDepth		("Graphics.Render.GrabWindowBufferDepth",			false);
Debug::Bool 	g_debugGrabBackBufferColour			("Graphics.Render.GrabBackBufferColour",			false);
Debug::Bool 	g_debugGrabBackBufferDepth			("Graphics.Render.GrabBackBufferDepth",				false);
Debug::Bool 	g_debugClearBufferColour			("Graphics.Render.ClearBufferColour",				false);
Debug::Colour 	g_debugClearColour					("Graphics.Render.ClearColour",						0.f, 0.f, 0.f, 1.f);

#if !CFG_GOLD
Debug::Float	g_debugShowBufferTop				("Graphics.Render.ShowBufferTop",					-0.9f, 		Debug::Range(-1.0f, 1.0f));
Debug::Float	g_debugShowBufferLeft				("Graphics.Render.ShowBufferLeft",					-0.9f, 		Debug::Range(-1.0f, 1.0f));
Debug::Float	g_debugShowBufferWidth				("Graphics.Render.ShowBufferWidth",					0.4f, 		Debug::Range(0.0f, 1.0f));
Debug::Float	g_debugShowBufferHeight				("Graphics.Render.ShowBufferHeight",				0.4f, 		Debug::Range(0.0f, 1.0f));
Debug::Int		g_debugShowWindowBufferIndex		("Graphics.Render.ShowWindowBufferIndex",			0,			Debug::Range(0, 3));
Debug::Bool 	g_debugShowWindowColourBuffer		("Graphics.Render.ShowWindowColourBuffer",			false);
Debug::Bool 	g_debugShowWindowDepthStencilBuffer	("Graphics.Render.ShowWindowDepthStencilBuffer",	false);
Debug::Bool 	g_debugShowBackColourBuffer			("Graphics.Render.ShowBackColourBuffer",			false);
Debug::Bool 	g_debugShowBackDepthStencilBuffer	("Graphics.Render.ShowBackDepthStencilBuffer",		false);
Debug::Int		g_debugShowFrontBufferIndex			("Graphics.Render.ShowFrontBufferIndex",			0,			Debug::Range(0, 1));
Debug::Bool 	g_debugShowFrontBuffer				("Graphics.Render.ShowFrontBuffer",					false);
Debug::Bool 	g_debugShowCurrentFrontBuffer		("Graphics.Render.ShowCurrentFrontBuffer",			false);
Debug::Bool 	g_debugShowSwappedFrontBuffer		("Graphics.Render.ShowSwappedFrontBuffer",			false);
Debug::Bool 	g_debugClearBeforeRestoreWindow		("Graphics.Render.ClearBeforeRestoreWindow",		false);
Debug::Bool 	g_debugDrawBeforeRestoreWindow		("Graphics.Render.DrawBeforeRestoreWindow",			false);
#endif

#if !CFG_GOLD
Debug::Bool		g_debugAppBaseCommonRender							("Graphics.Render.AppBase.CommonRender",						true);
Debug::Bool		g_debugAppBaseRenderOnce							("Graphics.Render.AppBase.RenderOnce",							true);
Debug::Bool		g_debugAppBasePreRenderSplitscreen					("Graphics.Render.AppBase.PreRenderSplitscreen",				true);
Debug::Bool		g_debugAppBaseRenderSplitscreenWithAA				("Graphics.Render.AppBase.RenderSplitscreenWithAA",				true);
Debug::Bool		g_debugAppBaseRenderSplitscreenWithoutAA			("Graphics.Render.AppBase.RenderSplitscreenWithoutAA",			true);
Debug::Bool		g_debugAppBaseRenderFullscreen						("Graphics.Render.AppBase.RenderFullscreen",					true);
#endif

#if PLATFORM_PS3
Debug::Bool		g_debugAppBaseForceSplitscreenWithAndWithoutAA		("Graphics.Render.AppBase.ForceSplitscreenWithAndWithoutAA",	false);
#endif

Debug::Bool		g_debugShrink2PlayerSplitscreenEnable				("Graphics.Render.Shrink2PlayerSplitscreenEnable",				false);
Debug::Float	g_debugShrink2PlayerSplitscreenBorder				("Graphics.Render.Shrink2PlayerSplitscreenBorder",				0.1f, Debug::Range(0.0f, 1.0f));
Debug::Float	g_debugShrink2PlayerSplitscreenOffset				("Graphics.Render.Shrink2PlayerSplitscreenOffset",				0.05f, Debug::Range(0.0f, 1.0f));

#if PLATFORM_PS3 && !CFG_GOLD
Debug::Bool		g_debugClear2and3PlayerSplitscreen					("Graphics.Render.Clear2and3PlayerSplitscreen",					false);
#endif

#if DUMP_OUT_OF_MEMORY
Debug::Bool		g_debugDumpFreeMemoryEnable							("Graphics.Render.DumpFreeMemoryEnable",						false);
Debug::Bool		g_debugDumpFreeMemoryAllocInfo						("Graphics.Render.DumpFreeMemoryAllocInfo",						false);
#endif

static Debug::Float GameFrameRate ("Camera2.GameFrameRate",  0.f, Debug::Range(0.f, 120.f));
static Debug::Float DebugFrameRate("Camera2.DebugFrameRate", 0.f, Debug::Range(0.f, 120.f));

// [Blur2PC] frame time the PC loop reports for the next update when update and render are
// decoupled (whole 1/60 s ticks); negative = measure real time. See C_Application::IdleLoop.
F32 g_Blur2PCForcedFrameTime = -1.f;

#if !defined(CFG_GOLD) && defined(USE_DISPLAY_WINDOWS) && defined(PLATFORM_PS3)
#define TAKE_SCREENSHOTS
#endif

void C_CaptureService::SetFps(F32 fps)
{
	GameFrameRate = fps;
	DebugFrameRate = fps;
}

F32 C_CaptureService::GetFps()
{
	return Maths::Max((F32)GameFrameRate, (F32)DebugFrameRate);
}

C_CaptureService g_CaptureService;

C_ApplicationBase* C_ApplicationBase::s_pInstance = NULL;

// C_ApplicationBase

C_ApplicationBase::C_ApplicationBase() :
	m_RunState(E_RunState_Run),
	m_FocusPaused(false),
	m_Created(false),
	m_Initialised(false),
	m_LoadingScreenEnabled(false),
	m_iSplitscreenView(-1),
	m_ViewSetup(E_ViewSetup_1),
	m_pDisplayDevice(NULL),
	m_pDisplayManager(NULL),
	m_InputUserCount(0),
	m_InputButtonCount(0),
	m_InputAxisCount(0),
	m_pInputDeviceManager(NULL),
	m_pGameInputManager(NULL),
	m_Resolution(0)
{
	BIZ_ASSERT(s_pInstance == NULL);
	s_pInstance = this;

	m_UpdatePresentRender = false;
	
	m_MainThreadId = MT::GetThreadId();

	#ifdef GAMEAPP_USE_DISPLAY_DEBUGRENDER
	m_pDebugRender = NULL;
	#else
	m_pBasicRender = NULL;
	#endif

	#ifdef GAMEAPP_USE_DISPLAY_FONTRENDER
	m_pFontRender = NULL;
	#endif

	#ifdef GAMEAPP_USE_DISPLAY_BASICRENDER
	m_pBasicRender = NULL;
	#endif

	#ifdef GAMEAPP_USE_DEBUGRENDER
	m_pDebugRenderSys = NULL;
	#endif

	#ifdef GAMEAPP_USE_FONTSYSTEM
	m_pFontSystem = NULL;
	#endif

	for(U32 i=0; i<E_UserInputIndex_Count; i++)
	{
		m_pKeyboardList[i] = NULL;
		m_pMouseList[i] = NULL;
		m_pPadList[i] = NULL;
	}

	// make sure split screen views doesn't get reallocated
	m_SplitscreenViews.Reserve(4);
}

C_ApplicationBase::~C_ApplicationBase()
{
	m_SplitscreenViews.Destroy();

	BIZ_ASSERT(s_pInstance == this);
	s_pInstance = NULL;
}

#ifdef PLATFORM_PC
// [Blur2PC] retail 0x005ae320: the viewport a view setup's window covers on a device of the given size
// (the arithmetic of Display::C_View::SetViewport)
static void SetViewportFromViewSetup(Display::C_Viewport& viewport, const Display::C_ViewSetup& viewSetup, const U32 deviceWidth, const U32 deviceHeight)
{
	Maths::C_Transform2d deviceTransformInv(viewSetup.GetDeviceCoords());
	deviceTransformInv.Invert();

	Maths::C_Transform2d windowTransform(viewSetup.m_WindowCoords);
	
	Maths::C_Transform2d viewportTransform(deviceTransformInv * windowTransform);

	viewport.m_X	= U32(0.5f + F32(deviceWidth) * viewportTransform.Offset().X());
	viewport.m_W	= U32(0.5f + F32(deviceWidth) * viewportTransform.Scale().X());
	viewport.m_Y	= U32(0.5f + F32(deviceHeight) * viewportTransform.Offset().Y());
	viewport.m_H	= U32(0.5f + F32(deviceHeight) * viewportTransform.Scale().Y());
	viewport.m_MinZ	= 0.0f;
	viewport.m_MaxZ	= 1.0f;
}

// [Blur2PC] retail 0x005ae090
static void SetViewportSize(Display::C_Viewport& viewport, const U32 width, const U32 height)
{
	viewport.m_X	= 0;
	viewport.m_Y	= 0;
	viewport.m_W	= width;
	viewport.m_H	= height;
	viewport.m_MinZ	= 0.0f;
	viewport.m_MaxZ	= 1.0f;
}

// [Blur2PC] retail Blur PC's window viewports of a view, set wherever the windows change (0x0084ebf0,
// 0x0084d9f0, 0x0084a240)
void SetWindowViewports(C_View& view, const Display::C_Window* pWindow, const U32 screenWidth, const U32 screenHeight)
{
	SetViewportFromViewSetup(view.m_ScreenViewport, view.m_ViewSetup, screenWidth, screenHeight);
	SetViewportSize(view.m_WindowViewport, pWindow->m_Width, pWindow->m_Height);
	SetViewportSize(view.m_WindowScreenViewport, pWindow->m_ScreenWidth, pWindow->m_ScreenHeight);
}
#endif // PLATFORM_PC

void C_ApplicationBase::SetViewSetup(const E_ViewSetup viewSetup)
{
#ifdef PLATFORM_PC
	// [Blur2PC] retail Blur PC (Blur.exe 1.2 0x0084d9f0)
	if(viewSetup != m_ViewSetup)
	{
		m_ViewSetup = viewSetup;
		g_debugViewSetup = viewSetup;

		m_pDisplayDevice->AcquireThreadOwnership();

		m_pDisplayDevice->DestroyWindows();
		AddDisplayWindows(m_pDisplayDevice->m_DeviceParams);
		m_pDisplayDevice->CreateWindows();

		const Display::C_BackBuffer& backBuffer = m_pDisplayDevice->GetBackBuffer();
		for(S32 viewIndex = 0; viewIndex < m_SplitscreenViews.Size(); ++viewIndex)
		{
			C_View& splitscreenView = m_SplitscreenViews[viewIndex];
			Display::C_Window* pWindow = m_pDisplayDevice->GetWindow(viewIndex);
			splitscreenView.m_View.Setup(splitscreenView.m_ViewSetup, backBuffer.GetWidth(), backBuffer.GetHeight());
			SetWindowViewports(splitscreenView, pWindow, m_pDisplayDevice->m_DeviceParams.m_ScreenWidth, m_pDisplayDevice->m_DeviceParams.m_ScreenHeight);
		}
		
		SendMessageToLayers(E_LayerMessage_Resize);

		m_pDisplayDevice->ReleaseThreadOwnership();
	}
#else
	if(viewSetup != m_ViewSetup)
	{
		m_ViewSetup = viewSetup;
		g_debugViewSetup = viewSetup;

		m_pDisplayDevice->AcquireThreadOwnership();
		m_pDisplayDevice->BlockUntilIdle();

		m_pDisplayDevice->DestroyWindows();
		AddDisplayWindows();
		m_pDisplayDevice->CreateWindows();

		for(S32 viewIndex = 0; viewIndex < m_SplitscreenViews.Size(); ++viewIndex)
		{
			C_View& splitscreenView = m_SplitscreenViews[viewIndex];

			#if defined(USE_DISPLAY_WINDOWS)
			Display::C_Window* pWindow = m_pDisplayDevice->GetWindow(viewIndex);
			const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();
			Maths::HVec2 hack(1024.0f, 768.0f);
			splitscreenView.m_ViewSetup.m_WindowCoords = Maths::C_Rect(pWindow->m_Coords.Min() * hack, pWindow->m_Coords.Max() * hack);
			splitscreenView.m_View.Setup(splitscreenView.m_ViewSetup, backBuffer.GetWidth(), backBuffer.GetHeight());
			#else
			m_pDisplayDevice->SetupView(&splitscreenView.m_View, splitscreenView.m_ViewSetup);
			#endif
		}
		
		SendMessageToLayers(E_LayerMessage_Resize);

		m_pDisplayDevice->ReleaseThreadOwnership();
	}
#endif // PLATFORM_PC
}

void C_ApplicationBase::SetSplitscreenViewCount(const S32 viewCount)
{
	m_SplitscreenViews.Resize(viewCount);
}

void C_ApplicationBase::SetInputUserCount(const S32 count)
{
	m_InputUserCount = count;
}

void C_ApplicationBase::SetInputButtonCount(const S32 count)
{
	m_InputButtonCount = count;
}

void C_ApplicationBase::SetInputAxisCount(const S32 count)
{
	m_InputAxisCount = count;
}

HRESULT C_ApplicationBase::Initialise(const C_ApplicationParams& params)
{
	BIZ_ASSERT(!m_Initialised);

	HRESULT hr = S_OK;

	#ifdef PLATFORM_PC
	// [Blur2PC] retail Blur PC (C_ApplicationBase::Initialise, 0x0084cf50) creates the display manager
	// first: the PC platform initialise reads the graphics settings and matches them to the adapter's modes.
	MEMORY_TRACKER_PUSH("graphics.display.manager");
	hr = CreateDisplay();
	MEMORY_TRACKER_POP("graphics.display.manager");

	if(FAILED(hr))
		return hr;
	#endif

	// Initialise platform specific gubbins

	MEMORY_TRACKER_PUSH("gameapp.platform");
	hr = PlatformInitialise(params);
	MEMORY_TRACKER_POP("gameapp.platform");

	if(FAILED(hr))
		return hr;

	#ifndef PLATFORM_PC
	// Create display

	MEMORY_TRACKER_PUSH("graphics.display.manager");
	hr = CreateDisplay();
	MEMORY_TRACKER_POP("graphics.display.manager");

	if(FAILED(hr))
		return hr;
	#endif

	// Create device

	MEMORY_TRACKER_PUSH("graphics.display.device");
	hr = CreateDevice();
	MEMORY_TRACKER_POP("graphics.display.device");

	if(FAILED(hr))
		return hr;

	// Create input manager

	MEMORY_TRACKER_PUSH("gameapp.input");
	hr = CreateInputManager(params);
	MEMORY_TRACKER_POP("gameapp.input");

	if(FAILED(hr))
		return hr;
//BIZ_ASSERT(!"HELLO!");
	// User-callback initialise

	MEMORY_TRACKER_PUSH("gameapp.user");
	hr = OnInitialise();
	MEMORY_TRACKER_POP("gameapp.user");

	if(FAILED(hr))
		return hr;

	// Create display debug render

	#ifdef GAMEAPP_USE_DISPLAY_DEBUGRENDER
	MEMORY_TRACKER_PUSH("gameapp.debugrender");
	BIZ_ASSERT(!m_pDebugRender);
	m_pDebugRender = new Display::C_DebugRender();
	MEMORY_TRACKER_POP("gameapp.debugrender");
	#else
	m_pBasicRender = new Display::C_BasicRender();
	#endif

	// Create debug render

	#ifdef GAMEAPP_USE_DEBUGRENDER
	MEMORY_TRACKER_PUSH("gameapp.debugrender");
	BIZ_ASSERT(!m_pDebugRenderSys);
	m_pDebugRenderSys = new Render::Debug::C_DebugRenderSys;
	DbUtil::DefaultDebugRenderSys(m_pDebugRenderSys);
	MEMORY_TRACKER_POP("gameapp.debugrender");
	#endif

	// Create font render

	#ifdef GAMEAPP_USE_FONTSYSTEM
	MEMORY_TRACKER_PUSH("gameapp.debugrender");
	BIZ_ASSERT(!m_pFontSystem);
	m_pFontSystem = new Render::Font::C_FontSystem();
	Render::Font::FontSystem(m_pFontSystem);
	MEMORY_TRACKER_POP("gameapp.debugrender");
	#endif

	// Create renderers

	MEMORY_TRACKER_PUSH("graphics.renderers");
	m_pDisplayDevice->CreateRenderers();
	m_pDisplayDevice->CreateRenderersVolatile();
	MEMORY_TRACKER_POP("graphics.renderers");
	
	// Create global graphics buffers

	#if defined(PLATFORM_PC) && !defined(USE_DISPLAY_WINDOWS)	// [Blur2PC] retail Blur PC publishes its display windows' targets instead (T089)
	Render::CreateGlobalColourBuffer();
	Render::CreateGlobalDepthStencilBuffer();
	#endif

	// Init screenshot capture service:	

	MEMORY_TRACKER_PUSH("gameapp.captureservice");
	g_CaptureService.Init();
	MEMORY_TRACKER_POP("gameapp.captureservice");

	Perf::InitialiseNetworkConnection();

	m_Initialised = true;
	
	return hr;
}

void C_ApplicationBase::Shutdown(bool force)
{
	//BIZ_ASSERT(m_Initialised);

	if (force)
		m_RunState = Framework::E_RunState_QuitForced;

	OnShutdown();

	m_LayerManager.ShutDown();

	#if defined(PLATFORM_PC) && !defined(USE_DISPLAY_WINDOWS)	// [Blur2PC] T089
	Render::DestroyGlobalColourBuffer();
	Render::DestroyGlobalDepthStencilBuffer();
	#endif
	
	m_pDisplayDevice->DestroyRenderersVolatile();
	m_pDisplayDevice->DestroyRenderers();

	#ifdef GAMEAPP_USE_FONTSYSTEM
	SAFE_DELETE_PTR(m_pFontSystem);
	Render::Font::FontSystem(NULL);
	#endif

	#ifdef GAMEAPP_USE_DEBUGRENDER
	SAFE_DELETE_PTR(m_pDebugRenderSys);
	DbUtil::DefaultDebugRenderSys(NULL);
	#endif

	#ifdef GAMEAPP_USE_DISPLAY_DEBUGRENDER
	SAFE_DELETE_PTR(m_pDebugRender);
	#else
	SAFE_DELETE_PTR(m_pBasicRender);
	#endif

	//Update resource system to free anything from the evict lists added by the shutdowns
	//otherwise the asserts in Display fire when the device is destroyed
	Resource::C_Manager::GetInstance()->Update();
	
	DestroyInputManager();

	DestroyDevice();

	DestroyDisplay();

	PlatformShutdown();

	m_SplitscreenViews.Destroy();

	m_Initialised = false;
}

#if PLATFORM_PS3
struct C_MemoryStat
{
	C_MemoryStat()
	{
		m_TotalSize		= 0;
		m_AllocCount	= 0;
		m_AllocSize		= 0;
	}

	C_MemoryStat(const U32 totalSize, const U32 allocCount, const U32 allocSize)
	{
		m_TotalSize		= totalSize;
		m_AllocCount	= allocCount;
		m_AllocSize		= allocSize;
	}

	U32 m_TotalSize;
	U32 m_AllocCount;
	U32 m_AllocSize;
};
#endif

void C_ApplicationBase::CommonUpdate()
{
	PERF_EVENT("App.CommonUpdate");

	CHECK_FOR_GRAPHICS_MEMORY_STOMPING();

#if DUMP_OUT_OF_MEMORY
	if(g_debugDumpFreeMemoryEnable || g_DumpOutOfMemory)
	{
		sys_timer_usleep(1000000); // Sleep for a bit so hopefully other threads will stop allocating memory

		// Alloc all the memory that has never been malloc'd

		U32 mmapperSize = 0;
		Btl::Vector<sys_memory_t> mmapperIds;
		Btl::Vector<C_MemoryStat> mmapperStats;
		mmapperStats.PushBack(C_MemoryStat(0, 0, 1024*1024));
		mmapperStats.PushBack(C_MemoryStat(0, 0, 1024*64));

		for(S32 i=0; i<mmapperStats.Size(); i++)
		{
			C_MemoryStat& mmapperStat = mmapperStats[i];

			while(1)
			{
				sys_memory_t mmapperId;
				U32 mmapperFlags = mmapperStat.m_AllocSize == 1024*64 ? SYS_MEMORY_GRANULARITY_64K : SYS_MEMORY_GRANULARITY_1M;
				S32 result = sys_mmapper_allocate_memory(mmapperStat.m_AllocSize, mmapperFlags, &mmapperId);

				if(result == CELL_OK)
				{
					mmapperStat.m_AllocCount++; 
					mmapperSize += mmapperStat.m_AllocSize;
					mmapperStat.m_TotalSize += mmapperStat.m_AllocSize;

					mmapperIds.PushBack(mmapperId);
				}
				else
				{
					break;
				}
			}
		}

		// Alloc all the memory that has previously been malloc'd

		U32 mallocSize = 0;
		Btl::Vector<void*> mallocPtrs;
		Btl::Vector<C_MemoryStat> mallocStats;
		mallocStats.PushBack(C_MemoryStat(0, 0, 1024*1024));
		mallocStats.PushBack(C_MemoryStat(0, 0, 1024*512));
		mallocStats.PushBack(C_MemoryStat(0, 0, 1024*256));
		mallocStats.PushBack(C_MemoryStat(0, 0, 1024*128));
		mallocStats.PushBack(C_MemoryStat(0, 0, 1024*64));
		mallocStats.PushBack(C_MemoryStat(0, 0, 1024*32));
		mallocStats.PushBack(C_MemoryStat(0, 0, 1024*16));

		for(S32 i=0; i<mallocStats.Size(); i++)
		{
			C_MemoryStat& mallocStat = mallocStats[i];

			while(1)
			{
				void* pMemory = malloc(mallocStat.m_AllocSize);	

				if(pMemory)
				{
					mallocStat.m_AllocCount++; 
					mallocSize += mallocStat.m_AllocSize;
					mallocStat.m_TotalSize += mallocStat.m_AllocSize;

					mallocPtrs.PushBack(pMemory);
				}
				else
				{
					break;
				}
			}
		}

		// Free the memory the has never been malloc'd

		for(S32 i=0; i<mmapperIds.Size(); i++)
			sys_mmapper_free_memory(mmapperIds[i]);
		mmapperIds.Clear();

		// Free the memory the has previously been malloc'd

		for(S32 i=0; i<mallocPtrs.Size(); i++)
			free(mallocPtrs[i]);
		mallocPtrs.Clear();

		// Print the memory stats

		U32 totalSize = mmapperSize + mallocSize;

		printf("Available free mmapper memory %.2f mb (%i bytes)\n", (F32)mmapperSize / (1024.0f*1024.0f), mmapperSize);
		
		if(g_debugDumpFreeMemoryAllocInfo)
		{
			for(U32 i=0; i<mmapperStats.Size(); i++)
			{
				const C_MemoryStat& mmapperStat = mmapperStats[i];

				printf("- AllocSize %9i AllocCount %3i TotalSize %9i\n", mmapperStat.m_AllocSize, mmapperStat.m_AllocCount, mmapperStat.m_TotalSize);
			}
		}

		printf("Available free malloc memory %.2f mb (%i bytes)\n", (F32)mallocSize / (1024.0f*1024.0f), mallocSize);

		if(g_debugDumpFreeMemoryAllocInfo)
		{
			for(U32 i=0; i<mallocStats.Size(); i++)
			{
				const C_MemoryStat& mallocStat = mallocStats[i];

				printf("- AllocSize %9i AllocCount %3i TotalSize %9i\n", mallocStat.m_AllocSize, mallocStat.m_AllocCount, mallocStat.m_TotalSize);
			}
		}

		printf("Available free total memory %.2f mb (%i bytes)\n", (F32)totalSize / (1024.0f*1024.0f), totalSize);

		mallocStats.Clear();
		
		mmapperStats.Clear();

		g_debugDumpFreeMemoryEnable = false;

		g_DumpOutOfMemory = false;
	}
#endif

	C_CoreSystems::Instance().Update();

	// create or destroy any pending layers
	m_LayerManager.Update();

	if (g_Blur2PCForcedFrameTime >= 0.f)
		m_Timer.ForceFrameTime(g_Blur2PCForcedFrameTime);	// [Blur2PC] decoupled loop
	else if (GameFrameRate < 0.0001f)
		m_Timer.ForceFrameTime(-1.f); // Auto frameskip
	else
		m_Timer.ForceFrameTime(1.f / GameFrameRate); // Specific frame rate

	// Check for slowing down the game with a debug camera:
	float gameSpeed = 1.0f;
	if (GetSplitscreenViewCount() >= 1)
		gameSpeed = GetSplitscreenView(0).DebugFeed().GameSpeed();
		
	m_Timer.SetTimerScale(gameSpeed);

	if (DebugFrameRate < 0.0001f)
		m_DebugTimer.ForceFrameTime(-1.f); // Auto frameskip
	else
		m_DebugTimer.ForceFrameTime(1.f / DebugFrameRate); // Specific frame rate

	m_Timer.Update();
	m_DebugTimer.Update();

	if(!IsLoadingScreenEnabled())
	{
		UpdateInputManager();
	}

	UpdateLayers();

	for(S32 i=0; i<m_SplitscreenViews.Size(); i++)
	{
		m_SplitscreenViews[i].Update(i, m_DebugTimer.GetFrameTime());
	}

	m_FullscreenView.Update(-1, m_DebugTimer.GetFrameTime());

	#if TRACK_DISPLAY2_RESOURCES
	if(g_debugReloadableShaders)
		Display2::RefreshShaders(true, true);
	#endif

	#if !CFG_GOLD
	DumpGraphicsHeaps();
	#endif
}

void C_ApplicationBase::CommonRender()
{
	GPU_TIMING(1, g_TimingColour_Render, "GPU.Render");

	#if !CFG_GOLD
	if(!g_debugAppBaseCommonRender)
		return;
	#endif

	CHECK_FOR_GRAPHICS_MEMORY_STOMPING();
	
	BIZ_ASSERT(m_pDisplayDevice);

	m_pDisplayDevice->SetPresentInterval((Display::E_PresentInterval)(S32)g_debugPresentInterval);

	{
		DEVICE_MARKER("BeginFrame");
		BeginFrame();
	}

	{
		PERF_EVENT("App.CommonRender");

		BeginSplitscreen();

		ScheduleOnce();

		ScheduleSplitscreen();

		#if !CFG_GOLD
		if(g_debugAppBaseRenderOnce)
		#endif
		{
			CPU_TIMING(3, Maths::RGBA(255, 255, 255, 255), "CPU.RenderOnce");
			GPU_TIMING(3, Maths::RGBA(255, 255, 255, 255), "GPU.RenderOnce");

			DEVICE_MARKER("RenderOnce");
			RenderOnce();
		}

		// [Blur2PC] the snapshot's PC path cleared the back buffer here; retail Blur PC (0x0084cff0) does not,
		// its views are drawn into display windows

		#if PLATFORM_PS3
		if(m_pDisplayDevice->AADisabled() || g_debugAppBaseForceSplitscreenWithAndWithoutAA)
		{
			CPU_TIMING(3, g_TimingColour_Update, "CPU.RenderSplitscreenWithAndWithoutAA");
			GPU_TIMING(3, g_TimingColour_Update, "GPU.RenderSplitscreenWithAndWithoutAA");

			DEVICE_MARKER("RenderSplitscreenWithAndWithoutAA");
			RenderSplitscreenWithAndWithoutAA();
		}
		else
		#endif
		{
			#if !CFG_GOLD
			if(g_debugAppBasePreRenderSplitscreen)
			#endif
			{
				CPU_TIMING(3, Maths::RGBA(255, 255, 255, 255), "CPU.PreRenderSplitscreen");
				GPU_TIMING(3, Maths::RGBA(255, 255, 255, 255), "GPU.PreRenderSplitscreen");

				DEVICE_MARKER("PreRenderSplitscreen");
				PreRenderSplitscreen();
			}

			#if !CFG_GOLD
			if(g_debugAppBaseRenderSplitscreenWithAA)
			#endif
			{
				CPU_TIMING(3, g_TimingColour_RenderSplitscreenWithAA, "CPU.RenderSplitscreenWithAA");
				GPU_TIMING(3, g_TimingColour_RenderSplitscreenWithAA, "GPU.RenderSplitscreenWithAA");

				DEVICE_MARKER("RenderSplitscreenWithAA");
				RenderSplitscreenWithAA();
			}

			#if !CFG_GOLD
			if(g_debugAppBaseRenderSplitscreenWithoutAA)
			#endif
			{
				CPU_TIMING(3, g_TimingColour_RenderSplitscreenWithoutAA, "CPU.RenderSplitscreenWithoutAA");
				GPU_TIMING(3, g_TimingColour_RenderSplitscreenWithoutAA, "GPU.RenderSplitscreenWithoutAA");

				DEVICE_MARKER("RenderSplitscreenWithoutAA");
				RenderSplitscreenWithoutAA();
			}
		}

		m_pDisplayDevice->InvalidateState();
		m_pDisplayDevice->DefaultStates();

		#if !CFG_GOLD
		if(g_debugAppBaseRenderFullscreen)
		#endif
		{
			CPU_TIMING(3, g_TimingColour_RenderFullscreen, "CPU.RenderFullscreen");
			GPU_TIMING(3, g_TimingColour_RenderFullscreen, "GPU.RenderFullscreen");

			DEVICE_MARKER("RenderFullscreen");
			RenderFullscreen();
		}
	}

	#if PLATFORM_PS3
	// wait until all deferred command buffers have been flushed
	while (!Display::g_DeferredCommandBufferManager.Flush())
	{
		Scheduler::Flush(Scheduler::JQ_GROUP_NONE, true, 30);
		sys_timer_usleep(30);
	}
	#endif

	{
		DEVICE_MARKER("EndFrame");
		EndFrame();
	}
}

bool C_ApplicationBase::BeginRender()
{
	PERF_EVENT("App.BeginRender");

	BIZ_ASSERT(m_pDisplayDevice);

	if(m_UpdatePresentRender)
		Present();

	DEVICE_MARKER("BeginRender");

	#ifdef PLATFORM_PC
	if(!CanRender())
		return false;

	// [Blur2PC] retail BeginRender (0x0084dbe0): apply graphics settings changed through the debug variables
	// or the window, then the view setup check the console path below also does
	UpdateGraphicsSettings();

	if(m_ViewSetup != g_debugViewSetup)
	{
		SetViewSetup(g_debugViewSetup);
	}
	#else
	{
		CPU_TIMING(5, Maths::RGBA(255, 255, 255, 255), "CPU.DisplayResize");
		GPU_TIMING(5, Maths::RGBA(255, 255, 255, 255), "GPU.DisplayResize");
		
		ResizeDisplayWindows();
	}
	{
		CPU_TIMING(5, Maths::RGBA(255, 255, 255, 255), "CPU.ViewResize");
		GPU_TIMING(5, Maths::RGBA(255, 255, 255, 255), "GPU.ViewResize");
		
		if(m_ViewSetup != g_debugViewSetup)
		{
			SetViewSetup(g_debugViewSetup);
		}
	}
	#endif // PLATFORM_PC

	{
		CPU_TIMING(4, Maths::RGBA(255, 255, 255, 255), "CPU.BeginScene");
		GPU_TIMING(4, Maths::RGBA(255, 255, 255, 255), "GPU.BeginScene");

		if(m_pDisplayDevice->BeginScene())
		{
			PerformanceLogging::BeginGPU(m_pDisplayDevice);

			return true;
		}
		else
		{
			return false;
		}
	}
}

void C_ApplicationBase::EndRender()
{
	{
		PERF_EVENT("App.EndRender");

		BIZ_ASSERT(m_pDisplayDevice);

		#ifdef GAMEAPP_USE_DISPLAY_DEBUGRENDER
		{
			CPU_TIMING(4, Maths::RGBA(255, 255, 255, 255), "CPU.DisplayDebug");
			GPU_TIMING(4, Maths::RGBA(255, 255, 255, 255), "GPU.DisplayDebug");

			DEVICE_MARKER("DisplayDebug");
			BIZ_ASSERT(m_pDebugRender);
			m_pDebugRender->Draw();
		}
		#endif
		
		{
			CPU_TIMING(4, Maths::RGBA(255, 255, 255, 255), "CPU.EndScene");
			GPU_TIMING(4, Maths::RGBA(255, 255, 255, 255), "GPU.EndScene");

			DEVICE_MARKER("EndScene");
			m_pDisplayDevice->EndScene();
		}
	}

	#ifdef TAKE_SCREENSHOTS
	if(g_debugGrabBackBufferColour)
	{
		m_pDisplayDevice->BlockUntilIdle();
		Util::C_String fileName = "";
		fileName.Printf("BackBufferColour.tga");
		Display::TakeScreenshot(fileName.Chars(), m_pDisplayDevice->GetBackBuffer().GetColourSurface());
		g_debugGrabBackBufferColour = false;
	}
	if(g_debugGrabBackBufferDepth)
	{
		m_pDisplayDevice->BlockUntilIdle();
		Util::C_String fileName = "";
		fileName.Printf("BackBufferDepth.tga");
		Display::TakeScreenshot(fileName.Chars(), m_pDisplayDevice->GetBackBuffer().GetDepthStencilSurface());
		g_debugGrabBackBufferDepth = false;
	}
	#endif // TAKE_SCREENSHOTS

	if(!m_UpdatePresentRender)
		Present();

	#ifdef GAMEAPP_USE_DEBUGRENDER
		Render::Debug::PrimitiveEndFrame();
	#endif

	#ifdef USER_ECLAY
	Time::C_Time frameTime;

	{
		static Time::C_Time previousTime = Time::GetPerformanceCounter();
		Time::C_Time time = Time::GetPerformanceCounter();
		frameTime = time - previousTime;
		previousTime = time;
		LOGDEBUG(g_log, "%lld", frameTime);
	}
	#endif

	//if (frameTime > 64000)
	//	Perf::CapturePerfData();

//	Perf::ProfilerCommon::Instance().EndFrame();

	DEVICE_MARKER("None");
}

void C_ApplicationBase::Present()
{
	PrePresent();

	BIZ_ASSERT(m_pDisplayDevice);

	CPU_TIMING(2, g_TimingColour_Present, "CPU.Present");
	GPU_TIMING(2, g_TimingColour_Present, "GPU.Present");

	PerformanceLogging::PrePresent(m_pDisplayDevice, m_UpdatePresentRender);

	DEVICE_MARKER("Present");
	m_pDisplayDevice->Present();

	PerformanceLogging::PostPresent(m_pDisplayDevice);
}

HRESULT C_ApplicationBase::PlatformInitialise(const C_ApplicationParams& params)
{
	return S_OK;
}

void C_ApplicationBase::PlatformShutdown()
{
}

HRESULT C_ApplicationBase::OnInitialise()
{
	return S_OK;
}

void C_ApplicationBase::OnShutdown()
{
}

void C_ApplicationBase::OnUpdate()
{
}

void C_ApplicationBase::OnRender()
{
}

void C_ApplicationBase::OnLoadingScreenUpdate()
{
}

void C_ApplicationBase::OnLoadingScreenRender()
{
}

HRESULT C_ApplicationBase::CreateDisplay()
{
	BIZ_ASSERT(!m_pDisplayManager);
	m_pDisplayManager = new Display::C_Manager();

	HRESULT hr = m_pDisplayManager->Initialise();
	if(FAILED(hr))
	{
		return hr;
	}

	#ifdef PLATFORM_PC
	// [Blur2PC] retail CreateDisplay (0x0084d140) fills the resolution list here, with no practical size cap
	SetupDisplayResolutions(10000, 10000, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, Display::E_Format_Tiled);
	#endif

	return S_OK;
}

void C_ApplicationBase::DestroyDisplay()
{
	if(m_pDisplayManager)
	{
		if(m_pDisplayManager->IsInitialised())
		{
			m_pDisplayManager->Shutdown();
		}

		SAFE_DELETE_PTR(m_pDisplayManager);
	}
}

HRESULT C_ApplicationBase::CreateDevice()
{
	BIZ_ASSERT(!m_pDisplayDevice);
	m_pDisplayDevice = new Display::C_Device();

	SetupDeviceParams();

	#ifdef PLATFORM_PC
	AddDisplayWindows(m_DisplayDeviceParams);	// [Blur2PC] retail Blur PC
	#else
	AddDisplayWindows();
	#endif

	#ifndef PLATFORM_PC
	SetupDisplayResolutions(
		m_DisplayDeviceParams.m_WindowBufferWidth,
		m_DisplayDeviceParams.m_WindowBufferHeight,
		m_DisplayDeviceParams.m_WindowBufferColourFormat,
		m_DisplayDeviceParams.m_WindowBufferMultisample,
		Display::E_Format_Tiled);
	#endif

	HRESULT hr = m_pDisplayManager->CreateDevice(m_pDisplayDevice, m_DisplayDeviceParams);
	if(FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}

void C_ApplicationBase::DestroyDevice()
{
	if(m_pDisplayManager && m_pDisplayDevice)
	{
		m_pDisplayManager->DestroyDevice(m_pDisplayDevice);
		
		m_pDisplayDevice = NULL;
	}
}
void C_ApplicationBase::SetupDeviceMemoryParams()
{
#if PLATFORM_PS3
	CellGcmConfig gcmConfig;
	cellGcmGetConfiguration(&gcmConfig);
	m_DisplayDeviceParams.m_CmdBufMemorySize		= 1024*256;					// How much main memory the device can use for command buffers
	m_DisplayDeviceParams.m_VideoMemorySize			= gcmConfig.localSize;		// How much local memory the device can use (this is the video memory)
	m_DisplayDeviceParams.m_SystemMemorySize		= 1024*1024*45;				// How much main memory the device can use (this is the system memory)
	m_DisplayDeviceParams.m_MainDeviceMemorySize	= 1024*1024*4;				// How much of main memory the device can use for internal render targets
	m_DisplayDeviceParams.m_LocalDeviceMemorySize	= 1024*1024*30;				// How much of local memory the device can use for internal render targets
	m_DisplayDeviceParams.m_UniqueBackBuffer		= true;
#endif
}

void C_ApplicationBase::SetupDeviceParams()
{
	SetupDeviceMemoryParams();

	Display::C_VideoMode videoMode;
	if(!m_pDisplayManager->GetVideoMode(&videoMode))
		return;

	#if defined(PLATFORM_PS3)
	m_DisplayDeviceParams.m_HiDef = videoMode.m_HiDef;
	m_DisplayDeviceParams.m_WideScreen = videoMode.m_WideScreen;
	#endif

	#if defined(PLATFORM_PS3)
	Display::E_Multisample windowMultisample = Display::E_Multisample_2;
	#else
	Display::E_Multisample windowMultisample = Display::E_Multisample_None;
	#endif

	#if defined(PLATFORM_PS3)
	if(videoMode.m_Height == 480 || videoMode.m_Height == 576 || videoMode.m_Height == 720)
	{
		m_DisplayDeviceParams.Setup(
			videoMode.m_Width, videoMode.m_Height,
			1280, 720, Display::E_Multisample_None,
			1280, 720, windowMultisample);

		m_DisplayDeviceParams.m_UniqueBackBuffer = videoMode.m_Height == 720 ? false : true;
	}
	else
	{
		m_DisplayDeviceParams.Setup(
			videoMode.m_Width, videoMode.m_Height,
			videoMode.m_Width, videoMode.m_Height, Display::E_Multisample_None,
			videoMode.m_Width, videoMode.m_Height, windowMultisample);

		m_DisplayDeviceParams.m_UniqueBackBuffer = true;
	}
	#else
	{
		m_DisplayDeviceParams.Setup(
			1280, 720,
			1280, 720, Display::E_Multisample_None,
			1280, 720, windowMultisample);
	}
	#endif
}

HRESULT C_ApplicationBase::CreateInputManager(const C_ApplicationParams& params)
{
	MEMORY_TRACKER_PUSH("gameapp.input");

	Input::C_DeviceManagerCreateParam inputParams;
	
	#ifdef PLATFORM_PC
	inputParams.m_hWnd = m_DisplayDeviceParams.m_hWnd;
	#endif
	
	
	BIZ_ASSERT(!m_pInputDeviceManager);
	m_pInputDeviceManager = Input::CreateDeviceManager(inputParams);
	
	for(U32 i=0; i<E_UserInputIndex_Count; i++)
	{
		#if defined(PLATFORM_PS3)
		m_pInputDeviceManager->CreateDevice(i, Input::E_Device_PadPS3, &m_pPadList[i]);
		#elif defined(PLATFORM_PC)
		m_pInputDeviceManager->CreateDevice(i, Input::E_Device_Pad360, &m_pPadList[i]);
		#endif

		#if defined(PLATFORM_PC)
		m_pInputDeviceManager->CreateDevice(i, Input::E_Device_Mouse, &m_pMouseList[i]);
		m_pInputDeviceManager->CreateDevice(i, Input::E_Device_Keyboard, &m_pKeyboardList[i]);
		#endif
	}

	m_InputUserCount	= params.m_InputUserCount;
	m_InputButtonCount	= params.m_InputButtonCount;
	m_InputAxisCount	= params.m_InputAxisCount;

	BIZ_ASSERT(!m_pGameInputManager);
	m_pGameInputManager = new Input::C_GameInputManager(m_InputUserCount, m_InputButtonCount, m_InputAxisCount);

	SetupUserControls();
	MEMORY_TRACKER_POP("gameapp.input");
	
	return S_OK;
}

void C_ApplicationBase::DestroyInputManager()
{
	SAFE_DELETE_PTR(m_pGameInputManager);

	for(U32 i=0; i<E_UserInputIndex_Count; i++)
	{
		SAFE_DELETE_PTR(m_pKeyboardList[i]);
		SAFE_DELETE_PTR(m_pMouseList[i]);
		SAFE_DELETE_PTR(m_pPadList[i]);
	}

	SAFE_DELETE_PTR(m_pInputDeviceManager);
}

void C_ApplicationBase::UpdateInputManager()
{
	Input::C_DeviceManager::GetInstance()->Update();
	Input::C_GameInputManager::GetInstance()->Update();
}

void C_ApplicationBase::SetupUserControls()
{
	BIZ_ASSERT(m_pGameInputManager);
	m_pGameInputManager->Resize(m_InputUserCount, m_InputButtonCount, m_InputAxisCount);
	
	for(S32 i=0; i<m_InputUserCount; i++)
	{
		SetupApplicationUserControls(Input::GetGameInput(i), m_pKeyboardList[i], m_pMouseList[i], m_pPadList[i]);
	}
}

void C_ApplicationBase::InsertLayer(C_Layer* pLayer)
{
	m_LayerManager.InsertLayer( pLayer );
}

void C_ApplicationBase::DestroyLayer(C_Layer* pLayer)
{
	m_LayerManager.DestroyLayer( pLayer );
}


void C_ApplicationBase::InsertLayerAsync(C_Layer* pLayer)
{
	m_LayerManager.InsertLayerAsync( pLayer );
}

void C_ApplicationBase::DestroyLayerAsync(C_Layer* pLayer)
{
	m_LayerManager.DestroyLayerAsync( pLayer );
}

void C_ApplicationBase::EnableLoadingScreen()
{
	m_LoadingScreenEnabled = true;
}

void C_ApplicationBase::DisableLoadingScreen()
{
	m_LoadingScreenEnabled = false;
}

bool C_ApplicationBase::IsLoadingScreenEnabled() const
{ 
	return m_LoadingScreenEnabled; 
}

void C_ApplicationBase::PrePresent()
{
}

void C_ApplicationBase::ProcessLoadingScreen()
{
}

void C_ApplicationBase::Pause( const S32 pauseId )
{
	m_LayerManager.Pause( pauseId );
}

void C_ApplicationBase::Resume( const S32 pauseId )
{
	m_LayerManager.Resume( pauseId );
}

bool C_ApplicationBase::IsPaused() const
{
	return m_LayerManager.IsPaused();
}

bool C_ApplicationBase::IsPaused( const S32 pauseId ) const
{
	return m_LayerManager.IsPaused( pauseId );
}

void C_ApplicationBase::ExitChecks()
{
}

void C_ApplicationBase::UpdateLayers()
{
	SendMessageToLayers(E_LayerMessage_Update);
	SendMessageToLayers(E_LayerMessage_GlobalUpdate);
}

void C_ApplicationBase::BeginFrame()
{
	PERF_EVENT("App.BeginFrame");

	SetupViews();

	SendMessageToLayers(E_LayerMessage_BeginFrame);
}

void C_ApplicationBase::SetupViews()
{
	// setup all the views
	for(S32 viewIndex = 0; viewIndex < m_SplitscreenViews.Size(); ++viewIndex)
	{
		C_View& splitscreenView = m_SplitscreenViews[viewIndex];
		const Cam2::I_Feed& feed = splitscreenView.DebugFeed();

		// Set start of motion blur:
		feed.ToViewSetup(splitscreenView.m_ViewSetup.m_ViewSetup3D, false);

		#if defined(USE_DISPLAY_WINDOWS)
		const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();

		#ifndef PLATFORM_PC	// [Blur2PC] retail Blur PC (0x0084dd40) sets the window coordinates only when adding the window
		Display::C_Window* pWindow = m_pDisplayDevice->GetWindow(viewIndex);
		Maths::HVec2 hack(1024.0f, 768.0f);
		splitscreenView.m_ViewSetup.m_WindowCoords = Maths::C_Rect(pWindow->m_Coords.Min() * hack, pWindow->m_Coords.Max() * hack);
		#endif
		splitscreenView.m_View.Setup(splitscreenView.m_ViewSetup, backBuffer.GetWidth(), backBuffer.GetHeight());
		#else
		m_pDisplayDevice->SetupView(&splitscreenView.m_View, splitscreenView.m_ViewSetup);
		#endif // USE_DISPLAY_WINDOWS

		// Copy matrices for start of motion blur to the 'previous' matrices:
		splitscreenView.m_View.SetPreviousFromCurrent();
		
		// Now setup the end of the motion blur:
		feed.ToViewSetup(splitscreenView.m_ViewSetup.m_ViewSetup3D, true);

		#if defined(USE_DISPLAY_WINDOWS)
		splitscreenView.m_View.Setup(splitscreenView.m_ViewSetup, backBuffer.GetWidth(), backBuffer.GetHeight());
		#else
		m_pDisplayDevice->SetupView(&splitscreenView.m_View, splitscreenView.m_ViewSetup);
		#endif // USE_DISPLAY_WINDOWS
	}
}

void C_ApplicationBase::BeginSplitscreen()
{
	PERF_EVENT("App.BeginSplitscreen");

	for(S32 viewIndex = 0; viewIndex < m_SplitscreenViews.Size(); ++viewIndex)
	{
		m_iSplitscreenView = viewIndex;
		SendMessageToLayers(E_LayerMessage_BeginSplitscreen);
	}
	m_iSplitscreenView = -1;
}

void C_ApplicationBase::ScheduleOnce()
{
	PERF_EVENT("App.ScheduleOnce");

	SendMessageToLayers(E_LayerMessage_ScheduleOnce);
}

void C_ApplicationBase::ScheduleSplitscreen()
{
	PERF_EVENT("App.ScheduleSplitscreen");

	for(S32 viewIndex = 0; viewIndex < m_SplitscreenViews.Size(); ++viewIndex)
	{
		PERF_EVENT("Splitscreen");

		C_View& splitscreenView = m_SplitscreenViews[viewIndex];
		m_iSplitscreenView = viewIndex;

		#ifdef USE_DISPLAY_WINDOWS
		Display::C_Window* pWindow = m_pDisplayDevice->GetWindow(viewIndex);
		Render::SetGlobalColourBuffer(pWindow->GetColourTexture());
		Render::SetGlobalDepthStencilBuffer(pWindow->GetDepthStencilTexture());
		static int dx11ScheduleDepthLogCount = 0;
		if (dx11ScheduleDepthLogCount++ < 12)
			Blur2PCAutomation::LogF("DX11 Schedule depth view=%d colour=%p depth=%p", viewIndex, pWindow->GetColourTexture(), pWindow->GetDepthStencilTexture());
		#ifdef PLATFORM_PC
		Render::SetGlobalStencilBuffer(pWindow->GetMaskTexture());	// [Blur2PC] retail Blur PC (0x0084de00, 0x006a1970)
		#endif
		#endif // USE_DISPLAY_WINDOWS
		
		SendMessageToLayers(E_LayerMessage_ScheduleSplitscreen);
	}
	m_iSplitscreenView = -1;
}

void C_ApplicationBase::RenderOnce()
{
	PERF_EVENT("App.RenderOnce");

	SendMessageToLayers(E_LayerMessage_RenderOnce);
}

void C_ApplicationBase::RenderSplitscreenWithAndWithoutAA()
{
#if PLATFORM_PS3
	PERF_EVENT("App.RenderSplitscreenWithAndWithoutAA");

	#if !CFG_GOLD
	if(g_debugClear2and3PlayerSplitscreen && (m_SplitscreenViews.Size() == 2 || m_SplitscreenViews.Size() == 3)) // Need to clear the colour buffer as we don't render to all the screen
	{
		m_pDisplayDevice->SetColourDepthStencilSurface();
		m_pDisplayDevice->Clear(Display::E_Clear_Colour);
	}
	#endif
	
	for(S32 viewIndex=0; viewIndex<m_SplitscreenViews.Size(); ++viewIndex)
	{
		PERF_EVENT("Splitscreen");

		// PreRenderSplitscreen
		{
			PERF_EVENT("NoAA.PreRenderSplitscreen");

			CPU_TIMING(3, Maths::RGBA(255, 255, 255, 255), "CPU.NoAA.PreRenderSplitscreen");
			GPU_TIMING(3, Maths::RGBA(255, 255, 255, 255), "GPU.NoAA.PreRenderSplitscreen");

			DEVICE_MARKER("NoAA.PreRenderSplitscreen");
			PreRenderSplitscreenView(viewIndex);
		}

		// RenderSplitscreenWithAA
		{
			PERF_EVENT("NoAA.RenderSplitscreenViewWithAA");

			CPU_TIMING(3, g_TimingColour_RenderSplitscreenWithAA, "CPU.NoAA.RenderSplitscreenViewWithAA");
			GPU_TIMING(3, g_TimingColour_RenderSplitscreenWithAA, "GPU.NoAA.RenderSplitscreenViewWithAA");

			DEVICE_MARKER("NoAA.RenderSplitscreenViewWithoutAA");
			RenderSplitscreenViewWithAA(viewIndex);
		}

		m_pDisplayDevice->SetColourDepthStencilSurface();

		// RenderSplitscreenWithoutAA
		{
			PERF_EVENT("NoAA.RenderSplitscreenViewWithoutAA");

			CPU_TIMING(3, g_TimingColour_RenderSplitscreenWithoutAA, "CPU.NoAA.RenderSplitscreenViewWithoutAA");
			GPU_TIMING(3, g_TimingColour_RenderSplitscreenWithoutAA, "GPU.NoAA.RenderSplitscreenViewWithoutAA");

			DEVICE_MARKER("NoAA.RenderSplitscreenViewWithoutAA");
			RenderSplitscreenViewWithoutAA(viewIndex);
		}
	}

	m_iSplitscreenView = -1;

	m_pDisplayDevice->SetColourDepthStencilSurface();
#endif
}

void C_ApplicationBase::PreRenderSplitscreenView(const S32 viewIndex)
{
	C_View& splitscreenView = m_SplitscreenViews[viewIndex];
	m_iSplitscreenView = viewIndex;

	#ifdef USE_DISPLAY_WINDOWS
	Display::C_Window* pWindow = m_pDisplayDevice->GetWindow(viewIndex);
	Render::SetGlobalColourBuffer(pWindow->GetColourTexture());
	Render::SetGlobalDepthStencilBuffer(pWindow->GetDepthStencilTexture());
	#ifdef PLATFORM_PC
	Render::SetGlobalStencilBuffer(pWindow->GetMaskTexture());	// [Blur2PC] retail Blur PC (0x0084e070, 0x006a1970)
	#endif
	#endif // USE_DISPLAY_WINDOWS

	SendMessageToLayers(E_LayerMessage_PreRenderSplitscreen);
}

#ifdef PLATFORM_PC
// [Blur2PC] retail Blur PC (Blur.exe 1.2 0x0084df00): the view is drawn into its display window, whose colour
// the device resolves and whose stencil it turns into the mask texture between the two layer passes
void C_ApplicationBase::RenderSplitscreenViewWithAA(const S32 viewIndex)
{
	C_View& splitscreenView = m_SplitscreenViews[viewIndex];
	m_iSplitscreenView = viewIndex;

	m_pDisplayDevice->SetWindow(viewIndex, true);

	Display::C_Window* pWindow = m_pDisplayDevice->GetWindow(viewIndex);
	Render::SetGlobalColourBuffer(pWindow->GetColourTexture());
	Render::SetGlobalDepthStencilBuffer(pWindow->GetDepthStencilTexture());
	Render::SetGlobalStencilBuffer(pWindow->GetMaskTexture());

	m_pDisplayDevice->SetView(&splitscreenView.m_View);
	m_pDisplayDevice->SetViewport(splitscreenView.m_WindowViewport);

	if(!g_debugClearBufferColour)
		m_pDisplayDevice->Clear(Display::E_Clear_Depth | Display::E_Clear_Stencil);
	else
		m_pDisplayDevice->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth | Display::E_Clear_Stencil, Maths::C_RGBA_U8(g_debugClearColour.Value().r, g_debugClearColour.Value().g, g_debugClearColour.Value().b, g_debugClearColour.Value().a));

	SendMessageToLayers(E_LayerMessage_RenderSplitscreenWithAA);

	m_pDisplayDevice->RenderWindowMask();

	SendMessageToLayers(E_LayerMessage_RenderSplitscreenWithoutAA);

	m_pDisplayDevice->EndWindow();

	m_pDisplayDevice->SetView(NULL);
}
#else
void C_ApplicationBase::RenderSplitscreenViewWithAA(const S32 viewIndex)
{
	C_View& splitscreenView = m_SplitscreenViews[viewIndex];
	m_iSplitscreenView = viewIndex;

	#if defined(USE_DISPLAY_WINDOWS)
	m_pDisplayDevice->SetWindow(viewIndex, true);
	m_pDisplayDevice->BeginWindow();
	
	Display::C_Window* pWindow = m_pDisplayDevice->GetWindow(viewIndex);
	Render::SetGlobalColourBuffer(pWindow->GetColourTexture());
	Render::SetGlobalDepthStencilBuffer(pWindow->GetDepthStencilTexture());
	#endif // USE_DISPLAY_WINDOWS

	#if defined(PLATFORM_PC)
	// [Blur2PC] PC has no display windows: PreRenderSplitscreen passes (planar reflection, downsamples)
	// leave their own target bound, and the main view then drew into it. Re-bind the back buffer
	// (cleared in CommonRender) so the scene lands where the StretchRect below and the post-process read it.
	m_pDisplayDevice->SetColourDepthStencilSurface();
	#endif

	m_pDisplayDevice->SetView(&splitscreenView.m_View);

	// set a viewport at origin
	Display::C_Viewport viewportOrigin = m_pDisplayDevice->GetViewport();
	viewportOrigin.m_X = 0;	viewportOrigin.m_Y = 0;

	#ifdef PLATFORM_PS3
	// on PS3 we check to see if our rendertarget is smaller and clamp the viewport if so
	// D3D clamps to rendertarget size automatically
	const Display::C_RenderTarget& renderTarget = m_pDisplayDevice->GetRenderTarget();
	viewportOrigin.m_W = viewportOrigin.m_W > renderTarget.GetWidth() ? renderTarget.GetWidth() : viewportOrigin.m_W;
	viewportOrigin.m_H = viewportOrigin.m_H > renderTarget.GetHeight() ? renderTarget.GetHeight() : viewportOrigin.m_H;
	#endif

	m_pDisplayDevice->SetViewport(viewportOrigin);

	#if PLATFORM_PS3
	// [Jan] This hack is required on the PS3 to clear the stencil. Without this hack the stencil won't get cleared.
	Display::C_StencilState state;
	state.m_StencilEnable = Display::E_Bool_True;
	state.m_StencilFunc = Display::E_CmpFunc_Equal;
	state.m_StencilDepthPass = Display::E_StencilOp_Keep;
	state.m_StencilRef = state.m_StencilWriteMask = state.m_StencilMask = 0xff;
	m_pDisplayDevice->SetStencilState(state);
	#endif // PLATFORM_PS3

	#if defined(USE_DISPLAY_WINDOWS)
	{
		#if PLATFORM_PS3
		if(0)//m_pDisplayDevice->AADisabled() || g_debugAppBaseForceSplitscreenWithAndWithoutAA) // Don't clear depth stencil buffer as reusing zcull information from prepass
		{
			if(g_debugClearBufferColour)
				m_pDisplayDevice->Clear(Display::E_Clear_Colour);
		}
		else
		#endif
		{
			if(!g_debugClearBufferColour)
				m_pDisplayDevice->Clear(Display::E_Clear_Depth | Display::E_Clear_Stencil);
			else
				m_pDisplayDevice->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth | Display::E_Clear_Stencil, Maths::C_RGBA_U8(g_debugClearColour.Value().r, g_debugClearColour.Value().g, g_debugClearColour.Value().b, g_debugClearColour.Value().a));
		}
	}
	#endif // USE_DISPLAY_WINDOWS

	SendMessageToLayers(E_LayerMessage_RenderSplitscreenWithAA);
	#if defined(PLATFORM_PC)
	SendMessageToLayers(E_LayerMessage_RenderSplitscreenWithoutAA);
	#endif // PLATFORM_PC
	

	#ifdef PLATFORM_PC
	IDirect3DSurface9 * pSourceSurface = Display::Device()->GetRenderTarget().GetColourSurface()->m_pd3dSurface;
	IDirect3DSurface9 * pDestSurface = Render::g_ColourBuffer.GetSurface().m_pd3dSurface;
	Display::Device()->GetD3DDevice()->StretchRect(pSourceSurface, 0, pDestSurface, 0, D3DTEXF_POINT);
	#endif // PLATFORM_PC

	#if defined(USE_DISPLAY_WINDOWS)
	m_pDisplayDevice->EndWindow();
	#endif // USE_DISPLAY_WINDOWS

	#ifdef TAKE_SCREENSHOTS
	pWindow = m_pDisplayDevice->GetWindow(viewIndex);
	if(g_debugGrabWindowBufferColour)
	{
		m_pDisplayDevice->BlockUntilIdle();
		Util::C_String fileName = "";
		fileName.Printf("WindowBufferColour%i.tga", viewIndex);
		Display::TakeScreenshot(fileName.Chars(), pWindow->GetColourSurface());
	}
	if(g_debugGrabWindowBufferDepth)
	{
		m_pDisplayDevice->BlockUntilIdle();
		Util::C_String fileName = "";
		fileName.Printf("WindowBufferDepth%i.tga", viewIndex);
		Display::TakeScreenshot(fileName.Chars(), pWindow->GetDepthStencilSurface());
	}
	#endif // TAKE_SCREENSHOTS
	
	m_pDisplayDevice->SetView(NULL);
}
#endif // PLATFORM_PC

void C_ApplicationBase::PreRenderSplitscreen()
{
	PERF_EVENT("App.PreRenderSplitscreen");

	for(S32 viewIndex = 0; viewIndex < m_SplitscreenViews.Size(); ++viewIndex)
	{
		PERF_EVENT("Splitscreen");

		PreRenderSplitscreenView(viewIndex);
	}

	m_iSplitscreenView = -1;
}

void C_ApplicationBase::RenderSplitscreenWithAA()
{
	PERF_EVENT("App.RenderSplitscreenWithAA");

	for(S32 viewIndex=0; viewIndex<m_SplitscreenViews.Size(); ++viewIndex)
	{
		PERF_EVENT("Splitscreen");

		RenderSplitscreenViewWithAA(viewIndex);
	}

	m_iSplitscreenView = -1;

	#if defined(USE_DISPLAY_WINDOWS)
	m_pDisplayDevice->SetColourDepthStencilSurface();
	#endif // USE_DISPLAY_WINDOWS

	#ifdef TAKE_SCREENSHOTS
	if(g_debugGrabWindowBufferColour)
		g_debugGrabWindowBufferColour = false;
	if(g_debugGrabWindowBufferDepth)
		g_debugGrabWindowBufferDepth = false;
	#endif // TAKE_SCREENSHOTS
}

#ifdef PLATFORM_PC
// [Blur2PC] retail Blur PC (Blur.exe 1.2 0x0084e180): copy the display window to its place on the bound
// colour surface (the back buffer)
void C_ApplicationBase::RenderSplitscreenViewWithoutAA(const S32 viewIndex)
{
	C_View& splitscreenView = m_SplitscreenViews[viewIndex];
	m_iSplitscreenView = viewIndex;

	m_pDisplayDevice->SetView(&splitscreenView.m_View);
	m_pDisplayDevice->SetViewport(splitscreenView.m_ScreenViewport);

	Display::C_Window* pWindow = m_pDisplayDevice->GetWindow(viewIndex);
	Render::SetGlobalColourBuffer(pWindow->GetColourTexture());
	Render::SetGlobalDepthStencilBuffer(pWindow->GetDepthStencilTexture());
	Render::SetGlobalStencilBuffer(pWindow->GetMaskTexture());

	BeginRenderSplitscreenWithoutAA();

	m_pDisplayDevice->RestoreWindow(viewIndex, Display::E_ColourWriteEnable_All);

	EndRenderSplitscreenWithoutAA();

	m_pDisplayDevice->SetView(NULL);
}
#else
void C_ApplicationBase::RenderSplitscreenViewWithoutAA(const S32 viewIndex)
{
	C_View& splitscreenView = m_SplitscreenViews[viewIndex];
	m_iSplitscreenView = viewIndex;

	m_pDisplayDevice->SetView(&splitscreenView.m_View);

	#if defined(USE_DISPLAY_WINDOWS)
	Display::C_Window* pWindow = m_pDisplayDevice->GetWindow(viewIndex);
	Render::SetGlobalColourBuffer(pWindow->GetColourTexture());
	Render::SetGlobalDepthStencilBuffer(pWindow->GetDepthStencilTexture());
	#endif // USE_DISPLAY_WINDOWS

	BeginRenderSplitscreenWithoutAA();

	#if !CFG_GOLD
	if(g_debugClearBeforeRestoreWindow)
		Display::Device()->Clear(Display::E_Clear_Colour|Display::E_Clear_Depth|Display::E_Clear_Stencil, Maths::RGBA(128, 0, 0, 255));
	if(g_debugDrawBeforeRestoreWindow)
		Display::BasicRender()->DrawColoured2DRect(50.0f, 50.0f, 250.0f, 250.0f, Maths::RGBA(0, 128, 0, 255));
	#endif

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
		BIZ_ASSERT(SUCCEEDED(hr));
	}
	#else
	#if defined(USE_DISPLAY_WINDOWS)
	m_pDisplayDevice->RestoreWindow(viewIndex);
	#endif // PLATFORM_PC

	SendMessageToLayers(E_LayerMessage_RenderSplitscreenWithoutAA);
	#endif // USE_DISPLAY_WINDOWS

	#if defined(PLATFORM_PS3)
	Render::SetGlobalColourBuffer(Display::Device()->GetLocalSharedColourTexture());
	Render::SetGlobalDepthStencilBuffer(Display::Device()->GetLocalSharedDepthStencilTexture());
	#endif // PLATFORM_PS3

	EndRenderSplitscreenWithoutAA();

	m_pDisplayDevice->SetView(NULL);
}
#endif // PLATFORM_PC

void C_ApplicationBase::RenderSplitscreenWithoutAA()
{
	PERF_EVENT("App.RenderSplitscreenWithoutAA");

	#if PLATFORM_PS3 && !CFG_GOLD
	if(g_debugClear2and3PlayerSplitscreen && (m_SplitscreenViews.Size() == 2 || m_SplitscreenViews.Size() == 3)) // Need to clear the colour buffer as we don't render to all the screen
	{
		m_pDisplayDevice->Clear(Display::E_Clear_Colour);
	}
	#endif

	#ifdef PLATFORM_PC
	// [Blur2PC] retail Blur PC (0x0084e250) restores the windows onto the back buffer (or onto an application
	// render texture at +0xac4 that nothing creates)
	m_pDisplayDevice->SetColourDepthStencilSurface();
	#endif
	
	for(S32 viewIndex=m_SplitscreenViews.Size()-1; viewIndex>=0; --viewIndex)
	{
		PERF_EVENT("Splitscreen");

		RenderSplitscreenViewWithoutAA(viewIndex);
	}

	ResetDebugRender();	// [Blur2PC] retail Blur PC's 0x0084e250 does not send this; kept for the snapshot's debug layer

	m_iSplitscreenView = -1;

	#if defined(USE_DISPLAY_WINDOWS) && !defined(PLATFORM_PC)
	m_pDisplayDevice->SetColourDepthStencilSurface();
	#endif // USE_DISPLAY_WINDOWS
}

void C_ApplicationBase::RenderFullscreen()
{
	PERF_EVENT("App.RenderFullscreen");

	m_pDisplayDevice->SetupView(&m_FullscreenView.m_View, m_FullscreenView.m_ViewSetup);
	m_pDisplayDevice->SetView(&m_FullscreenView.m_View);

	#if PLATFORM_PS3
	if(m_SplitscreenViews.Size() > 1)
	{
		Display::C_StencilState stencilState;
		stencilState.m_StencilEnable		= Display::E_Bool_True;
		stencilState.m_StencilFunc			= Display::E_CmpFunc_Always;
		stencilState.m_StencilRef			= 0x00000000;
		stencilState.m_StencilMask			= 0xFFFFFFFF;
		stencilState.m_StencilWriteMask		= 0xFFFFFFFF;
		stencilState.m_StencilFail			= Display::E_StencilOp_Keep;
		stencilState.m_StencilDepthFail		= Display::E_StencilOp_Keep;
		stencilState.m_StencilDepthPass		= Display::E_StencilOp_Keep;
		m_pDisplayDevice->SetStencilState(stencilState);
		m_pDisplayDevice->Clear(Display::E_Clear_Depth|Display::E_Clear_Stencil);
	}
	#endif

	SendMessageToLayers(E_LayerMessage_RenderFullscreen);

	// We want to render the fullscreen debug over the top of the UI:
	SendMessageToLayers(E_LayerMessage_RenderFullscreenDebug);


	if(g_debugShowFps || g_bShowFps)
	{
		F32 fps = m_Timer.GetFrameRate();
		F32 msec = 0.f;
		if (fps > 1e-4f)
			msec = 1000.f / fps;
		
		Util::C_String str;
		str.Printf("fps:%5.1f  (%5.2f ms)", m_Timer.GetFrameRate(), msec);
		Display::FontRender()->Draw(str.Chars(), 50.f, 768.f-50.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
	}

#if PLATFORM_PS3 && !CFG_GOLD
	F32 x = g_debugShowBufferLeft;
	F32 y = g_debugShowBufferTop;
	F32 w = g_debugShowBufferWidth;
	F32 h = g_debugShowBufferHeight;

	if(g_debugShowWindowColourBuffer && g_debugShowWindowBufferIndex >= 0 && g_debugShowWindowBufferIndex < m_pDisplayDevice->GetWindowCount())
		Display::Quad::DrawPartScreen(x, y, x+w, y+h, m_pDisplayDevice->GetWindow(g_debugShowWindowBufferIndex)->GetColourTexture(), &Display::PixelShader::m_PixelShaderTextureRGB);
	x += (w * 1.1f);

	if(g_debugShowWindowDepthStencilBuffer && g_debugShowWindowBufferIndex >= 0 && g_debugShowWindowBufferIndex < m_pDisplayDevice->GetWindowCount())
		Display::Quad::DrawPartScreen(x, y, x+w, y+h, m_pDisplayDevice->GetWindow(g_debugShowWindowBufferIndex)->GetDepthStencilTexture(), &Display::PixelShader::m_PixelShaderTextureD);
	y += (h * 1.1f); x = g_debugShowBufferLeft;

	if(g_debugShowBackColourBuffer)
		Display::Quad::DrawPartScreen(x, y, x+w, y+h, m_pDisplayDevice->GetBackBuffer().GetColourTexture(), &Display::PixelShader::m_PixelShaderTextureRGB);
	x += (w * 1.1f);
	
	if(g_debugShowBackDepthStencilBuffer)
		Display::Quad::DrawPartScreen(x, y, x+w, y+h, m_pDisplayDevice->GetBackBuffer().GetDepthStencilTexture(), &Display::PixelShader::m_PixelShaderTextureD);
	 y += (h * 1.1f); x = g_debugShowBufferLeft;

	if(g_debugShowFrontBuffer && g_debugShowFrontBufferIndex >= 0 && g_debugShowFrontBufferIndex < m_pDisplayDevice->GetFrontBufferCount())
		Display::Quad::DrawPartScreen(x, y, x+w, y+h, m_pDisplayDevice->GetFrontBuffer(g_debugShowFrontBufferIndex).GetTexture(), &Display::PixelShader::m_PixelShaderTextureRGB);
	y += (h * 1.1f); x = g_debugShowBufferLeft;
	 
	if(g_debugShowCurrentFrontBuffer)
		Display::Quad::DrawPartScreen(x, y, x+w, y+h, m_pDisplayDevice->GetFrontBuffer().GetTexture(), &Display::PixelShader::m_PixelShaderTextureRGB);
	 x += (w * 1.1f);

	if(g_debugShowSwappedFrontBuffer)
		Display::Quad::DrawPartScreen(x, y, x+w, y+h, m_pDisplayDevice->GetSwappedFrontBuffer().GetTexture(), &Display::PixelShader::m_PixelShaderTextureRGB);
	 y += (h * 1.1f); x = g_debugShowBufferLeft;
#endif

	m_pDisplayDevice->SetView(NULL);
}

void C_ApplicationBase::EndFrame()
{
	PERF_EVENT("App.EndFrame");

	SendMessageToLayers(E_LayerMessage_EndFrame);
}

void C_ApplicationBase::BeginRenderSplitscreenWithoutAA()
{
	PERF_EVENT("App.BeginRenderSplitscreenWithoutAA");
	SendMessageToLayers(E_LayerMessage_BeginRenderSplitscreenWithoutAA);
}

void C_ApplicationBase::EndRenderSplitscreenWithoutAA()
{
	PERF_EVENT("App.EndRenderSplitscreenWithoutAA");
	SendMessageToLayers(E_LayerMessage_EndRenderSplitscreenWithoutAA);
}

void C_ApplicationBase::ResetDebugRender()
{
	SendMessageToLayers(E_LayerMessage_ResetDebugRender);
}

HRESULT C_ApplicationBase::SendMessageToLayers(const U32 message)
{
	return m_LayerManager.SendMessageToLayers( message );
}

#ifdef PLATFORM_PC
// [Blur2PC] retail Blur PC (Blur.exe 1.2 0x0084e6a0): the windows are sized from the device parameters given
#define DISPLAY_WINDOW_PARAMS deviceParams,
void C_ApplicationBase::AddDisplayWindows(const Display::C_DeviceParams& deviceParams)
#else
#define DISPLAY_WINDOW_PARAMS
void C_ApplicationBase::AddDisplayWindows()
#endif
{
	m_pDisplayDevice->ClearWindows();

	m_SplitscreenViews.Resize(0);

	switch(m_ViewSetup)
	{
	case E_ViewSetup_1:
		AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(1.0f, 1.0f)));
		break;
	case E_ViewSetup_2h:
		if(g_debugShrink2PlayerSplitscreenEnable)
		{
			F32 border = (F32)g_debugShrink2PlayerSplitscreenBorder;
			F32 offset = (F32)g_debugShrink2PlayerSplitscreenOffset;			
			AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(offset, 0.0f), Maths::HVec2(1.0f-(border*2.0f)+offset, 0.5f)));
			AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.0f+(border*2.0f-offset), 0.5f), Maths::HVec2(1.0f-offset, 1.0f)));
		}
		else
		{
			AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(1.0f, 0.5f)));
			AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.0f, 0.5f), Maths::HVec2(1.0f, 1.0f)));
		}
		break;
	case E_ViewSetup_2v:
		AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(0.5f, 1.0f)));
		AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.5f, 0.0f), Maths::HVec2(1.0f, 1.0f)));
		break;
	case E_ViewSetup_3:
		AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.25f, 0.0f), Maths::HVec2(0.75f, 0.5f)));
		AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.0f, 0.5f), Maths::HVec2(0.5f, 1.0f)));
		AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.5f, 0.5f), Maths::HVec2(1.0f, 1.0f)));
		break;
	case E_ViewSetup_3h:
		AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(1.0f, 0.5f)));
		AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.0f, 0.5f), Maths::HVec2(0.5f, 1.0f)));
		AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.5f, 0.5f), Maths::HVec2(1.0f, 1.0f)));
		break;
	case E_ViewSetup_3v:
		AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(0.5f, 1.0f)));
		AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.5f, 0.0f), Maths::HVec2(1.0f, 0.5f)));
		AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.5f, 0.5f), Maths::HVec2(1.0f, 1.0f)));
		break;
	case E_ViewSetup_4:
		AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(0.5f, 0.5f)));
		AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.5f, 0.0f), Maths::HVec2(1.0f, 0.5f)));
		AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.0f, 0.5f), Maths::HVec2(0.5f, 1.0f)));
		AddDisplayWindow(DISPLAY_WINDOW_PARAMS Maths::C_Rect(Maths::HVec2(0.5f, 0.5f), Maths::HVec2(1.0f, 1.0f)));
		break;
	default:
		BIZ_ASSERT(0 && "Unsupported view setup");
		break;
	}
}
#undef DISPLAY_WINDOW_PARAMS

#ifdef PLATFORM_PC
// [Blur2PC] retail Blur PC (Blur.exe 1.2 0x0084ebf0)
void C_ApplicationBase::AddDisplayWindow(const Display::C_DeviceParams& deviceParams, const Maths::C_Rect& windowCoords)
{
	S32 viewNo = m_SplitscreenViews.Size();
	BIZ_ASSERT(viewNo < 4);

	const S32 windowIndex = m_pDisplayDevice->AddWindow(windowCoords);
	Display::C_Window* pWindow = m_pDisplayDevice->GetWindow(windowIndex);
	pWindow->Resize(
		deviceParams.m_WindowBufferWidth,
		deviceParams.m_WindowBufferHeight,
		deviceParams.m_WindowBufferMultisample,
		deviceParams.m_ScreenWidth,
		deviceParams.m_ScreenHeight,
		deviceParams.m_WindowBufferColourFormat,
		deviceParams.m_WindowBufferDepthStencilFormat);

	m_SplitscreenViews.Resize(viewNo + 1);

	C_View& view = m_SplitscreenViews[viewNo];

	Maths::HVec2 hack(1024.0f, 768.0f);
	view.m_ViewSetup.m_WindowCoords = Maths::C_Rect(pWindow->m_Coords.Min() * hack, pWindow->m_Coords.Max() * hack);
	SetWindowViewports(view, pWindow, deviceParams.m_ScreenWidth, deviceParams.m_ScreenHeight);

	view.m_ViewFlags |= Render::E_ViewFlag_3d;
	view.m_ViewFlags |= Render::E_ViewFlag_EnablePostProcess;
	view.m_ViewFlags |= Render::E_ViewFlag_EnableDeferred;
	view.m_View.SetUserIndex(viewNo);

	view.m_ViewSetup.m_ViewSetup3D.m_WindowProportion = (windowCoords.Max().X() - windowCoords.Min().X()) / (windowCoords.Max().Y() - windowCoords.Min().Y());
	view.m_ViewSetup.m_ViewSetup3D.m_AspectRatio = Display::Device()->GetAspectRatio();
}
#else
void C_ApplicationBase::AddDisplayWindow(const Maths::C_Rect& windowCoords)
{
	S32 viewNo = m_SplitscreenViews.Size();
	BIZ_ASSERT(viewNo < 4);

	#ifndef PLATFORM_PC
	const S32 windowIndex = m_pDisplayDevice->AddWindow(windowCoords);
	Display::C_Window* pWindow = m_pDisplayDevice->GetWindow(windowIndex);
	pWindow->Resize(
		m_DisplayDeviceParams.m_WindowBufferWidth,
		m_DisplayDeviceParams.m_WindowBufferHeight,
		m_DisplayDeviceParams.m_WindowBufferMultisample,
		m_DisplayDeviceParams.m_WindowBufferColourFormat,
		m_DisplayDeviceParams.m_WindowBufferDepthStencilFormat);
	#endif // PLATFORM_PC

	m_SplitscreenViews.Resize(viewNo + 1);

	C_View& view = m_SplitscreenViews[viewNo];

	view.m_ViewFlags |= Render::E_ViewFlag_3d;
	view.m_ViewFlags |= Render::E_ViewFlag_EnablePostProcess;
	view.m_ViewFlags |= Render::E_ViewFlag_EnableDeferred;
	view.m_View.SetUserIndex(viewNo);

	view.m_ViewSetup.m_ViewSetup3D.m_WindowProportion = (windowCoords.Max().X() - windowCoords.Min().X()) / (windowCoords.Max().Y() - windowCoords.Min().Y());
	view.m_ViewSetup.m_ViewSetup3D.m_AspectRatio = Display::Device()->GetAspectRatio();
}
#endif // PLATFORM_PC

void C_ApplicationBase::ResizeDisplayWindows()
{
#ifndef PLATFORM_PC
	if(g_debugChangeResolution || (g_debugResolution != 0 && g_debugResolution != m_Resolution))
	{
		Display::C_Resolution* pResolution = Display::g_Resolutions.GetResolution(g_debugResolution);
		if(pResolution)
		{
			m_pDisplayDevice->m_DeviceParams.m_WindowBufferWidth			= pResolution->m_Width;
			m_pDisplayDevice->m_DeviceParams.m_WindowBufferHeight			= pResolution->m_Height;
			m_pDisplayDevice->m_DeviceParams.m_WindowBufferMultisample		= pResolution->m_Multisample;
			

			#if defined(PLATFORM_PS3)
			m_pDisplayDevice->BlockUntilIdle();
			#endif

			m_pDisplayDevice->PreResizeRenderers();

			m_pDisplayDevice->ResizeWindows();

			m_pDisplayDevice->PostResizeRenderers();
		}
		
		m_Resolution = g_debugResolution;

		g_debugChangeResolution = false;
	}
#endif
}

void C_ApplicationBase::SetupDisplayResolutions(const U32 width, const U32 height, const Display::E_Format format, const Display::E_Multisample multisample, const U32 flags)
{
	Display::g_Resolutions.SetMaxResolution(width, height, format, multisample, flags);
	
	Display::g_Resolutions.ResetResolutions();

	Display::g_Resolutions.AddResolution(320, 200, Display::E_Format_A8R8G8B8, Display::E_Multisample_None,		flags);
	Display::g_Resolutions.AddResolution(640, 480, Display::E_Format_A8R8G8B8, Display::E_Multisample_4,		flags);
	Display::g_Resolutions.AddResolution(1024, 600, Display::E_Format_A8R8G8B8, Display::E_Multisample_2,		flags);
	Display::g_Resolutions.AddResolution(1280, 720, Display::E_Format_A8R8G8B8, Display::E_Multisample_None,	flags);
	Display::g_Resolutions.AddResolution(1280, 720, Display::E_Format_A8R8G8B8, Display::E_Multisample_2,		flags);
	
	Display::g_Resolutions.SetResolutions(&g_debugResolution);

	g_debugResolution.SetValue((E_Resolution)E_Resolution_Disabled);
}

bool C_ApplicationBase::GetUpdatePresentRender() const
{
	return m_UpdatePresentRender;
}

void C_ApplicationBase::SetUpdatePresentRender(const bool enable)
{
	m_UpdatePresentRender = enable;
}

}; // namespace Framework

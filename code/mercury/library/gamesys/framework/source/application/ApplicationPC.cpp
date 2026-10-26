#include "FrameworkPch.h"
#include <camera2/Feed.h>	// [Blur2PC] render interpolation offset
// [Blur2PC] render interpolation hooks (world_render ModelInstance.cpp)
void Blur2PCFrameLimitWait();	// [Blur2PC] Timer.cpp
namespace WorldRender
{
	void Blur2PCInterpEnable(bool enable);
	void Blur2PCInterpTick();
	void Blur2PCInterpSetTickSeconds(F32 seconds);
	void Blur2PCInterpBeginRender(F32 alpha);
	void Blur2PCInterpEndRender();
}
#include <system/debug/Blur2PCAutomation.h>	// [Blur2PC]
#include "Framework/Application/ApplicationPC.h"
#include "Framework/Application/TimingColours.h"	// [Blur2PC] mini perf monitor timings
#include "Framework/Layer/Layer.h"

#include <System/Serialise.h>
#include <System/perf/ProfilerBackend.h>

#include <Input/DeviceManager.h>
#include <Input/Input.h>
#include <Input/Device.h>
#include <Input/MousePC.h>

#include <Render/Utils/Globals.h>

#include <atlbase.h>
#include <atlwin.h>
#include <wtsapi32.h>
#include <shlobj.h>						// [Blur2PC] SHGetFolderPathA (settings.xml location)

#include <System/XML/XmlParser.h>		// [Blur2PC] settings.xml
#include <Display/Device/DeviceDebug.h>	// [Blur2PC] Display::g_Resolutions

#if defined(USER_LLWRIGHT)
#define DEBUG_OUTPUT_WINDOWS_MESSAGES 0
#endif

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

LRESULT CALLBACK ApplicationWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#if !CFG_SPEED
extern S32 g_d3dOcclusionQueryCount;
#endif

namespace Framework
{

// C_AppClass

C_AppClass::C_AppClass() :
	m_WndClassAtom(0),
	m_WndClassName("Default Class Name")
{
}

C_AppClass::~C_AppClass()
{
}

// C_AppWindow

C_AppWindow::C_AppWindow() :
	m_Name("Default Window Name"),
	m_Style(0),
	m_X(0),
	m_Y(0),
	m_W(640),
	m_H(480),
	m_hDC(NULL),
	m_hWnd(NULL),
	m_hMenu(NULL)
{
}

C_AppWindow::~C_AppWindow()
{
}

// C_Application

C_Application* C_Application::s_pInstance = NULL;

C_Application::C_Application() :
	m_Active(true),
	m_HasFocus(true),
	m_Windowed(true),
	m_ClipCursor(true),
	m_ShowCursor(true),
	m_CameraLook(false),
	m_hInstance(NULL),
	m_ResolutionIndex(0),		// [Blur2PC] retail C_Application state, see docs/analysis.md "Graphics options (PC)"
	m_MultisampleIndex(0),
	m_GraphicalDetail(0),
	m_DesktopRefreshRate(0),
	m_MinWidth(640),
	m_MinHeight(480),
	m_CursorShown(true),
	m_TrackWindowSize(true)
{
	assert(s_pInstance == NULL);
	s_pInstance = this;
}

C_Application::~C_Application()
{
	assert(s_pInstance == this);
	s_pInstance = NULL;
}

HRESULT C_Application::PlatformInitialise(const C_ApplicationParams& params)
{
	// [Blur2PC] retail PlatformInitialise (0x0084bc80) reads settings.xml before the window exists
	if(!LoadSettings())
		return S_FALSE;

	m_hInstance = params.m_PC.m_hInstance;

	if(MyRegisterClass(params) != S_OK)
		return S_FALSE;

	if(MyCreateWindow(params) != S_OK)
		return S_FALSE;

	InitMouse();

	return S_OK;
}

void C_Application::PlatformShutdown()
{
}

void C_Application::SetupDeviceParams()
{
	C_ApplicationBase::SetupDeviceParams();

	// [Blur2PC] retail (0x0084d210) also records the desktop refresh rate
	Display::C_VideoMode desktopMode;
	if(m_pDisplayManager->GetDesktopVideoMode(&desktopMode))
		m_DesktopRefreshRate = desktopMode.m_RefreshRate;

	m_DisplayDeviceParams.m_hDC			= m_Window.m_hDC;
	m_DisplayDeviceParams.m_hWnd			= m_Window.m_hWnd;
	m_DisplayDeviceParams.m_hInstance		= m_hInstance;
	m_DisplayDeviceParams.m_Windowed		= m_Windowed;
}

// [Blur2PC] retail CanRender (0x0084a400). The window-size polling the original PC code did here moved to
// UpdateGraphicsSettings in retail; a lost device is reset through ResetDevice.
bool C_Application::CanRender()
{
	assert(m_pDisplayDevice);

	if(m_Window.m_hWnd == NULL)
		return false;

	// [Blur2PC] scripted runs keep their window minimised (MyCreateWindow) and still render, so the
	// back-buffer screenshots work; not in retail
	if(IsIconic(m_Window.m_hWnd) && !Blur2PCAutomation::IsActive())
		return false;

	if(m_pDisplayDevice->GetLost())
	{
		ResetDevice();
		return m_pDisplayDevice->CanRender();
	}

	return true;
}

#if 0	// [Blur2PC] original PC CanRender, kept for reference; replaced by the retail version above
bool C_Application::CanRenderOriginal()
{
	assert(m_pDisplayDevice);

	bool forceReset = false;

	if(m_Windowed)
	{
		RECT rect;
		GetClientRect(m_Window.m_hWnd, &rect);

		U32 newWidth = rect.right - rect.left;
		U32 newHeight = rect.bottom - rect.top;
		
		if(newWidth != m_DisplayDeviceParams.m_Width || newHeight != m_DisplayDeviceParams.m_Height)
		{
			forceReset = true;

			m_DisplayDeviceParams.m_Width = newWidth;
			m_DisplayDeviceParams.m_Height = newHeight;
		}
		
		if (newWidth <= 0 || newHeight <= 0)
			return false;
	}

	if(m_pDisplayDevice->GetLost() || forceReset)
	{
		if(m_pDisplayDevice->CanReset())
		{
			m_pDisplayDevice->DestroyRenderersVolatile();
			m_pDisplayDevice->DestroyVolatile();

			Render::DestroyGlobalColourBuffer();
			Render::DestroyGlobalDepthStencilBuffer();

			#if !CFG_SPEED
			BIZ_ASSERT(g_d3dOcclusionQueryCount == 0);
			#endif
			
			if(m_pDisplayDevice->CreateVolatile(m_DisplayDeviceParams))
			{
				if(m_pDisplayDevice->CreateRenderersVolatile())
				{
					Render::CreateGlobalColourBuffer();
					Render::CreateGlobalDepthStencilBuffer();

					const Display::C_BackBuffer& backBuffer = m_pDisplayDevice->GetBackBuffer();

					for(S32 viewIndex = 0; viewIndex < m_SplitscreenViews.Size(); ++viewIndex)
					{
						C_View& splitscreenView = m_SplitscreenViews[viewIndex];
						splitscreenView.m_View.Setup(splitscreenView.m_ViewSetup, backBuffer.GetWidth(), backBuffer.GetHeight());
					}

					m_LayerManager.SendMessageToLayers(E_LayerMessage_Resize);

					#if defined(USER_LLWRIGHT)
					LOGINFO(g_Log, "DisplayDevice : Reset 1\n");
					#endif
				}
				else
				{
					#if defined(USER_LLWRIGHT)
					LOGINFO(g_Log, "DisplayDevice : Reset 0\n");
					#endif

					LOGWARN(g_Log, "Failed to create volatile display device renderers\n");
				}
			}
			else
			{
				#if defined(USER_LLWRIGHT)
				LOGINFO(g_Log, "DisplayDevice : Reset 0\n");
				#endif

				LOGWARN(g_Log, "Failed to create volatile display device\n");
				return false;
			}
		}

		if(m_pDisplayDevice->CanRender())
		{
			#if defined(USER_LLWRIGHT)
			LOGINFO(g_Log, "DisplayDevice : Reset 1 Render 1\n");
			#endif

			return true;
		}
		else
		{
			#if defined(USER_LLWRIGHT)
			LOGINFO(g_Log, "DisplayDevice : Reset 1 Render 0\n");
			#endif

			return false;
		}
	}

	return true;
}
#endif

HRESULT C_Application::MyRegisterClass(const C_ApplicationParams& params)
{
	HICON hIconLarge = LoadIcon(m_hInstance, MAKEINTRESOURCE(params.m_PC.m_IconResourceLarge));
	HICON hIconSmall = LoadIcon(m_hInstance, MAKEINTRESOURCE(params.m_PC.m_IconResourceSmall));

	memset(&m_Class.m_WndClassEx, 0, sizeof(m_Class.m_WndClassEx));

	m_Class.m_WndClassEx.cbSize        		= sizeof(m_Class.m_WndClassEx); 
	m_Class.m_WndClassEx.style         		= 0;
	m_Class.m_WndClassEx.lpfnWndProc   		= (WNDPROC)ApplicationWindowProc;
	m_Class.m_WndClassEx.cbClsExtra    		= 0;
	m_Class.m_WndClassEx.cbWndExtra    		= 0;
	m_Class.m_WndClassEx.hInstance     		= m_hInstance;
	m_Class.m_WndClassEx.hIcon         		= hIconLarge;
	m_Class.m_WndClassEx.hCursor       		= LoadCursor(0, IDC_ARROW);
	m_Class.m_WndClassEx.hbrBackground 		= (HBRUSH)(COLOR_WINDOW+1);
	m_Class.m_WndClassEx.lpszMenuName  		= NULL;
	m_Class.m_WndClassEx.lpszClassName 		= m_Class.m_WndClassName.Chars();
	m_Class.m_WndClassEx.hIconSm			= hIconSmall;

	m_Class.m_WndClassAtom = RegisterClassEx(&m_Class.m_WndClassEx);

	if(m_Class.m_WndClassAtom != 0)
	{
		return S_OK;
	}
	else
	{
		return S_FALSE;
	}
}

// [Blur2PC] retail window styles (0x0084aa40, 0x0084ab80): a fixed-size captioned window, or a popup covering
// the screen. Both are created visible.
static const DWORD kWindowedStyle	= WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;	// 0x16ca0000
static const DWORD kFullscreenStyle	= WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;									// 0x96000000

// [Blur2PC] retail CreateWindow (0x0084aa40): the window's client area is placed at the saved position and
// sized to the chosen resolution; the device is then sized to the client area.
HRESULT C_Application::MyCreateWindow(const C_ApplicationParams& params)
{
	m_Window.m_Style = m_Windowed ? kWindowedStyle : kFullscreenStyle;

	// [Blur2PC] scripted runs (_work/run/run_script.ps1) create the window without WS_VISIBLE and leave it
	// hidden until the device exists (D3D9 CreateDevice fails on a minimised window); Run() then shows it
	// minimised without activation, so it sits on the taskbar and never takes focus. Not in retail.
	DWORD createStyle = m_Window.m_Style;
	if(Blur2PCAutomation::IsActive())
		createStyle &= ~WS_VISIBLE;

	RECT rect;
	GetWindowRectFromClient(&rect);

	// [Blur2PC] the title bar shows the data directory the game reads (the working directory by now), so
	// a window says which copy of the game data it runs. Not in retail, whose title is the name alone;
	// m_Name itself stays "Blur", it also names the settings folder.
	char title[MAX_PATH + 64];
	char dataDir[MAX_PATH];
	const DWORD dataDirLength = GetCurrentDirectoryA(sizeof dataDir, dataDir);
	if(dataDirLength > 0 && dataDirLength < sizeof dataDir)
		_snprintf_s(title, sizeof title, _TRUNCATE, "%s - %s", m_Window.m_Name.Chars(), dataDir);
	else
		_snprintf_s(title, sizeof title, _TRUNCATE, "%s", m_Window.m_Name.Chars());

	m_Window.m_hWnd = CreateWindowExA(
		0,
		m_Class.m_WndClassName.Chars(),
		title,
		createStyle,
		rect.left,
		rect.top,
		rect.right - rect.left,
		rect.bottom - rect.top,
		NULL,
		m_Window.m_hMenu,
		m_hInstance,
		(LPVOID)&m_Window);

	if(!m_Window.m_hWnd)
		return S_FALSE;

	if(createStyle != m_Window.m_Style && !Blur2PCAutomation::IsActive())
		ShowWindow(m_Window.m_hWnd, SW_SHOW);

	HICON hIcon = LoadIconA(GetModuleHandleA(NULL), MAKEINTRESOURCEA(101));
	SendMessageA(m_Window.m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	SendMessageA(m_Window.m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

	UpdateWindow(m_Window.m_hWnd);

	m_Window.m_hDC = GetDC(m_Window.m_hWnd);

	GetClientRect(m_Window.m_hWnd, &rect);
	SetDisplaySize(rect.right - rect.left, rect.bottom - rect.top, m_DisplayDeviceParams.m_RefreshRate);

	return S_OK;
}

void C_Application::MyKeyDown(const U8 key)
{
	switch(key)
	{
	case VK_ESCAPE:
		// [Blur2PC] Escape closed the window (a framework development shortcut). The game uses Escape
		// for Back/Cancel in its menus, so it no longer quits; close the window or use the menus to exit.
		break;
	}
}

void C_Application::MyKeyUp(const U8 key)
{
}

HRESULT C_Application::Run()
{
	Display::Device()->ReleaseThreadOwnership();

	// [Blur2PC] scripted runs: the device exists now, show the window minimised without activation (see
	// MyCreateWindow); not in retail
	if(Blur2PCAutomation::IsActive())
		ShowWindow(m_Window.m_hWnd, SW_SHOWMINNOACTIVE);

	MSG  msg;
	msg.message = WM_NULL;
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	while(msg.message != WM_QUIT)
	{
		bool gotMsg = PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0 ? true : false;

		if(gotMsg)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			IdleLoop();
		}
	}

	return S_OK;
}

void C_Application::SetActive(const bool active)
{
	if(active)
	{
		//m_ClipCursor = true;

		if(!m_Active)
		{
			#if DEBUG_OUTPUT_WINDOWS_MESSAGES
			LOGINFO(g_Log, "Setting ACTIVE\n");
			#endif

			m_Active = true;

			UpdateCursor();
		}
	}
	else
	{
		//m_ClipCursor = false;

		if(m_Active)
		{
			#if DEBUG_OUTPUT_WINDOWS_MESSAGES
			LOGINFO(g_Log, "Setting INACTIVE\n");
			#endif
			
			m_Active = false;

			UpdateCursor();
		}
	}
}

void C_Application::InitMouse()
{
	RAWINPUTDEVICE rid[1];
    rid[0].usUsagePage	= HID_USAGE_PAGE_GENERIC; 
    rid[0].usUsage		= HID_USAGE_GENERIC_MOUSE; 
    rid[0].dwFlags		= RIDEV_INPUTSINK;   
    rid[0].hwndTarget	= m_Window.m_hWnd;
    BOOL result = RegisterRawInputDevices(rid, 1, sizeof(rid[0]));
	if(!result)
	{
		LOGWARN( Framework::g_Log, "Failed to initialise raw input mouse device" );
	}

	ShowCursorForWindowMode();	// [Blur2PC] retail InitMouse (0x00849c60)
}

static double s_Blur2PCAccum = 1.0;
extern F32 g_Blur2PCForcedFrameTime;	// [Blur2PC] ApplicationBase.cpp	// [Blur2PC] real time not yet consumed by 60 Hz updates

void C_Application::IdleLoop()
{
	UpdateCursor();

	{	// [Blur2PC] frame timing scopes the MiniPerfMonitor reads, as in Application360.cpp Run(); the PC loop had none
	CPU_TIMING(0, Maths::RGBA(255, 255, 255, 255), "CPU");

	CHECK_MEMORY_INTEGRITY();

	// [Blur2PC] Decoupled update/render. The simulation is hard-wired to 1/60 s steps (car physics
	// integrates 1/60 per step), and much per-frame game logic assumed one frame per step. With
	// decoupling on (default), the game update runs only when 1/60 s of real time has accrued, and every
	// loop renders; model matrices and the camera are blended between the last two updates.
	static int s_decouple = -1;
	if (s_decouple < 0)
	{
		// on by default; BLUR2PC_DECOUPLE=0 restores one update per rendered frame
		const char* e = getenv("BLUR2PC_DECOUPLE");
		s_decouple = (e && e[0] == '0') ? 0 : 1;
		::WorldRender::Blur2PCInterpEnable(s_decouple != 0);
	}
	bool doUpdate = true;
	if (s_decouple)
	{
		static LARGE_INTEGER s_freq = { 0 }, s_last = { 0 };
		LARGE_INTEGER now;
		if (!s_freq.QuadPart) QueryPerformanceFrequency(&s_freq);
		QueryPerformanceCounter(&now);
		if (s_last.QuadPart)
			s_Blur2PCAccum += (double)(now.QuadPart - s_last.QuadPart) / (double)s_freq.QuadPart;
		s_last = now;
		// consume whole ticks (at most 4, the game's step cap) and tell the game exactly that much time
		// passed, so each update runs a whole number of 1/60 s steps; a longer hitch drops the rest
		const double tick = 1.0 / 60.0;
		int ticks = (int)(s_Blur2PCAccum / tick);
		doUpdate = ticks >= 1;
		if (doUpdate)
		{
			if (ticks > 4)
				ticks = 4;
			s_Blur2PCAccum -= ticks * tick;
			if (s_Blur2PCAccum >= tick)
				s_Blur2PCAccum = 0.0;
			g_Blur2PCForcedFrameTime = (F32)(ticks * tick);
		}
	}

	if (doUpdate)
	{
		if (s_decouple)
		{
			::WorldRender::Blur2PCInterpTick();
			::WorldRender::Blur2PCInterpSetTickSeconds(g_Blur2PCForcedFrameTime);
			Cam2::I_Feed::Blur2PCAdvanceTick();
		}
		CPU_TIMING(1, g_TimingColour_Update, "CPU.Update");	// [Blur2PC] as Application360.cpp
		OnUpdate();
	}

	// [Blur2PC] render between the previous and the latest update, at the fraction of a tick elapsed
	F32 renderAlpha = 1.0f;
	if (s_decouple)
	{
		renderAlpha = (F32)(s_Blur2PCAccum / (1.0 / 60.0));
		renderAlpha = renderAlpha < 0.0f ? 0.0f : (renderAlpha > 1.0f ? 1.0f : renderAlpha);
		Cam2::I_Feed::SetRenderOffset(renderAlpha - 1.0f);
		::WorldRender::Blur2PCInterpBeginRender(renderAlpha);
	}

	Display::Device()->AcquireThreadOwnership();
	{	// [Blur2PC] GPU/render timing scopes as Application360.cpp
		GPU_TIMING_BEGIN_FRAME();
		{
			GPU_TIMING(0, Maths::RGBA(255, 255, 255, 255), "GPU");
			{
				CPU_TIMING(1, g_TimingColour_Render, "CPU.Render");
				OnRender();
			}
		}
		GPU_TIMING_END_FRAME();
		// [Blur2PC] 360 runs this after releasing the device; on PC it reads D3D9 queries, so it stays inside
		GPU_TIMING_UPDATE();
	}
	Display::Device()->ReleaseThreadOwnership();

	if (s_decouple)
	{
		::WorldRender::Blur2PCInterpEndRender();
		Cam2::I_Feed::SetRenderOffset(0.0f);
	}
	{	// [Blur2PC] rendered frame rate report (the timer's report counts updates)
		static DWORD s_t0 = GetTickCount(); static int s_frames = 0, s_updates = 0;
		++s_frames; if (doUpdate) ++s_updates;
		const DWORD t = GetTickCount();
		if (t - s_t0 >= 5000) { LOGINFO(Debug::GetLogger("blur2pc.render"), "render %.1f fps, update %.1f Hz\n", s_frames * 1000.0f / (t - s_t0), s_updates * 1000.0f / (t - s_t0)); s_t0 = t; s_frames = s_updates = 0; }
	}

	Blur2PCFrameLimitWait();	// [Blur2PC] optional render cap (Timer.cpp); inside "CPU" so the monitor's FPS counts it, like the 360's vsync wait in Present
	}	// [Blur2PC] end of "CPU"

	Perf::ProfilerCommon::Instance().EndFrame();

	#ifdef USE_CPU_TIMINGS	// [Blur2PC] as Application360.cpp
	Perf::C_CPUTimingStats::UpdateStats();
	#endif

	if (m_RunState != E_RunState_Run)
	{
		SendMessage(m_Window.m_hWnd, WM_CLOSE, 0, 0);
	}
}

bool C_Application::IsCursorInWindow()
{
	RECT rect;
	GetWindowRect(m_Window.m_hWnd, &rect);

	POINT point;
	BOOL result = GetCursorPos(&point);

	if(point.x < rect.left)
		return false;
	else
	if(point.x > rect.right)
		return false;
	else
	if(point.y < rect.top)
		return false;
	else
	if(point.y > rect.bottom)
		return false;
	else
		return true;
}

void C_Application::OnMouseButton(const U32 button, const bool state)
{
	if(button == 0)
	{
		bool isCursorInWindow = IsCursorInWindow();

		if(isCursorInWindow)
			m_CameraLook = state;
		else
			m_CameraLook = false;
	}

	for(U32 i=0; i<E_UserInputIndex_Count; i++)
	{
		if(m_pMouseList[i])
		{
			((Input::C_MousePC*)m_pMouseList[i])->SetButtonValue(button, state ? 1.0f : 0.0f);
		}	
	}
}

void C_Application::OnMouseMoveAbsolute(const S32 x, const S32 y)
{
	if(!m_CameraLook)
		return;

	for(U32 i=0; i<E_UserInputIndex_Count; i++)
	{
		if(m_pMouseList[i])
		{
			((Input::C_MousePC*)m_pMouseList[i])->SetAxisValue(Input::C_Mouse::E_Axis_XPos, (F32)x);
			((Input::C_MousePC*)m_pMouseList[i])->SetAxisValue(Input::C_Mouse::E_Axis_YPos, (F32)y);
		}	
	}
}

void C_Application::OnMouseMoveRelative(const S32 x, const S32 y)
{
	if(!m_CameraLook)
		return;

	for(U32 i=0; i<E_UserInputIndex_Count; i++)
	{
		if(m_pMouseList[i])
		{
			((Input::C_MousePC*)m_pMouseList[i])->SetAxisValue(Input::C_Mouse::E_Axis_DeltaX, (F32)x);
			((Input::C_MousePC*)m_pMouseList[i])->SetAxisValue(Input::C_Mouse::E_Axis_DeltaY, (F32)-y);
		}	
	}
}

void C_Application::MyShowCursor(const bool show)
{
	return;

	if(show)
	{
		if(!m_ShowCursor)
		{
			::ShowCursor(true);
			m_ShowCursor = true;
		}
	}
	else
	{
		if(m_ShowCursor)
		{
			::ShowCursor(false);
			m_ShowCursor = false;
		}
	}
}

void C_Application::UpdateCursor()
{
	return;

	RECT rcClip;
	GetWindowRect(m_Window.m_hWnd, &rcClip); 

	if(m_ClipCursor)
	{
		MyShowCursor(false);

		POINT point;
		BOOL result = GetCursorPos(&point);

		bool inWindow = true;
		if(point.x < rcClip.left)
			inWindow = false;
		if(point.x > rcClip.right)
			inWindow = false;
		if(point.y < rcClip.top)
			inWindow = false;
		if(point.y > rcClip.bottom)
			inWindow = false;

		if(!inWindow)
		{
			point.x = Maths::Max(point.x, rcClip.left);
			point.x = Maths::Min(point.x, rcClip.right);
			point.y = Maths::Max(point.y, rcClip.top);
			point.y = Maths::Min(point.y, rcClip.bottom);

			SetCursorPos(point.x, point.y);
		}

		::ClipCursor(&rcClip);
	}
	else
	{
		::ClipCursor(NULL); 

		MyShowCursor(true);
	}
}

void C_Application::FocusChange(const bool gotFocus)
{
	// [Blur2PC] unattended scripted runs must not pause when someone uses another window
	if (!gotFocus && Blur2PCAutomation::IsActive())
		return;
	if(m_HasFocus)
	{
		if(!gotFocus)
		{
			m_HasFocus = false;
		
			SetActive(false);
		}
	}
	else
	{
		if(gotFocus)
		{
			m_HasFocus = true;
		
			SetActive(true);
		}
	}
}

LRESULT C_Application::MyWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(hWnd == m_Window.m_hWnd)
	{
		switch(uMsg)
		{
			case WM_PAINT:
			{
				#if DEBUG_OUTPUT_WINDOWS_MESSAGES
				LOGINFO(g_Log, "WindowsMessage : WM_PAINT\n");
				#endif
			}
			break;

			case WM_ENTERSIZEMOVE:
			{
				#if DEBUG_OUTPUT_WINDOWS_MESSAGES
				LOGINFO(g_Log, "WindowsMessage : WM_ENTERSIZEMOVE\n");
				#endif

				SetFocusPaused(true);
			}
			break;

			case WM_EXITSIZEMOVE:
			{
				#if DEBUG_OUTPUT_WINDOWS_MESSAGES
				LOGINFO(g_Log, "WindowsMessage : WM_EXITSIZEMOVE\n");
				#endif

				SetFocusPaused(false);
			}
			break;

			case WM_ENTERMENULOOP:
			{
				#if DEBUG_OUTPUT_WINDOWS_MESSAGES
				LOGINFO(g_Log, "WindowsMessage : WM_ENTERMENULOOP\n");
				#endif

				SetFocusPaused(true);
			}
			break;

			case WM_EXITMENULOOP:
			{
				#if DEBUG_OUTPUT_WINDOWS_MESSAGES
				LOGINFO(g_Log, "WindowsMessage : WM_EXITMENULOOP\n");
				#endif

				SetFocusPaused(false);
			}
			break;

			case WM_SYSCOMMAND:
			{
				#if DEBUG_OUTPUT_WINDOWS_MESSAGES
				LOGINFO(g_Log, "WindowsMessage : WM_SYSCOMMAND\n");
				#endif

				// [Blur2PC] retail WndProc (0x0084ae70): full screen ignores maximise, size, move and minimise;
				// the screen saver and monitor power-down are always refused.
				switch(wParam & 0xfff0)
				{
				case SC_MAXIMIZE:
				case SC_SIZE:
				case SC_MOVE:
				case SC_MINIMIZE:
					if(!m_Windowed)
						return 0;
					break;
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
					return 0;
				}
			}
			break;

			case WM_SYSKEYUP:
			{
				// [Blur2PC] retail: Alt+Enter toggles full screen, Alt+F4 closes the window
				if(wParam == VK_RETURN)
				{
					ApplyGraphicsOptions(m_ResolutionIndex, m_MultisampleIndex, !m_Windowed, m_DisplayDeviceParams.m_PresentInterval == Display::E_PresentInterval_One);
				}
				else if(wParam == VK_F4)
				{
					SendMessageA(hWnd, WM_CLOSE, 0, 0);
				}
			}
			break;

			case WM_SIZING:
			{
				// [Blur2PC] retail: the window may not be dragged smaller than 640x480, and a user resize is
				// followed by UpdateGraphicsSettings
				RECT* pRect = (RECT*)lParam;
				if(pRect)
				{
					S32 width = pRect->right - pRect->left;
					S32 height = pRect->bottom - pRect->top;
					ClampToMinimumSize(&width, &height);
					m_TrackWindowSize = true;

					if(wParam == WMSZ_LEFT || wParam == WMSZ_TOPLEFT || wParam == WMSZ_BOTTOMLEFT)
						pRect->left = pRect->right - width;
					else
						pRect->right = pRect->left + width;

					if(wParam == WMSZ_TOP || wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT)
						pRect->top = pRect->bottom - height;
					else
						pRect->bottom = pRect->top + height;
				}
			}
			break;

			case WM_LBUTTONDOWN:
			{
				#if DEBUG_OUTPUT_WINDOWS_MESSAGES
				LOGINFO(g_Log, "WindowsMessage : WM_LBUTTONDOWN\n");
				#endif

				SetActive(true);
			}
			break;

			/*case WM_ACTIVATEAPP:
			{
				#if DEBUG_OUTPUT_WINDOWS_MESSAGES
				LOGINFO(g_Log, "WindowsMessage : WM_ACTIVATEAPP\n");
				#endif

				switch(wParam)
				{
					case 0:
					{
						SetActive(true);
					}
					break;
					
					default:
					{
						SetActive(false);
					}
					break;
				}
			}
			break;*/

			case WM_ACTIVATE:
			{
				#if DEBUG_OUTPUT_WINDOWS_MESSAGES
				LOGINFO(g_Log, "WindowsMessage : WM_ACTIVATE\n");
				#endif

				switch(wParam)
				{
					case WA_INACTIVE:
					{
						FocusChange(false);
					}
					break;

					case WA_ACTIVE:
					{
						FocusChange(true);
					}
					break;

					case WA_CLICKACTIVE:
					{
						FocusChange(true);
					}
					break;
				}
			}
			break;

			case WM_SETFOCUS:
			{
				#if DEBUG_OUTPUT_WINDOWS_MESSAGES
				LOGINFO(g_Log, "WindowsMessage : WM_SETFOCUS\n");
				#endif

				FocusChange(true);
			}
			break;

			case WM_KILLFOCUS:
			{
				#if DEBUG_OUTPUT_WINDOWS_MESSAGES
				LOGINFO(g_Log, "WindowsMessage : WM_KILLFOCUS\n");
				#endif

				FocusChange(false);
			}
			break;

			case WM_SIZE:
			{
				#if DEBUG_OUTPUT_WINDOWS_MESSAGES
				LOGINFO(g_Log, "WindowsMessage : WM_SIZE\n");
				#endif

				if(wParam == SIZE_MINIMIZED)
				{
				}
				else
				{
					RECT rect;
					GetClientRect(m_Window.m_hWnd, &rect);
					if(rect.top == 0 && rect.bottom == 0)
					{
					}
					else
					if(wParam == SIZE_MAXIMIZED)
					{
					}
					else
					if(wParam == SIZE_RESTORED)
					{
					}
				}
			}
			break;

			case WM_DESTROY:
			{
				#if DEBUG_OUTPUT_WINDOWS_MESSAGES
				LOGINFO(g_Log, "WindowsMessage : WM_DESTROY\n");
				#endif
				
				::ClipCursor(NULL);

				m_Window.m_hWnd = NULL;	// [Blur2PC] retail: nothing renders into a destroyed window

				PostQuitMessage(0);
			}
			break;

			case WM_KEYDOWN:
			{
				#if DEBUG_OUTPUT_WINDOWS_MESSAGES
				LOGINFO(g_Log, "WindowsMessage : WM_KEYDOWN\n");
				#endif

				MyKeyDown((U8)wParam);
			}
			break;

			case WM_KEYUP:
			{
				#if DEBUG_OUTPUT_WINDOWS_MESSAGES
				LOGINFO(g_Log, "WindowsMessage : WM_KEYUP\n");
				#endif

				MyKeyUp((U8)wParam);
			}
			break;

			case WM_CHAR:
			{
				#if DEBUG_OUTPUT_WINDOWS_MESSAGES
				LOGINFO(g_Log, "WindowsMessage : WM_CHAR\n");
				#endif
			}
			break;

			case WM_MOUSEMOVE:
			{
				#if DEBUG_OUTPUT_WINDOWS_MESSAGES
				//LOGINFO(g_Log, "WindowsMessage : WM_MOUSEMOVE\n");
				#endif

				const S32 xPos = GET_X_LPARAM(lParam); 
				const S32 yPos = GET_Y_LPARAM(lParam);

				OnMouseMoveAbsolute(xPos, yPos);
			}
			break;

			case WM_INPUT: 
			{
				#if DEBUG_OUTPUT_WINDOWS_MESSAGES
				//LOGINFO(g_Log, "WindowsMessage : WM_INPUT\n");
				#endif

				UINT dataSize = sizeof(RAWINPUT);
				BYTE data[sizeof(RAWINPUT)];
	    		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, data, &dataSize, sizeof(RAWINPUTHEADER));
	    		
				RAWINPUT* pRawInput = (RAWINPUT*)data;

				if(pRawInput->header.dwType == RIM_TYPEMOUSE) 
				{
					const S32 xDelta = pRawInput->data.mouse.lLastX;
					const S32 yDelta = pRawInput->data.mouse.lLastY;
					
					if(pRawInput->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN)
						OnMouseButton(0, true);
					if(pRawInput->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP)
						OnMouseButton(0, false);
					
					if(pRawInput->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN)
						OnMouseButton(1, true);
					if(pRawInput->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP)
						OnMouseButton(1, false);
					
					if(pRawInput->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN)
						OnMouseButton(2, true);
					if(pRawInput->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_UP)
						OnMouseButton(2, false);
					
					if(pRawInput->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)
						OnMouseButton(3, true);
					if(pRawInput->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP)
						OnMouseButton(3, false);
					
					if(pRawInput->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)
						OnMouseButton(4, true);
					if(pRawInput->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP)
						OnMouseButton(4, false);
					
					OnMouseMoveRelative(xDelta, yDelta);
				} 
			}
			break;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool C_Application::LoadingScreenEnabledState() const
{
	return false;
}

// ------------------------------------------------------------------------------------------------------------
// [Blur2PC] Retail Blur PC graphics settings. Recovered from Blur.exe 1.2 with Ghidra (tools/ghidra_retail);
// retail addresses are given per function. Details and what is not verified: docs/analysis.md
// "Graphics options (PC)".
// ------------------------------------------------------------------------------------------------------------

enum E_GraphicsOption
{
	E_GraphicsOption_First = 0,
};

}; // namespace Framework

DEBUG_ENUM(Framework::E_GraphicsOption)
	DE_OPTION(Framework::E_GraphicsOption_First,	"0")
END_DEBUG_ENUM

namespace Framework
{

// retail registers these five in the PC framework (0x00e2a4b0, 0x00e2a890, 0x00e2a8d0 and neighbours); the
// resolution and multisample option lists are filled from Display::g_Resolutions
static Debug::Bool						g_debugFrameworkWindowed		("Graphics.Framework.Windowed", false);
static Debug::Bool						g_debugFrameworkVsync			("Graphics.Framework.Vsync", true);
static Debug::Int						g_debugFrameworkGraphicalDetail	("Graphics.Framework.GraphicalDetail", 0);
static Debug::Enum<E_GraphicsOption>	g_debugFrameworkResolution		("Graphics.Framework.Resolution", E_GraphicsOption_First);
static Debug::Enum<E_GraphicsOption>	g_debugFrameworkMultisample		("Graphics.Framework.Multisample", E_GraphicsOption_First);

// retail SetupDisplayResolutions for PC (0x0084e2e0): every adapter mode and every supported multisample
void C_Application::SetupDisplayResolutions(const U32 width, const U32 height, const Display::E_Format format, const Display::E_Multisample multisample, const U32 flags)
{
	Display::g_Resolutions.SetMaxResolution(width, height, format, multisample, flags);

	for(S32 i=0; i<m_pDisplayManager->GetVideoModeCount(); i++)
	{
		const Display::C_VideoMode& videoMode = m_pDisplayManager->GetVideoMode(i);
		Display::g_Resolutions.AddResolution(videoMode.m_Width, videoMode.m_Height, videoMode.m_Format, Display::E_Multisample_None, flags, videoMode.m_RefreshRate);
	}

	for(S32 i=0; i<m_pDisplayManager->GetMultisampleCount(); i++)
	{
		Display::g_Resolutions.AddMultisample(m_pDisplayManager->GetMultisample(i).m_Multisample);
	}

	Display::g_Resolutions.SetResolutions(&g_debugFrameworkResolution, &g_debugFrameworkMultisample);

	g_debugFrameworkResolution.SetValue(E_GraphicsOption_First);
	g_debugFrameworkMultisample.SetValue(E_GraphicsOption_First);
}

// Collects the attributes of the <shared_graphical_settings> element of settings.xml.
class C_GraphicalSettingsXml : public Sys::C_XmlContent
{
public:
	C_GraphicalSettingsXml() : m_Found(false) {}

	virtual bool StartElement(const Util::C_String& name, const Sys::C_XmlAttributeList& attributes)
	{
		if(!m_Found && name == "shared_graphical_settings")
		{
			m_Found = true;
			for(int i=0; i<attributes.Elements(); i++)
				m_Attributes.Add(attributes.GetKeyByIndex(i), attributes.GetValueByIndex(i));
		}
		return true;
	}

	bool					m_Found;
	Sys::C_XmlAttributeList	m_Attributes;
};

// retail LoadSettings (0x0084b570)
bool C_Application::LoadSettings()
{
	// %APPDATA%\Bizarre Creations\<window name>\settings.xml
	char appData[MAX_PATH + 4];
	memset(appData, 0, sizeof(appData));
	SHGetFolderPathA(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, appData);

	Util::C_String folder;
	folder.Printf("%s\\Bizarre Creations\\%s", appData, m_Window.m_Name.Chars());
	m_SettingsFileName.Printf("%s\\settings.xml", folder.Chars());

	// [Blur2PC] scripted runs keep their own settings file so they never change the player's; not in retail
	if(const char* pOverride = getenv("BLUR2PC_SETTINGS"))
	{
		m_SettingsFileName = pOverride;
	}
	else
	{
		Util::C_String bizarre;
		bizarre.Printf("%s\\Bizarre Creations", appData);
		CreateDirectoryA(bizarre.Chars(), NULL);
		CreateDirectoryA(folder.Chars(), NULL);
	}

	C_GraphicalSettingsXml settings;
	{
		HANDLE hFile = CreateFileA(m_SettingsFileName.Chars(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			const DWORD size = GetFileSize(hFile, NULL);
			if(size != INVALID_FILE_SIZE && size > 0)
			{
				char* pBuffer = new char[size + 1];
				DWORD read = 0;
				if(ReadFile(hFile, pBuffer, size, &read, NULL) && read == size)
				{
					pBuffer[size] = 0;
					Sys::XmlParse(settings, pBuffer, (int)size);
				}
				delete [] pBuffer;
			}
			CloseHandle(hFile);
		}
	}

	// retail reads the attributes by position: width, height, x, y, multisample, windowed, vsync, graphical_detail
	if(settings.m_Found && settings.m_Attributes.Elements() >= 8)
	{
		m_Window.m_W = atoi(settings.m_Attributes.GetValueByIndex(0));
		m_Window.m_H = atoi(settings.m_Attributes.GetValueByIndex(1));
		m_Window.m_X = atoi(settings.m_Attributes.GetValueByIndex(2));
		m_Window.m_Y = atoi(settings.m_Attributes.GetValueByIndex(3));
		m_MultisampleIndex = atoi(settings.m_Attributes.GetValueByIndex(4));
		m_Windowed = atoi(settings.m_Attributes.GetValueByIndex(5)) == 1;
		m_DisplayDeviceParams.m_PresentInterval = atoi(settings.m_Attributes.GetValueByIndex(6)) == 1 ? Display::E_PresentInterval_One : Display::E_PresentInterval_Immediate;
		m_GraphicalDetail = atoi(settings.m_Attributes.GetValueByIndex(7));
		ClampToMinimumSize(&m_Window.m_W, &m_Window.m_H);
	}
	else
	{
		// no settings yet: full screen at the desktop resolution, graphical detail 2
		Display::C_VideoMode desktopMode;
		if(!m_pDisplayManager->GetDesktopVideoMode(&desktopMode))
			return false;

		m_Window.m_W = desktopMode.m_Width;
		m_Window.m_H = desktopMode.m_Height;
		m_Windowed = false;
		m_GraphicalDetail = 2;
	}

	// [Blur2PC] scripted runs never take over the screen (BLUR2PC_FULLSCREEN=1 keeps the saved full screen
	// setting, for tests of the full screen start); not in retail
	if(Blur2PCAutomation::IsActive() && getenv("BLUR2PC_FULLSCREEN") == NULL)
		m_Windowed = true;

	S32 resolutionIndex = Display::g_Resolutions.FindResolution(m_Window.m_W, m_Window.m_H);
	if(resolutionIndex == -1)
	{
		if(!m_Windowed)
		{
			// full screen needs a real mode: take the nearest one
			resolutionIndex = Display::g_Resolutions.FindClosestResolution(m_Window.m_W, m_Window.m_H);
			const Display::C_Resolution* pResolution = Display::g_Resolutions.GetResolution(resolutionIndex);
			if(pResolution)
			{
				m_Window.m_W = pResolution->m_Width;
				m_Window.m_H = pResolution->m_Height;
			}
		}
		else
		{
			// a window may be any size
			resolutionIndex = 0;
		}
	}
	m_ResolutionIndex = resolutionIndex;

	const Display::C_MultisampleOption* pMultisample = Display::g_Resolutions.GetMultisample(m_MultisampleIndex);
	// [Blur2PC] retail LoadSettings writes the choice to the display windows' multisample (+0x134, T089);
	// the back buffer is never multisampled
	if(pMultisample)
	{
		m_DisplayDeviceParams.m_WindowBufferMultisample = pMultisample->m_Multisample;
	}
	else
	{
		m_MultisampleIndex = 0;
		m_DisplayDeviceParams.m_WindowBufferMultisample = Display::E_Multisample_None;
	}

	g_debugFrameworkResolution.SetValue((E_GraphicsOption)m_ResolutionIndex);
	g_debugFrameworkMultisample.SetValue((E_GraphicsOption)m_MultisampleIndex);
	g_debugFrameworkWindowed.SetValue(m_Windowed);
	g_debugFrameworkVsync.SetValue(m_DisplayDeviceParams.m_PresentInterval == Display::E_PresentInterval_One);
	g_debugFrameworkGraphicalDetail.SetValue((int)m_GraphicalDetail);

	LOGINFO(g_Log, "Graphics settings %s: %ix%i at %i,%i, resolution %i, multisample %i, windowed %i, vsync %i, detail %i\n",
		m_SettingsFileName.Chars(), m_Window.m_W, m_Window.m_H, m_Window.m_X, m_Window.m_Y, m_ResolutionIndex, m_MultisampleIndex,
		m_Windowed ? 1 : 0, m_DisplayDeviceParams.m_PresentInterval == Display::E_PresentInterval_One ? 1 : 0, m_GraphicalDetail);

	return true;
}

// retail SaveSettings (0x0084a050, document layout from 0x0084a4a0)
void C_Application::SaveSettings()
{
	if(m_SettingsFileName.Length() == 0)
		return;

	// [Blur2PC] a scripted run (forced windowed) never writes the player's settings file unless it was
	// given its own with BLUR2PC_SETTINGS; not in retail
	if(Blur2PCAutomation::IsActive() && getenv("BLUR2PC_SETTINGS") == NULL)
		return;

	Util::C_String xml;
	xml.Printf(
		"<application_settings version=\"1\"/>\r\n"
		"<shared_graphical_settings width=\"%i\" height=\"%i\" x=\"%i\" y=\"%i\" multisample=\"%i\" windowed=\"%i\" vsync=\"%i\" graphical_detail=\"%i\"/>\r\n",
		m_Window.m_W, m_Window.m_H, m_Window.m_X, m_Window.m_Y, m_MultisampleIndex,
		m_Windowed ? 1 : 0,
		m_DisplayDeviceParams.m_PresentInterval == Display::E_PresentInterval_One ? 1 : 0,
		m_GraphicalDetail);

	HANDLE hFile = CreateFileA(m_SettingsFileName.Chars(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		LOGWARN(g_Log, "Failed to write %s\n", m_SettingsFileName.Chars());
		return;
	}
	DWORD written = 0;
	WriteFile(hFile, xml.Chars(), (DWORD)xml.Length(), &written, NULL);
	CloseHandle(hFile);
}

// retail 0x00849750 (called on the window object, whose second size pair is 640x480)
void C_Application::ClampToMinimumSize(S32* pWidth, S32* pHeight) const
{
	if(*pWidth <= m_MinWidth)
		*pWidth = m_MinWidth;
	if(*pHeight <= m_MinHeight)
		*pHeight = m_MinHeight;
}

// retail 0x0084a450: the resolution becomes the window's client size, the device's back buffer size and the
// display windows' buffer and screen size
void C_Application::SetDisplaySize(const U32 width, const U32 height, const U32 refreshRate)
{
	m_Window.m_W = width;
	m_Window.m_H = height;
	m_DisplayDeviceParams.m_Width = width;
	m_DisplayDeviceParams.m_Height = height;
	m_DisplayDeviceParams.m_WindowBufferWidth = width;		// [Blur2PC] T089
	m_DisplayDeviceParams.m_WindowBufferHeight = height;
	m_DisplayDeviceParams.m_ScreenWidth = width;
	m_DisplayDeviceParams.m_ScreenHeight = height;
	m_DisplayDeviceParams.m_RefreshRate = refreshRate;
	ResizeDisplayWindows();
}

// [Blur2PC] display window 0 (retail 0x00427660 and 0x00427750 read the device's first window)
U32 C_Application::GetDisplayWindowWidth() const
{
	return m_pDisplayDevice->GetWindow(0)->m_Width;
}

U32 C_Application::GetDisplayWindowHeight() const
{
	return m_pDisplayDevice->GetWindow(0)->m_Height;
}

Display::E_Multisample C_Application::GetDisplayWindowMultisample() const
{
	return m_pDisplayDevice->GetWindow(0)->m_Multisample;
}

// [Blur2PC] retail 0x008498d0 (name ours): close the game window, the same path as Alt+F4 (T146)
void C_Application::Blur2PCCloseWindow()
{
	SendMessageA(m_Window.m_hWnd, WM_CLOSE, 0, 0);
}

// [Blur2PC] retail 0x0084a390: resize the display windows from the application's device parameters; their
// targets follow at the next device reset (T089)
void C_Application::ResizeDisplayWindows()
{
	for(S32 viewIndex = 0; viewIndex < m_SplitscreenViews.Size(); ++viewIndex)
	{
		m_pDisplayDevice->GetWindow(viewIndex)->Resize(
			m_DisplayDeviceParams.m_WindowBufferWidth,
			m_DisplayDeviceParams.m_WindowBufferHeight,
			m_DisplayDeviceParams.m_WindowBufferMultisample,
			m_DisplayDeviceParams.m_ScreenWidth,
			m_DisplayDeviceParams.m_ScreenHeight,
			m_DisplayDeviceParams.m_WindowBufferColourFormat,
			m_DisplayDeviceParams.m_WindowBufferDepthStencilFormat);
	}
}

// retail 0x008496b0: window rectangle for the saved client position and size, kept on screen
void C_Application::GetWindowRectFromClient(RECT* pRect) const
{
	pRect->left = m_Window.m_X;
	pRect->right = m_Window.m_X + m_Window.m_W;
	pRect->top = m_Window.m_Y;
	pRect->bottom = m_Window.m_Y + m_Window.m_H;
	AdjustWindowRect(pRect, m_Window.m_Style, FALSE);
	if(pRect->left < 0)
	{
		pRect->right = pRect->right - pRect->left;
		pRect->left = 0;
	}
	if(pRect->top < 0)
	{
		pRect->bottom = pRect->bottom - pRect->top;
		pRect->top = 0;
	}
}

// retail 0x00849710
void C_Application::MoveWindowToClientRect()
{
	RECT rect;
	GetWindowRectFromClient(&rect);
	SetWindowPos(m_Window.m_hWnd, HWND_NOTOPMOST, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOCOPYBITS | SWP_FRAMECHANGED);
}

// retail: the cursor is hidden in full screen and shown in a window
void C_Application::ShowCursorForWindowMode()
{
	if(!m_Windowed)
	{
		if(m_CursorShown)
		{
			::ShowCursor(FALSE);
			m_CursorShown = false;
		}
	}
	else if(!m_CursorShown)
	{
		::ShowCursor(TRUE);
		m_CursorShown = true;
	}
}

// retail 0x00849860
void C_Application::GetGraphicsOptions(S32* pResolutionIndex, S32* pMultisampleIndex, bool* pWindowed, bool* pVsync) const
{
	*pResolutionIndex = m_ResolutionIndex;
	*pMultisampleIndex = m_MultisampleIndex;
	*pWindowed = m_Windowed;
	*pVsync = m_DisplayDeviceParams.m_PresentInterval == Display::E_PresentInterval_One;
}

// retail 0x0084a200
void C_Application::SetGraphicalDetail(const U32 graphicalDetail)
{
	if(m_GraphicalDetail != graphicalDetail)
	{
		g_debugFrameworkGraphicalDetail.SetValue((int)graphicalDetail);
		m_GraphicalDetail = graphicalDetail;
		ApplyGraphicalDetail(graphicalDetail);
		SaveSettings();
	}
}

// retail ResetDevice (0x0084a240): saves the settings, then recreates the device's volatile resources
bool C_Application::ResetDevice()
{
	SaveSettings();

	if(m_pDisplayDevice->CanReset())
	{
		m_pDisplayDevice->DestroyRenderersVolatile();
		m_pDisplayDevice->DestroyVolatile();

		#if !CFG_SPEED
		BIZ_ASSERT(g_d3dOcclusionQueryCount == 0);
		#endif

		if(!m_pDisplayDevice->CreateVolatile(m_DisplayDeviceParams))
		{
			LOGWARN(g_Log, "Failed to create volatile display device\n");
			return false;
		}

		// [Blur2PC] retail also sets the views' window viewports from the recreated windows (T089)
		const Display::C_BackBuffer& backBuffer = m_pDisplayDevice->GetBackBuffer();
		for(S32 viewIndex = 0; viewIndex < m_SplitscreenViews.Size(); ++viewIndex)
		{
			C_View& splitscreenView = m_SplitscreenViews[viewIndex];
			splitscreenView.m_View.Setup(splitscreenView.m_ViewSetup, backBuffer.GetWidth(), backBuffer.GetHeight());
			SetWindowViewports(splitscreenView, m_pDisplayDevice->GetWindow(viewIndex), m_pDisplayDevice->m_DeviceParams.m_ScreenWidth, m_pDisplayDevice->m_DeviceParams.m_ScreenHeight);
		}

		m_LayerManager.SendMessageToLayers(E_LayerMessage_Resize);

		if(!m_pDisplayDevice->CreateRenderersVolatile())
		{
			LOGWARN(g_Log, "Failed to create volatile display device renderers\n");
		}
	}

	return true;
}

// retail ApplyGraphicsOptions (0x0084ab80)
void C_Application::ApplyGraphicsOptions(const S32 resolutionIndex, const S32 multisampleIndex, const bool windowed, const bool vsync)
{
	bool reset = false;
	bool multisampleChanged = false;

	if(multisampleIndex != m_MultisampleIndex)
	{
		m_MultisampleIndex = multisampleIndex;
		g_debugFrameworkMultisample.SetValue((E_GraphicsOption)multisampleIndex);

		// [Blur2PC] retail: the display windows' multisample (+0x134), then 0x0084a390 (T089)
		const Display::C_MultisampleOption* pMultisample = Display::g_Resolutions.GetMultisample(multisampleIndex);
		m_DisplayDeviceParams.m_WindowBufferMultisample = pMultisample ? pMultisample->m_Multisample : Display::E_Multisample_None;

		reset = true;
		multisampleChanged = true;
		ResizeDisplayWindows();
	}

	if(windowed != m_Windowed)
	{
		g_debugFrameworkWindowed.SetValue(windowed);

		m_Window.m_Style = windowed ? kWindowedStyle : kFullscreenStyle;
		m_Windowed = windowed;
		m_DisplayDeviceParams.m_Windowed = windowed;

		SetWindowLongA(m_Window.m_hWnd, GWL_STYLE, m_Window.m_Style);
		MoveWindowToClientRect();

		const Display::C_Resolution* pResolution = Display::g_Resolutions.GetResolution(m_ResolutionIndex);
		RECT clientRect;
		GetClientRect(m_Window.m_hWnd, &clientRect);
		if(pResolution && ((S32)pResolution->m_Width != clientRect.right - clientRect.left || (S32)pResolution->m_Height != clientRect.bottom - clientRect.top))
		{
			m_TrackWindowSize = false;
		}

		reset = true;
	}

	if((m_DisplayDeviceParams.m_PresentInterval == Display::E_PresentInterval_One) != vsync)
	{
		g_debugFrameworkVsync.SetValue(vsync);
		m_DisplayDeviceParams.m_PresentInterval = vsync ? Display::E_PresentInterval_One : Display::E_PresentInterval_Immediate;
		reset = true;
	}

	bool applyDetail = false;
	if(resolutionIndex == 0)
	{
		// index 0 means "the mode nearest the current size" in full screen; a window keeps its size
		if(!m_Windowed)
		{
			const S32 closest = Display::g_Resolutions.FindClosestResolution(m_Window.m_W, m_Window.m_H);
			m_ResolutionIndex = closest;
			g_debugFrameworkResolution.SetValue((E_GraphicsOption)closest);

			const Display::C_Resolution* pResolution = Display::g_Resolutions.GetResolution(closest);
			if(pResolution)
				SetDisplaySize(pResolution->m_Width, pResolution->m_Height, pResolution->m_RefreshRate);
		}
		applyDetail = multisampleChanged;
	}
	else if(resolutionIndex != m_ResolutionIndex)
	{
		m_ResolutionIndex = resolutionIndex;
		g_debugFrameworkResolution.SetValue((E_GraphicsOption)resolutionIndex);

		const Display::C_Resolution* pResolution = Display::g_Resolutions.GetResolution(resolutionIndex);
		if(pResolution)
		{
			SetDisplaySize(pResolution->m_Width, pResolution->m_Height, pResolution->m_RefreshRate);

			if(m_Windowed)
			{
				MoveWindowToClientRect();

				RECT clientRect;
				GetClientRect(m_Window.m_hWnd, &clientRect);
				if((S32)pResolution->m_Width != clientRect.right - clientRect.left || (S32)pResolution->m_Height != clientRect.bottom - clientRect.top)
				{
					m_TrackWindowSize = false;
				}
			}
		}

		reset = true;
		applyDetail = true;
	}
	else
	{
		applyDetail = multisampleChanged;
	}

	// size-dependent game resources follow the new back buffer
	if(applyDetail)
		ApplyGraphicalDetail(m_GraphicalDetail);

	if(reset)
		ResetDevice();

	ShowCursorForWindowMode();
}

// retail UpdateGraphicsSettings (0x0084bad0), called from BeginRender
void C_Application::UpdateGraphicsSettings()
{
	const bool vsync = m_DisplayDeviceParams.m_PresentInterval == Display::E_PresentInterval_One;

	const U32 graphicalDetail = (U32)(int)g_debugFrameworkGraphicalDetail;
	if(m_GraphicalDetail != graphicalDetail)
	{
		g_debugFrameworkGraphicalDetail.SetValue((int)graphicalDetail);
		m_GraphicalDetail = graphicalDetail;
		ApplyGraphicalDetail(graphicalDetail);
		SaveSettings();
	}

	const S32 resolutionIndex = (S32)(E_GraphicsOption)g_debugFrameworkResolution;
	const S32 multisampleIndex = (S32)(E_GraphicsOption)g_debugFrameworkMultisample;
	const bool windowed = g_debugFrameworkWindowed;
	const bool vsyncVar = g_debugFrameworkVsync;

	if((resolutionIndex == 0 || resolutionIndex == m_ResolutionIndex) && multisampleIndex == m_MultisampleIndex && windowed == m_Windowed && vsyncVar == vsync)
	{
		if(m_Windowed && m_TrackWindowSize)
		{
			RECT clientRect;
			GetClientRect(m_Window.m_hWnd, &clientRect);
			const S32 width = clientRect.right - clientRect.left;
			const S32 height = clientRect.bottom - clientRect.top;

			POINT origin;
			origin.x = 0;
			origin.y = 0;
			ClientToScreen(m_Window.m_hWnd, &origin);

			// remember where the player moved the window (more than 10 pixels)
			if((origin.x > 0 && abs(origin.x - m_Window.m_X) > 10) || (origin.y > 0 && abs(origin.y - m_Window.m_Y) > 10))
			{
				m_Window.m_X = origin.x;
				m_Window.m_Y = origin.y;
				SaveSettings();
			}

			// a resized window becomes the resolution
			if((width != 0 && width != m_Window.m_W) || (height != 0 && height != m_Window.m_H))
			{
				m_ResolutionIndex = 0;
				g_debugFrameworkResolution.SetValue(E_GraphicsOption_First);
				SetDisplaySize(width, height, 0);
				ApplyGraphicalDetail(m_GraphicalDetail);
				ResetDevice();
			}
		}
		return;
	}

	ApplyGraphicsOptions(resolutionIndex, multisampleIndex, windowed, vsyncVar);
}

}; // namespace Framework

LRESULT CALLBACK ApplicationWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(Framework::C_Application::GetInstance())
	{
		return Framework::C_Application::GetInstance()->MyWindowProc(hWnd, uMsg, wParam, lParam);
	}
	else
	{
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

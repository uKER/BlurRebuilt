#include "StdAfx.h"
#include "Samples/System/PCApplication.h"
#include "System/Artery/Artery.h"
#include <system/Perf/ProfilerBackend.h>
#include <system/memory/memorytracker.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//int PreStaticInit()
//{
//	return 0;
//}

namespace Samples
{

C_PCApplication::C_PCApplication() :
	m_PauseUpdate(false),
	m_PauseRender(false),
	m_Minimised(false),
	m_Maximised(false),
	m_InSizeMove(false),
	m_Active(false),
	m_MinimizedWhileFullscreen(false),
	m_Windowed(false),
	m_Capture(false),
	m_HasFocus(true),
	m_WindowResized(false),
	m_pName(NULL),
	m_hDC(NULL),
	m_hWnd(NULL),
	m_hInstance(NULL)
{
	Sys::C_FileSystem::Instance().SetWorkingDirectory("Image\\");

	m_Name = "MercuryTestAppPC";
}

C_PCApplication::~C_PCApplication()
{
//#if(!(defined(USER_LLOYDW) || defined(USER_LEEW))) // [LloydW] Don't link to debug view cos zonealarm doesn't like the debug view on my laptop...
	UnlinkFromDebugView();
//#endif
	DestroyWindow(m_hWnd);
	UnregisterClass(m_pName, m_hInstance);
}

bool C_PCApplication::Create()
{
//#ifndef USER_LLOYDW // [LloydW] Don't link to debug view cos zonealarm doesn't like the debug view on my laptop...
	LinkToDebugView(m_Name.Chars());
//#endif

	m_pName = m_DeviceParams.m_pName;
	m_hInstance = m_DeviceParams.m_hInstance;

	if(!InitialiseClass())
		return false;

	if(!InitialiseWindow())
		return false;

	m_DeviceParams.m_hDC = m_hDC;
	m_DeviceParams.m_hWnd = m_hWnd;
	m_DeviceParams.m_BeginVerticesSize = 4096 * 1024;

	m_Windowed = m_DeviceParams.m_Windowed;

	return C_Application::Create();
}

bool C_PCApplication::InitialiseClass()
{
	WNDCLASS wndClass;
	memset(&wndClass, 0, sizeof(wndClass));
	wndClass.style         = 0;
	wndClass.lpfnWndProc   = WndProc;
	wndClass.cbClsExtra    = 0;
	wndClass.cbWndExtra    = 0;
	wndClass.hInstance     = m_hInstance;
	wndClass.hIcon         = LoadIcon(m_hInstance, IDI_APPLICATION);
	wndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = NULL;
	wndClass.lpszMenuName  = 0;
	wndClass.lpszClassName = m_pName;

	if(RegisterClass(&wndClass))
	{
		return true;
	}
	else
	{
		LOGFATAL(Display::g_Log, "Registering the window class\n");
		return false;
	}
}

bool C_PCApplication::InitialiseWindow()
{
	U32 style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | (m_DeviceParams.m_Windowed ? WS_OVERLAPPEDWINDOW : WS_POPUP);

	m_hWnd = CreateWindow(m_pName, m_pName, style, 0, 0, m_DeviceParams.m_Width, m_DeviceParams.m_Height, NULL, NULL, m_hInstance, NULL);

	if(!m_hWnd)
	{
		return FALSE;
	}

	ShowWindow(m_hWnd, m_DeviceParams.m_CmdShow);
	UpdateWindow(m_hWnd);

	m_hDC = GetDC(m_hWnd);

	return true;
}

void C_PCApplication::Update()
{
	if(m_Capture)
	{
		RECT rect;
		GetWindowRect(m_hWnd, &rect);

		POINT center;
		center.x = (rect.left + rect.right) / 2;
		center.y = (rect.top + rect.bottom) / 2;

		POINT cursor;
		GetCursorPos(&cursor);

		F32 moveX = -(F32)(center.x - cursor.x) * 0.05f;
		F32 moveY  = (F32)(center.y - cursor.y) * 0.05f;

		for(S32 i=0; i<m_GameViewCount; i++)
		{
			if(moveX != 0.f)
				m_GameViews[i].m_Camera.EnableAction(E_CameraAction_Ax, -moveX);
			else
				m_GameViews[i].m_Camera.DisableAction(E_CameraAction_Ax);

			if(moveY != 0.f)
				m_GameViews[i].m_Camera.EnableAction(E_CameraAction_Az, -moveY);
			else
				m_GameViews[i].m_Camera.DisableAction(E_CameraAction_Az);
		}

		SetCursorPos(center.x, center.y);		
	}
	else
	{
		for(S32 i=0; i<m_GameViewCount; i++)
		{
			m_GameViews[i].m_Camera.DisableAction(E_CameraAction_Ax);
			m_GameViews[i].m_Camera.DisableAction(E_CameraAction_Az);
		}
	}

	C_Application::Update();
	
	MyUpdate();

	if(Display::MenuRenderExists() && Display::MenuRender()->GetShow())
	{
		if(m_Input.IsKeyDn(16) && m_Input.IsKeyDn(37))
			Display::MenuRender()->IncX(-2.f);
		if(m_Input.IsKeyDn(16) && m_Input.IsKeyDn(38))
			Display::MenuRender()->IncY(-2.f);
		if(m_Input.IsKeyDn(16) && m_Input.IsKeyDn(39))
			Display::MenuRender()->IncX(2.f);
		if(m_Input.IsKeyDn(16) && m_Input.IsKeyDn(40))
			Display::MenuRender()->IncY(2.f);
		if(m_Input.IsKeyDn(16) && m_Input.IsKeyDn(187))
		{
			Display::MenuRender()->IncW(1.01f);
			Display::MenuRender()->IncH(1.01f);
		}
		if(m_Input.IsKeyDn(16) && m_Input.IsKeyDn(189))
		{
			Display::MenuRender()->IncW(0.99f);
			Display::MenuRender()->IncH(0.99f);
		}
	}
}

void C_PCApplication::KeyDown(const U32 key)
{
	m_Input.KeyDn(key);

	switch(key)
	{
	case VK_ESCAPE:
		SendMessage(m_hWnd, WM_CLOSE, 0, 0);
		break;
	case 'W':
		for(S32 i=0; i<m_GameViewCount; i++)
		m_GameViews[i].m_Camera.EnableAction(E_CameraAction_Fw, 1.f);
		break;
	case 'S':
		for(S32 i=0; i<m_GameViewCount; i++)
		m_GameViews[i].m_Camera.EnableAction(E_CameraAction_Bw, 1.f);
		break;
	case 'A':
		for(S32 i=0; i<m_GameViewCount; i++)
		m_GameViews[i].m_Camera.EnableAction(E_CameraAction_Lt, 1.f);
		break;
	case 'D':
		for(S32 i=0; i<m_GameViewCount; i++)
		m_GameViews[i].m_Camera.EnableAction(E_CameraAction_Rt, 1.f);
		break;
	case 'Q':
		for(S32 i=0; i<m_GameViewCount; i++)
		m_GameViews[i].m_Camera.EnableAction(E_CameraAction_Dn, 1.f);
		break;
	case 'E':
		for(S32 i=0; i<m_GameViewCount; i++)
		m_GameViews[i].m_Camera.EnableAction(E_CameraAction_Up, 1.f);
		break;
	case 37:
		if(!m_Input.IsKeyDn(16))
		{
			if(Display::MenuRenderExists() && Display::MenuRender()->GetShow() && !m_Input.IsKeyDn(16))
				if(m_Input.IsKeyDn(17))
					Display::MenuRender()->GetCurrItem()->Dec();
				else
					Display::MenuRender()->GetCurrItem()->Lt();
			else
				m_pSection->ExitSection();
		}
		break;
	case 38:
		if(!m_Input.IsKeyDn(16))
		{
			if(Display::MenuRenderExists() && Display::MenuRender()->GetShow())
				if(m_Input.IsKeyDn(16))
					Display::MenuRender()->IncY(-2.f);
				else
					Display::MenuRender()->GetCurrItem()->Up();
			else
				m_pSection->PrevSection();
		}
		break;
	case 39:
		if(!m_Input.IsKeyDn(16))
		{
			if(Display::MenuRenderExists() && Display::MenuRender()->GetShow())
				if(m_Input.IsKeyDn(17))
					Display::MenuRender()->GetCurrItem()->Inc();
				else
					Display::MenuRender()->GetCurrItem()->Rt();
			else
				m_pSection->EnterSection();
		}
		break;
	case 40:
		if(!m_Input.IsKeyDn(16))
		{
			if(Display::MenuRenderExists() && Display::MenuRender()->GetShow() && !m_Input.IsKeyDn(16))
				Display::MenuRender()->GetCurrItem()->Dn();
			else
				m_pSection->NextSection();
		}
		break;
	case 'P':
		m_Timer.TogglePaused();
		break;
	case 'M':
		if(Display::MenuRenderExists())
			Display::MenuRender()->SetShow(!Display::MenuRender()->GetShow());
		break;
	case 'C':
		if(m_GameViewCount > 0)
		{
			m_GameViews[0].m_Camera.Save();
		}
		break;
	}

	if(m_pSection)
		m_pSection->InputSection(m_Input, m_Timer);
}

void C_PCApplication::KeyUp(const U32 key)
{
	m_Input.KeyUp(key);

	switch(key)
	{
	case 'W':
		for(S32 i=0; i<m_GameViewCount; i++)
			m_GameViews[i].m_Camera.DisableAction(E_CameraAction_Fw);
		break;
	case 'S':
		for(S32 i=0; i<m_GameViewCount; i++)
			m_GameViews[i].m_Camera.DisableAction(E_CameraAction_Bw);
		break;
	case 'A':
		for(S32 i=0; i<m_GameViewCount; i++)
			m_GameViews[i].m_Camera.DisableAction(E_CameraAction_Lt);
		break;
	case 'D':
		for(S32 i=0; i<m_GameViewCount; i++)
			m_GameViews[i].m_Camera.DisableAction(E_CameraAction_Rt);
		break;
	case 'Q':
		for(S32 i=0; i<m_GameViewCount; i++)
			m_GameViews[i].m_Camera.DisableAction(E_CameraAction_Dn);
		break;
	case 'E':
		for(S32 i=0; i<m_GameViewCount; i++)
			m_GameViews[i].m_Camera.DisableAction(E_CameraAction_Up);
		break;
	}
}

void C_PCApplication::MouseDown()
{
	//assert(!m_Capture);
	m_Capture = true;

	ShowCursor(false);
	GetCursorPos(&m_Cursor);

	RECT windowRect;
	GetWindowRect(m_hWnd, &windowRect);

	POINT windowCenter;
	windowCenter.x = (windowRect.left + windowRect.right) / 2;
	windowCenter.y = (windowRect.top + windowRect.bottom) / 2;

	SetCursorPos(windowCenter.x, windowCenter.y);
}

void C_PCApplication::MouseUp()
{
	//assert(m_Capture);
	m_Capture = false;
	SetCursorPos(m_Cursor.x, m_Cursor.y);	

	ShowCursor(true);
}

void C_PCApplication::Run()
{
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
			Debug::NetworkUpdate();

			Update();
			Render();
		}
	}
}

LRESULT C_PCApplication::MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//LOGINFO(g_Log, "C_PCApplication::MsgProc - message %i wParam %i lParam %i\n", message, wParam, lParam);

	switch(message) 
	{
	case WM_PAINT:
		{
			if(m_pDevice && m_pDevice->CanRender())
			{
				C_Application::Render();
			}
		}
		break;
	case WM_KEYDOWN:
		{
			KeyDown((U32)wParam);
		}
		break;
	case WM_KEYUP:
		{
			KeyUp((U32)wParam);
		}
		break;
	 case WM_LBUTTONDOWN:
		{
			MouseDown();
		}
		break;
	case WM_LBUTTONUP:
		{
			MouseUp();
		}
		break;
	case WM_SIZE:
		{
			if(wParam == SIZE_MINIMIZED)
			{
				m_PauseUpdate = true;
				m_PauseRender = true;
				LOGINFO(g_Log, "C_PCApplication::MsgProc - Pausing app\n");

				m_Minimised = true;
				m_Maximised = false;

				LOGINFO(g_Log, "C_PCApplication::MsgProc - SIZE_MINIMIZED\n");
			}
			else
            {
				RECT rect;
				GetClientRect(m_hWnd, &rect);
				if(rect.top == 0 && rect.bottom == 0)
				{
				}
				else if(wParam == SIZE_MAXIMIZED)
				{
					if(m_Minimised)
					{
						m_PauseUpdate = false;
						m_PauseRender = false;
						LOGINFO(g_Log, "C_PCApplication::MsgProc - Unpausing app\n");
					}

					m_Minimised	= false;
					m_Maximised	= true;

					// TODO: Check for window resize...

					LOGINFO(g_Log, "C_PCApplication::MsgProc - SIZE_MAXIMIZED\n");
				}
				else if(wParam == SIZE_RESTORED)
				{
					if(m_Maximised)
					{
						m_Maximised = false;

						// TODO: Check for window resize...

						LOGINFO(g_Log, "C_PCApplication::MsgProc - SIZE_RESTORED - m_Maximised\n");
					}
					else if(m_Minimised)
					{
						m_PauseUpdate = false;
						m_PauseRender = false;
						LOGINFO(g_Log, "C_PCApplication::MsgProc - Unpausing app\n");

						m_Minimised = false;

						// TODO: Check for window resize...

						LOGINFO(g_Log, "C_PCApplication::MsgProc - SIZE_RESTORED - m_Minimised\n");
					}
					else if(m_InSizeMove)
					{
					}
					else
					{
						// TODO: Check for window resize...
					}
				}
			}
		}
		break;
	 case WM_ENTERSIZEMOVE:
		 {
			 m_PauseUpdate = true;
			 m_PauseRender = true;
			 LOGINFO(g_Log, "C_PCApplication::MsgProc - Pausing app\n");

			 m_InSizeMove = true;
		 }
		 break;
	 case WM_EXITSIZEMOVE:
		 {
			 m_PauseUpdate = false;
			 m_PauseRender = false;
			 LOGINFO(g_Log, "C_PCApplication::MsgProc - Unpausing app\n");

			 m_InSizeMove = false;

			 // TODO: Check for window resize...
		 }
		 break;
	  case WM_ACTIVATEAPP:
		  {
			  if(wParam == TRUE && !m_Active)
			  {
				  m_Active = true;

				  if(m_MinimizedWhileFullscreen)
				  {
					  m_PauseUpdate = false;
					  m_PauseRender = false;
					  LOGINFO(g_Log, "C_PCApplication::MsgProc - Unpausing app\n");

					  m_MinimizedWhileFullscreen = false;

					  LOGINFO(g_Log, "C_PCApplication::MsgProc - WM_ACTIVATEAPP - wParam == TRUE - m_MinimizedWhileFullscreen\n");
				  }

				  // TODO: Disable shortcut keys...

				  LOGINFO(g_Log, "C_PCApplication::MsgProc - WM_ACTIVATEAPP - wParam == TRUE\n");
			  }
			  else
			  if(wParam == FALSE && m_Active)
			  {
				  m_Active = false;

				  if(!m_Windowed)
				  {
						m_PauseUpdate = true;
						m_PauseRender = true;
						LOGINFO(g_Log, "C_PCApplication::MsgProc - Pausing app\n");

						m_MinimizedWhileFullscreen = true;

						LOGINFO(g_Log, "C_PCApplication::MsgProc - WM_ACTIVATEAPP - wParam == FALSE - m_MinimizedWhileFullscreen\n");
				  }

				  // TODO: Enable shortcut keys...

				  LOGINFO(g_Log, "C_PCApplication::MsgProc - WM_ACTIVATEAPP - wParam == FALSE - m_MinimizedWhileFullscreen\n");
			  }
		  }
		  break;
		case WM_ENTERMENULOOP:
			{
				m_PauseUpdate = true;
				m_PauseRender = true;
				LOGINFO(g_Log, "C_PCApplication::MsgProc - Pausing app\n");
			}
			break;
		case WM_EXITMENULOOP:
			{
				m_PauseUpdate = false;
				m_PauseRender = false;
				LOGINFO(g_Log, "C_PCApplication::MsgProc - Unpausing app\n");
			}
			break;
	/*case WM_SIZE:
		{
			switch (wParam)
			{
			case SIZE_RESTORED:
				LOGINFO(g_Log, "C_PCApplication::MsgProc - SIZE_RESTORED\n");
				//if(m_pDevice)
					//Resize();
				break;
			case SIZE_MINIMIZED:
				LOGINFO(g_Log, "C_PCApplication::MsgProc - SIZE_MINIMIZED\n");
				break;
			case SIZE_MAXIMIZED:
				LOGINFO(g_Log, "C_PCApplication::MsgProc - SIZE_MAXIMIZED\n");
				if(m_pDevice)
					Resize();
				break;
			}
		}
		break;
	case WM_ENTERSIZEMOVE:
		{
			//LOGINFO(g_Log, "C_PCApplication::MsgProc - WM_ENTERSIZEMOVE\n");
		}
		break;
	case WM_EXITSIZEMOVE:
		{
			//LOGINFO(g_Log, "C_PCApplication::MsgProc - WM_EXITSIZEMOVE\n");
			if(m_pDevice)
				Resize();
		}
		break;
	case WM_KEYDOWN:
		{
			KeyDown((U32)wParam);
		}
		break;
	case WM_KEYUP:
		{
			KeyUp((U32)wParam);
		}
		break;
	 case WM_LBUTTONDOWN:
		{
			MouseDown();
		}
		break;
	case WM_LBUTTONUP:
		{
			MouseUp();
		}
		break;
	case 522:
		{
			if((S32)wParam > 0)
				m_GameViews[0].m_Camera.m_Speed *= 1.2f;
			else
				m_GameViews[0].m_Camera.m_Speed *= 0.8f;
		}
		break;
	case WM_ACTIVATE:
		{
			switch(wParam)
            {
				case WA_INACTIVE:
					LOGINFO(g_Log, "C_PCApplication::MsgProc - WA_INACTIVE\n");
					//FocusChange(false);
					break;
				case WA_ACTIVE:
					LOGINFO(g_Log, "C_PCApplication::MsgProc - WA_ACTIVE\n");
					FocusChange(true);
					break;
				case WA_CLICKACTIVE:
					LOGINFO(g_Log, "C_PCApplication::MsgProc - WA_CLICKACTIVE\n");
					FocusChange(true);
					break;
			}
		}
		break;
	case WM_SETFOCUS:
		{
			LOGINFO(g_Log, "C_PCApplication::MsgProc - WM_SETFOCUS\n");
			//FocusChange(true);
		}
		break;
	case WM_KILLFOCUS:
		{
			LOGINFO(g_Log, "C_PCApplication::MsgProc - WM_KILLFOCUS\n");
			//FocusChange(false);
		}
		break;*/
	case WM_CLOSE:
        {
			Destroy();

			if(m_hWnd)
			{
				DestroyWindow(m_hWnd);
				m_hWnd = NULL;
			}

			if(m_hInstance)
			{
				UnregisterClass(m_pName, m_hInstance);
				m_hInstance = NULL;
			}
		}
		break;
	case WM_DESTROY:
		{
			 PostQuitMessage(0);
		}
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

bool C_PCApplication::ResetDevice()
{
	m_pDevice->DestroyVolatile();
	m_pDevice->DestroyRenderersVolatile();

	if(m_DeviceParams.m_Windowed)
	{
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		
		m_DeviceParams.m_Width = rect.right - rect.left;
		m_DeviceParams.m_Height = rect.bottom - rect.top;
	}

	if(!m_pDevice->CreateVolatile(m_DeviceParams))
	{
		return false;
	}

	// setup views again because window dimensions might have changed!
	SetupViews();

	// set up view in device as well because section resize might query it
	for(S32 i=0; i<m_GameViewCount; ++i)
	{
		m_pDevice->SetupViewIndex(m_GameViews[i].m_ViewIndex, m_GameViews[i].m_ViewSetup);
	}

	// deal with any resizing in the section (eg full screen RTs, post effects)
	m_pSection->Resize();

	m_pDevice->CreateRenderersVolatile();

	return true;
}

void C_PCApplication::FocusChange(const bool gotFocus)
{
	if(m_HasFocus)
	{
		if(!gotFocus)
		{
			m_HasFocus = false;
		}
	}
	else
	{
		if(gotFocus)
		{
			m_HasFocus = true;
		}
	}
}

void C_PCApplication::Render()
{
	assert(m_pDevice);
#if 1
	if(m_Windowed)
	{
		if(m_PauseRender || !m_Active)
		{
			Sleep(50);
		}
	}
	else
	{    
		if(m_PauseRender || !m_Active)
		{
			Sleep(50);
			return;
		}
	}

	if(!m_PauseRender)
	{
		if(m_DeviceParams.m_Windowed)
		{
			RECT rect;
			GetClientRect(m_hWnd, &rect);

			U32 newWidth = rect.right - rect.left;
			U32 newHeight = rect.bottom - rect.top;
			
			if(newWidth != m_DeviceParams.m_Width || newHeight != m_DeviceParams.m_Height)
			{
				m_WindowResized = true;

				m_DeviceParams.m_Width = newWidth;
				m_DeviceParams.m_Height = newHeight;
			}
		}

		if(m_pDevice->GetLost() || m_WindowResized)
		{
			if(m_pDevice->CanReset())
			{
				if(!ResetDevice())
				{
					return;
				}

				m_WindowResized = false;
			}
			else
			{
				return;
			}
		}

		if(m_pDevice->CanRender())
		{
			C_Application::Render();
		}
	}
#else
	if(m_pDevice->GetLost() || m_PauseRender || !m_Active)
	{
		Sleep(50);
	}
	
	if(!m_PauseRender)
	{
		if(m_pDevice->GetLost())
		{
			if(m_pDevice->CanReset())
			{
				if(!ResetDevice())
				{
					return;
				}
			}
		}
		else
		{
			if(m_pDevice->CanRender())
			{
				C_Application::Render();
			}
		}
	}
#endif
}

}; // namespace System

void GameLoadDebugVariables()
{
}

void GameSaveDebugVariables()
{
}

void LinkToDebugView(const char* name)
{
	WSADATA WsaData;
	WSAStartup(WINSOCK_VERSION, &WsaData);

	ArteryStartup(true, name);

	Debug::NetworkStart();
	
	Memory::InitialiseNetworkConnection();

	Perf::InitialiseNetworkConnection();
}

void UnlinkFromDebugView()
{
	ArteryCleanup();
	WSACleanup();
}

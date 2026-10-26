#include "StdAfx.h"
#include "ApplicationPC.h"

LRESULT CALLBACK ApplicationWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

C_Application* g_pApplication = NULL;

C_ApplicationParams::C_ApplicationParams()
{
	m_BackBufferWidth		= 640;
	m_BackBufferHeight		= 480;
	m_Windowed				= true;
	m_hInstance				= NULL;
}

C_Application::C_Application()
{
	m_pMouse				= NULL;
	m_pKeyboard				= NULL;
	m_ClassAtom				= 0;
	m_ClassName				= "Default Class Name";
	m_WindowName			= "Default Window Name";
	m_WindowStyle			= 0;
	m_WindowX				= 0;
	m_WindowY				= 0;
	m_WindowWidth			= 0;
	m_WindowHeight			= 0;
	m_hDC					= NULL;
	m_hWnd					= NULL;
	m_hMenu					= NULL;
	m_hInstance				= NULL;

	assert(g_pApplication == NULL);
	g_pApplication = this;
}

C_Application::~C_Application()
{
	assert(g_pApplication == this);
	g_pApplication = NULL;
}

bool C_Application::Create(C_ApplicationParams& params)
{
	m_hInstance = params.m_hInstance;

	HRESULT hr = InitialiseClass(params);
	if(FAILURE(hr))
		return false;

	hr = InitialiseWindow(params);
	if(FAILURE(hr))
		return false;

	m_DeviceParams.m_Width			= params.m_BackBufferWidth;
	m_DeviceParams.m_Height			= params.m_BackBufferHeight;
	m_DeviceParams.m_hWnd			= m_hWnd;
	m_DeviceParams.m_hInstance		= m_hInstance;

	m_pMouse = new C_Mouse(0, &m_InputManager);
	m_pKeyboard = new C_Keyboard(0, &m_InputManager);

	m_pMouse->m_hWnd = m_hWnd;

	return C_ApplicationBase::Create();
}

void C_Application::Destroy()
{
	SAFE_DELETE_PTR(m_pMouse);
	SAFE_DELETE_PTR(m_pKeyboard);
	
	C_ApplicationBase::Destroy();

	DestroyWindow(m_hWnd);
	
	UnregisterClass(m_ClassName.Chars(), m_hInstance);
}

void C_Application::Update()
{
	m_pMouse->Update();

	m_pKeyboard->Update();

	m_Camera.Update(m_pKeyboard, m_pMouse, m_Timer.m_FrameTime);

	C_ApplicationBase::Update();
}

void C_Application::Run()
{
	MSG msg;
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
			Update();
			Render();
		}
	}
}

HRESULT C_Application::InitialiseClass(C_ApplicationParams& params)
{
	memset(&m_ClassEx, 0, sizeof(m_ClassEx));

	m_ClassEx.cbSize        		= sizeof(m_ClassEx); 
	m_ClassEx.style         		= 0;
	m_ClassEx.lpfnWndProc   		= (WNDPROC)ApplicationWindowProc;
	m_ClassEx.cbClsExtra    		= 0;
	m_ClassEx.cbWndExtra    		= 0;
	m_ClassEx.hInstance     		= m_hInstance;
	m_ClassEx.hIcon         		= NULL;
	m_ClassEx.hCursor       		= NULL;
	m_ClassEx.hbrBackground 		= (HBRUSH)(COLOR_WINDOW+1);
	m_ClassEx.lpszMenuName  		= NULL;
	m_ClassEx.lpszClassName 		= m_ClassName.Chars();
	m_ClassEx.hIconSm				= NULL;

	m_ClassAtom = RegisterClassEx(&m_ClassEx);

	if(m_ClassAtom != 0)
	{
		return S_OK;
	}
	else
	{
		return S_FALSE;
	}
}

HRESULT C_Application::InitialiseWindow(C_ApplicationParams& params)
{
	m_WindowWidth	= params.m_BackBufferWidth;
	m_WindowHeight	= params.m_BackBufferHeight;
	m_WindowStyle	= WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | (params.m_Windowed ? WS_OVERLAPPEDWINDOW : WS_POPUP);

	m_hWnd = CreateWindow(
		m_ClassName.Chars(),
		m_WindowName.Chars(),
		m_WindowStyle,
		m_WindowX,
		m_WindowY,
		m_WindowWidth,
		m_WindowHeight,
		NULL,
		m_hMenu,
		m_hInstance,
		NULL);

	if(!m_hWnd)
		return S_FALSE;

	ShowWindow(m_hWnd, params.m_Windowed ? SW_SHOW : SW_HIDE);

	UpdateWindow(m_hWnd);

	m_hDC = GetDC(m_hWnd);

	return S_OK;
}

LRESULT C_Application::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(hWnd == m_hWnd)
	{
		switch(uMsg)
		{
			case WM_LBUTTONDOWN:
			{
				m_pMouse->MouseDown(E_MouseButton_Left);
			}
			break;

			case WM_LBUTTONUP:
			{
				m_pMouse->MouseUp(E_MouseButton_Left);
			}
			break;

			case WM_KEYDOWN:
			{
				switch(wParam)
				{
				case 27:
					PostQuitMessage(0);
					break;
				default:
					m_pKeyboard->KeyDown(wParam);
					break;
				}
			}
			break;

			case WM_KEYUP:
			{
				m_pKeyboard->KeyUp(wParam);
			}
			break;

			case 522: // Mouse wheel
			{
				if((S32)wParam > 0)
					m_pMouse->WheelUp();
				else
					m_pMouse->WheelDown();
			}
			break;

			case WM_DESTROY:
			{
				PostQuitMessage(0);
			}
			break;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK ApplicationWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(g_pApplication)
	{
		return g_pApplication->MsgProc(hWnd, uMsg, wParam, lParam);
	}
	else
	{
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

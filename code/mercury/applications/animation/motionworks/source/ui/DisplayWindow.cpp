#include "stdafx.h"

#include "DisplayWindow.h"
#include "../Camera.h"
#include "../PreviewSection.h"

#include <System/Debug/Log/DebugLog.h>

// generic rendering
#include "Render/Render/GridRender.h"

// debug rendering
#include "Render/Debug/DebugRender.h"
#include "Render/Debug/ImmediateDebugRender.h"
#include <Render/Font/Font.h>

// for debug view
#include "System/Artery/Artery.h"

#include <system/perf/ProfilerBackend.h>


// C_DisplayWindow

IMPLEMENT_CLASS(C_DisplayWindow, wxWindow)

BEGIN_EVENT_TABLE(C_DisplayWindow, wxWindow)
  EVT_MOUSE_EVENTS(C_DisplayWindow::OnMouseEvent)
  EVT_MOUSE_CAPTURE_LOST(C_DisplayWindow::OnMouseCaptureLostEvent)
  EVT_KEY_DOWN(C_DisplayWindow::OnKeyDownEvent)
  EVT_KEY_UP(C_DisplayWindow::OnKeyUpEvent)
  EVT_KILL_FOCUS(C_DisplayWindow::OnKillFocusEvent)
  //EVT_TIMER(wxID_ANY, C_DisplayWindow::OnTimer)
  EVT_IDLE(C_DisplayWindow::OnIdle)
  EVT_SIZE(C_DisplayWindow::OnSize)
  EVT_PAINT(C_DisplayWindow::OnPaint)
END_EVENT_TABLE()

C_DisplayWindow::C_DisplayWindow( C_Camera& camera, wxWindow *parent, wxWindowID id,
                    const wxPoint &pos, const wxSize &size )
        : wxWindow( parent, id, pos, size, wxSUNKEN_BORDER | wxCLIP_CHILDREN ),
        m_camera(camera)
{

	SetBackgroundStyle( wxBG_STYLE_CUSTOM );

	// create display manager
	m_pDisplay = new Display::C_Manager();
	
	// create display device
	m_pDevice = new Display::C_Device();
	
	m_pFontSystem = new Render::Font::C_FontSystem;
	Render::Font::FontSystem( m_pFontSystem );

	// create display library debug renderer
	m_pDebugRender = new Display::C_DebugRender();

	// create render library debug renderer
	m_pDebugRenderSys = new Render::Debug::C_DebugRenderSys;
	DbUtil::DefaultDebugRenderSys(m_pDebugRenderSys);

	m_pViewRenderer = NULL;

	m_MouseSensitivity = 0.4f;
	m_MouseButtonDown = -1;

	// set rendering to occur every at 100hz by default
	//m_UpdateTimer.SetOwner(this, wxID_ANY);
	SetRefreshRate(100);

	m_Initialised = false;


//	LinkToDebugView();
}

C_DisplayWindow::~C_DisplayWindow()
{
	DestroyRenderers();

	delete m_pDebugRenderSys;
	DbUtil::DefaultDebugRenderSys(NULL);

	delete m_pDebugRender;

	delete m_pFontSystem;

	if(m_pDevice->IsCreated())
		m_pDisplay->DestroyDevice(m_pDevice);

	m_pDisplay->Shutdown();
	delete m_pDisplay;
	m_pDisplay = NULL;
}

// initialisation functions

bool C_DisplayWindow::Create()
{
	if(m_pDevice->IsCreated())
		return false;

	if(!m_pDisplay->Initialise())
		return false;

	Display::C_VideoMode videoMode;
	if(!m_pDisplay->GetVideoMode(&videoMode))
		return false;

	HWND hwnd = (HWND)GetHandle();
	
	// set width and height to zero to tell D3D to pick the size itself
	m_DeviceParams.m_Width = 0;
	m_DeviceParams.m_Height = 0;
	m_DeviceParams.m_Multisample = Display::E_Multisample_4;
	m_DeviceParams.m_hWnd = hwnd;
	m_DeviceParams.m_BeginVerticesSize = 2048 * 1024;

	if(!Display::Manager()->CreateDevice(m_pDevice, m_DeviceParams))
		return false;

	// create renderers
	if(!m_pDevice->CreateRenderersVolatile())
		return false;

	if(!m_pDevice->CreateRenderers())
		return false;

	// add a default view
	Display::C_View view;
	m_ViewIndex = m_pDevice->AddView(Display::E_ViewType_2D | Display::E_ViewType_3D, 0);
	
	// set up view so it has the correct viewport dimensions etc
	m_pDevice->SetupViewIndex(m_ViewIndex, m_ViewSetup);
	m_pDevice->SetViewIndex(m_ViewIndex);
	
	m_Initialised = OnCreate();

	m_PrevTime = Time::GetPerformanceCounter();
	m_FrameTime = 0.f;

	return m_Initialised;
}

void C_DisplayWindow::DestroyRenderers()
{
	m_pDevice->DestroyRenderersVolatile();
	m_pDevice->DestroyRenderers();
}

bool C_DisplayWindow::SetSection(C_PreviewSection* pSection)
{
#if 1
	bool bSuccess = true;
	m_pViewRenderer = pSection;
	bSuccess = bSuccess && m_pDevice->CreateRenderersVolatile();
	bSuccess = bSuccess && m_pDevice->CreateRenderers();
	
	return bSuccess;


#else
	bool bSuccess = true;
	if(m_pSection)
	{
		m_pSection->DestroyVolatile();
		m_pSection->Destroy();
	}

	// create any new renderers which need creating
	bSuccess = bSuccess && m_pDevice->CreateRenderersVolatile();
	bSuccess = bSuccess && m_pDevice->CreateRenderers();

	if(bSuccess)
		m_pSection = pSection;

	return bSuccess;
#endif

	return true;
}

bool C_DisplayWindow::OnCreate()
{
	return true;
}

// render functions

void C_DisplayWindow::OnRender() const
{
	// this function is overriden by derived class to perform custom rendering
}

void C_DisplayWindow::Render() const
{
	PERF_EVENT( "C_DisplayWindow.Render" );
	if(!m_Initialised)
		return;
		
	if(!m_pDevice->IsCreated())
		return;

	m_pDevice->SetView(NULL, false);
	m_pDevice->BeginScene();
	m_pDevice->SetupViewIndex(m_ViewIndex, m_ViewSetup);
	m_pDevice->SetViewIndex(m_ViewIndex);

	// if we are using Samples::C_Section then call render here
	if(m_pViewRenderer)
	{
		m_pViewRenderer->OnRender();
	}
	else
	{
		m_pDevice->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(128, 128, 128, 255));

		// render stuff here
		{
			Render::DrawGrid(Maths::RGBA(192, 192, 192, 255));

			// draw an axis at the orbit point
			//m_View.m_Camera.Render();

			OnRender();
			
			DbUtil::DefaultDebugRenderSys()->RenderDeferred();
			m_pDebugRender->Draw();
		}
	}

	m_pDevice->SetView(NULL, false);
	m_pDevice->EndScene();

	// its possible we lost device while renderering, skip present if so
	if(m_pDevice->GetD3DDevice()->TestCooperativeLevel() == D3D_OK)
		m_pDevice->Present();
}

// update functions

void C_DisplayWindow::OnUpdate()
{
	// this function is overriden by derived class to perform custom update
}


void C_DisplayWindow::OnIdle( wxIdleEvent& e )
{
	#ifndef CFG_DEBUG
	if ( wxTheApp->IsActive() )
	#endif
	{
		Debug::NetworkUpdate();
		UpdateAndRender();
		
		e.RequestMore();
	}
}

void C_DisplayWindow::OnPaint( wxPaintEvent& e )
{
	wxPaintDC dc(this);
	
	if (m_pDevice->IsCreated())
	{
		PERF_EVENT("C_DisplayWindow.OnPaint");
		HRESULT hr = m_pDevice->GetD3DDevice()->TestCooperativeLevel();

		// if ok then render, otherwise try and try and reset if we are not lost
		if(hr == S_OK)
			Render();
		else if(hr == D3DERR_DEVICENOTRESET)
			Reset();
	}
	
	Perf::Profiler::Instance().EndFrame();
}

//void C_DisplayWindow::OnTimer(wxTimerEvent& event)
//{
//	Debug::NetworkUpdate();
//
//	UpdateAndRender();
//}

void C_DisplayWindow::UpdateAndRender()
{
	{
		PERF_EVENT("C_DisplayWindow.UpdateAndRender.Update");

		Time::C_Time currTime = Time::GetPerformanceCounter();

		m_FrameTime = (F32)(currTime - m_PrevTime) / (F32)SYS_USEC_PER_SEC;
		m_PrevTime = currTime;

		const F32 kMaxFrameTime = 1.f / 30.f;
		m_FrameTime = Maths::Min(m_FrameTime, kMaxFrameTime);

		OnUpdate();
		
		m_camera.SetupView( m_ViewSetup );
	}
#if 1

	Refresh( false );
	Update();
	//Perf::Profiler::Instance().EndFrame();
	
#else
		
	if (m_pDevice->IsCreated())
	{
		PERF_EVENT("C_DisplayWindow.UpdateAndRender.Render");
		HRESULT hr = m_pDevice->GetD3DDevice()->TestCooperativeLevel();

		// if ok then render, otherwise try and try and reset if we are not lost
		if(hr == S_OK)
			Render();
		else if(hr == D3DERR_DEVICENOTRESET)
			Reset();
	}
		
	Perf::Profiler::Instance().EndFrame();
#endif
}

void C_DisplayWindow::SetRefreshRate(S32 refreshRate)
{
	if(refreshRate > 0)
	{
		m_RefreshMs = 1000 / refreshRate;
		m_RefreshRate = 1000 / m_RefreshMs;
		m_RefreshDelta = 1.0f / (F32)m_RefreshRate;

		//if(m_UpdateTimer.IsRunning())
		//	m_UpdateTimer.Stop();

		//m_UpdateTimer.Start(m_RefreshMs);
	}
}

// input functions

void C_DisplayWindow::OnMouseEvent(wxMouseEvent& event)
{
	wxRect clientRect( GetClientRect() );
	wxPoint clientPos = event.GetPosition(); // ScreenToClient( event.GetPosition() );
	bool insideClientRect = clientRect.Contains( clientPos );

	if ( wxTheApp->IsActive() )
	{
		//SetFocus();
	}
	wxPoint point = event.GetPosition();
	
	if ( insideClientRect && m_MouseButtonDown == -1)
	{
		if(event.LeftDown())
		{
			CaptureMouseCursor(0, point);
			event.Skip();
		}
		else if(event.MiddleDown())
		{
			CaptureMouseCursor(1, point);
		}
		else if(event.RightDown())
		{
			CaptureMouseCursor(2, point);
		}
		else
		{
			event.Skip();
		}
	}
	else if(event.Dragging() && m_MouseButtonDown > -1)
	{
	#if 0
		wxRect windowRect = GetClientRect();
		wxPoint windowCenter;
		windowCenter.x = windowRect.GetX() + (windowRect.GetWidth() / 2);
		windowCenter.y = windowRect.GetY() + (windowRect.GetHeight() / 2);

		S32 diffX = windowCenter.x - point.x;
		S32 diffY = windowCenter.y - point.y;

		if(diffX < 0)
			++diffX;
		else if(diffX > 0)
			--diffX;

		if(diffY < 0)
			++diffY;
		else if(diffY > 0)
			--diffY;

		F32 moveX = -(F32)diffX * m_MouseSensitivity;
		F32 moveY  = (F32)diffY * m_MouseSensitivity;

		// left down, do rotation
		if(m_MouseButtonDown == 0)
		{
			if(moveX != 0.f)
				m_camera.EnableInput(E_CameraInput_RotateLR, -moveX);
			else
				m_camera.DisableInput(E_CameraInput_RotateLR);

			if(moveY != 0.f)
				m_camera.EnableInput(E_CameraInput_RotateUD, -moveY);
			else
				m_camera.DisableInput(E_CameraInput_RotateUD);
		}
		else if(m_MouseButtonDown == 1)
		{
			if(moveX != 0.f)
				m_camera.EnableInput(E_CameraInput_MoveLR, moveX);
			else
				m_camera.DisableInput(E_CameraInput_MoveLR);

			if(moveY != 0.f)
				m_camera.EnableInput(E_CameraInput_MoveUD, moveY);
			else
				m_camera.DisableInput(E_CameraInput_MoveUD);
		}
		else if(m_MouseButtonDown == 2)
		{
			//if(m_camera.GetType() != E_CameraType_Orbit)
			//{
			//	if(moveX != 0.f)
			//		m_View.m_Camera.EnableAction(Samples::E_CameraAction_Tx, moveX);
			//	else
			//		m_View.m_Camera.DisableAction(Samples::E_CameraAction_Tx);
			//}

			if(moveY != 0.f)
				m_camera.EnableInput(E_CameraInput_Zoom, moveY);
			else
				m_camera.DisableInput(E_CameraInput_Zoom);
		}

		WarpPointer(windowCenter.x, windowCenter.y);
		
	#endif
		UpdateAndRender();
	}
	else if(( event.LeftUp() || event.MiddleUp() || event.RightUp() ) )
	{
		//WarpPointer(m_CursorPosition.x, m_CursorPosition.y);
		ReleaseMouseCursor();
	}
	else
	{
		event.Skip();
	}

	m_MousePoint = point;
}

void C_DisplayWindow::CaptureMouseCursor(S32 mouseButton, wxPoint point)
{
	CaptureMouse();
	m_MouseButtonDown = mouseButton;
	
	m_CursorPosition = point;

	wxRect windowRect = GetClientRect();
	wxPoint windowCenter;
	windowCenter.x = windowRect.GetX() + (windowRect.GetWidth() / 2);
	windowCenter.y = windowRect.GetY() + (windowRect.GetHeight() / 2);

	//WarpPointer(windowCenter.x, windowCenter.y);
	//ShowCursor(false);
}

void C_DisplayWindow::ReleaseMouseCursor()
{
	if(m_MouseButtonDown != -1)
	{
		// make sure mouse controlled camera actions are disabled
		//m_camera.DisableAllInputs();

		//ShowCursor(true);
		m_MouseButtonDown = -1;
		#ifdef __WXMSW__
			// Under Windows we can be pretty sure this test will give
			// the correct results
		if (wxWindow::GetCapture() == this)
		#endif
			ReleaseMouse();
	}
}

void C_DisplayWindow::OnMouseCaptureLostEvent(wxMouseCaptureLostEvent& event)
{
	ReleaseMouseCursor();
}

void C_DisplayWindow::OnKeyDownEvent(wxKeyEvent& event)
{
	event.Skip();
}

void C_DisplayWindow::OnKeyUpEvent(wxKeyEvent& event)
{
	event.Skip();
}

void C_DisplayWindow::OnKillFocusEvent(wxFocusEvent& event)
{
	// kill all camera movement
	m_camera.DisableAllInputs();

	ReleaseMouseCursor();
}

// resize and reset functions

void C_DisplayWindow::OnSize(wxSizeEvent& event)
{
	wxSize size = event.GetSize();

	// update the size of the back buffer and reset device
	if(abs(((S32)m_DeviceParams.m_Width - size.x)) > 0 || abs(((S32)m_DeviceParams.m_Height - size.y)) > 0)
	{
		Reset();
	}
}

bool C_DisplayWindow::Reset()
{
	m_pDevice->DestroyVolatile();
	m_pDevice->DestroyRenderersVolatile();

	wxSize clientSize = GetClientSize();

	S32 w = Maths::Max( clientSize.GetWidth(), 16 );
	S32 h = Maths::Max( clientSize.GetHeight(), 16 );

	m_DeviceParams.m_Width = w;
	m_DeviceParams.m_Height = h;

	m_pDevice->CreateVolatile(m_DeviceParams);
	m_pDevice->CreateRenderersVolatile();
	
	m_camera.SetAspectRatio( (F32)m_DeviceParams.m_Width / (F32)m_DeviceParams.m_Height );
	m_ViewSetup.m_ViewSetup3D.m_NearClip = 0.01f;
	m_ViewSetup.m_ViewSetup3D.m_FarClip = 500.f;
	
	m_ViewSetup.m_ViewSetup2D.m_ViewExtents.SetDimensions(0.f, 0.f, w, h);

	// set up view so it has the correct viewport dimensions etc
	m_pDevice->SetupViewIndex(m_ViewIndex, m_ViewSetup);
	m_pDevice->SetViewIndex(m_ViewIndex);

	OnResize();

	return true;
}




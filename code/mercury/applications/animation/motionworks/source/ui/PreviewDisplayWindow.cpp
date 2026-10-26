#include "stdafx.h"
#include "PreviewDisplayWindow.h"
#include "../PreviewState.h"
#include "Frame.h"
#include "../Db.h"
#include "../GlobalSignals.h"

IMPLEMENT_CLASS(C_PreviewDisplayWindow, C_DisplayWindow)

C_PreviewDisplayWindow::C_PreviewDisplayWindow( C_Camera& camera,
	wxWindow *parent,
	wxWindowID winId,
	const wxPoint &pos,
	const wxSize &size ):
	C_DisplayWindow(camera, parent, winId, pos, size)
{

	Connect( wxEVT_KEY_DOWN, wxKeyEventHandler(C_PreviewDisplayWindow::OnKeyDown ) );
	Connect( wxEVT_KEY_UP, wxKeyEventHandler(C_PreviewDisplayWindow::OnKeyUp ) );
	

	#define CONNECT_MOUSE(evt, x)	Connect(evt, wxMouseEventHandler(x));
	#define CONNECT_ANY_MOUSE(x)	CONNECT_MOUSE(wxEVT_LEFT_DOWN,x); \
		CONNECT_MOUSE(wxEVT_LEFT_UP, x); \
		CONNECT_MOUSE(wxEVT_MIDDLE_DOWN,x); \
		CONNECT_MOUSE(wxEVT_MIDDLE_UP,x); \
		CONNECT_MOUSE(wxEVT_RIGHT_DOWN,x); \
		CONNECT_MOUSE(wxEVT_RIGHT_UP,x); \
		CONNECT_MOUSE(wxEVT_MOTION,x); \
		CONNECT_MOUSE(wxEVT_LEFT_DCLICK,x) \
		CONNECT_MOUSE(wxEVT_MIDDLE_DCLICK,x) \
		CONNECT_MOUSE(wxEVT_RIGHT_DCLICK,x) \
		CONNECT_MOUSE(wxEVT_MOUSEWHEEL,x);
		
	CONNECT_ANY_MOUSE(C_PreviewDisplayWindow::OnMouseEvent);
	
	Connect(wxEVT_SIZE, wxSizeEventHandler(C_PreviewDisplayWindow::OnSize));
	
	m_debounce = false;
	
	#undef CONNECT_MOUSE
	#undef CONNECT_ANY_MOUSE
}

bool C_PreviewDisplayWindow::OnCreate()
{
	return true;
}

void C_PreviewDisplayWindow::OnUpdate()
{
	gClipPreviewState->Update(m_FrameTime);
	gFrame->SetPlaybackTime( gClipPreviewState->GetTime() );

	//if (m_pSection)
	//{
	//	m_pSection->OnUpdate( m_Input, m_Timer);
	//}
}

inline void Toggle( bool& value )
{
	value = !value;
}

void C_PreviewDisplayWindow::OnKeyDown( wxKeyEvent& e )
{
	if ( m_debounce )
	{
		return;
	}
	m_debounce = true;
	switch (e.GetKeyCode())
	{
		case 'F':
		{
			// Toggle between orbit character and orbit selected.
			switch ( gClipPreviewState->GetCameraMode() )
			{
				case E_CameraMode_OrbitCharacter:
					C_GlobalSignals::Instance().cameraChanged(E_CameraMode_OrbitSelected);
					break;
				case E_CameraMode_OrbitSelected:
				default:
					C_GlobalSignals::Instance().cameraChanged(E_CameraMode_OrbitCharacter);
					break;
			}
			break;
		}			
		
		case 'Z':
			C_GlobalSignals::Instance().cameraChanged(E_CameraMode_AttachToJoint);
			break;
		
		case 'S':
			Toggle( gClipPreviewState->showSkeleton );
			break;
		
		case 'P':
			Toggle( gClipPreviewState->paused );
			break;
		
		case 'N':
			Toggle( gClipPreviewState->showJointNames );
			break;
		
		case 'H':
			Toggle( gClipPreviewState->hideUnselectedJoints );
			break;
			
		case 'R':
			gClipPreviewState->m_dynamicsContext.Reset();
			break;
		
		default:
			e.Skip();
	}
}

void C_PreviewDisplayWindow::OnKeyUp( wxKeyEvent& e )
{
	e.Skip();
	m_debounce = false;
}

void C_PreviewDisplayWindow::OnMouseEvent(wxMouseEvent& e)
{
	wxRect clientRect( GetClientRect() );
	wxPoint clientPos = e.GetPosition(); // ScreenToClient( event.GetPosition() );
	bool insideClientRect = clientRect.Contains( clientPos );
	
	if (true) //insideClientRect)
	{
		U32 buttons = 0;
		if (e.LeftIsDown()) buttons |= 1;
		if (e.MiddleIsDown()) buttons |= 2;
		if (e.RightIsDown()) buttons |= 4;

		if (e.LeftDown() || e.RightDown() || e.MiddleDown())
		{
			C_GlobalSignals::Instance().onViewportMouseDown(clientPos.x, clientPos.y, buttons);
			//CaptureMouse();
		}
		else if (e.LeftUp() || e.RightUp() || e.MiddleUp())
		{
			C_GlobalSignals::Instance().onViewportMouseUp(clientPos.x, clientPos.y, buttons);
			//ReleaseMouse();
		}
		else if (e.LeftDClick() || e.RightDClick() || e.MiddleDClick())
		{
			C_GlobalSignals::Instance().onViewportMouseDown(clientPos.x, clientPos.y, buttons);
			C_GlobalSignals::Instance().onViewportMouseUp(clientPos.x, clientPos.y, buttons);
		}
		else
		{
			C_GlobalSignals::Instance().onViewportMouseMove(clientPos.x, clientPos.y, buttons);
		}
	}
	e.Skip();
}

void C_PreviewDisplayWindow::OnSize(wxSizeEvent& e)
{
	wxRect c = GetClientRect();
	
	C_GlobalSignals::Instance().onViewportResize(c.GetWidth(), c.GetHeight());
	
	e.Skip();
};

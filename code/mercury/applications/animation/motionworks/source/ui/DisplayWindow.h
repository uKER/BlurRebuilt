#ifndef WX_DISPLAY_WINDOW_H
#define WX_DISPLAY_WINDOW_H

#pragma once

// display library
#include <Display/Core/Includes.h>
#include <Display.h>

//// samples library
//#include <Samples/System/View.h>
//#include <Samples/System/Timer.h>
//#include <samples/system/Input.h>

class C_Camera;

namespace Render
{
	namespace Debug
	{
		class C_DebugRenderSys;
	}
	
	namespace Font
	{
		class C_FontSystem;
	}
}

class C_PreviewSection;

class C_DisplayWindow : public wxWindow
{
public:
	C_DisplayWindow( C_Camera& camera, wxWindow *parent, wxWindowID, const wxPoint &pos, const wxSize &size );
	~C_DisplayWindow();

	// creation
	bool Create();
	void DestroyRenderers();

	// input
	void OnMouseEvent(wxMouseEvent& event);
	void OnMouseCaptureLostEvent(wxMouseCaptureLostEvent& event);
	void OnKeyUpEvent(wxKeyEvent& event);
	void OnKeyDownEvent(wxKeyEvent& event);
	void OnKillFocusEvent(wxFocusEvent& event);

	// update
	//void OnTimer(wxTimerEvent& event);
	void OnIdle( wxIdleEvent& e );
	void SetRefreshRate(S32 refreshRate);
	
	// paint
	void OnPaint( wxPaintEvent& e );
	
	// resize
	void OnSize(wxSizeEvent& event);
	virtual void OnResize() {};

	// section
	bool SetSection(C_PreviewSection* pSection);

	// device
	Display::C_Device*	GetDevice() { return m_pDevice; }
	
protected:

	C_Camera&							m_camera;

	// display library
	Display::C_Manager*					m_pDisplay;
	Display::C_Device*					m_pDevice;
	Display::C_DebugRender*				m_pDebugRender;
	Display::C_DeviceParams				m_DeviceParams;
	Display::C_ViewSetup				m_ViewSetup;
	S32									m_ViewIndex;
	Time::C_Time						m_PrevTime;
	F32									m_FrameTime;

	// render library
	Render::Debug::C_DebugRenderSys*	m_pDebugRenderSys;
	Render::Font::C_FontSystem*			m_pFontSystem;

	C_PreviewSection*					m_pViewRenderer;

	wxPoint								m_CursorPosition;

	S32									m_RefreshRate;
	S32									m_RefreshMs;
	F32									m_RefreshDelta;
	
	S32									m_MouseButtonDown;
	wxPoint								m_MousePoint;
	F32									m_MouseSensitivity;

	bool								m_Initialised;

	// create
	virtual bool						OnCreate();

	// update
	void								UpdateAndRender();
	virtual void						OnUpdate();

	// render
	void								Render() const;
	virtual void						OnRender() const;

	// input
	void								CaptureMouseCursor(S32 mouseButton, wxPoint point);
	void								ReleaseMouseCursor();

	// reset
	bool								Reset();


	DECLARE_CLASS(C_DisplayWindow)
    DECLARE_EVENT_TABLE()
};

#endif // WX_DISPLAY_WINDOW_H



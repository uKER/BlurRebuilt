#ifndef SAMPLES_PCAPPLICATION_H
#define SAMPLES_PCAPPLICATION_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/System/Application.h"

namespace Samples
{

class C_PCApplication : public C_Application
{
public:
	
	C_PCApplication();
	virtual ~C_PCApplication();

	bool Create();

	void Run();

	LRESULT MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	void Update();

private:
	void KeyDown(const U32 key);
	void KeyUp(const U32 key);

	void MouseDown();
	void MouseUp();

	bool ResetDevice();

	void FocusChange(const bool gotFocus);

	void Render();

private:

	bool InitialiseClass();
	bool InitialiseWindow();

protected:

	bool		m_PauseUpdate;
	bool		m_PauseRender;
	bool		m_Minimised;
	bool		m_Maximised;
	bool		m_InSizeMove;
	bool		m_Active;
	bool		m_MinimizedWhileFullscreen;
	bool		m_Windowed;
	bool		m_Capture;
	bool		m_HasFocus;
	bool		m_WindowResized;
	POINT		m_Cursor;
	HDC			m_hDC;
	HWND		m_hWnd;
	LPCTSTR		m_pName;
	HINSTANCE	m_hInstance;
};

}; //namespace Samples

void LinkToDebugView(const char* name);
void UnlinkFromDebugView();

#endif // SAMPLES_PCAPPLICATION_H

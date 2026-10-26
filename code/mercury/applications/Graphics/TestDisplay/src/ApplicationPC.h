#ifndef APPLICATION_PC_H
#define APPLICATION_PC_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ApplicationBase.h"

struct C_ApplicationParams : public C_ApplicationBaseParams
{
	C_ApplicationParams();

	bool						m_Windowed;
	HINSTANCE					m_hInstance;
};

class C_Application : public C_ApplicationBase
{
public:

	C_Application();
	~C_Application();

	bool Create(C_ApplicationParams& params);
	void Destroy();
	void Update();

	void Run();

	HRESULT InitialiseClass(C_ApplicationParams& params);
	HRESULT InitialiseWindow(C_ApplicationParams& params);

	LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	C_Mouse*				m_pMouse;
	C_Keyboard*				m_pKeyboard;

	WNDCLASSEX				m_ClassEx;
	ATOM					m_ClassAtom;
	Util::C_String			m_ClassName;
	
	Util::C_String			m_WindowName;
	U32						m_WindowStyle;
	S32						m_WindowX;
	S32						m_WindowY;
	S32						m_WindowWidth;
	S32						m_WindowHeight;

	HDC						m_hDC;
	HWND					m_hWnd;
	HMENU					m_hMenu;
	HINSTANCE				m_hInstance;
};

#endif // APPLICATION_PC_H

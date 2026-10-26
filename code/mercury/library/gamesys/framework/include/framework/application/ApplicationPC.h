#ifndef FRAMEWORK_APPLICATION_PC_H
#define FRAMEWORK_APPLICATION_PC_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Framework/Application/ApplicationBase.h"

namespace Framework
{

//! C_AppClass. PC application class.

class C_AppClass
{
public:

	//! Constructor.
	C_AppClass();

	//! Destructor.
	~C_AppClass();

public:

	WNDCLASSEX			m_WndClassEx;
	ATOM				m_WndClassAtom;
	Util::C_String		m_WndClassName;
};

//! C_AppWindow. PC application window.

class C_AppWindow
{
public:

	//! Constructor.
	C_AppWindow();

	//! Destructor.
	~C_AppWindow();

public:

	Util::C_String		m_Name;
	U32					m_Style;
	S32					m_X;
	S32					m_Y;
	S32					m_W;
	S32					m_H;
	HDC					m_hDC;
	HWND				m_hWnd;
	HMENU				m_hMenu;
};

//! C_Application. PC application.

class C_Application : public C_ApplicationBase
{
public:

	//! Constructor.
	C_Application();

	//! Destructor.
	virtual ~C_Application();

	//! Gets the PC application instane.
	static C_Application* GetInstance()
	{
		return s_pInstance;
	}

	bool IsActive()
	{
		return m_Active;
	}

	void SetActive(const bool active);

	//! PC application run.
	virtual HRESULT Run();

	//! PC window message handler.
	LRESULT MyWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual bool CanRender();
	bool LoadingScreenEnabledState() const;

	// [Blur2PC] Retail Blur PC graphics settings (Blur.exe 1.2 C_Application, 0x00849xxx-0x0084bxxx).
	// See docs/analysis.md "Graphics options (PC)".

	//! Applies a resolution index, multisample index, windowed flag and vsync flag (retail 0x0084ab80).
	void ApplyGraphicsOptions(const S32 resolutionIndex, const S32 multisampleIndex, const bool windowed, const bool vsync);

	//! Current graphics options (retail 0x00849860).
	void GetGraphicsOptions(S32* pResolutionIndex, S32* pMultisampleIndex, bool* pWindowed, bool* pVsync) const;

	//! Sets the graphical detail level 1-4 (retail 0x0084a200).
	void SetGraphicalDetail(const U32 graphicalDetail);

	//! Graphical detail level 1-4 (retail 0x00849830).
	U32 GetGraphicalDetail() const { return m_GraphicalDetail; }

	//! Size and multisample of display window 0, which follow the device parameters (0x0084a390 on a settings
	//! change, before the detail hook; 0x005acf50 when the device is created). Read by the mirror and
	//! reflection sizing (retail 0x00427660, 0x00427750).
	U32 GetDisplayWindowWidth() const;
	U32 GetDisplayWindowHeight() const;
	Display::E_Multisample GetDisplayWindowMultisample() const;

	//! [Blur2PC] retail 0x0084a390: resize the display windows from the application's device parameters.
	void ResizeDisplayWindows();

	//! Writes %APPDATA%\Bizarre Creations\<name>\settings.xml (retail 0x0084a050).
	void SaveSettings();

	//! [Blur2PC] retail 0x008498d0 (name ours): sends WM_CLOSE to the game window (T146).
	void Blur2PCCloseWindow();

protected:

	//! Platform specific application initialise.
	virtual HRESULT PlatformInitialise(const C_ApplicationParams& params);

	//! Platform specific application shutdown.
	virtual void PlatformShutdown();

	//! Platform specific device params setup.
	virtual void SetupDeviceParams();

	virtual void MyKeyDown(const U8 key);

	virtual void MyKeyUp(const U8 key);

	HRESULT MyRegisterClass(const C_ApplicationParams& params);
	
	HRESULT MyCreateWindow(const C_ApplicationParams& params);

	void InitMouse();

	void IdleLoop();

	void OnMouseButton(const U32 button, const bool state);

	void OnMouseMoveAbsolute(const S32 x, const S32 y);
	
	void OnMouseMoveRelative(const S32 x, const S32 y);

	void MyShowCursor(const bool show);

	void UpdateCursor();

	void FocusChange(const bool gotFocus);

	bool IsCursorInWindow();

	// [Blur2PC] retail Blur PC graphics settings helpers
	virtual void UpdateGraphicsSettings();
	virtual void SetupDisplayResolutions(const U32 width, const U32 height, const Display::E_Format format, const Display::E_Multisample multisample, const U32 flags);
	bool LoadSettings();
	void SetDisplaySize(const U32 width, const U32 height, const U32 refreshRate);
	void GetWindowRectFromClient(RECT* pRect) const;
	void MoveWindowToClientRect();
	bool ResetDevice();
	void ShowCursorForWindowMode();
	void ClampToMinimumSize(S32* pWidth, S32* pHeight) const;

protected:

	C_AppClass					m_Class;
	C_AppWindow					m_Window;
	bool						m_Active;
	bool						m_Windowed;
	bool						m_HasFocus;
	bool						m_ClipCursor;
	bool						m_ShowCursor;
	bool						m_CameraLook;
	HINSTANCE					m_hInstance;

	// [Blur2PC] retail Blur PC graphics settings state
	Util::C_String				m_SettingsFileName;		// %APPDATA%\Bizarre Creations\<window name>\settings.xml
	S32							m_ResolutionIndex;		// index into Display::g_Resolutions; 0 also means "fit the window"
	S32							m_MultisampleIndex;		// index into Display::g_Resolutions' multisample list
	U32							m_GraphicalDetail;		// 1-4
	U32							m_DesktopRefreshRate;
	S32							m_MinWidth;				// retail C_AppWindow keeps 640x480 as the smallest size
	S32							m_MinHeight;
	bool						m_CursorShown;
	bool						m_TrackWindowSize;		// retail +0xbf5: follow user moves/resizes of the window

private:
	
	static C_Application*		s_pInstance;
};

}; // namespace Framework

#endif // FRAMEWORK_APPLICATION_PC_H

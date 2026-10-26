#ifndef WNDCORE_H
#define WNDCORE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Xt/Impl/Sz.h>
#include "WndMath.h"

//
// BaseWindow
//

class BaseWindow
{
public:
	BaseWindow();
	virtual ~BaseWindow();

public:
	bool Create(LPTSTR title, int x, int y, int cx, int cy, HWND parent = 0, HMENU menu = 0);
	bool Create(int x, int y, int cx, int cy, HWND parent = 0, HMENU menu = 0);
	bool Create(const WinPoint& position, const WinSize& size, HWND parent = 0, HMENU menu = 0);
	bool Create(const WinRect& rect, HWND parent = 0, HMENU menu = 0);
	bool Create(HWND parent = 0, HMENU menu = 0);
	void Destroy();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual bool PreTranslateMessage(MSG& msg);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	virtual TCHAR* GetWindowClass();

public:
	void Attach(HWND hwnd);
	void Detach();

public:
	inline operator HWND()
	{
		return _hwnd;
	}

protected:
	HWND	_hwnd;
	bool	_wndClassRedraw;

private:
	BaseWindow(const BaseWindow&);
	BaseWindow& operator=(const BaseWindow&);
};

//
// Window
//

class Window : public BaseWindow
{
public:
	Window();
	virtual ~Window();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
};

//
// ChildWindow
//

class ChildWindow : public BaseWindow
{
public:
	ChildWindow();
	virtual ~ChildWindow();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
};

//
// FrameWindow
//

class FrameWindow : public Window
{
public:
	FrameWindow();
	virtual ~FrameWindow();

public:
	inline HWND GetMdiWnd() const
	{
		return _mdiWnd;
	}

public:
	virtual bool PreCreateMdiWindow(CLIENTCREATESTRUCT* ccs);
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual bool PreTranslateMessage(MSG& msg);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	virtual TCHAR* GetWindowClass();

protected:
	HWND _mdiWnd;
};

//
// MdiWindow
//

class MdiWindow : public BaseWindow
{
public:
	MdiWindow();
	virtual ~MdiWindow();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	virtual TCHAR* GetWindowClass();
};

//
// DlgWindow
//

class DlgWindow : public Window
{
public:
	DlgWindow();
	virtual ~DlgWindow();

public:
	intptr_t DoModal(const WinPoint& position, const WinSize& size, HWND parent);
	bool EndModal(intptr_t result);

public:
	void _DlgSaveFocus();
	void _DlgRestoreFocus();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual bool PreTranslateMessage(MSG& msg);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

private:
	enum DlgFlags
	{
		kDlgInModalLoop		= 1,
		kDlgExitModalLoop	= 2
	};

private:
	HWND		_dlgSavedFocus;
	DWORD		_dlgFlags;
	intptr_t	_dlgResult;
};

//
// PopupWindow
//

class PopupWindow : public BaseWindow
{
public:
	PopupWindow();
	virtual ~PopupWindow();

public:
	intptr_t DoModal(const WinPoint& position, const WinSize& size, HWND parent);
	bool EndModal(intptr_t result);

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	virtual bool DismissTest(const WinPoint& pt);

private:
	enum DlgFlags
	{
		kDlgInModalLoop		= 1,
		kDlgExitModalLoop	= 2,
	};

private:
	DWORD		_dlgFlags;
	intptr_t	_dlgResult;
};

//
// WndCorePumpContext
//

class WndCorePumpContext
{
public:
	virtual ~WndCorePumpContext();

public:
	virtual bool PreRemove(MSG& msg);
	virtual bool PreTranslate(MSG& msg);
};

//
// PreTranslateMessage
//

void RegisterDialogWindow(BaseWindow* window);
void UnregisterDialogWindow(BaseWindow* window);

void RegisterDialogWindowHwnd(HWND wnd);
void UnregisterDialogWindowHwnd(HWND wnd);

void RegisterWindowForPreTranslate(BaseWindow* window);
void UnregisterWindowForPreTranslate(BaseWindow* window);

bool WndCorePreTranslateMessage(MSG& msg, WndCorePumpContext* context);

//
// PumpMessage
//

bool WndCorePumpMessage(WndCorePumpContext* context);

// TODO: Put this somewhere else

HMODULE GetXtWinIconDll();
void InvalidateWindow(HWND hwnd);
void ClipRectToMonitor(WinRect* rc, bool considerWholeRect);

Sz8 SzGetWindowText(HWND hwnd);
Sz16 SzGetWindowTextW(HWND hwnd);

#endif // WNDCORE_H
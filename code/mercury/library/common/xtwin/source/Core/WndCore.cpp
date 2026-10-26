#include "Core/XtWinPch.h"
#include "XtWin/WndCore.h"
#include "XtWin/Core.h"
#include "Core/List.h"
#include <Xt/Impl/XtCore.h>
#include <Xt/Impl/XtUt.h>

//
// Window procedure implementations
//

LRESULT WINAPI CoreWndProcImpl(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BaseWindow* window = (BaseWindow*)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if ((window == 0) && (msg == WM_NCCREATE))
		(window = (BaseWindow*)(((CREATESTRUCT*)lParam)->lpCreateParams))->Attach(hwnd);

	LRESULT result = window ? window->WndProc(msg, wParam, lParam) : DefWindowProc(hwnd, msg, wParam, lParam);

	if ((window != 0) && (msg == WM_NCDESTROY))
		window->Detach();

	return result;
}

LRESULT WINAPI CoreFrameWndProcImpl(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BaseWindow* window = (BaseWindow*)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if ((window == 0) && (msg == WM_NCCREATE))
		(window = (BaseWindow*)(((CREATESTRUCT*)lParam)->lpCreateParams))->Attach(hwnd);

	LRESULT result = window ? window->WndProc(msg, wParam, lParam) : DefFrameProc(hwnd, 0, msg, wParam, lParam);

	if ((window != 0) && (msg == WM_NCDESTROY))
		window->Detach();

	return result;
}

LRESULT WINAPI CoreMdiWndProcImpl(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BaseWindow* window = (BaseWindow*)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if ((window == 0) && (msg == WM_NCCREATE))
	{
		CREATESTRUCT*		cs	= (CREATESTRUCT*)lParam;
		MDICREATESTRUCT*	mcs	= (MDICREATESTRUCT*)cs->lpCreateParams;

		window = (BaseWindow*)mcs->lParam;

		window->Attach(hwnd);
	}

	LRESULT result = window ? window->WndProc(msg, wParam, lParam) : DefMDIChildProc(hwnd, msg, wParam, lParam);

	if ((window != 0) && (msg == WM_NCDESTROY))
		window->Detach();

	return result;
}

//
// BaseWindow
//

BaseWindow::BaseWindow() :
	_hwnd(0),
	_wndClassRedraw(false)
{
}

BaseWindow::~BaseWindow()
{
	Destroy();
}

bool BaseWindow::Create(LPTSTR title, int x, int y, int cx, int cy, HWND parent, HMENU menu)
{
	Destroy();

	CREATESTRUCT cs = { 0 };

	cs.lpCreateParams	= this;
	cs.hInstance		= 0;
	cs.hMenu			= menu;
	cs.hwndParent		= parent;
	cs.cx				= cx;
	cs.cy				= cy;
	cs.x				= x;
	cs.y				= y;
	cs.style			= 0;
	cs.lpszName			= title;
	cs.lpszClass		= GetWindowClass();
	cs.dwExStyle		= 0;

	if (!PreCreateWindow(&cs))
		return false;

	return (CreateWindowEx(
			cs.dwExStyle, cs.lpszClass, cs.lpszName, cs.style,
			cs.x, cs.y, cs.cx, cs.cy, cs.hwndParent,
			cs.hMenu, cs.hInstance, cs.lpCreateParams
		) != 0);
}

bool BaseWindow::Create(int x, int y, int cx, int cy, HWND parent, HMENU menu)
{
	return Create(_T(""), x, y, cx, cy, parent, menu);
}

bool BaseWindow::Create(const WinPoint& position, const WinSize& size, HWND parent, HMENU menu)
{
	return Create(_T(""), position.x, position.y, size.cx, size.cy, parent, menu);
}

bool BaseWindow::Create(const WinRect& rect, HWND parent, HMENU menu)
{
	return Create(_T(""), rect.left, rect.top, rect.Width(), rect.Height(), parent, menu);
}

bool BaseWindow::Create(HWND parent, HMENU menu)
{
	return Create(_T(""), CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, menu);
}

void BaseWindow::Destroy()
{
	if (_hwnd != 0)
	{
		DestroyWindow(_hwnd);
		assert(_hwnd == 0);
	}
}

bool BaseWindow::PreCreateWindow(CREATESTRUCT* cs)
{
	return true;
}

bool BaseWindow::PreTranslateMessage(MSG& msg)
{
	return false;
}

LRESULT BaseWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_NCCALCSIZE:
		{
			if (_wndClassRedraw && wParam)
				return DefWindowProc(_hwnd, msg, wParam, lParam) | WVR_REDRAW;
		}
		break;
	}

	return DefWindowProc(_hwnd, msg, wParam, lParam);
}

TCHAR* BaseWindow::GetWindowClass()
{
	TCHAR*		windowClassName = _T("WindowClassImpl");
	static bool	setup			= false;

	if (!setup)
	{
		WNDCLASSEX wcx;

		ZeroMemory(&wcx, sizeof(wcx));

		wcx.cbSize			= sizeof(wcx);
		wcx.lpszClassName	= windowClassName;
		wcx.lpfnWndProc		= CoreWndProcImpl;
		wcx.hCursor			= LoadCursor(0, IDC_ARROW);
		wcx.hInstance		= GetModuleHandle(0);
		wcx.style			= CS_DBLCLKS;

		setup = (RegisterClassEx(&wcx) != 0);
	}

	return windowClassName;
}

void BaseWindow::Attach(HWND hwnd)
{
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG)(LONG_PTR)this);
	_hwnd = hwnd;
}

void BaseWindow::Detach()
{
	SetWindowLongPtr(_hwnd, GWLP_USERDATA, 0);
	_hwnd = 0;
}

//
// Window
//

Window::Window()
{
}

Window::~Window()
{
}

bool Window::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!BaseWindow::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_OVERLAPPEDWINDOW;
	cs->dwExStyle	|= WS_EX_OVERLAPPEDWINDOW;

	return true;
}

//
// ChildWindow
//

ChildWindow::ChildWindow()
{
}

ChildWindow::~ChildWindow()
{
}

bool ChildWindow::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!BaseWindow::PreCreateWindow(cs))
		return false;

	cs->style |= WS_CHILD | WS_VISIBLE;

	return true;
}

//
// FrameWindow
//

FrameWindow::FrameWindow() :
	_mdiWnd(0)
{
}

FrameWindow::~FrameWindow()
{
}

bool FrameWindow::PreCreateMdiWindow(CLIENTCREATESTRUCT* ccs)
{
	return true;
}

bool FrameWindow::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!Window::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_CLIPCHILDREN;
	cs->dwExStyle	&= ~WS_EX_CLIENTEDGE;

	return true;
}

bool FrameWindow::PreTranslateMessage(MSG& msg)
{
	if ((_mdiWnd != 0) && (TranslateMDISysAccel(_mdiWnd, &msg) != FALSE))
		return true;

	return Window::PreTranslateMessage(msg);
}

LRESULT FrameWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			CLIENTCREATESTRUCT ccs = { 0 };

			if (!PreCreateMdiWindow(&ccs))
				return -1;

			if ((_mdiWnd = CreateWindowEx(
					0, _T("MDICLIENT"), 0, WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, 0, 0, 0, 0, *this, 0, 0, &ccs
				)) == 0)
			{
				return -1;
			}

			RegisterWindowForPreTranslate(this);
		}
		break;

		case WM_DESTROY:
			UnregisterWindowForPreTranslate(this);
		break;
	}

	return DefFrameProc(_hwnd, _mdiWnd, msg, wParam, lParam);
}

TCHAR* FrameWindow::GetWindowClass()
{
	TCHAR*		windowClassName	= _T("FrameWindowClassImpl");
	static bool	setup			= false;

	if (!setup)
	{
		WNDCLASSEX wcx;

		ZeroMemory(&wcx, sizeof(wcx));

		wcx.cbSize			= sizeof(wcx);
		wcx.lpszClassName	= windowClassName;
		wcx.lpfnWndProc		= CoreFrameWndProcImpl;
		wcx.hCursor			= LoadCursor(0, IDC_ARROW);
		wcx.hInstance		= GetModuleHandle(0);
		wcx.style			= CS_DBLCLKS;

		setup = (RegisterClassEx(&wcx) != 0);
	}

	return windowClassName;
}

//
// MdiWindow
//

MdiWindow::MdiWindow()
{
}

MdiWindow::~MdiWindow()
{
}

bool MdiWindow::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!BaseWindow::PreCreateWindow(cs))
		return false;

	cs->dwExStyle |= WS_EX_MDICHILD;

	BOOL maximised = FALSE;

	SendMessage(cs->hwndParent, WM_MDIGETACTIVE, 0, (LPARAM)&maximised);

	if (maximised)
		cs->style |= WS_MAXIMIZE;

	return true;
}

LRESULT MdiWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefMDIChildProc(_hwnd, msg, wParam, lParam);
}

TCHAR* MdiWindow::GetWindowClass()
{
	TCHAR*		windowClassName	= _T("MdiWindowClassImpl");
	static bool	setup			= false;

	if (!setup)
	{
		WNDCLASSEX wcx;

		ZeroMemory(&wcx, sizeof(wcx));

		wcx.cbSize			= sizeof(wcx);
		wcx.lpszClassName	= windowClassName;
		wcx.lpfnWndProc		= CoreMdiWndProcImpl;
		wcx.hCursor			= LoadCursor(0, IDC_ARROW);
		wcx.hInstance		= GetModuleHandle(0);
		wcx.style			= CS_DBLCLKS;

		setup = (RegisterClassEx(&wcx) != 0);
	}

	return windowClassName;
}

//
// DlgWindow
//

DlgWindow::DlgWindow() :
	_dlgSavedFocus(0),
	_dlgFlags(0),
	_dlgResult(0)
{
}

DlgWindow::~DlgWindow()
{
}

intptr_t DlgWindow::DoModal(const WinPoint& position, const WinSize& size, HWND parent)
{
	if ((_dlgFlags & kDlgInModalLoop) || (*this != (HWND)0))
		return -1;

	bool shouldEnable = (parent != 0) && (EnableWindow(parent, FALSE) == FALSE);

	_dlgResult	= -1;
	_dlgFlags	= kDlgInModalLoop;

	if (Create(position, size, parent))
	{
		ShowWindow(*this, SW_SHOW);

		while((_dlgFlags & kDlgExitModalLoop) == 0)
		{
			if (!WndCorePumpMessage(0))
				break;
		}

		SetWindowPos(*this, 0, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	if (parent)
	{
		if (shouldEnable)
			EnableWindow(parent, TRUE);

		if (GetActiveWindow() == *this)
			SetActiveWindow(parent);
	}

	Destroy();

	_dlgFlags = 0;

	return _dlgResult;
}

bool DlgWindow::EndModal(intptr_t result)
{
	if ((_dlgFlags & kDlgInModalLoop) == 0)
		return false;

	_dlgResult = result;

	_dlgFlags |= kDlgExitModalLoop;

	PostMessage(*this, WM_NULL, 0, 0);

	return true;
}

void DlgWindow::_DlgSaveFocus()
{
	HWND hwnd = GetFocus();

	if (hwnd && IsChild(*this, hwnd))
		_dlgSavedFocus = hwnd;
}

void DlgWindow::_DlgRestoreFocus()
{
	if ((_dlgFlags & kDlgExitModalLoop) || IsMinimized(*this))
		return;

	if (!_dlgSavedFocus || !IsWindow(_dlgSavedFocus) || (_dlgSavedFocus == *this))
	{
		if (!IsWindow(_dlgSavedFocus = GetNextDlgTabItem(*this, 0, FALSE)))
			return;
	}

	//if ((SendMessage(_savedFocus, WM_GETDLGCODE, 0, 0) & DLGC_HASSETSEL) != 0)
	//	SendMessage(_savedFocus, EM_SETSEL, 0, -1);

	SetFocus(_dlgSavedFocus);
}

bool DlgWindow::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!BaseWindow::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
	cs->dwExStyle	|= WS_EX_OVERLAPPEDWINDOW | WS_EX_CONTROLPARENT | WS_EX_DLGMODALFRAME;
	cs->dwExStyle	&= ~WS_EX_CLIENTEDGE;

	return true;
}

bool DlgWindow::PreTranslateMessage(MSG& msg)
{
	if (IsDialogMessage(*this, &msg))
		return true;

	return Window::PreTranslateMessage(msg);
}

LRESULT DlgWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
			if (Window::WndProc(msg, wParam, lParam) < 0)
				return -1;

			RegisterWindowForPreTranslate(this);
		return 0;

		case WM_DESTROY:
			UnregisterWindowForPreTranslate(this);
		break;

		case WM_CLOSE:
			SendMessage(*this, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), (LPARAM)GetDlgItem(*this, IDCANCEL));
		return 0;

		case WM_ERASEBKGND:
		{
			HDC			hdc			= (HDC)wParam;
			COLORREF	oldBkColour	= SetBkColor(hdc, GetSysColor(COLOR_3DFACE));

			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &WinRect(*this), 0, 0, 0);

			SetBkColor(hdc, oldBkColour);
		}
		return 0;

		case WM_NEXTDLGCTL:
		{
			HWND focus = 0;

			if (!LOWORD(lParam))
				focus = GetNextDlgTabItem(*this, GetFocus(), wParam ? TRUE : FALSE);
			else
				focus = (HWND)wParam;

			SetFocus(focus);
		}
		return 0;

		case WM_SHOWWINDOW:
			if (wParam == FALSE)
				_DlgSaveFocus();
		break;

		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE)
				_DlgSaveFocus();
			else
				_DlgRestoreFocus();
		break;

		case WM_SETFOCUS:
			_DlgRestoreFocus();
		return 0;
	}

	return Window::WndProc(msg, wParam, lParam);
}

//
// PopupWindow
//

class PopupWindowPumpContext : public WndCorePumpContext
{
public:
	PopupWindowPumpContext(PopupWindow* wnd) :
		_wnd(wnd),
		_stop(false)
	{
	}

public:
	virtual bool PreRemove(MSG& msg)
	{
		// Exit popup if we receive a mouse down outside of our window

		switch(msg.message)
		{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONDBLCLK:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONDBLCLK:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONDBLCLK:
			case WM_XBUTTONDOWN:
			case WM_XBUTTONDBLCLK:
			{
				WinPoint pt(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));

				MapWindowPoints(msg.hwnd, *_wnd, &pt, 1);

				if (_wnd->DismissTest(pt))
				{
					_stop = true;

					return true;
				}
			}
			break;

			case WM_MOUSEWHEEL:
			case WM_NCLBUTTONDOWN:
			case WM_NCLBUTTONDBLCLK:
			case WM_NCMBUTTONDOWN:
			case WM_NCMBUTTONDBLCLK:
			case WM_NCRBUTTONDOWN:
			case WM_NCRBUTTONDBLCLK:
			case WM_NCXBUTTONDOWN:
			case WM_NCXBUTTONDBLCLK:
			{
				WinPoint pt(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));

				if (_wnd->DismissTest(pt.ScreenToClient(*_wnd)))
				{
					_stop = true;

					return true;
				}
			}
			break;
		}
		
		return false;
	}

	bool PreTranslate(MSG& msg)
	{
		// Redirect all mouse and keyboard messages to our window

		switch(msg.message)
		{
			case WM_MOUSEMOVE:
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_LBUTTONDBLCLK:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_MBUTTONDBLCLK:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_RBUTTONDBLCLK:
			case WM_XBUTTONDOWN:
			case WM_XBUTTONUP:
			case WM_XBUTTONDBLCLK:
			{
				WinPoint pt(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));

				MapWindowPoints(msg.hwnd, *_wnd, &pt, 1);

				msg.hwnd	= *_wnd;
				msg.lParam	= MAKELPARAM(pt.x, pt.y);
			}
			break;

			case WM_MOUSEWHEEL:
			case WM_NCMOUSEMOVE:
			case WM_NCLBUTTONDOWN:
			case WM_NCLBUTTONUP:
			case WM_NCLBUTTONDBLCLK:
			case WM_NCMBUTTONDOWN:
			case WM_NCMBUTTONUP:
			case WM_NCMBUTTONDBLCLK:
			case WM_NCRBUTTONDOWN:
			case WM_NCRBUTTONUP:
			case WM_NCRBUTTONDBLCLK:
			case WM_NCXBUTTONDOWN:
			case WM_NCXBUTTONUP:
			case WM_NCXBUTTONDBLCLK:
				msg.hwnd = *_wnd;
			break;

			case WM_KEYDOWN:
			case WM_KEYUP:
			case WM_CHAR:
			case WM_DEADCHAR:
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_SYSCHAR:
			case WM_SYSDEADCHAR:
			case WM_UNICHAR:
				msg.hwnd = *_wnd;
			break;
		}

		return false;
	}

	bool Stop()
	{
		return _stop;
	}

private:
	PopupWindow*	_wnd;
	bool			_stop;
};

PopupWindow::PopupWindow() :
	_dlgFlags(0),
	_dlgResult(0)
{
}

PopupWindow::~PopupWindow()
{
}

intptr_t PopupWindow::DoModal(const WinPoint& position, const WinSize& size, HWND parent)
{
	// Must have a parent window

	if (!parent)
		return -1;

	// Parent window must be of the same thread

	if (GetCurrentThreadId() != GetWindowThreadProcessId(parent, 0))
		return -1;

	// Check we aren't already running

	if ((_dlgFlags & kDlgInModalLoop) || (*this != (HWND)0))
		return -1;

	_dlgResult	= -1;
	_dlgFlags	= kDlgInModalLoop;

	// Position on nearest monitor

	WinRect rc(position, size);

	ClipRectToMonitor(&rc, false);

	if (Create(rc.Position(), rc.Size(), parent))
	{
		// Show the window, but don't activate

		ShowWindow(*this, SW_SHOWNOACTIVATE);

		// As we can't be the active window (because we'll steal the title bar focus) we need to set capture
		// on the parents window, and steal the mouse messages

		SetCapture(parent);

		PopupWindowPumpContext pc(this);

		while((_dlgFlags & kDlgExitModalLoop) == 0)
		{
			// Stop if we've been asked to

			if (pc.Stop())
				break;

			// Stop if our app isn't foreground anymore

			HWND activeWindow = GetActiveWindow();

			if ((activeWindow != parent) && !IsChild(activeWindow, parent))
				break;

			// Stop if someone has stolen focus

			if (GetCapture() != parent)
				break;

			// Pump messages

			if (!WndCorePumpMessage(&pc))
				break;
		}

		ReleaseCapture();
	}

	Destroy();

	_dlgFlags = 0;

	return _dlgResult;
}

bool PopupWindow::EndModal(intptr_t result)
{
	if ((_dlgFlags & kDlgInModalLoop) == 0)
		return false;

	_dlgResult = result;

	_dlgFlags |= kDlgExitModalLoop;

	PostMessage(*this, WM_NULL, 0, 0);

	return true;
}

bool PopupWindow::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!BaseWindow::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_POPUP | WS_BORDER;
	cs->dwExStyle	|= WS_EX_TOOLWINDOW | WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_TOPMOST;

	return true;
}

LRESULT PopupWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CLOSE:
			EndModal(0);
		return 0;

		case WM_ERASEBKGND:
		{
			HDC			hdc			= (HDC)wParam;
			COLORREF	oldBkColour	= SetBkColor(hdc, GetSysColor(COLOR_3DFACE));

			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &WinRect(*this), 0, 0, 0);

			SetBkColor(hdc, oldBkColour);
		}
		return 0;

		case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;

		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE)
				EndModal(0);
		break;

		case WM_SYSKEYDOWN:
			if (wParam == VK_MENU)
				EndModal(0);
		break;
	}

	return BaseWindow::WndProc(msg, wParam, lParam);
}

bool PopupWindow::DismissTest(const WinPoint& pt)
{
	WinRect rc;
	GetWindowRect(*this, &rc);

	return PtInRect(&rc, pt.ClientToScreen(*this)) == FALSE;
}

//
// WndCorePumpContext
//

WndCorePumpContext::~WndCorePumpContext()
{
}

bool WndCorePumpContext::PreRemove(MSG& msg)
{
	return false;
}

bool WndCorePumpContext::PreTranslate(MSG& msg)
{
	return false;
}

//
// PreTranslateMessage
//

List<BaseWindow*> gWndCoreDialogList;
List<HWND> gWndCoreDialogListHwnd;
XtUt::Dictionary<HWND, BaseWindow*> gWndCorePreTranslateList;

void RegisterDialogWindow(BaseWindow* window)
{
	for(int i = 0; i < gWndCoreDialogList.Length(); i++)
		if (gWndCoreDialogList[i] == window)
			return;

	gWndCoreDialogList.Append(new BaseWindow*(window));
}

void UnregisterDialogWindow(BaseWindow* window)
{
	for(int i = 0; i < gWndCoreDialogList.Length(); i++)
	{
		if (gWndCoreDialogList[i] == window)
		{
			gWndCoreDialogList.Remove(i);
			break;
		}
	}

	if (gWndCoreDialogList.Length() <= 0)
		gWndCoreDialogList.Clear();
}

void RegisterDialogWindowHwnd(HWND wnd)
{
	for(int i = 0; i < gWndCoreDialogListHwnd.Length(); i++)
		if (gWndCoreDialogListHwnd[i] == wnd)
			return;

	gWndCoreDialogListHwnd.Append(new HWND(wnd));
}

void UnregisterDialogWindowHwnd(HWND wnd)
{
	for(int i = 0; i < gWndCoreDialogListHwnd.Length(); i++)
	{
		if (gWndCoreDialogListHwnd[i] == wnd)
		{
			gWndCoreDialogListHwnd.Remove(i);
			break;
		}
	}

	if (gWndCoreDialogListHwnd.Length() <= 0)
		gWndCoreDialogListHwnd.Clear();
}

void RegisterWindowForPreTranslate(BaseWindow* window)
{
	XtUt::Insert(&gWndCorePreTranslateList, (HWND)*window, new BaseWindow*(window));
}

void UnregisterWindowForPreTranslate(BaseWindow* window)
{
	HWND hwnd = *window;

	XtUt::Remove(&gWndCorePreTranslateList,
			XtUt::FirstOf(&gWndCorePreTranslateList, hwnd),
			XtUt::LastOf(&gWndCorePreTranslateList, hwnd)
		);
}

BaseWindow* WndCoreGetWindowFromHandle(HWND hwnd)
{
	BaseWindow** window = gWndCorePreTranslateList[hwnd];

	return window ? *window : 0;
}

bool WndCorePreTranslateMessage(MSG& msg, WndCorePumpContext* context)
{
	if (CallMsgFilter(&msg, 0x5100))
		return true;

	if (context && context->PreTranslate(msg))
		return true;

	if (msg.hwnd != 0)
	{
		/*if (msg.message == WM_MOUSEWHEEL)
		{
			if ((GetWindowLong(msg.hwnd, GWL_STYLE) & WS_POPUP) == 0)
			{
				WinPoint pt(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));

				HWND target = WindowFromPoint(pt);

				if (target)
					msg.hwnd = target;
			}
		}*/

		for(HWND hwnd = msg.hwnd; hwnd != 0; hwnd = GetParent(hwnd))
		{
			BaseWindow* window = WndCoreGetWindowFromHandle(hwnd);

			if (!window)
				continue;

			if (window->PreTranslateMessage(msg))
				return true;
		}

		for(int i = 0; i < gWndCoreDialogList.Length(); i++)
		{
			HWND hwnd = *gWndCoreDialogList[i];

			/*if (
					((hwnd == msg.hwnd) || IsChild(hwnd, msg.hwnd)) &&
					(msg.message == WM_KEYDOWN) &&
					(msg.wParam == VK_TAB) &&
					(GetKeyState(VK_CONTROL) < 0)
				)
			{
				continue;
			}*/

			if (IsDialogMessage(hwnd, &msg))
				return true;
		}

		for(int i = 0; i < gWndCoreDialogListHwnd.Length(); i++)
		{
			HWND hwnd = gWndCoreDialogListHwnd[i];

			if (IsDialogMessage(hwnd, &msg))
				return true;
		}
	}
	
	return false;
}

//
// PumpMessage
//

bool WndCorePumpMessage(WndCorePumpContext* context)
{
	MSG msg;

	if (context)
	{
		// TODO: For some reason this messes up NM_RCLICK notifications for ListViews

		for(;;)
		{
			while(!PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE))
				WaitMessage();

			int msgNo = msg.message;

			if (context && context->PreRemove(msg))
				return true;

			MSG dummy;

			if (PeekMessage(&dummy, 0, msgNo, msgNo, PM_REMOVE))
				break;
		}
	}
	else
		GetMessage(&msg, 0, 0, 0);

	if (msg.message == WM_QUIT)
	{
		PostQuitMessage((int)msg.wParam);

		return false;
	}

	if (!WndCorePreTranslateMessage(msg, context))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

// TODO: Put this somewhere else

HMODULE GetXtWinIconDll()
{
	static HMODULE mod;

	if (!mod)
		mod = LoadLibraryEx("xtwinres.dll", 0, LOAD_LIBRARY_AS_DATAFILE);

	return mod;
}

void InvalidateWindow(HWND hwnd)
{
	if (hwnd != 0)
		InvalidateRect(hwnd, 0, FALSE);
}

void ClipRectToMonitor(WinRect* rc, bool considerWholeRect)
{
	HMONITOR monitor;

	if (considerWholeRect)
		monitor = MonitorFromRect(rc, MONITOR_DEFAULTTONEAREST);
	else
		monitor = MonitorFromPoint(rc->Position(), MONITOR_DEFAULTTONEAREST);

	MONITORINFO mi = { 0 };
	mi.cbSize = sizeof(mi);

	if (!GetMonitorInfo(monitor, &mi))
		return;

	int width	= rc->Width();
	int height	= rc->Height();

	rc->left	= Clamp(rc->left, mi.rcWork.left, mi.rcWork.right - width);
	rc->top		= Clamp(rc->top, mi.rcWork.top, mi.rcWork.bottom - height);
	rc->right	= rc->left + width;
	rc->bottom	= rc->top  + height;
}

//
// SzGetWindowText
//

Sz8 SzGetWindowText(HWND hwnd)
{
	if (!hwnd)
		return Sz8();

	int length = GetWindowTextLength(hwnd);

	if (length <= 0)
		return Sz8();

	Sz8 result;

	if (char* text = result.Capture(length, false))
		result.Truncate(GetWindowText(hwnd, text, length + 1));

	return result;
}

Sz16 SzGetWindowTextW(HWND hwnd)
{
	if (!hwnd)
		return Sz16();

	int length = GetWindowTextLengthW(hwnd);

	if (length <= 0)
		return Sz16();

	Sz16 result;

	if (wchar_t* text = result.Capture(length, false))
		result.Truncate(GetWindowTextW(hwnd, text, length + 1));

	return result;
}

//
// WinTextMeasurer
//

WinTextMeasurer::WinTextMeasurer()
{
	_ic = CreateIC(_T("DISPLAY"), 0, 0, 0);

	SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
}

WinTextMeasurer::~WinTextMeasurer()
{
	if (_ic)
	{
		SelectObject(_ic, GetStockObject(DEFAULT_GUI_FONT));
		DeleteDC(_ic);
	}
}

void WinTextMeasurer::SetFont(HFONT font)
{
	if (_ic)
	{
		if (font)
			SelectObject(_ic, font);
		else
			SelectObject(_ic, GetStockObject(DEFAULT_GUI_FONT));
	}
}

WinSize WinTextMeasurer::Measure(const char* text, const WinSize& minSize)
{
	if (!_ic)
		return minSize;

	WinSize sz;
	GetTextExtentPoint32(_ic, text, strlen(text), &sz);

	return WinSize(Max(minSize.cx, sz.cx), Max(minSize.cy, sz.cy));
}

WinSize WinTextMeasurer::Measure(const wchar_t* text, const WinSize& minSize)
{
	if (!_ic)
		return minSize;

	WinSize sz;
	GetTextExtentPoint32W(_ic, text, lstrlenW(text), &sz);

	return WinSize(Max(minSize.cx, sz.cx), Max(minSize.cy, sz.cy));
}
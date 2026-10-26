#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/WndCore.h"
#include "XtWin/SendEmail.h"
#include "XtWin/AsyncOpWin.h"
#include "XtWin/Resource.h"
#include <PsApi.h>

const int IDC_LIST		= 3001;
const int IDC_PROGRESS	= 3002;

//
// AsyncOpImpl*
//

class AsyncOpWin : public DlgWindow
{
public:
	AsyncOpWin(AsyncOpImpl* op);
	virtual ~AsyncOpWin();

public:
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

private:
	AsyncOpImpl* _op;

private:
	HWND _list;
	HWND _close;
	HWND _progress;

friend class AsyncOpImpl;
};

class AsyncOpImpl
{
public:
	AsyncOpImpl(HWND parentWindow);
	~AsyncOpImpl();

public:
	bool InvokeEx(AsyncOpCallback callback, void* context, int stackSize, bool noWindow);

public:
	void NewMessage(AsyncOpIcon icon, const XtString& text);
	void AppendMessage(const XtString& text);

public:
	void SetTitle(const char* title);
	void SetProgress(float fraction);
	void SetStayOpen();

public:
	bool IsDone();

private:
	void NewMessageSub(AsyncOpIcon icon, const XtString& text);

private:
	static void ThreadProc(void* context);

private:
	HWND		_parentWindow;
	AsyncOpWin	_window;
	bool		_keepOpen;
	bool		_isCancelled;

private:
	ArteryThread	_thread;
	ArteryLock		_lock;
	volatile bool	_done;

private:
	AsyncOpCallback	_callback;
	void*			_context;
	bool			_result;
	int				_stackSize;

private:
	DWORD	_lastProgressUpdate;
	int		_colWidth;
	int		_oldPos;

friend class AsyncOpWin;
friend class AsyncOp;
};

//
// AsyncOpWin
//

AsyncOpWin::AsyncOpWin(AsyncOpImpl* op) :
	_op(op)
{
}

AsyncOpWin::~AsyncOpWin()
{
}

LRESULT AsyncOpWin::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			if (DlgWindow::WndProc(msg, wParam, lParam) < 0)
				return -1;

			// Disable window frame close button

			EnableMenuItem(GetSystemMenu(*this, FALSE), SC_CLOSE, MF_DISABLED | MF_BYCOMMAND);

			// Create child windows

			if ((_list = CreateWindowEx(
					WS_EX_CLIENTEDGE, WC_LISTVIEW, _T(""), LVS_REPORT | WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_SHOWSELALWAYS, 0, 0, 0, 0, *this, (HMENU)(intptr_t)IDC_LIST, 0, 0
				)) == 0)
			{
				return -1;
			}

			if ((_close = CreateWindowEx(
					0, WC_BUTTON, _T("Cancel"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, *this, (HMENU)(intptr_t)IDCANCEL, 0, 0
				)) == 0)
			{
				return -1;
			}

			if ((_progress = CreateWindowEx(
					0, PROGRESS_CLASS, _T(""), WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, *this, (HMENU)(intptr_t)IDC_PROGRESS, 0, 0
				)) == 0)
			{
				return -1;
			}

			SetFocus(_list);

			// Close button

			SendMessage(_close, WM_SETFONT, (WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT), FALSE);

			// Progress bar

			SendMessage(_progress, PBM_SETRANGE32, 0, 1000);

			_op->SetProgress(0.0f);

			// List view

			ListView_SetExtendedListViewStyleEx(_list,
					LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP,
					LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP
				);

			{
				HIMAGELIST il = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 3, 0);

				if (il)
				{
					{
						HBITMAP bitmap = (HBITMAP)LoadImage(GetXtWinIconDll(),
								MAKEINTRESOURCE(IDB_XTWIN_FIELD_INFO),
							IMAGE_BITMAP, 16, 16, LR_CREATEDIBSECTION | LR_VGACOLOR);

						if (bitmap)
							ImageList_AddMasked(il, bitmap, RGB(255, 0, 255));

						DeleteObject(bitmap);
					}
					{
						HBITMAP bitmap = (HBITMAP)LoadImage(GetXtWinIconDll(),
								MAKEINTRESOURCE(IDB_XTWIN_FIELD_WARNING),
							IMAGE_BITMAP, 16, 16, LR_CREATEDIBSECTION | LR_VGACOLOR);

						if (bitmap)
							ImageList_AddMasked(il, bitmap, RGB(255, 0, 255));

						DeleteObject(bitmap);
					}
					{
						HBITMAP bitmap = (HBITMAP)LoadImage(GetXtWinIconDll(),
								MAKEINTRESOURCE(IDB_XTWIN_FIELD_ERROR),
							IMAGE_BITMAP, 16, 16, LR_CREATEDIBSECTION | LR_VGACOLOR);

						if (bitmap)
							ImageList_AddMasked(il, bitmap, RGB(255, 0, 255));

						DeleteObject(bitmap);
					}

					ListView_SetImageList(_list, il, LVSIL_SMALL);
				}
			}

			{
				LVCOLUMN col = { 0 };

				col.mask	= LVCF_TEXT | LVCF_WIDTH;
				col.pszText	= _T("Message");
				col.cx		= _op->_colWidth;

				ListView_InsertColumn(_list, 0, &col);
			}

			SetTimer(*this, WM_APP, 1000, 0);

			if (!_op->_thread.Create(AsyncOpImpl::ThreadProc, _op, _op->_stackSize))
				return -1;
		}
		return 0;

		case WM_DESTROY:
			_op->_thread.Close();
		break;

		case WM_SIZE:
		{
			WinRect rc(*this);

			SetWindowPos(_list, 0, rc.left + 4, rc.top + 4, rc.Width() - 8, rc.Height() - 34, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(_close, 0, rc.right - 64, rc.bottom - 24, 60, 20, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(_progress, 0, rc.left + 4, rc.bottom - 22, rc.Width() - 74, 16, SWP_NOACTIVATE | SWP_NOZORDER);
		}
		break;

		case WM_APP:
		{
			if (_op->IsDone())
			{
				SetWindowText(_close, "Close");
				EnableWindow(_close, TRUE);

				GetSystemMenu(*this, TRUE);

				RedrawWindow(*this, 0, 0, RDW_FRAME | RDW_INVALIDATE);

				if (!_op->_keepOpen)
				{
					if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) == 0)
					{
						EndModal(IDOK);
					}
				}
			}
		}
		return 0;

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDCANCEL:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						if (_op->IsDone())
						{
							EndModal(IDOK);
						}
						else
						{
							SetWindowText(_close, "Cancel");
							EnableWindow(_close, FALSE);

							_op->_keepOpen		= true;
							_op->_isCancelled	= true;
						}
					}
				return 0;
			}
		}
		break;

		case WM_NOTIFY:
		{
			NMHDR* hdr = (NMHDR*)lParam;

			if ((hdr->idFrom == IDC_LIST) && (hdr->code == NM_RCLICK)) // WTF is up with NM_RCLICK
			{
				NMITEMACTIVATE* ia = (NMITEMACTIVATE*)hdr;

				ClientToScreen(_list, &ia->ptAction);

				HMENU menu = CreatePopupMenu();

				AppendMenu(menu, MF_STRING, 1, _T("Copy Selected"));
				AppendMenu(menu, MF_STRING, 2, _T("Copy All"));
				AppendMenu(menu, MF_STRING, 3, _T("Send as Email"));

				int result = TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON, ia->ptAction.x, ia->ptAction.y, 0, *this, 0);

				DestroyMenu(menu);

				if ((result == 1) || (result == 2) || (result == 3))
				{
					int index = -1;

					XtString text;

					if (result == 1)
					{
						while((index = ListView_GetNextItem(_list, index, LVNI_SELECTED)) != -1)
						{
							TCHAR msg[512] = _T("");

							LVITEM item = { 0 };

							item.iItem		= index;
							item.mask		= LVIF_TEXT | LVIF_IMAGE;
							item.pszText	= msg;
							item.cchTextMax	= 512;

							ListView_GetItem(_list, &item);

							if (item.iImage == 0)
								Append<char>(text, "*");
							else if (item.iImage == 1)
								Append<char>(text, "?");
							else if (item.iImage == 2)
								Append<char>(text, "!!!");

							Append<char>(text, "\t");
							Append<char>(text, msg);
							Append<char>(text, "\r\n");
						}
					}
					else
					{
						int count = ListView_GetItemCount(_list);

						for(int i = 0; i < count; i++)
						{
							TCHAR msg[512] = _T("");

							LVITEM item = { 0 };

							item.iItem		= i;
							item.mask		= LVIF_TEXT | LVIF_IMAGE;
							item.pszText	= msg;
							item.cchTextMax	= 512;

							ListView_GetItem(_list, &item);

							if (item.iImage == 0)
								Append<char>(text, "*");
							else if (item.iImage == 1)
								Append<char>(text, "?");
							else if (item.iImage == 2)
								Append<char>(text, "!!!");

							Append<char>(text, "\t");
							Append<char>(text, msg);
							Append<char>(text, "\r\n");
						}
					}

					if (result == 3)
					{
						XtString dbv;
						dbv = Format("%i", XtGetDbVersion());

						XtString body;

						body = "\r\n\r\n\r\nSorcery build date: " + GetExeBuildTime() + "\r\n";
						body += "Xt db version: " + dbv + "\r\n\r\n";
						body += "Loaded modules:\r\n";

						HMODULE modules[2048];
						DWORD needed = 0;

						if (EnumProcessModules(GetCurrentProcess(), modules, sizeof(modules), &needed) != FALSE)
						{
							int count = needed / sizeof(HMODULE);

							for(int i = 0; i < count; i++)
							{
								char fname[MAX_PATH];

								if (GetModuleFileNameEx(GetCurrentProcess(), modules[i], fname, MAX_PATH) > 0)
								{
									body += XtString("\t") + fname + "\r\n";
								}
							}
						}

						body += "\r\nLog:\r\n" + text;

						SendEmail("Sorcery Error Log", body);
					}
					else
					{
						if (OpenClipboard(*this))
						{
							EmptyClipboard();

							HGLOBAL hdl = GlobalAlloc(GMEM_MOVEABLE, text.Length() + 1);

							if (hdl != 0)
							{
								void* mem = GlobalLock(hdl);

								if (mem)
								{
									memcpy(mem, (const char*)text, text.Length() + 1);

									GlobalUnlock(hdl);

									SetClipboardData(CF_TEXT, hdl);
								}
								else
									GlobalFree(hdl);
							}

							CloseClipboard();
						}
					}
				}

				return 0;
			}
		}
		break;

		case WM_TIMER:
		{
			if (wParam == WM_APP)
			{
				DWORD timeSince = GetTickCount() - _op->_lastProgressUpdate;

				if ((timeSince > 3000) && !_op->IsDone())
				{
					LONG style = GetWindowLong(_progress, GWL_STYLE);

					if ((style & PBS_MARQUEE) == 0)
					{
						SetWindowLong(_progress, GWL_STYLE, style | PBS_MARQUEE);
						SendMessage(_progress, PBM_SETMARQUEE, TRUE, 50);
					}

					_op->_lastProgressUpdate = GetTickCount();
				}

				return 0;
			}
		}
		break;
	}

	return DlgWindow::WndProc(msg, wParam, lParam);
}

//
// AsyncOpImpl
//

AsyncOpImpl::AsyncOpImpl(HWND parentWindow) :
	_parentWindow(parentWindow),
	_window(this),
	_done(false),
	_keepOpen(false),
	_isCancelled(false),
	_result(false),
	_lastProgressUpdate(0),
	_colWidth(600 - 32 - 4),
	_oldPos(-1)
{
}

AsyncOpImpl::~AsyncOpImpl()
{
}

bool AsyncOpImpl::InvokeEx(AsyncOpCallback callback, void* context, int stackSize, bool noWindow)
{
	_callback			= callback;
	_context			= context;
	_done				= false;
	_keepOpen			= false;
	_isCancelled		= false;
	_result				= false;
	_lastProgressUpdate	= 0;
	_oldPos				= -1;
	_stackSize			= stackSize;

	bool modalResult = true;

	if (!noWindow)
	{
		WinPoint pos(CW_USEDEFAULT, CW_USEDEFAULT);

		if (_parentWindow != 0)
		{
			WinRect rc;
			GetWindowRect(_parentWindow, &rc);

			pos = WinPoint(
					rc.left + (rc.Width() / 2) - 300,
					rc.top + (rc.Height() / 2) - 150
				);
		}

		modalResult = _window.DoModal(pos, WinSize(600, 300), _parentWindow) > 0;
	}
	else
	{
		_result = _callback(&AsyncOp(this), _context);
	}

	_thread.Close();

	_callback	= 0;
	_context	= 0;

	return modalResult && _result;
}

void AsyncOpImpl::NewMessage(AsyncOpIcon icon, const XtString& text)
{
	if (!_window)
		return;

	const char* start	= text;
	const char* ptr		= text;

	for(;;)
	{
		while((*ptr != '\r') && (*ptr != '\n') && (*ptr != '\0'))
			ptr++;

		NewMessageSub(icon, XtString(start, (int)(ptr - start)));

		while((*ptr == '\r') || (*ptr == '\n'))
			ptr++;

		if (*ptr == '\0')
			break;

		start	= ptr;
		icon	= kAoiNone;
	}
}

void AsyncOpImpl::AppendMessage(const XtString& text)
{
	if (!_window)
		return;

	NewMessage(kAoiNone, text);
}

void AsyncOpImpl::SetTitle(const char* title)
{
	if (!_window)
		return;

	SetWindowText(_window, title);
}

void AsyncOpImpl::SetProgress(float fraction)
{
	if (!_window)
		return;

	int pos = (int)(fraction * 1000.0f);

	if (pos < 0)
		pos = 0;
	else if (pos > 1000)
		pos = 1000;

	LONG style = GetWindowLong(_window._progress, GWL_STYLE);

	if ((style & PBS_MARQUEE) != 0)
	{
		SetWindowLong(_window._progress, GWL_STYLE, style & ~PBS_MARQUEE);
		PostMessage(_window._progress, PBM_SETMARQUEE, FALSE, 1000);
	}

	if (_oldPos != pos)
	{
		_oldPos = pos;

		PostMessage(_window._progress, PBM_SETPOS, pos, 0);
	}

	_lastProgressUpdate = GetTickCount();
}

void AsyncOpImpl::SetStayOpen()
{
	_keepOpen = true;
}

bool AsyncOpImpl::IsDone()
{
	_lock.Wait();
	bool done = _done;
	_lock.Release();

	return done;
}

void AsyncOpImpl::NewMessageSub(AsyncOpIcon icon, const XtString& text)
{
	LVITEM item = { 0 };

	item.mask		= LVIF_TEXT | LVIF_IMAGE;
	item.iItem		= (int)ListView_GetItemCount(_window._list);
	item.iSubItem	= 0;

	XtString indented;

	if (icon == kAoiNone)
		indented = "  " + text;
	else
		indented = text;

	item.pszText = (LPSTR)(const char*)indented;

	if (icon == kAoiError)
		item.iImage = 2;
	else if (icon == kAoiWarning)
		item.iImage = 1;
	else if (icon == kAoiInfo)
		item.iImage = 0;
	else
		item.iImage = I_IMAGENONE;

	item.iItem = ListView_InsertItem(_window._list, &item);

	ListView_EnsureVisible(_window._list, item.iItem, false);

	{
		HDC ic = CreateIC(_T("DISPLAY"), 0, 0, 0);
		SelectObject(ic, GetStockObject(DEFAULT_GUI_FONT));

		WinSize sz;
		GetTextExtentPoint32(ic, text, text.Length(), &sz);

		sz.cx += 40;

		if (sz.cx > _colWidth)
		{
			_colWidth = sz.cx;
			ListView_SetColumnWidth(_window._list, 0, _colWidth);
		}

		DeleteDC(ic);
	}

	if ((icon == kAoiWarning) || (icon == kAoiError))
		_keepOpen = true;
}

void AsyncOpImpl::ThreadProc(void* context)
{
	AsyncOpImpl* op = (AsyncOpImpl*)context;

	AsyncOp interfaceOp(op);

	op->_result = op->_callback(&interfaceOp, op->_context);

	op->SetProgress(1.0f);

	HWND hwnd = op->_window;

	op->_lock.Wait();
	op->_done = true;
	op->_lock.Release();

	PostMessage(hwnd, WM_APP, 0, 0);
}

//
// AsyncOp
//

AsyncOp::AsyncOp(class AsyncOpImpl* impl) :
	_impl(impl)
{
}

AsyncOp::~AsyncOp()
{
}

void AsyncOp::NewMessage(AsyncOpIcon icon, const XtString& text)
{
	_impl->NewMessage(icon, text);
}

void AsyncOp::AppendMessage(const XtString& text)
{
	_impl->AppendMessage(text);
}

void AsyncOp::SetTitle(const char* title)
{
	_impl->SetTitle(title);
}

void AsyncOp::SetProgress(float fraction)
{
	_impl->SetProgress(fraction);
}

void AsyncOp::SetStayOpen()
{
	_impl->SetStayOpen();
}

HWND AsyncOp::Window()
{
	return _impl->_window;
}

bool AsyncOp::IsCancelled()
{
	return _impl->_isCancelled;
}

bool AsyncInvokeEx(HWND parentWindow, AsyncOpCallback callback, void* context, int stackSize, bool noWindow)
{
	AsyncOpImpl impl(parentWindow);

	return impl.InvokeEx(callback, context, stackSize, noWindow);
}
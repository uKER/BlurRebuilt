#include "Core/XtWinPch.h"
#include "FinderImpl.h"

FcCombo::FcCombo(FinderRow* row) :
	FinderControl(row),
	_hwnd(0),
	_id(this)
{
}

FcCombo::~FcCombo()
{
}

void FcCombo::Clear()
{
	if (!_hwnd)
		return;

	SendMessage(_hwnd, CB_RESETCONTENT, 0, 0);
}

void FcCombo::AddString(const char* text)
{
	if (!_hwnd)
		return;

	SendMessage(_hwnd, CB_ADDSTRING, 0, (LPARAM)text);
}

void FcCombo::SetDroppedWidth(int width)
{
	if (!_hwnd)
		return;

	SendMessage(_hwnd, CB_SETDROPPEDWIDTH, (WPARAM)width, 0);
}

void FcCombo::SetSelection(int index)
{
	if (!_hwnd)
		return;

	SendMessage(_hwnd, CB_SETCURSEL, index, 0);
}

int FcCombo::GetSelection()
{
	if (!_hwnd)
		return -1;

	return (int)SendMessage(_hwnd, CB_GETCURSEL, 0, 0);
}

bool FcCombo::Create()
{
	if (!_id.Allocate())
		return false;

	if ((_hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "XTWINCOMBOBOX", "", 
		CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DISABLENOSCROLL,
		0, 0, 0, 0, *Row(), _id.IdAsHMenu(), 0, 0)) == 0)
	{
		return false;
	}

	SendMessage(_hwnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), FALSE);

	OnCreate();

	return true;
}

void FcCombo::Layout(WinRect& rc)
{
	if (!_hwnd)
		return;

	EnableWindow(_hwnd, IsEnabled());
	SetWindowPos(_hwnd, 0, rc.left, rc.top, rc.Width(), rc.Height() + 200, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
}

void FcCombo::OnCommand(int id, int code)
{
	if (id == _id)
	{
		if (code == CBN_SETFOCUS)
			OnFocus();
		else if (code == CBN_SELCHANGE)
			OnChanged();
	}
}
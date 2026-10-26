#include "Core/XtWinPch.h"
#include "FinderImpl.h"

FcEdit::FcEdit(FinderRow* row) :
	FinderControl(row),
	_hwnd(0),
	_id(this)
{
}

FcEdit::~FcEdit()
{
}

void FcEdit::SetText(const XtString& text)
{
	if (_hwnd)
		SetWindowText(_hwnd, text);
}

XtString FcEdit::GetText()
{
	return SzGetWindowText(_hwnd);
}

bool FcEdit::Create()
{
	if (!_id.Allocate())
		return false;

	if ((_hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, "", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, *Row(), _id.IdAsHMenu(), 0, 0)) == 0)
	{
		return false;
	}

	SendMessage(_hwnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), FALSE);

	OnCreate();

	return true;
}

void FcEdit::Layout(WinRect& rc)
{
	if (!_hwnd)
		return;

	EnableWindow(_hwnd, IsEnabled());
	SetWindowPos(_hwnd, 0, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
}

void FcEdit::OnCommand(int id, int code)
{
	if (id == _id)
	{
		if (code == EN_SETFOCUS)
			OnFocus();
		else if (code == EN_CHANGE)
			OnChanged();
	}
}
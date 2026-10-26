#include "Core/StdAfx.h"
#include "XtWin/Core.h"
#include "Application/SorceryApp.h"
#include "Application/SorceryView.h"
#include "DynamicForm/DynamicForm.h"
#include "Xt/XtTool.h"
#include "Xt/Impl/Grimoire.h"
#include "Resource/Resource.h"

//
// DynamicForm
//

class SfcDynamicForm : public ScarabFormContext
{
public:
	SfcDynamicForm(SorceryDocument* doc) :
		_doc(doc)
	{
	}

public:
	virtual void OpenRecord(Xt::DomRecord* record)
	{
		_doc->_app._window.OpenRecord(record);
	}

private:
	SorceryDocument* _doc;
};

//
// DynamicFormMdiWindow
//

DynamicFormMdiWindow::DynamicFormMdiWindow(SorceryView& view) :
	_view(view),
	_form(0),
	_lastFocus(0)
{
}

DynamicFormMdiWindow::~DynamicFormMdiWindow()
{
	delete _form;
}

bool DynamicFormMdiWindow::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!MdiWindow::PreCreateWindow(cs))
		return false;

	cs->dwExStyle |= WS_EX_CLIENTEDGE;

	return true;
}

LRESULT DynamicFormMdiWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			SendMessage(*this, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_SORCERY)));

			WinRect rcClient(*this);

			if ((_form = new ScarabForm(new SfcDynamicForm(_view._document))) == 0)
				return -1;

			if (!_form->Create(rcClient, *this))
				return -1;

			DWORD s = GetTickCount();

			switch(_view._document->_kind)
			{
				case kSdkRecord:
					if (Xt::DomRecord* dom = AsRecord(_view._document->_dom))
						_form->SetRecord(dom);
				break;

				case kSdkPack:
					if (Xt::DomPack* dom = AsPack(_view._document->_dom))
						_form->SetDom(&dom->properties, FindTypeByName(_view._app._db->schema, "XtPackProperties"));
				break;

				case kSdkDb:
					if (Xt::DomDb* dom = AsDb(_view._document->_dom))
						_form->SetDom(&dom->properties, FindTypeByName(_view._app._db->schema, "XtDbProperties"));
				break;
			}

			DebugLn("SetDom: %i", GetTickCount() - s);

			_lastFocus = 0;

			UpdateWindowTitle();

			RegisterDialogWindow(this);
		}
		break;

		case WM_DESTROY:
			UnregisterDialogWindow(this);
		break;

		case WM_CLOSE:
			_view.Destroy(true);
		return 0;

		case WM_SIZE:
		{
			WinRect rcClient(*this);

			SetWindowPos(*_form, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
		}
		break;

		case WM_LBUTTONDOWN:
			if (!IsChild(*this, GetFocus()))
				SendMessage(*this, kWmNotifySetFocus, 0, 0);
		return 0;

		case kWmNotifyActivate:
			if (LOWORD(wParam) == WA_INACTIVE)
				_lastFocus = GetFocus();
		return 0;

		case kWmNotifySetFocus:
			if (_lastFocus && IsChild(*this, _lastFocus) && IsWindowEnabled(_lastFocus))
				SetFocus(_lastFocus);
			else
				SetFocus(*_form);
		return 0;

		case WM_MDIACTIVATE:
			if ((HWND)lParam == *this)
			{
				if (_lastFocus && IsChild(*this, _lastFocus) && IsWindowEnabled(_lastFocus))
					SetFocus(_lastFocus);
				else
					SetFocus(*_form);
			}
			else if ((HWND)wParam == *this)
				_lastFocus = GetFocus();
		return 0;

	    case DM_GETDEFID: return 0;
	    case DM_SETDEFID: return 0;
	}

	return MdiWindow::WndProc(msg, wParam, lParam);
}

void DynamicFormMdiWindow::AdjustSizeToForm()
{
	if (!_form)
		return;

	WinSize sz = _form->IdealSize();

	WinRect rc(0, 0, sz.cx, sz.cy);

	AdjustWindowRectEx(&rc, GetWindowLong(*this, GWL_STYLE), FALSE, GetWindowLong(*this, GWL_EXSTYLE));

	WINDOWPLACEMENT wp = { 0 };
	wp.length = sizeof(wp);

	if (GetWindowPlacement(*this, &wp) == FALSE)
		return;

	WinRect rcMain(_view._app._window.GetMdiWnd());

	int width	= Clamp(rc.Width() + 36, 64, Max<int>(rcMain.Width() - wp.rcNormalPosition.left, 64));
	int height	= Clamp(rc.Height() + 48, 64, Max<int>(rcMain.Height() - wp.rcNormalPosition.top, 64));

	wp.rcNormalPosition.right	= wp.rcNormalPosition.left + width;
	wp.rcNormalPosition.bottom	= wp.rcNormalPosition.top + height;

	SetWindowPlacement(*this, &wp);
}

void DynamicFormMdiWindow::UpdateWindowTitle()
{
	XtString title;

	if (Xt::DomRecord* record = AsRecord(_view._document->_dom))
	{
		title = record->name + "." + record->root.type;

		if (record->dirty)
			title = title + "*";

		if (Xt::DomPack* pack = PackOf(record))
			title = pack->name + ": " + title;
	}
	else if (Xt::DomPack* pack = AsPack(_view._document->_dom))
	{
		title = "Pack: " + pack->name;

		if (pack->dirty)
			title = title + "*";
	}
	else if (Xt::DomDb* db = AsDb(_view._document->_dom))
	{
		title = "Db: " + db->name;

		if (db->dirty)
			title = title + "*";
	}
	else
		title = "(unnamed)";

	SetWindowText(*this, title);
}
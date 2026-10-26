#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

SnWideString::SnWideString(LfWideString* field) :
	_field(field),
	_id(this),
	_manualSet(0)
{
	_dom.SetCallback(DomCallback, this);
}

SnWideString::~SnWideString()
{
}

void SnWideString::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(AcquireField(parentDom, _field->name));
}

void SnWideString::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireField(parentDom, index));
}

bool SnWideString::OnBuild()
{
	if (!_id.Allocate())
		return false;

	if (_edit == 0)
	{
		int extraFlags = (_field->lines > 0) ? (ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL) : 0;

		if (!_edit.Create(WS_EX_CLIENTEDGE, WC_EDITW, L"", ES_AUTOHSCROLL | WS_CHILD | WS_TABSTOP | extraFlags, *Panel(), _id))
			return false;

		SendMessage(_edit, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
	}

	SyncControls();

	return true;
}

void SnWideString::OnMeasure(HDC ic)
{
	_size = WinSize(300, Max(1, _field->lines) * 20);
}

void SnWideString::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_edit, _client.left, _client.top, 300, Max(1, _field->lines) * 20, SWP_SHOWWINDOW);
}

void SnWideString::OnCommand(int id, int code)
{
	Xt::DomField* field = IsEnabled() ? AsField(_dom) : 0;

	if (field)
	{
		if ((id == _id) && (code == EN_CHANGE))
		{
			SendMessage(_edit, EM_EMPTYUNDOBUFFER, 0, 0);

			if (!_manualSet)
			{
				_manualSet++;

				Set(field, WcToMb(SzGetWindowTextW(_edit)));

				_manualSet--;
			}
		}
	}
}

HBRUSH SnWideString::OnControlColour(int id, int msg, HDC hdc, HWND hwnd)
{
	if (id == _id)
	{
		if (_field->attrs.FindAttr("ReadOnly") != 0)
		{
			SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
		}
	}

	return ScarabNode::OnControlColour(id, msg, hdc, hwnd);
}

ScarabClipTarget SnWideString::ClipTarget()
{
	return ScarabClipTarget(AsField(_dom), _field);
}

void SnWideString::SetDom(Xt::DomField* field)
{
	SetEnabled(Monitor(&_dom, field) != 0);

	SyncControls();
}

void SnWideString::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnWideString* node)
{
	if (event->code == Xt::kDomRemoved)
		node->SetDom(0);
	else if (*monitor == event->from)
		node->SyncControls();
}

void SnWideString::SyncControls()
{
	if (_manualSet > 0)
		return;

	Xt::DomField*	field			= IsEnabled() ? AsField(_dom) : 0;
	BOOL			enableControls	= field ? TRUE : FALSE;

	Edit_SetReadOnly(_edit, _field->attrs.FindAttr("ReadOnly") != 0);

	EnableWindow(_edit, enableControls);

	_manualSet++;

	SetWindowTextW(_edit, enableControls ? MbToWc(field->value) : L"");

	_manualSet--;
}
#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

SnString::SnString(LfString* field) :
	_field(field),
	_id(this),
	_manualSet(0)
{
	_dom.SetCallback(DomCallback, this);
}

SnString::~SnString()
{
	LazyCheck();
}

void SnString::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(AcquireField(parentDom, _field->name));
}

void SnString::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireField(parentDom, index));
}

bool SnString::OnBuild()
{
	if (!_id.Allocate())
		return false;

	if (_edit == 0)
	{
		int extraFlags = (_field->lines > 0) ? (ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL) : 0;

		if (!_edit.Create(WS_EX_CLIENTEDGE, WC_EDIT, _T(""), ES_AUTOHSCROLL | WS_CHILD | WS_TABSTOP | extraFlags, *Panel(), _id))
			return false;

		SendMessage(_edit, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
	}

	SyncControls();

	return true;
}

void SnString::OnMeasure(HDC ic)
{
	_size = WinSize(300, Max(1, _field->lines) * 20);
}

void SnString::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_edit, _client.left, _client.top, 300, Max(1, _field->lines) * 20, SWP_SHOWWINDOW);
}

void SnString::OnCommand(int id, int code)
{
	Xt::DomField* field = IsEnabled() ? AsField(_dom) : 0;

	if (field)
	{
		bool lazyBinding = _field->attrs.FindAttr("LazyBinding") != 0;

		if (id == _id)
		{
			switch(code)
			{
				case EN_CHANGE:
					SendMessage(_edit, EM_EMPTYUNDOBUFFER, 0, 0);

					if (!lazyBinding)
					{
						if (!_manualSet)
						{
							_manualSet++;

							Set(field, SzGetWindowText(_edit));

							_manualSet--;
						}
					}
				break;

				case EN_KILLFOCUS:
					if (lazyBinding)
					{
						Sz8 text = SzGetWindowText(_edit);

						if (AsText(field) != text)
						{
							if (!_manualSet)
							{
								_manualSet++;

								Set(field, text);

								_manualSet--;
							}
						}
					}
				break;
			}
		}
	}
}

HBRUSH SnString::OnControlColour(int id, int msg, HDC hdc, HWND hwnd)
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

ScarabClipTarget SnString::ClipTarget()
{
	return ScarabClipTarget(AsField(_dom), _field);
}

void SnString::SetDom(Xt::DomField* field)
{
	LazyCheck();

	SetEnabled(Monitor(&_dom, field) != 0);

	SyncControls();
}

void SnString::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnString* node)
{
	if (event->code == Xt::kDomRemoved)
		node->SetDom(0);
	else if (*monitor == event->from)
		node->SyncControls();
}

void SnString::SyncControls()
{
	if (_manualSet > 0)
		return;

	Xt::DomField*	field			= IsEnabled() ? AsField(_dom) : 0;
	BOOL			enableControls	= field ? TRUE : FALSE;

	Edit_SetReadOnly(_edit, _field->attrs.FindAttr("ReadOnly") != 0);

	EnableWindow(_edit, enableControls);

	_manualSet++;

	SetWindowText(_edit, enableControls ? field->value : "");

	_manualSet--;
}

void SnString::LazyCheck()
{
	if (!_manualSet && _field && _edit)
	{
		if (Xt::DomField* field = AsField(_dom))
		{
			if (_field->attrs.FindAttr("LazyBinding") != 0)
			{
				Sz8 text = SzGetWindowText(_edit);

				if (AsText(field) != text)
				{
					_manualSet++;

					Set(field, text);

					_manualSet--;
				}
			}
		}
	}
}
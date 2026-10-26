#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

XtI64 GetCrap(LarvaField* field, ScarabNode* node, const char* attrName, const char* paramName, XtI64 defValue)
{
	ScarabClipTarget ct(0, 0);
	
	if (node && node->Parent())
		ct = node->Parent()->ClipTarget();

	if (ct.field)
	{
		if (LarvaAttr* attr = ct.field->attrs.FindAttr("Override"))
		{
			if (LarvaParam* param = attr->FindParam(field->name))
			{
				if (LarvaAttrList* al = param->AsAttr())
				{
					if (LarvaAttr* attr2 = al->FindAttr(attrName))
					{
						if (LarvaParam* param2 = attr2->FindParam(paramName))
						{
							return param2->AsInt();
						}
					}
				}
			}
		}
	}

	return field->attrs.GetInt(attrName, paramName, defValue);
}

const int kTrackWidth = 300;

SnInt::SnInt(LfInt* field) :
	_field(field),
	_idEdit(this),
	_idSlider(this),
	_manualSet(0)
{
	_dom.SetCallback(DomCallback, this);
}

SnInt::~SnInt()
{
}

void SnInt::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(AcquireField(parentDom, _field->name));
}

void SnInt::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireField(parentDom, index));
}

bool SnInt::OnBuild()
{
	if (!_idEdit.Allocate() || !_idSlider.Allocate())
		return false;

	if (_edit == 0)
	{
		if (!_edit.Create(WS_EX_CLIENTEDGE, WC_EDIT, _T(""), ES_AUTOHSCROLL | WS_CHILD | WS_TABSTOP, *Panel(), _idEdit))
			return false;

		SendMessage(_edit, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
	}

	if (_slider == 0)
	{
		if (!_slider.Create(0, _T("XTWINTRACKBAR"), _T(""), TBS_NOTICKS | TBS_BOTTOM | TBS_HORZ | WS_CHILD, *Panel(), _idSlider))
			return false;

		SendMessage(_slider, TBM_SETRANGE, TRUE, MAKELONG(0, kTrackWidth));
	}

	SyncControls();

	return true;
}

void SnInt::OnMeasure(HDC ic)
{
	_size = WinSize((_rangeLow != _rangeHigh) ? 374 : 70, 20);
}

void SnInt::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_edit, _client.left, _client.top, 70, 20, SWP_SHOWWINDOW);
	layoutContext->SetWindowPos(_slider, _client.left + 74, _client.top, 300, 20, (_rangeLow != _rangeHigh) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
}

void SnInt::OnCommand(int id, int code)
{
	Xt::DomField* field = IsEnabled() ? AsField(_dom) : 0;

	if (field)
	{
		if ((id == _idEdit) && (code == EN_CHANGE))
		{
			SendMessage(_edit, EM_EMPTYUNDOBUFFER, 0, 0);

			if (!_manualSet)
			{
				_manualSet++;

				XtString text = SzGetWindowText(_edit);

				Set(field, text);

				if (_rangeLow != _rangeHigh)
				{
					XtI64 value = _strtoi64(text, 0, (int)_field->radix);

					int pos = Clamp((int)((((double)(value - _rangeLow) / (double)(_rangeHigh - _rangeLow)) * (double)kTrackWidth) + 0.5), 0, kTrackWidth);

					SendMessage(_slider, TBM_SETPOS, TRUE, pos);
				}

				_manualSet--;
			}
		}
	}
}

void SnInt::OnScroll(int id, int msg, int method, HWND scrollBar)
{
	Xt::DomField* field = IsEnabled() ? AsField(_dom) : 0;

	if (_rangeLow != _rangeHigh)
	{
		if (field && (id == _idSlider) && !_manualSet)
		{
			float factor = (float)(int)SendMessage(_slider, TBM_GETPOS, 0, 0) / (float)kTrackWidth;

			XtI64 value = Lerp(_rangeLow, _rangeHigh, factor);

			char buf[128];
			_i64toa_s(value, buf, sizeof(buf), (int)_field->radix);

			_manualSet++;

			Set(field, buf);

			SetWindowText(_edit, buf);

			_manualSet--;
		}
	}
}

HBRUSH SnInt::OnControlColour(int id, int msg, HDC hdc, HWND hwnd)
{
	if (id == _idEdit)
	{
		if (_field->attrs.FindAttr("ReadOnly") != 0)
		{
			SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
		}
	}

	return ScarabNode::OnControlColour(id, msg, hdc, hwnd);
}

ScarabClipTarget SnInt::ClipTarget()
{
	return ScarabClipTarget(AsField(_dom), _field);
}

void SnInt::SetDom(Xt::DomField* field)
{
	SetEnabled(Monitor(&_dom, field) != 0);

	SyncControls();
}

void SnInt::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnInt* node)
{
	if (event->code == Xt::kDomRemoved)
		node->SetDom(0);
	else if (*monitor == event->from)
		node->SyncControls();
}

void SnInt::SyncControls()
{
	if (_manualSet > 0)
		return;

	_rangeLow	= GetCrap(_field, this, "Range", "0", 0);
	_rangeHigh	= GetCrap(_field, this, "Range", "1", 0);

	bool enableSlider = _rangeLow != _rangeHigh;

	Xt::DomField*	field			= IsEnabled() ? AsField(_dom) : 0;
	BOOL			enableControls	= field ? TRUE : FALSE;

	Edit_SetReadOnly(_edit, _field->attrs.FindAttr("ReadOnly") != 0);

	EnableWindow(_edit,		enableControls);
	EnableWindow(_slider,	enableSlider && enableControls && (_field->attrs.FindAttr("ReadOnly") == 0));

	ShowWindow(_slider, enableSlider ? SW_SHOW : SW_HIDE);

	_manualSet++;

	SetWindowText(_edit, enableControls ? field->value : "");

	if (enableSlider)
	{
		XtI64 value = _strtoui64(enableControls ? field->value : "", 0, (int)_field->radix);

		int pos = Clamp((int)((((double)(value - _rangeLow) / (double)(_rangeHigh - _rangeLow)) * (double)kTrackWidth) + 0.5), 0, kTrackWidth);

		SendMessage(_slider, TBM_SETPOS, TRUE, pos);
	}

	_manualSet--;
}
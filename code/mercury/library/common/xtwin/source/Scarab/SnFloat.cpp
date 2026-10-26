#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

double GetCrap(LarvaField* field, ScarabNode* node, const char* attrName, const char* paramName, double defValue)
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
							return param2->AsFloat();
						}
					}
				}
			}
		}
	}

	return field->attrs.GetFloat(attrName, paramName, defValue);
}

const int kTrackWidth = 300;

SnFloat::SnFloat(LfFloat* field) :
	_field(field),
	_idEdit(this),
	_idSlider(this),
	_idIcon(this),
	_manualSet(0),
	_rangeLow(0.0),
	_rangeHigh(0.0),
	_controlWidth(0)
{
	_dom.SetCallback(DomCallback, this);

	_controlWidth = (int)_field->attrs.GetInt("Width", "0", 70);
}

SnFloat::~SnFloat()
{
}

void SnFloat::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(AcquireField(parentDom, _field->name));
}

void SnFloat::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireField(parentDom, index));
}

bool SnFloat::OnBuild()
{
	if (!_idEdit.Allocate() || !_idSlider.Allocate() || !_idIcon.Allocate())
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

	/*if (_icon == 0)
	{
		if (!_icon.Create(0, "STATIC", "", SS_BITMAP | SS_CENTERIMAGE | WS_CHILD, *Panel(), _idIcon))
			return false;
		
		SendMessage(_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)LoadImage(
				GetXtWinIconDll(), MAKEINTRESOURCE(IDB_FIELD_WARNING), IMAGE_BITMAP, 0, 0, LR_SHARED | LR_VGACOLOR | LR_LOADTRANSPARENT
			));
	}*/

	SyncControls();

	return true;
}

void SnFloat::OnMeasure(HDC ic)
{
	_size = WinSize(_controlWidth + ((_rangeLow != _rangeHigh) ? 304 : 0), 20);
}

void SnFloat::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_edit, _client.left, _client.top, _controlWidth, 20, SWP_SHOWWINDOW);
	layoutContext->SetWindowPos(_slider, _client.left + _controlWidth + 4, _client.top, 300, 20, (_rangeLow != _rangeHigh) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
	//SetWindowPos(_icon, HWND_BOTTOM, _client.left + 378, _client.top + 2, 16, 16, SWP_SHOWWINDOW);
}

void SnFloat::OnCommand(int id, int code)
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
					double value = atof(text);

					int pos = Clamp((int)((((double)(value - _rangeLow) / (double)(_rangeHigh - _rangeLow)) * (double)kTrackWidth) + 0.5), 0, kTrackWidth);

					SendMessage(_slider, TBM_SETPOS, TRUE, pos);
				}

				_manualSet--;
			}
		}
	}
}

void SnFloat::OnScroll(int id, int msg, int method, HWND scrollBar)
{
	Xt::DomField* field = IsEnabled() ? AsField(_dom) : 0;

	if (_rangeLow != _rangeHigh)
	{
		if (field && (id == _idSlider) && !_manualSet)
		{
			float factor = (float)(int)SendMessage(_slider, TBM_GETPOS, 0, 0) / (float)kTrackWidth;

			double value = Lerp(_rangeLow, _rangeHigh, factor);

			char buf[128];
			_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%.3f", value);

			_manualSet++;

			Set(field, buf);

			SetWindowText(_edit, buf);

			_manualSet--;
		}
	}
}

HBRUSH SnFloat::OnControlColour(int id, int msg, HDC hdc, HWND hwnd)
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

ScarabClipTarget SnFloat::ClipTarget()
{
	return ScarabClipTarget(AsField(_dom), _field);
}

void SnFloat::SetDom(Xt::DomField* field)
{
	SetEnabled(Monitor(&_dom, field) != 0);

	SyncControls();
}

void SnFloat::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnFloat* node)
{
	if (event->code == Xt::kDomRemoved)
		node->SetDom(0);
	else if (*monitor == event->from)
		node->SyncControls();
}

void SnFloat::SyncControls()
{
	if (_manualSet > 0)
		return;

	_rangeLow	= GetCrap(_field, this, "Range", "0", 0.0f);
	_rangeHigh	= GetCrap(_field, this, "Range", "1", 0.0f);

	bool enableSlider = (_rangeLow != _rangeHigh);

	Xt::DomField*	field			= IsEnabled() ? AsField(_dom) : 0;
	BOOL			enableControls	= field ? TRUE : FALSE;

	Edit_SetReadOnly(_edit, _field->attrs.FindAttr("ReadOnly") != 0);

	EnableWindow(_edit,		enableControls);
	EnableWindow(_slider,	enableControls && enableSlider && (_field->attrs.FindAttr("ReadOnly") == 0));

	ShowWindow(_slider, enableSlider ? SW_SHOW : SW_HIDE);

	_manualSet++;

	SetWindowText(_edit, enableControls ? field->value : "");

	if (enableSlider)
	{
		double value = atof(enableControls ? field->value : "");

		int pos = Clamp((int)((((double)(value - _rangeLow) / (double)(_rangeHigh - _rangeLow)) * (double)kTrackWidth) + 0.5), 0, kTrackWidth);

		SendMessage(_slider, TBM_SETPOS, TRUE, pos);
	}

	_manualSet--;
}
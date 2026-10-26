#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

SnFlagBool::SnFlagBool(LarvaOption* field) :
	_field(field),
	_id(this),
	_manualSet(0)
{
	_labelVisible	= false;
	_labelText		= _field->label;

	_dom.SetCallback(DomCallback, this);
}

SnFlagBool::~SnFlagBool()
{
}

void SnFlagBool::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(AcquireField(parentDom, _field->name));
}

void SnFlagBool::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireField(parentDom, index));
}

bool SnFlagBool::OnBuild()
{
	if (!_id.Allocate())
		return false;

	if (_button == 0)
	{
		if (!_button.Create(0, WC_BUTTON, _labelText, BS_AUTOCHECKBOX | BS_CHECKBOX | WS_CHILD | WS_TABSTOP, *Panel(), _id))
			return false;

		SendMessage(_button, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
	}

	SyncControls();

	return true;
}

void SnFlagBool::OnMeasure(HDC ic)
{
	_size = WinSize(_labelSize.cx + 20, 14);
}

void SnFlagBool::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_button, _client.left, _client.top, _labelSize.cx + 20, 14, SWP_SHOWWINDOW);
}

void SnFlagBool::OnCommand(int id, int code)
{
	Xt::DomField* field = IsEnabled() ? AsField(_dom) : 0;

	if (field)
	{
		if ((id == _id) && (code == BN_CLICKED) && !_manualSet)
		{
			bool checked = (int)SendMessage(_button, BM_GETCHECK, 0, 0) == BST_CHECKED;

			_manualSet++;

			Set(field, checked ? "1" : "0");

			_manualSet--;
		}
	}
}

void SnFlagBool::SetDom(Xt::DomField* field)
{
	SetEnabled(Monitor(&_dom, field) != 0);

	SyncControls();
}

void SnFlagBool::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnFlagBool* node)
{
	if (event->code == Xt::kDomRemoved)
		node->SetDom(0);
	else if (*monitor == event->from)
		node->SyncControls();
}

void SnFlagBool::SyncControls()
{
	if (_manualSet > 0)
		return;

	Xt::DomField*	field			= IsEnabled() ? AsField(_dom) : 0;
	BOOL			enableControls	= field ? TRUE : FALSE;
	bool			checked			= atoi(AsText(field)) != 0;

	EnableWindow(_button, enableControls && (_field->attrs.FindAttr("ReadOnly") == 0));

	_manualSet++;

	SendMessage(_button, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);

	_manualSet--;
}
#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

SnEnum::SnEnum(LfEnum* field) :
	_field(field),
	_id(this),
	_manualSet(0),
	_default(0)
{
	_dom.SetCallback(DomCallback, this);
}

SnEnum::~SnEnum()
{
}

void SnEnum::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(AcquireField(parentDom, _field->name));
}

void SnEnum::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireField(parentDom, index));
}

bool SnEnum::OnBuild()
{
	if (!_id.Allocate())
		return false;

	if (_combo == 0)
	{
		if (!_combo.Create(WS_EX_CLIENTEDGE, _T("XTWINCOMBOBOX"), _T(""),
					CBS_DROPDOWNLIST | WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DISABLENOSCROLL,
			200, 20 + 200, *Panel(), _id))
		{
			return false;
		}

		SendMessage(_combo, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);

		PopulateCombo();
	}

	SyncControls();

	return true;
}

void SnEnum::OnMeasure(HDC ic)
{
	_size = WinSize(200, 20);
}

void SnEnum::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_combo, _client.left, _client.top, 200, 20 + 200, SWP_SHOWWINDOW | SWP_NOSIZE);
}

void SnEnum::OnCommand(int id, int code)
{
	Xt::DomField* field = IsEnabled() ? AsField(_dom) : 0;

	if (field)
	{
		if ((id == _id) && (code == CBN_SELCHANGE) && !_manualSet)
		{
			int index = ((int)SendMessage(_combo, CB_GETCURSEL, 0, 0));

			_manualSet++;

			if ((index >= 0) && (index < (int)_comboItems.count))
			{
				LarvaOption* option = _comboItems[index];

				Set(field, option->name);
			}
			else
				Set(field, "");

			_manualSet--;
		}
	}
}

ScarabClipTarget SnEnum::ClipTarget()
{
	return ScarabClipTarget(AsField(_dom), _field);
}

void SnEnum::SetDom(Xt::DomField* field)
{
	SetEnabled(Monitor(&_dom, field) != 0);

	SyncControls();
}

void SnEnum::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnEnum* node)
{
	if (event->code == Xt::kDomRemoved)
		node->SetDom(0);
	else if (*monitor == event->from)
		node->SyncControls();
}

bool SnEnum::ComboCompare(LarvaOption* lhs, LarvaOption* rhs)
{
	return _stricmp(lhs->label, rhs->label) < 0;
}

void SnEnum::SyncControls()
{
	if (_manualSet > 0)
		return;

	Xt::DomField*	field			= IsEnabled() ? AsField(_dom) : 0;
	BOOL			enableControls	= field ? TRUE : FALSE;
	int				found			= _default;

	XtString optName = AsText(field);

	for(XtUt::Array<LarvaOption*>::Iterator it(&_comboItems); it; ++it)
	{
		if (optName == (*it)->name)
			found = (int)it.index;
	}

	EnableWindow(_combo, enableControls && (_field->attrs.FindAttr("ReadOnly") == 0));

	_manualSet++;

	SendMessage(_combo, CB_SETCURSEL, found, 0);

	_manualSet--;
}

void SnEnum::PopulateCombo()
{
	Free(&_comboItems);

	for(XtUt::List<LarvaOption>::Iterator it(&_field->type->options); it; ++it)
		AppendEx(&_comboItems, (LarvaOption*)it);

	//XtUt::MergeSort(&_comboItems, ComboCompare);

	_default = 0;

	for(int i = 0; i < (int)Count(&_comboItems); i++)
	{
		if (_comboItems[i] == _field->type->options[0])
		{
			_default = i;
			break;
		}
	}

	SendMessage(_combo, CB_RESETCONTENT, 0, 0);

	for(XtUt::Array<LarvaOption*>::Iterator it(&_comboItems); it; ++it)
		SendMessage(_combo, CB_ADDSTRING, 0, (LPARAM)(LPSTR)(const char*)(*it)->label);
}
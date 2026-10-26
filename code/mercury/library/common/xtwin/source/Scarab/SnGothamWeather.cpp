#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

SnGothamWeather::SnGothamWeather(LfControl* field) :
	SnControl(field),
	_idCombo(this)
{
	_domCity.SetCallback(DomCallback, this);
	_domWeather.SetCallback(DomCallback, this);
}

SnGothamWeather::~SnGothamWeather()
{
}

void SnGothamWeather::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(parentDom);
}

void SnGothamWeather::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireBlock(parentDom, index));
}

bool SnGothamWeather::OnBuild()
{
	if (!_idCombo.Allocate())
		return false;

	if (_wndCombo == 0)
	{
		if (!_wndCombo.Create(WS_EX_CLIENTEDGE, _T("XTWINCOMBOBOX"), _T(""),
				CBS_DROPDOWNLIST | WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DISABLENOSCROLL,
			200, 20 + 200, *Panel(), _idCombo))
		{
			return false;
		}

		SendMessage(_wndCombo, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
	}

	SyncControls();

	return true;
}

void SnGothamWeather::OnMeasure(HDC ic)
{
	_size = WinSize(200, 20);
}

void SnGothamWeather::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_wndCombo, _client.left, _client.top, 200, 20 + 200, SWP_SHOWWINDOW | SWP_NOSIZE);
}

void SnGothamWeather::OnCommand(int id, int code)
{
	Xt::DomField* fieldCity		= IsEnabled() ? AsField(_domCity) : 0;
	Xt::DomField* fieldWeather	= IsEnabled() ? AsField(_domWeather) : 0;

	if (fieldCity && fieldWeather)
	{
		if ((id == _idCombo) && (code == CBN_SETFOCUS))
		{
			PopulateCombo();
		}
		else if ((id == _idCombo) && (code == CBN_SELCHANGE))
		{
			int index = (int)SendMessage(_wndCombo, CB_GETCURSEL, 0, 0) - 1;

			if ((index >= 0) && (index < (int)_items.count))
				Set(fieldWeather, _items[index].name);
			else
				Set(fieldWeather, "");
		}
	}
}

void SnGothamWeather::SetDom(Xt::DomBlock* block)
{
	Xt::DomRecord* record = RecordOf(block);

	if (!record)
		return;

	bool hasCity	= Monitor(&_domCity, AcquireField(&record->root.data, "City")) != 0;
	bool hasWeather	= Monitor(&_domWeather, AcquireField(block, "Weather")) != 0;

	SetEnabled(hasCity && hasWeather);

	SyncControls();
}

void SnGothamWeather::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnGothamWeather* node)
{
	if (event->code == Xt::kDomRemoved)
		node->SetDom(0);
	else if (*monitor == event->from)
		node->SyncControls();
}

void SnGothamWeather::SyncControls()
{
	Xt::DomField* fieldCity		= IsEnabled() ? AsField(_domCity) : 0;
	Xt::DomField* fieldWeather	= IsEnabled() ? AsField(_domWeather) : 0;

	bool enableCombo = false;

	if (fieldCity)
	{
		if (Xt::DomRecord* record = FindRecordById(Panel()->Db(), Xt::ParseId(fieldCity->value)))
			enableCombo = true;
	}

	EnableWindow(_wndCombo, enableCombo);

	PopulateCombo();
}

void SnGothamWeather::PopulateCombo()
{
	Xt::DomField* fieldCity		= IsEnabled() ? AsField(_domCity) : 0;
	Xt::DomField* fieldWeather	= IsEnabled() ? AsField(_domWeather) : 0;

	SendMessage(_wndCombo, CB_RESETCONTENT, 0, 0);

	Free(&_items);

	if (fieldCity &&  fieldWeather)
	{
		SendMessage(_wndCombo, CB_ADDSTRING, 0, (LPARAM)"(null)");

		XtId recordId = Xt::ParseId(fieldCity->value);

		if (Xt::DomRecord* record = FindRecordById(Panel()->Db(), recordId))
		{
			if (Xt::DomArray* environment = AcquireArray(&record->root.data, "Environment"))
			{
				Allocate(&_items, 0, Count(environment));

				for(Xt::DomArray::IndexIterator it(environment); it; ++it)
				{
					Xt::DomBlock* block = AsBlock(it);

					if (!block)
						continue;

					if (Item* item = AppendN(&_items, 1))
						item->name = AsText(At(block, "Name"));
				}
			}
		}

		XtUt::MergeSort(&_items, ComboCompare);

		int found = 0;

		for(XtUt::Array<Item>::Iterator it(&_items); it; ++it)
		{
			if (it->name == fieldWeather->value)
				found = (int)(it.index + 1);

			SendMessage(_wndCombo, CB_ADDSTRING, 0, (LPARAM)(LPSTR)(const char*)it->name);
		}

		SendMessage(_wndCombo, CB_SETCURSEL, found, 0);
	}
}

bool SnGothamWeather::ComboCompare(const Item& lhs, const Item& rhs)
{
	return _stricmp(lhs.name, rhs.name) < 0;
}
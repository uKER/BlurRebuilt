#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

SnHandle::SnHandle(LfHandle* field) :
	_field(field),
	_idCombo(this),
	_idOpen(this)
{
	_dom.SetCallback(DomCallback, this);
}

SnHandle::~SnHandle()
{
}

void SnHandle::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(AcquireField(parentDom, _field->name));
}

void SnHandle::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireField(parentDom, index));
}

bool SnHandle::OnBuild()
{
	if (!_idCombo.Allocate() || !_idOpen.Allocate())
		return false;

	if (_combo == 0)
	{
		if (!_combo.Create(WS_EX_CLIENTEDGE, _T("XTWINCOMBOBOX"), _T(""),
				CBS_DROPDOWNLIST | WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DISABLENOSCROLL | CBS_OWNERDRAWFIXED,
			300, 20 + 200, *Panel(), _idCombo))
		{
			return false;
		}

		SendMessage(_combo, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
		SendMessage(_combo, CB_SETDROPPEDWIDTH, 300, 0);
	}

	if (_open == 0)
	{
		if (!_open.Create(0, WC_BUTTON, _T("Open"), BS_FLAT | WS_CHILD | WS_TABSTOP, *Panel(), _idOpen))
			return false;

		SendMessage(_open, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
	}

	SyncControls();

	return true;
}

void SnHandle::OnMeasure(HDC ic)
{
	_size = WinSize(358, 20);
}

void SnHandle::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_combo, _client.left, _client.top, 300, 20 + 200, SWP_SHOWWINDOW | SWP_NOSIZE);
	layoutContext->SetWindowPos(_open, _client.left + 304, _client.top, 54, 20, SWP_SHOWWINDOW);
}

ScarabClipTarget SnHandle::ClipTarget()
{
	return ScarabClipTarget(AsField(_dom), _field);
}

void SnHandle::OnCommand(int id, int code)
{
	Xt::DomField* field = IsEnabled() ? AsField(_dom) : 0;

	if (field)
	{
		if ((id == _idCombo) && (code == CBN_SETFOCUS))
		{
			PopulateCombo(false);
		}
		else if ((id == _idCombo) && (code == CBN_SELCHANGE))
		{
			int index = (int)SendMessage(_combo, CB_GETCURSEL, 0, 0);

			if (Item* item = At(&_items, index))
				Set(field, Format("%08X", item->id));
			else
				Set(field, "");
		}
		else if ((id == _idOpen) && (code == BN_CLICKED))
		{
			XtId id = Xt::ParseId(AsText(field));

			if (Xt::DomRecord* record = FindRecordById(Panel()->Db(), id))
			{
				Panel()->FormContext()->OpenRecord(record);
			}
		}
	}
}

bool SnHandle::MeasureItem(int id, MEASUREITEMSTRUCT* mis)
{
	if (id != _idCombo)
		return false;

	mis->itemHeight = Panel()->DrawContext()->labelSizeY + 2;

	return true;
}

bool SnHandle::DrawItem(int id, DRAWITEMSTRUCT* dis)
{
	if (id != _idCombo)
		return false;

	if (dis->itemID == -1)
		return true;

	XtString recordName;
	XtString packName;

	if (Item* item = At(&_items, dis->itemID))
	{
		if (Xt::DomRecord* record = FindRecordById(Panel()->Db(), item->id))
		{
			recordName = item->label;

			if (Xt::DomPack* pack = Xt::PackOf(record))
				packName = "[" + pack->name + "]";
		}
		else
			recordName = "(null)";
	}
	else
		recordName = "(null)";

	TEXTMETRIC tm;
	GetTextMetrics(dis->hDC, &tm);

	WinSize szRecord;
	GetTextExtentPoint32(dis->hDC, recordName, recordName.Length(), &szRecord);

	WinSize szPack;
	GetTextExtentPoint32(dis->hDC, packName, packName.Length(), &szPack);

	int x = LOWORD(GetDialogBaseUnits()) / 8;
	int y = (dis->rcItem.bottom + dis->rcItem.top - tm.tmHeight) / 2;

	COLORREF oldBk = SetBkColor(dis->hDC,
			GetSysColor((dis->itemState & ODS_SELECTED) ? COLOR_HIGHLIGHT : COLOR_WINDOW)
		);

	COLORREF oldFg = SetTextColor(dis->hDC,
			GetSysColor((dis->itemState & ODS_SELECTED) ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT)
		);

	int recordLeft	= dis->rcItem.left + 1;
	int packLeft	= dis->rcItem.right - 2 - szPack.cx;

	if (recordLeft + szRecord.cx > (packLeft - 2))
	{
		WinSize szDots;
		GetTextExtentPoint32(dis->hDC, "...", 3, &szDots);

		const char*	p = recordName;
		int			l = recordName.Length();

		while(*p != '\0')
		{
			p++;
			l--;

			GetTextExtentPoint32(dis->hDC, p, l, &szRecord);

			if (recordLeft + szRecord.cx + szDots.cx < (packLeft - 2))
			{
				recordName = XtString("...") + p;
				break;
			}
		}

		if (*p == '\0')
			recordLeft = packLeft - 2 - szRecord.cx;
	}

	ExtTextOut(dis->hDC, recordLeft, y, ETO_CLIPPED | ETO_OPAQUE, &dis->rcItem, recordName, recordName.Length(), NULL);

	if ((dis->itemState & ODS_SELECTED) == 0)
		SetTextColor(dis->hDC, GetSysColor(COLOR_GRAYTEXT));

	ExtTextOut(dis->hDC, packLeft, y, ETO_CLIPPED, &dis->rcItem, packName, packName.Length(), NULL);
 
	SetTextColor(dis->hDC, oldFg);
	SetBkColor(dis->hDC, oldBk);
	
	if (dis->itemState & ODS_FOCUS)
		DrawFocusRect(dis->hDC, &dis->rcItem);

	return true;
}

void SnHandle::SetDom(Xt::DomField* field)
{
	SetEnabled(Monitor(&_dom, field) != 0);

	SyncControls();
}

void SnHandle::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnHandle* node)
{
	if (event->code == Xt::kDomRemoved)
		node->SetDom(0);
	else if (*monitor == event->from)
		node->SyncControls();
}

bool SnHandle::ComboCompare(const Item& lhs, const Item& rhs)
{
	return _stricmp(lhs.label, rhs.label) < 0;
}

void SnHandle::SyncControls()
{
	Xt::DomField*	field			= IsEnabled() ? AsField(_dom) : 0;
	bool			enableControls	= field ? TRUE : FALSE;
	bool			isValid			= false;

	XtId id = Xt::ParseId(AsText(IsEnabled() ? AsField(_dom) : 0));

	if (Xt::DomRecord* record = FindRecordById(Panel()->Db(), id))
	{
		LarvaSchema* schema = Panel()->Db()->schema;

		if (IsBaseOf(_field->baseType, TypeOf(schema, record)))
			isValid = true;
	}

	EnableWindow(_combo,	enableControls);
	EnableWindow(_open,		enableControls && isValid);

	PopulateCombo(GetFocus() != _combo);
}

void SnHandle::PopulateCombo(bool fast)
{
	Free(&_items);

	if (_combo == 0)
		return;

	if (!IsEnabled())
	{
		SendMessage(_combo, CB_RESETCONTENT, 0, 0);
		return;
	}

	XtId id = Xt::ParseId(AsText(IsEnabled() ? AsField(_dom) : 0));

	if (Item* item = XtUt::AppendExN(&_items, 1))
	{
		item->id	= 0;
		item->label	= "(null)";
	}

	if (fast)
	{
		if (Xt::DomRecord* record = FindRecordById(Panel()->Db(), id))
			AddItem(record, true);
	}
	else
	{
		Xt::DomRecord* selfRecord = FindRecordById(Panel()->Db(), id);

		for(Xt::DomDb::Iterator itPack(Panel()->Db()); itPack; ++itPack)
		{
			for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
			{
				AddItem(itRecord, selfRecord == itRecord);
			}
		}
	}

	XtUt::MergeSort(&_items, ComboCompare, 1, _items.count);

	int found = 0;
	int width = 300;

	HDC ic = 0;

	if (!fast)
	{
		ic = CreateIC(_T("DISPLAY"), 0, 0, 0);
		SelectObject(ic, Panel()->DrawContext()->labelFont);
	}

	SendMessage(_combo, CB_RESETCONTENT, 0, 0);

	for(XtUt::Array<Item>::Iterator it(&_items); it; ++it)
	{
		if (it->id == id)
			found = (int)it.index;

		SendMessage(_combo, CB_ADDSTRING, 0, 0);

		if (!fast)
		{
			WinSize sz;
			GetTextExtentPoint32(ic, it->label, it->label.Length(), &sz);

			width = Max(width, (int)sz.cx);
		}
	}

	if (!fast)
	{
		int packWidth = 0;

		for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator it(&Panel()->Db()->packs); it; ++it)
		{
			XtString packName = "[" + it->name + "]";

			WinSize szPack;
			GetTextExtentPoint32(ic, packName, packName.Length(), &szPack);

			packWidth = Max(packWidth, (int)szPack.cx);
		}

		DeleteDC(ic);
		SendMessage(_combo, CB_SETDROPPEDWIDTH, Max(4 + width + 4 + packWidth + 4 + 12, 300), 0);
	}

	SendMessage(_combo, CB_SETCURSEL, found, 0);
}

int CountChars(const char* text, int num)
{
	const char* start	= text;
	const char* best	= text;

	for(; *text != '\0'; text++)
	{
		if (*text == '.')
		{
			best = text + 1;

			if (--num <= 0)
				break;
		}
	}

	return best - start;
}

bool MatchPrefix(const char* s, const char* d, int count)
{
	while(*s && *d && (*s == *d))
	{
		if (--count <= 0)
			return true;

		s++;
		d++;
	}

	return false;
}

void SnHandle::AddItem(Xt::DomRecord* record, bool bypassFilter)
{
	LarvaSchema*	schema		= Panel()->Db()->schema;
	LtStruct*		recordType	= TypeOf(schema, record);

	if (!recordType)
		return;

	if (!IsBaseOf(_field->baseType, recordType))
		return;

	if (!bypassFilter)
	{
		int pathTokens = (int)Max(_field->attrs.GetInt("Filter", "byRecordPathTokens", 0), 0I64);

		if (pathTokens > 0)
		{
			if (Xt::DomRecord* self = RecordOf(_dom))
			{
				if (!MatchPrefix(self->name, record->name, CountChars(self->name, pathTokens)))
					return;
			}
		}
	}

	Item* item = AppendExN(&_items, 1);

	if (!item)
		return;

	item->id = record->id;

	XtString label = EvaluateSummaryText(Db(), schema, TypeOf(schema, record), &record->root, "{__name}");

	if (*label)
		item->label = label;
	else
		item->label = record->name;
}
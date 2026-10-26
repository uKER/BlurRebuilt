#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

SnLocString::SnLocString(LfLocString* field) :
	_field(field),
	_idRecord(this),
	_idRow(this),
	_idPreview(this),
	_idOpen(this),
	_manualSet(0)
{
	_domRecord.SetCallback(DomCallback, this);
	_domRow.SetCallback(DomCallback, this);
}

SnLocString::~SnLocString()
{
}

void SnLocString::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(AcquireBlock(parentDom, _field->name));
}

void SnLocString::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireBlock(parentDom, index));
}

bool SnLocString::OnBuild()
{
	if (!_idRecord.Allocate() || !_idRow.Allocate() || !_idPreview.Allocate() || !_idOpen.Allocate())
		return false;

	if (_record == 0)
	{
		if (!_record.Create(WS_EX_CLIENTEDGE, _T("XTWINCOMBOBOX"), _T(""),
				CBS_DROPDOWNLIST | WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DISABLENOSCROLL | CBS_OWNERDRAWFIXED,
			240, 20 + 200, *Panel(), _idRecord))
		{
			return false;
		}

		SendMessage(_record, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
		SendMessage(_record, CB_SETDROPPEDWIDTH, 400, 0);
	}

	if (_row == 0)
	{
		if (!_row.Create(WS_EX_CLIENTEDGE, _T("XTWINCOMBOBOX"), _T(""),
				CBS_DROPDOWNLIST | WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DISABLENOSCROLL,
			250, 20 + 200, *Panel(), _idRow))
		{
			return false;
		}

		SendMessage(_row, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
		SendMessage(_row, CB_SETDROPPEDWIDTH, 400, 0);
	}

	if (_preview == 0)
	{
		if (!_preview.Create(0, WC_EDITW, L"", ES_AUTOHSCROLL | WS_CHILD | WS_TABSTOP | WS_BORDER, *Panel(), _idPreview))
			return false;

		SendMessage(_preview, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);

		Edit_SetReadOnly(_preview, TRUE);
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

void SnLocString::OnMeasure(HDC ic)
{
	_size = WinSize(498, 46);
}

void SnLocString::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_record, _client.left, _client.top, 240, 20 + 200, SWP_SHOWWINDOW | SWP_NOSIZE);
	layoutContext->SetWindowPos(_row, _client.left + 248, _client.top, 250, 20 + 200, SWP_SHOWWINDOW | SWP_NOSIZE);
	layoutContext->SetWindowPos(_preview, _client.left, _client.top + 28, 450, 18, SWP_SHOWWINDOW);
	layoutContext->SetWindowPos(_open, _client.left + 458, _client.top + 27, 40, 20, SWP_SHOWWINDOW);
}

ScarabClipTarget SnLocString::ClipTarget()
{
	return ScarabClipTarget(AsBlock(_domBlock), _field);
}

void SnLocString::OnCommand(int id, int code)
{
	Xt::DomField* fieldRecord	= IsEnabled() ? AsField(_domRecord) : 0;
	Xt::DomField* fieldRow		= IsEnabled() ? AsField(_domRow) : 0;

	if (fieldRecord && fieldRow && !_manualSet)
	{
		if ((id == _idRecord) && (code == CBN_SETFOCUS))
		{
			PopulateRecord();
		}
		else if ((id == _idRow) && (code == CBN_SETFOCUS))
		{
			PopulateRow(false);
		}
		else if ((id == _idOpen) && (code == BN_CLICKED))
		{
			if (Xt::DomRecord* record = FindRecordById(Db(), Xt::ParseId(AsText(fieldRecord))))
				Panel()->FormContext()->OpenRecord(record);
		}
		else if ((id == _idRecord) && (code == CBN_SELCHANGE))
		{
			int				index	= (int)SendMessage(_record, CB_GETCURSEL, 0, 0);
			XtId			id		= (XtId)SendMessage(_record, CB_GETITEMDATA, index, 0);
			Xt::DomRecord*	record	= FindRecordById(Panel()->Db(), id);

			_manualSet++;

			if (record)
			{
				char idStr[64];

				_snprintf_s(idStr, sizeof(idStr), _TRUNCATE, "%08X", record->id);

				Set(fieldRecord, idStr);
			}
			else
				Set(fieldRecord, "");

			PopulateRow(GetFocus() != _row);

			_manualSet--;
		}
		else if ((id == _idRow) && (code == CBN_SELCHANGE))
		{
			int index = (int)SendMessage(_row, CB_GETCURSEL, 0, 0) - 1;

			_manualSet++;

			if ((index >= 0) && (index < (int)_items.count))
			{
				char idStr[64];

				_snprintf_s(idStr, sizeof(idStr), _TRUNCATE, "%08X", _items[index].id);

				Set(fieldRow, idStr);
			}
			else
				Set(fieldRow, "");

			_manualSet--;
		}
	}
}

void SnLocString::SetDom(Xt::DomBlock* block)
{
	bool hasRecord	= Monitor(&_domRecord, AcquireField(block, "record")) != 0;
	bool hasRow		= Monitor(&_domRow, AcquireField(block, "id")) != 0;

	Monitor(&_domBlock, block);

	SetEnabled(hasRecord && hasRow);

	SyncControls();
}

void SnLocString::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnLocString* node)
{
	if (event->code == Xt::kDomRemoved)
		node->SetDom(0);
	else if (*monitor == event->from)
		node->SyncControls();
}

void SnLocString::SyncControls()
{
	Xt::DomField*	fieldRecord	= IsEnabled() ? AsField(_domRecord) : 0;
	Xt::DomField*	fieldRow	= IsEnabled() ? AsField(_domRow) : 0;

	bool enableRecord	= fieldRecord != 0;
	bool enableRow		= false;

	if (fieldRecord)
	{
		if (Xt::DomRecord* record = FindRecordById(Panel()->Db(), Xt::ParseId(fieldRecord->value)))
			enableRow = true;
	}

	EnableWindow(_record,	enableRecord);
	EnableWindow(_row,		enableRow);
	EnableWindow(_preview,	enableRecord);

	if (!_manualSet)
	{
		PopulateRecord();
		PopulateRow(GetFocus() != _row);
	}

	bool canOpen = false;

	if (fieldRecord && fieldRow)
	{
		XtId recordId	= Xt::ParseId(fieldRecord->value);
		XtId rowId		= Xt::ParseId(fieldRow->value);

		if (Xt::DomRecord* record = FindRecordById(Panel()->Db(), recordId))
		{
			if (Xt::DomBlock* block = Xt::FindLocEntryById(record, rowId))
			{
				static XtString kEnglish = "english";

				canOpen = true;

				SetWindowTextW(_preview, MbToWc(AsText(At(block, kEnglish))));
			}
			else
				SetWindowTextW(_preview, L"");
		}
		else
			SetWindowTextW(_preview, L"");
	}
	else
		SetWindowTextW(_preview, L"");

	EnableWindow(_open, canOpen);
}

namespace
{
	bool SortByName(Xt::DomRecord* lhs, Xt::DomRecord* rhs)
	{
		return _stricmp(lhs->name, rhs->name) < 0;
	}
}

void SnLocString::PopulateRecord()
{
	Xt::DomField* field = IsEnabled() ? AsField(_domRecord) : 0;

	SendMessage(_record, CB_RESETCONTENT, 0, 0);

	if (field)
	{
		SendMessage(_record, CB_ADDSTRING, 0, (LPARAM)"(null)");

		XtId id = Xt::ParseId(field->value);

		LarvaSchema* schema = Panel()->Db()->schema;

		int found = 0;

		if (LtStruct* locTableType = (LtStruct*)schema->types["XtLocTable"])
		{
			if (locTableType->kind == kLtStruct)
			{
				XtUt::Array<Xt::DomRecord*> recs;

				for(Xt::DomDb::Iterator itPack(Panel()->Db()); itPack; ++itPack)
				{
					for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
					{
						LarvaType* itType = TypeOf(schema, itRecord);

						if (!itType || (itType->kind != kLtStruct))
							continue;

						if (!IsBaseOf(locTableType, (LtStruct*)itType))
							continue;

						XtUt::AppendEx(&recs, &*itRecord);
					}
				}

				XtUt::HeapSort(&recs, SortByName);

				for(XtUt::Array<Xt::DomRecord*>::Iterator itRecord(&recs); itRecord; ++itRecord)
				{
					if ((*itRecord)->id == id)
						found = 1 + itRecord.index;

					int addIndex = (int)SendMessage(_record, CB_ADDSTRING, 0, (LPARAM)(LPSTR)"");

					if (addIndex >= 0)
					{
						SendMessage(_record, CB_SETITEMDATA, addIndex, (*itRecord)->id);
					}
				}
			}
		}

		SendMessage(_record, CB_SETCURSEL, found, 0);
	}
}

void SnLocString::PopulateRow(bool fast)
{
	Xt::DomField* fieldRecord	= IsEnabled() ? AsField(_domRecord) : 0;
	Xt::DomField* fieldRow		= IsEnabled() ? AsField(_domRow) : 0;

	SendMessage(_row, CB_RESETCONTENT, 0, 0);

	Free(&_items);

	if (fieldRecord &&  fieldRow)
	{
		SendMessage(_row, CB_ADDSTRING, 0, (LPARAM)"(null)");

		XtId recordId	= Xt::ParseId(fieldRecord->value);
		XtId rowId		= Xt::ParseId(fieldRow->value);

		if (Xt::DomRecord* record = FindRecordById(Panel()->Db(), recordId))
		{
			if (fast)
			{
				if (Xt::DomBlock* block = Xt::FindLocEntryById(record, rowId))
				{
					if (Item* item = XtUt::AppendExN(&_items, 1))
					{
						static XtString kName = "name";

						item->id		= rowId;
						item->label		= AsText(At(block, kName));
					}
				}
			}
			else
			{
				if (Xt::DomArray* strings = AcquireArray(&record->root.data, "strings"))
				{
					Allocate(&_items, 0, Count(strings));

					for(Xt::DomArray::IndexIterator it(strings); it; ++it)
					{
						Xt::DomBlock* block = AsBlock(it);

						if (!block)
							continue;

						if (Item* item = AppendN(&_items, 1))
						{
							static XtString kName = "name";

							item->id		= it.Key();
							item->label		= AsText(At(block, kName));
						}
					}
				}
			}
		}

		XtUt::MergeSort(&_items, ComboCompare);

		int found = 0;

		for(XtUt::Array<Item>::Iterator it(&_items); it; ++it)
		{
			if (it->id == rowId)
				found = (int)(it.index + 1);

			SendMessage(_row, CB_ADDSTRING, 0, (LPARAM)(LPSTR)(const char*)it->label);
		}

		SendMessage(_row, CB_SETCURSEL, found, 0);
	}
}

bool SnLocString::ComboCompare(const Item& lhs, const Item& rhs)
{
	return _stricmp(lhs.label, rhs.label) < 0;
}

HBRUSH SnLocString::OnControlColour(int id, int msg, HDC hdc, HWND hwnd)
{
	if (id == _idPreview)
	{
		if (IsEnabled() && AsField(_domRecord))
		{
			SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
			SetTextColor(hdc, RGB(64, 64, 64));

			return GetSysColorBrush(COLOR_BTNFACE);
		}
		else
		{
			SetBkColor(hdc, GetSysColor(COLOR_BTNHIGHLIGHT));
			SetTextColor(hdc, RGB(127, 127, 127));

			return GetSysColorBrush(COLOR_BTNHIGHLIGHT);
		}
	}

	return ScarabNode::OnControlColour(id, msg, hdc, hwnd);
}

bool SnLocString::MeasureItem(int id, MEASUREITEMSTRUCT* mis)
{
	if (id != _idRecord)
		return false;

	mis->itemHeight = Panel()->DrawContext()->labelSizeY + 2;

	return true;
}

bool SnLocString::DrawItem(int id, DRAWITEMSTRUCT* dis)
{
	if (id != _idRecord)
		return false;

	if (dis->itemID == -1)
		return true;

	XtString recordName;
	XtString packName;

	if (Xt::DomRecord* record = FindRecordById(Panel()->Db(), dis->itemData))
	{
		recordName = record->name;

		if (Xt::DomPack* pack = Xt::PackOf(record))
			packName = "[" + pack->name + "]";
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
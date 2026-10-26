#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

static XtString kUserRefDefaultSummary = "[{__type}-{__id}]";

SnUserRef::SnUserRef(LfControl* field) :
	SnControl(field),
	_idCombo(this),
	_dom(DomCallback, this),
	_watch(DomCallback, this)
{
}

SnUserRef::~SnUserRef()
{
}

void SnUserRef::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(parentDom);
}

void SnUserRef::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireBlock(parentDom, index));
}

bool SnUserRef::OnBuild()
{
	if (!_idCombo.Allocate())
		return false;

	if (_wndCombo == 0)
	{
		if (!_wndCombo.Create(WS_EX_CLIENTEDGE, _T("XTWINCOMBOBOX"), _T(""),
				CBS_DROPDOWNLIST | WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DISABLENOSCROLL | CBS_OWNERDRAWFIXED,
			300, 20 + 200, *Panel(), _idCombo))
		{
			return false;
		}

		SendMessage(_wndCombo, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
		SendMessage(_wndCombo, CB_SETDROPPEDWIDTH, 300, 0);
	}

	SyncControls();

	return true;
}

void SnUserRef::OnMeasure(HDC ic)
{
	_size = WinSize(300, 20);
}

void SnUserRef::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_wndCombo, _client.left, _client.top, 300, 20 + 200, SWP_SHOWWINDOW | SWP_NOSIZE);
}

ScarabClipTarget SnUserRef::ClipTarget()
{
	return ScarabClipTarget(AsField(_dom), _field);
}

void SnUserRef::OnCommand(int id, int code)
{
	Xt::DomBlock* block = IsEnabled() ? AsBlock(_dom) : 0;

	if (block)
	{
		if ((id == _idCombo) && (code == CBN_SETFOCUS))
		{
			PopulateCombo();
		}
		else if ((id == _idCombo) && (code == CBN_SELCHANGE))
		{
			int		index	= (int)SendMessage(_wndCombo, CB_GETCURSEL, 0, 0);
			Item*	item	= At(&_items, index);

			if (item)
			{
				XtString dstFieldName = GetString("UserRef", "field");

				if (XtsIsName(dstFieldName))
					Set(block, dstFieldName, item->id);
			}
		}
	}
}

bool SnUserRef::MeasureItem(int id, MEASUREITEMSTRUCT* mis)
{
	if (id != _idCombo)
		return false;

	mis->itemHeight = Panel()->DrawContext()->labelSizeY + 2;

	return true;
}

bool SnUserRef::DrawItem(int id, DRAWITEMSTRUCT* dis)
{
	if (id != _idCombo)
		return false;

	if (dis->itemID == -1)
		return true;

	XtString text;

	if (Item* str = At(&_items, dis->itemID))
		text = str->label;
	else
		text = "?Error?";

	TEXTMETRIC tm;
	GetTextMetrics(dis->hDC, &tm);

	WinSize szRecord;
	GetTextExtentPoint32(dis->hDC, text, text.Length(), &szRecord);

	int x = LOWORD(GetDialogBaseUnits()) / 8;
	int y = (dis->rcItem.bottom + dis->rcItem.top - tm.tmHeight) / 2;

	COLORREF oldBk = SetBkColor(dis->hDC,
			GetSysColor((dis->itemState & ODS_SELECTED) ? COLOR_HIGHLIGHT : COLOR_WINDOW)
		);

	COLORREF oldFg = SetTextColor(dis->hDC,
			GetSysColor((dis->itemState & ODS_SELECTED) ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT)
		);

	ExtTextOut(dis->hDC, dis->rcItem.left + 1, y, ETO_CLIPPED | ETO_OPAQUE, &dis->rcItem, text, text.Length(), NULL);

	SetTextColor(dis->hDC, oldFg);
	SetBkColor(dis->hDC, oldBk);
	
	if (dis->itemState & ODS_FOCUS)
		DrawFocusRect(dis->hDC, &dis->rcItem);

	return true;
}

bool SnUserRef::AddItem(LtStruct* type, Xt::DomComponent* component)
{
	if (!type || !component)
		return false;

	LtStruct* baseType = GetBaseType();

	if (!baseType)
		return false;

	if (!IsBaseOf(baseType, type))
		return false;

	XtString id = AsText(At(&component->data, GetString("UserRef", "source")));

	if (!*id)
		return false;

	Item* item = XtUt::AppendEx(&_items, new Item);

	if (!item)
		return false;

	XtString label = EvaluateSummaryText(Db(), Schema(), type, component, kUserRefDefaultSummary);

	item->id	= id;
	item->label	= *label ? label : Format("[%s-%08X]", type->name.Text(), id);
	item->type	= type;

	Monitor(&item->dom, component);

	return true;
}

void SnUserRef::SetDom(Xt::DomBlock* parentDom)
{
	SetEnabled(Monitor(&_dom, parentDom) != 0);

	SyncControls();
}

void SnUserRef::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnUserRef* node)
{
	if ((monitor == &node->_dom) && (event->code == Xt::kDomRemoved))
		node->SetDom(0);
	else if (*monitor == event->from)
		node->SyncControls();
	else if (monitor == &node->_watch)
	{
		int index = (int)SendMessage(node->_wndCombo, CB_GETCURSEL, 0, 0);

		if (Item* item = At(&node->_items, index))
		{
			if (item->type)
			{
				XtString label = EvaluateSummaryText(node->Panel()->Db(), node->Panel()->Db()->schema, item->type, AsComponent(item->dom), kUserRefDefaultSummary);

				item->label	= *label ? label : Format("[%s-%08X]", item->type->name.Text(), item->id);

				if (node->_wndCombo)
					InvalidateWindow(node->_wndCombo);
			}
		}
	}
}

bool SnUserRef::ComboCompare(const Item* lhs, const Item* rhs)
{
	return lhs->label < rhs->label;
}

void SnUserRef::SyncControls()
{
	Xt::DomBlock*	block			= IsEnabled() ? AsBlock(_dom) : 0;
	BOOL			enableControls	= block ? TRUE : FALSE;

	EnableWindow(_wndCombo, enableControls);

	PopulateCombo();
}

class UserRefComponentsVisitor : public LvComponents
{
	SnUserRef* _userRef;

public:
	UserRefComponentsVisitor(SnUserRef* userRef) :
		_userRef(userRef)
	{
	}

public:
	virtual bool Visit(LtStruct* type, Xt::DomComponent* dom)
	{
		_userRef->AddItem(type, dom);

		return true;
	}
};

void SnUserRef::PopulateCombo()
{
	Free(&_items);

	if (!IsEnabled())
	{
		SendMessage(_wndCombo, CB_RESETCONTENT, 0, 0);
		return;
	}

	XtString id = IsEnabled() ? AsText(At(AsBlock(_dom), GetString("UserRef", "field"))) : kXtNullString;

	if (Item* item = XtUt::AppendEx(&_items, new Item))
	{
		item->id	= "";
		item->label	= "(null)";
	}

	Xt::DomDb* db = Panel()->Db();

	if (Xt::DomRecord* record = RecordOf(_dom))
	{
		PopulateComboByRecord(record);

		if (LtStruct* type = AsStruct(TypeOf(db->schema, record)))
			PopulateComboByType(record, type);
	}

	XtUt::MergeSort(&_items, ComboCompare, 1, _items.count);

	int found = -1;
	int width = 300;

	HDC ic = CreateIC(_T("DISPLAY"), 0, 0, 0);

	SelectObject(ic, Panel()->DrawContext()->labelFont);

	SendMessage(_wndCombo, CB_RESETCONTENT, 0, 0);

	for(XtUt::List<Item>::Iterator it(&_items); it; ++it)
	{
		if (it->id == id)
			found = (int)it.index;

		SendMessage(_wndCombo, CB_ADDSTRING, 0, (LPARAM)(LPSTR)(const char*)it->label);

		WinSize sz;
		GetTextExtentPoint32(ic, it->label, it->label.Length(), &sz);

		width = Max(width, (int)sz.cx);
	}

	DeleteDC(ic);

	SendMessage(_wndCombo, CB_SETCURSEL, found, 0);
	SendMessage(_wndCombo, CB_SETDROPPEDWIDTH, width + 8, 0);

	if (Item* item = At(&_items, found))
		Monitor(&_watch, item->dom);
	else
		Monitor(&_watch, 0);
}

void SnUserRef::PopulateComboByRecord(Xt::DomRecord* record)
{
	if (LtStruct* type = TypeOf(Schema(), record))
		VisitAllComponents(Schema(), type, &record->root, &UserRefComponentsVisitor(this));
}

void SnUserRef::PopulateComboByType(Xt::DomRecord* baseRec, LtStruct* type)
{
	if (!type)
		return;

	Xt::DomDb* db = Panel()->Db();

	if (type->base)
		PopulateComboByType(baseRec, type->base->type);

	for(XtUt::List<LarvaField>::Iterator it(&type->fields); it; ++it)
	{
		if (it->attrs.GetBool("UserRef", "base"))
		{
			Xt::DomElement* el = At(&baseRec->root.data, it);

			if (Xt::DomField* field = AsField(el))
			{
				if (Xt::DomRecord* record = Xt::FindRecordById(db, Xt::ParseId(AsText(field))))
				{
					PopulateComboByRecord(record);
				}
			}
			else if (Xt::DomArray* arr = AsArray(el))
			{
				for(Xt::DomArray::IndexIterator itArr(arr); itArr; ++itArr)
				{
					if (Xt::DomField* field = AsField(itArr))
					{
						if (Xt::DomRecord* record = Xt::FindRecordById(db, Xt::ParseId(AsText(field))))
						{
							PopulateComboByRecord(record);
						}
					}
				}
			}
		}
	}
}

LtStruct* SnUserRef::GetBaseType()
{
	XtString typeName = GetString("UserRef", "type");

	return AsStruct(FindTypeByName(Panel()->Db()->schema, typeName));
}
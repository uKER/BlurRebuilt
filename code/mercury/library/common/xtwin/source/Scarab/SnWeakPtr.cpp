#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

static XtString kWeakPtrDefaultSummary = "[{__type}-{__id}]";

SnWeakPtr::SnWeakPtr(LfWeakPtr* field) :
	_field(field),
	_idCombo(this),
	_idDropper(this),
	_dom(DomCallback, this),
	_watch(DomCallback, this)
{
}

SnWeakPtr::~SnWeakPtr()
{
}

void SnWeakPtr::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(AcquireRef(parentDom, _field->name));
}

void SnWeakPtr::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireRef(parentDom, index));
}

bool SnWeakPtr::OnBuild()
{
	if (!_idCombo.Allocate() || !_idDropper.Allocate())
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

	if (HasDropper())
	{
		if (_wndDropper == 0)
		{
			if (!_wndDropper.Create(0, WC_BUTTON, _T("!"), BS_FLAT | WS_CHILD | WS_TABSTOP, *Panel(), _idDropper))
				return false;

			SendMessage(_wndDropper, WM_SETFONT, (WPARAM)Panel()->DrawContext()->iconFont2, FALSE);
		}
	}

	SyncControls();

	return true;
}

void SnWeakPtr::OnMeasure(HDC ic)
{
	_size = WinSize(300 + (HasDropper() ? 24 : 0), 20);
}

void SnWeakPtr::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_wndCombo, _client.left, _client.top, 300, 20 + 200, SWP_SHOWWINDOW | SWP_NOSIZE);

	if (_wndDropper)
		layoutContext->SetWindowPos(_wndDropper, _client.left + 304, _client.top, 20, 20, SWP_SHOWWINDOW);
}

ScarabClipTarget SnWeakPtr::ClipTarget()
{
	return ScarabClipTarget(AsRef(_dom), _field);
}

void SnWeakPtr::OnCommand(int id, int code)
{
	Xt::DomRef* ref = IsEnabled() ? AsRef(_dom) : 0;

	if (ref)
	{
		if ((id == _idCombo) && (code == CBN_SETFOCUS))
		{
			PopulateCombo(false);
		}
		else if ((id == _idCombo) && (code == CBN_SELCHANGE))
		{
			int		index	= (int)SendMessage(_wndCombo, CB_GETCURSEL, 0, 0);
			Item*	item	= At(&_items, index);

			if (item)
			{
				Monitor(&_watch, item ? (Xt::DomElement*)item->dom : 0);
				Set(ref, item->id);
			}
		}
		else if ((id == _idDropper) && (code == BN_CLICKED))
		{
			Panel()->FormContext()->BeginDropper(_field, ref);
		}
	}
}

bool SnWeakPtr::MeasureItem(int id, MEASUREITEMSTRUCT* mis)
{
	if (id != _idCombo)
		return false;

	mis->itemHeight = Panel()->DrawContext()->labelSizeY + 2;

	return true;
}

bool SnWeakPtr::DrawItem(int id, DRAWITEMSTRUCT* dis)
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

bool SnWeakPtr::AddItem(LtStruct* type, Xt::DomComponent* component, XtId fastId)
{
	if (!type || !component)
		return false;

	if (fastId && (fastId != component->id))
		return false;

	if (!component->id)
		return false;

	if (!IsBaseOf(_field->baseType, type))
		return false;

	Item* item = XtUt::AppendEx(&_items, new Item);

	if (!item)
		return false;

	XtString label = EvaluateSummaryText(Db(), Schema(), type, component, kWeakPtrDefaultSummary);

	item->id	= component->id;
	item->label	= *label ? label : Format("[%s-%08X]", type->name.Text(), component->id);
	item->type	= type;

	Monitor(&item->dom, component);

	return true;
}

void SnWeakPtr::SetDom(Xt::DomRef* ref)
{
	SetEnabled(Monitor(&_dom, ref) != 0);

	SyncControls();
}

void SnWeakPtr::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnWeakPtr* node)
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
				XtString label = EvaluateSummaryText(node->Db(), node->Schema(), item->type, AsComponent(item->dom), kWeakPtrDefaultSummary);

				item->label	= *label ? label : Format("[%s-%08X]", item->type->name.Text(), item->id);

				if (node->_wndCombo)
					InvalidateWindow(node->_wndCombo);
			}
		}
	}
}

bool SnWeakPtr::ComboCompare(const Item* lhs, const Item* rhs)
{
	return lhs->label < rhs->label;
}

void SnWeakPtr::SyncControls()
{
	if (_wndCombo == 0)
		return;

	Xt::DomRef*	ref				= IsEnabled() ? AsRef(_dom) : 0;
	BOOL		enableControls	= ref ? TRUE : FALSE;

	EnableWindow(_wndCombo, enableControls);

	if (_wndDropper != 0)
		EnableWindow(_wndDropper, enableControls);

	PopulateCombo(GetFocus() != _wndCombo);
}

//
// SnWeakPtr::PopulateCombo
//

class WfComponents : public LvComponents
{
	SnWeakPtr*	_ptr;
	XtId		_fastId;

public:
	WfComponents(SnWeakPtr* ptr, XtId fastId) :
		_ptr(ptr),
		_fastId(fastId)
	{
	}

public:
	virtual bool Visit(LtStruct* type, Xt::DomComponent* dom)
	{
		if (type && dom)
			_ptr->AddItem(type, dom, _fastId);

		return true;
	}
};

void SnWeakPtr::PopulateCombo(bool fast)
{
	Free(&_items);

	if (_wndCombo == 0)
		return;

	if (!IsEnabled())
	{
		SendMessage(_wndCombo, CB_RESETCONTENT, 0, 0);
		return;
	}

	XtId id = AsId(IsEnabled() ? AsRef(_dom) : 0);

	if (Item* item = XtUt::AppendEx(&_items, new Item))
	{
		item->id	= 0;
		item->label	= "(null)";
	}

	if (Xt::DomRecord* record = RecordOf(_dom))
	{
		LarvaSchema* schema = Schema();

		VisitAllComponents(schema, TypeOf(schema, record), &record->root, &WfComponents(this, fast ? id : 0));
	}

	XtUt::MergeSort(&_items, ComboCompare, 1, _items.count);

	int found = -1;
	int width = 300;

	HDC ic = 0;

	if (!fast)
	{
		ic = CreateIC(_T("DISPLAY"), 0, 0, 0);
		SelectObject(ic, Panel()->DrawContext()->labelFont);
	}

	SendMessage(_wndCombo, CB_RESETCONTENT, 0, 0);

	for(XtUt::List<Item>::Iterator it(&_items); it; ++it)
	{
		if (it->id == id)
			found = (int)it.index;

		SendMessage(_wndCombo, CB_ADDSTRING, 0, 0);

		if (!fast)
		{
			WinSize sz;
			GetTextExtentPoint32(ic, it->label, it->label.Length(), &sz);

			width = Max(width, (int)sz.cx);
		}
	}

	if (!fast)
	{
		DeleteDC(ic);
		SendMessage(_wndCombo, CB_SETDROPPEDWIDTH, width + 8, 0);
	}

	SendMessage(_wndCombo, CB_SETCURSEL, found, 0);

	if (Item* item = At(&_items, found))
		Monitor(&_watch, item->dom);
	else
		Monitor(&_watch, 0);
}

bool SnWeakPtr::HasDropper()
{
	return _field->attrs.FindAttr("Dropper") != 0;
}
#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

SnArrayHeader::SnArrayHeader(SnArray* array) :
	_notifyDom(DomCallback, this),
	_array(array),
	_index(0),
	_idCombo(this),
	_idAdd(this),
	_idRemove(this),
	_idMoveUp(this),
	_idMoveDown(this),
	_idDuplicate(this),
	_miniMode(false)
{
}

SnArrayHeader::~SnArrayHeader()
{
}

bool SnArrayHeader::OnBuild()
{
	if (!_idCombo.Allocate() || !_idAdd.Allocate() || !_idRemove.Allocate() || !_idMoveUp.Allocate() || !_idMoveDown.Allocate() || !_idDuplicate.Allocate())
		return false;

	if (_combo == 0)
	{
		if (!_combo.Create(WS_EX_CLIENTEDGE, _T("XTWINCOMBOBOX"), _T(""),
				CBS_DROPDOWNLIST | WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DISABLENOSCROLL | CBS_OWNERDRAWFIXED,
			200, 20 + 200, *Panel(), _idCombo))
		{
			return false;
		}

		SendMessage(_combo, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
	}

	if (_duplicate == 0)
	{
		if (!_duplicate.Create(0, WC_BUTTON, _T("Dup"), BS_FLAT | WS_CHILD | WS_TABSTOP, *Panel(), _idDuplicate))
			return false;

		SendMessage(_duplicate, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
	}

	if (_add == 0)
	{
		if (!_add.Create(0, WC_BUTTON, _T("Add"), BS_FLAT | WS_CHILD | WS_TABSTOP, *Panel(), _idAdd))
			return false;

		SendMessage(_add, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
	}

	if (_remove == 0)
	{
		if (!_remove.Create(0, WC_BUTTON, _T("Remove"), BS_FLAT | WS_CHILD | WS_TABSTOP, *Panel(), _idRemove))
			return false;

		SendMessage(_remove, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
	}

	if (_moveUp == 0)
	{
		if (!_moveUp.Create(0, WC_BUTTON, _T("<"), BS_FLAT | WS_CHILD | WS_TABSTOP, *Panel(), _idMoveUp))
			return false;

		SetWindowTextW(_moveUp, L"Ç");

		SendMessage(_moveUp, WM_SETFONT, (WPARAM)Panel()->DrawContext()->iconFont, FALSE);
	}

	if (_moveDown == 0)
	{
		if (!_moveDown.Create(0, WC_BUTTON, _T(">"), BS_FLAT | WS_CHILD | WS_TABSTOP, *Panel(), _idMoveDown))
			return false;

		SetWindowTextW(_moveDown, L"È");

		SendMessage(_moveDown, WM_SETFONT, (WPARAM)Panel()->DrawContext()->iconFont, FALSE);
	}

	SyncControls(true);

	return true;
}

void SnArrayHeader::OnMeasure(HDC ic)
{
	if (_miniMode)
		_size = WinSize(16, 20);
	else
		_size = WinSize(402, 29);
}

void SnArrayHeader::OnLayout(ScarabLayoutContext* layoutContext)
{
	if (_miniMode)
	{
		SetWindowText(_add, "+");

		layoutContext->SetWindowPos(_add, _client.left, _client.top, 16, 16, SWP_SHOWWINDOW);

		layoutContext->SetWindowPos(_combo, _client.left, _client.top, 200, 20 + 200, SWP_HIDEWINDOW | SWP_NOSIZE);
		layoutContext->SetWindowPos(_duplicate, _client.left + 208, _client.top, 34, 20, SWP_HIDEWINDOW);
		layoutContext->SetWindowPos(_remove, _client.left + 292, _client.top, 54, 20, SWP_HIDEWINDOW);
		layoutContext->SetWindowPos(_moveUp, _client.left + 354, _client.top, 20, 20, SWP_HIDEWINDOW);
		layoutContext->SetWindowPos(_moveDown, _client.left + 382, _client.top, 20, 20, SWP_HIDEWINDOW);
	}
	else
	{
		SetWindowText(_add, "Add");

		layoutContext->SetWindowPos(_combo, _client.left, _client.top, 200, 20 + 200, SWP_SHOWWINDOW | SWP_NOSIZE);
		layoutContext->SetWindowPos(_duplicate, _client.left + 208, _client.top, 34, 20, SWP_SHOWWINDOW);
		layoutContext->SetWindowPos(_add, _client.left + 250, _client.top, 34, 20, SWP_SHOWWINDOW);
		layoutContext->SetWindowPos(_remove, _client.left + 292, _client.top, 54, 20, SWP_SHOWWINDOW);
		layoutContext->SetWindowPos(_moveUp, _client.left + 354, _client.top, 20, 20, SWP_SHOWWINDOW);
		layoutContext->SetWindowPos(_moveDown, _client.left + 382, _client.top, 20, 20, SWP_SHOWWINDOW);
	}
}

void SnArrayHeader::OnCommand(int id, int code)
{
	LarvaSchema*	schema	= Panel()->Db()->schema;
	Xt::DomArray*	array	= AsArray(_array->_dom);

	if (array)
	{
		if ((id == _idCombo) && (code == CBN_SETFOCUS))
		{
			SyncControls(true);
		}
		else if ((id == _idCombo) && (code == CBN_SELCHANGE))
		{
			_index = (int)SendMessage(_combo, CB_GETCURSEL, 0, 0);

			SyncControls(false);

			_array->OnIndexChanged();

			Monitor(&_notifyDom, At(AsArray(_array->_dom), Index()));
		}
		else if ((id == _idAdd) && (code == BN_CLICKED))
		{
			if (Xt::DomComponent* com = new Xt::DomComponent)
			{
				if (LtStruct* type = AsStruct(_array->_field->Type()))
				{
					GenerateIdForComponentAndChildren(Schema(), type, com, true);
					SetDefaults(schema, type, com);
				}

				if (Append(array, com))
					_index = (int)Count(array) - 1;
			}

			SyncControls(true);

			_array->OnIndexChanged();

			Monitor(&_notifyDom, At(AsArray(_array->_dom), Index()));
		}
		else if ((id == _idRemove) && (code == BN_CLICKED))
		{
			Remove(array, Index());

			_index = Clamp(_index, 0, Max((int)Count(array) - 1, 0));

			SyncControls(true);

			_array->OnIndexChanged();

			Monitor(&_notifyDom, At(AsArray(_array->_dom), Index()));
		}
		else if ((id == _idMoveUp) && (code == BN_CLICKED))
		{
			int currIndex	= Index();
			int newIndex	= Index() - 1;

			if ((currIndex >= 0) && (currIndex < (int)Count(array)))
			{
				if ((newIndex >= 0) && (newIndex < (int)Count(array)))
				{
					_index = newIndex;

					Swap(array, currIndex, newIndex);

					SyncControls(true);

					_array->OnIndexChanged();

					Monitor(&_notifyDom, At(AsArray(_array->_dom), Index()));
				}
			}
		}
		else if ((id == _idMoveDown) && (code == BN_CLICKED))
		{
			int currIndex	= Index();
			int newIndex	= Index() + 1;

			if ((currIndex >= 0) && (currIndex < (int)Count(array)))
			{
				if ((newIndex >= 0) && (newIndex < (int)Count(array)))
				{
					_index = newIndex;

					Swap(array, currIndex, newIndex);

					SyncControls(true);

					_array->OnIndexChanged();

					Monitor(&_notifyDom, At(AsArray(_array->_dom), Index()));
				}
			}
		}
		else if ((id == _idDuplicate) && (code == BN_CLICKED))
		{
			if (Xt::DomElement* newElement = Duplicate(schema, _array->_field->field, At(array, Index())))
			{
				if (Xt::Insert(array, Index() + 1, newElement))
					_index = Clamp(Index() + 1, 0, (int)Count(array) - 1);

				SyncControls(true);

				_array->OnIndexChanged();

				Monitor(&_notifyDom, At(AsArray(_array->_dom), Index()));
			}
		}
	}
}

void SnArrayHeader::OnPaint(HDC hdc, const WinRect& rcPaint)
{
	ScarabNode::OnPaint(hdc, rcPaint);

	if (!_miniMode)
	{
		HPEN	oldPen		= (HPEN)SelectObject(hdc, Panel()->DrawContext()->borderPen);
		HBRUSH	oldBrush	= (HBRUSH)SelectObject(hdc, Panel()->DrawContext()->headerBrush);

		Rectangle(hdc,
				Parent()->_location.left,	Parent()->_location.top,
				Parent()->_location.right,	Parent()->_location.top + 36
			);

		SelectObject(hdc, oldBrush);
		SelectObject(hdc, oldPen);
	}
}

bool SnArrayHeader::MeasureItem(int id, MEASUREITEMSTRUCT* mis)
{
	if (id != _idCombo)
		return false;

	mis->itemHeight = Panel()->DrawContext()->labelSizeY + 2;

	return true;
}

bool SnArrayHeader::DrawItem(int id, DRAWITEMSTRUCT* dis)
{
	if (id != _idCombo)
		return false;

	if (dis->itemID == -1)
		return true;

	XtString text;

	if (XtString* str = At(&_items, dis->itemID))
		text = *str;
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

int SnArrayHeader::Index()
{
	Xt::DomArray* array = AsArray(_array->_dom);

	if (!array)
		return -1;

	if (_index < 0)
		return 0;

	if (_index >= (int)Count(array))
		return (int)(Count(array) - 1);

	return _index;
}

void SnArrayHeader::SyncControls(bool populateCombo)
{
	Xt::DomArray*	array			= AsArray(_array->_dom);
	BOOL			enableControls	= array ? TRUE : FALSE;

	SetEnabled(array != 0);

	EnableWindow(_combo,		enableControls && (Count(array) > 0));
	EnableWindow(_add,			enableControls);
	EnableWindow(_remove,		enableControls && (Count(array) > 0));
	EnableWindow(_duplicate,	enableControls && (Count(array) > 0));

	EnableWindow(_moveUp,	enableControls && (Count(array) > 0) && (_index > 0));
	EnableWindow(_moveDown,	enableControls && (Count(array) > 0) && ((_index + 1) < (int)Count(array)));

	if (populateCombo)
		PopulateCombo();
}

void SnArrayHeader::PopulateCombo()
{
	Xt::DomArray*	array			= AsArray(_array->_dom);
	BOOL			enableControls	= array ? TRUE : FALSE;

	Free(&_items);

	SendMessage(_combo, CB_RESETCONTENT, 0, 0);

	int width = 200;

	if (array)
	{
		LarvaSchema* schema = Panel()->Db()->schema;

		HDC ic = CreateIC(_T("DISPLAY"), 0, 0, 0);

		SelectObject(ic, Panel()->DrawContext()->labelFont);

		for(Xt::DomArray::IndexIterator it(array); it; ++it)
		{
			XtString summary = EvaluateSummaryText(Db(), schema, _array->_field->field, it);

			XtString text;

			text = Format("%i: %s", it.index, (const char*)summary);

			WinSize sz;
			GetTextExtentPoint32(ic, text, text.Length(), &sz);

			width = Max(width, (int)sz.cx);

			if (!AppendEx(&_items, text))
				break;

			SendMessage(_combo, CB_ADDSTRING, 0, (LPARAM)(const char*)text);
		}

		DeleteDC(ic);
	}

	SendMessage(_combo, CB_SETCURSEL, Index(), 0);
	SendMessage(_combo, CB_SETDROPPEDWIDTH, width + 20, 0);

	Monitor(&_notifyDom, At(AsArray(_array->_dom), _index));
}

void SnArrayHeader::SetMiniMode(bool miniMode)
{
	if (_miniMode != miniMode)
	{
		_miniMode = miniMode;

		Panel()->DirtyLayout();
	}
}

void SnArrayHeader::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnArrayHeader* hdr)
{
	if (Xt::DomElement* el = At(AsArray(hdr->_array->_dom), hdr->_index))
	{
		if (XtString* text = At(&hdr->_items, hdr->_index))
		{
			XtString label = EvaluateSummaryText(hdr->Db(), hdr->Schema(), hdr->_array->_field->field, el);

			*text = Format("%i: %s", hdr->_index, (const char*)label);
		}

		if (hdr->_combo)
			InvalidateWindow(hdr->_combo);
	}
}
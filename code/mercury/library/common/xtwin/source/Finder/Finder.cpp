#include "Core/XtWinPch.h"
#include "FinderImpl.h"
#include <XtWin/SendEmail.h>

const int kFinder_FindNow	= 0x1001;
const int kFinder_NewSearch	= 0x1002;

struct FinderListCompareCtx
{
	FinderDlg*	dlg;
	int			column;
};

LPARAM ListView_GetItemParam(HWND listWnd, int index)
{
	LVITEM item = { 0 };

	item.mask	= LVIF_PARAM;
	item.iItem	= index;

	if (!ListView_GetItem(listWnd, &item))
		return -1;

	return item.lParam;
}

//
// FinderDlgContext
//

FinderDlgContext::FinderDlgContext()
{
}

FinderDlgContext::~FinderDlgContext()
{
}

Xt::DomDb* FinderDlgContext::Db()
{
	return 0;
}

void FinderDlgContext::OpenRecord(Xt::DomRecord* record)
{
}

//
// FinderColumn
//

FinderColumn::FinderColumn(const XtString& title) :
	_title(title)
{
}

FinderColumn::~FinderColumn()
{
}

XtString FinderColumn::Evaluate(FinderResult* result)
{
	return kXtNullString;
}

Xt::FieldDom FinderColumn::Get(FinderResult* result)
{
	return Xt::FieldDom();
}

void FinderColumn::Set(FinderResult* result, Xt::FieldDom value)
{
}

//
// FinderDlg
//

FinderDlg::FinderDlg(FinderDlgContext* ctx) :
	_ctx(ctx),
	_listWnd(0),
	_statusWnd(0),
	_findNowWnd(0),
	_newSearchWnd(0),
	_deferLayout(false)
{
}

FinderDlg::~FinderDlg()
{
}

FinderRow* FinderDlg::AddRow(FinderRow* row)
{
	if (!XtUt::AppendEx(&_rows, row))
		return 0;

	if (!row->Create(*this))
	{
		// TODO: Should do something here rather than just leaving a big blank space!
	}
	else
	{
		DeferLayout();
	}

	return row;
}

void FinderDlg::SetupDefaultLayout()
{
	Free(&_rows);

	AddRow(new FrRecordOfType(this));
	AddRow(new FrObjectsOfType(this));
	AddRow(new FrRecordsNamed(this));
	AddRow(new FrDependsOn(this));
	AddRow(new FrExpression(this));
	AddRow(new FrExpression(this));
	AddRow(new FrExpression(this));
	AddRow(new FrUsingLocString(this));

	DoLayout();
}

void FinderDlg::SetupFindDependencies(Xt::DomRecord* record)
{
	Free(&_rows);

	AddRow(new FrRecordOfType(this));
	AddRow(new FrObjectsOfType(this));
	AddRow(new FrRecordsNamed(this));
	FrDependsOn* fr = (FrDependsOn*)AddRow(new FrDependsOn(this));
	AddRow(new FrExpression(this));
	AddRow(new FrExpression(this));
	AddRow(new FrExpression(this));
	AddRow(new FrUsingLocString(this));

	if (fr)
	{
		fr->SetEnabled(true);
		fr->SelectRecord(record);
	}

	DoLayout();

	UpdateWindow(*this);

	ExecuteQuery();
}

bool FinderDlg::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!DlgWindow::PreCreateWindow(cs))
		return false;

	cs->style |= WS_VISIBLE;
	cs->dwExStyle |= WS_EX_APPWINDOW;

	return true;
}

LRESULT FinderDlg::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			if (DlgWindow::WndProc(msg, wParam, lParam) < 0)
				return -1;

			SetWindowText(*this, "Find");

			// List

			_listWnd = CreateWindowEx(
					WS_EX_CLIENTEDGE, WC_LISTVIEW, "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | LVS_REPORT | LVS_SHOWSELALWAYS, 0, 0, 0, 0, *this, 0, 0, 0
				);

			ListView_SetExtendedListViewStyleEx(_listWnd,
					LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP,
					LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP
				);

			// Status

			_statusWnd = CreateWindowEx(0, STATUSCLASSNAME, _T(""), SBARS_SIZEGRIP | SBARS_TOOLTIPS | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, *this, 0, 0, 0);

			SendMessage(_statusWnd, SB_SETTEXT, SBT_NOBORDERS, (LPARAM)_T(""));

			// Find Now

			_findNowWnd = CreateWindowEx(0, WC_BUTTON, _T("&Find Now"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, *this, (HMENU)(intptr_t)kFinder_FindNow, 0, 0);

			SendMessage(_findNowWnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), FALSE);

			// New Search

			_newSearchWnd = CreateWindowEx(0, WC_BUTTON, _T("&New Search"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, *this, (HMENU)(intptr_t)kFinder_NewSearch, 0, 0);

			SendMessage(_newSearchWnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), FALSE);

			// Rows

			SetupDefaultLayout();
		}
		return 0;

		case WM_DESTROY:
			Free(&_rows);
		return 0;

		case WM_CLOSE:
			if (GetActiveWindow() == *this)
				SetActiveWindow(GetWindow(*this, GW_OWNER));

			DestroyWindow(*this);
		return 0;

		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* mmi = (MINMAXINFO*)lParam;

			mmi->ptMinTrackSize = WinPoint(480, 300);
		}
		return 0;

		case WM_SIZE:
			DoLayout();
		return 0;

		case WM_COMMAND:
			if ((LOWORD(wParam) == kFinder_FindNow) && (HIWORD(wParam) == BN_CLICKED))
			{
				ExecuteQuery();
			}
			else if ((LOWORD(wParam) == kFinder_NewSearch) && (HIWORD(wParam) == BN_CLICKED))
			{
				SetupDefaultLayout();
			}
		break;

		case WM_NOTIFY:
		{
			NMHDR* hdr = (NMHDR*)lParam;

			if (hdr->hwndFrom == _listWnd)
			{
				switch(hdr->code)
				{
					case LVN_KEYDOWN:
					{
						NMLVKEYDOWN* kd = (NMLVKEYDOWN*)hdr;

						if ((kd->wVKey == 'A') && (GetKeyState(VK_CONTROL) & 0x8000))
						{
							for(int curIndex = -1; (curIndex = ListView_GetNextItem(_listWnd, curIndex, LVNI_ALL)) >= 0; )
							{
								ListView_SetItemState(_listWnd, curIndex, LVIS_SELECTED, LVIS_SELECTED);
							}

							return 0;
						}
					}
					break;

					case LVN_COLUMNCLICK:
					{
						NMLISTVIEW* lvn = (NMLISTVIEW*)hdr;

						FinderListCompareCtx ctx;

						ctx.dlg		= this;
						ctx.column	= lvn->iSubItem;

						ListView_SortItems(_listWnd, ListCompare, &ctx);
					}
					break;

					case NM_DBLCLK:
					{
						NMITEMACTIVATE* ia = (NMITEMACTIVATE*)hdr;

						if (FinderResult* result = At(&_results, ListView_GetItemParam(_listWnd, ia->iItem)))
						{
							if (Xt::DomRecord* record = AsRecord(result->record))
							{
								_ctx->OpenRecord(record);
							}
						}
					}
					return 0;

					case NM_RCLICK:
					{
						NMITEMACTIVATE* ia = (NMITEMACTIVATE*)hdr;
						int itemIndex = ListView_GetItemParam(_listWnd, ia->iItem);

						FinderColumn* col = At(&_columns, ia->iSubItem - 3);

						HMENU menu = CreatePopupMenu();

						AppendMenu(menu, MF_STRING, 1, "Open");

						if (col)
						{
							if (FinderResult* result = At(&_results, itemIndex))
							{
								AppendMenu(menu, MF_STRING, 2, "Set " + col->Title() + " for selected to '" + col->Evaluate(result) + "'");
							}
						}

						AppendMenu(menu, MF_STRING, 3, "Copy record names");

						SetMenuDefaultItem(menu, 1, FALSE);

						WinPoint pt;
						GetCursorPos(&pt);

						int result = TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, *this, 0);

						DestroyMenu(menu);

						switch(result)
						{
							case 1:
							{
								if (FinderResult* result = At(&_results, itemIndex))
								{
									if (Xt::DomRecord* record = AsRecord(result->record))
									{
										_ctx->OpenRecord(record);
									}
								}
							}
							break;

							case 2:
							{
								if (LarvaSchema* schema = Db() ? Db()->schema : 0)
								{
									if (FinderResult* result = At(&_results, itemIndex))
									{
										Xt::FieldDom newValue = col->Get(result);

										for(int curIndex = -1; (curIndex = ListView_GetNextItem(_listWnd, curIndex, LVNI_SELECTED)) >= 0; )
										{
											if (curIndex == itemIndex)
												continue;

											FinderResult* fr = At(&_results, ListView_GetItemParam(_listWnd, curIndex));

											if (!fr)
												continue;

											col->Set(fr, newValue);
										}

										RefreshQueryColumns();
									}
								}
							}
							break;

							case 3:
							{
								XtString result;

								for(int curIndex = -1; (curIndex = ListView_GetNextItem(_listWnd, curIndex, LVNI_SELECTED)) >= 0; )
								{
									FinderResult* fr = At(&_results, ListView_GetItemParam(_listWnd, curIndex));

									if (!fr)
										continue;

									Xt::DomRecord* record = AsRecord(fr->record);

									if (!record)
										continue;

									Xt::DomPack* pack = PackOf(record);

									LtStruct* type = TypeOf(Db() ? Db()->schema : 0, record);

									if (pack)
										result += pack->name + ": ";

									result += record->name + "." + (type ? type->label : record->root.type) + "\r\n";
								}

								SetClipboardText(*this, result);
							}
							break;
						}
					}
					return 0;
				}
			}
		}
		break;

		case (WM_APP + 1):
			if (_deferLayout)
				DoLayout();
		return 0;
	}

	return DlgWindow::WndProc(msg, wParam, lParam);
}

void FinderDlg::DeferLayout()
{
	if (_deferLayout || !*this)
		return;

	_deferLayout = true;

	PostMessage(*this, (WM_APP + 1), 0, 0);
}

void FinderDlg::DoLayout()
{
	_deferLayout = false;

	SendMessage(_statusWnd, WM_SIZE, 0, 0);

	WinRect rcWin(*this);
	WinRect rcStatus(_statusWnd);

	SetWindowPos(_findNowWnd, 0, rcWin.right - 84, rcWin.top + 4, 80, 20, SWP_NOZORDER | SWP_NOACTIVATE);
	SetWindowPos(_newSearchWnd, 0, rcWin.right - 84, rcWin.top + 28, 80, 20, SWP_NOZORDER | SWP_NOACTIVATE);

	int labelMargin = 0;

	for(XtUt::List<FinderRow>::Iterator it(&_rows); it; ++it)
		labelMargin = Max(labelMargin, it->_labelWidth);

	if (labelMargin > 0)
		labelMargin += 4;

	for(XtUt::List<FinderRow>::Iterator it(&_rows); it; ++it)
		it->_labelMargin = labelMargin;

	int		rowHeight	= 4;
	bool	needSpace	= false;
	int		count		= 0;
	int		rowWidth	= Max(rcWin.Width() - 80 - 12, 0);

	for(XtUt::List<FinderRow>::Iterator it(&_rows); it; ++it)
	{
		if (needSpace)
			rowHeight += 4;

		SetWindowPos((HWND)*it, 0, rcWin.left + 4, rcWin.top + rowHeight, rowWidth, it->Height(), SWP_NOZORDER | SWP_NOACTIVATE);

		it->DoLayout();

		rowHeight += it->Height();

		if (it->Height() > 0)
		{
			needSpace = true;
			count++;
		}
		else
			needSpace = 0;
	}

	if (count > 0)
		rowHeight += 4;

	rcWin.top		+= rowHeight;
	rcWin.bottom	-= rcStatus.Height();

	if (rcWin.top > rcWin.bottom)
		rcWin.bottom = rcWin.top;

	SetWindowPos(_listWnd, 0, rcWin.left, rcWin.top, rcWin.Width(), rcWin.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
}

Xt::DomDb* FinderDlg::Db()
{
	return _ctx ? _ctx->Db() : 0;
}

LarvaSchema* FinderDlg::Schema()
{
	Xt::DomDb* db = Db();

	return db ? db->schema : 0;
}

void FinderDlg::AddResult(const Xt::QueryResult* result)
{
	if (!result)
		return;

	Xt::DomRecord* record = RecordOf(result->dom);

	if (!record)
		return;

	XtUt::Ptr<FinderResult> r = new FinderResult;

	if (!r)
		return;

	Monitor(&r->record, record);
	Monitor(&r->dom, result->dom);

	r->path		= result->path;
	r->field	= result->field;
	r->type		= result->type;

	XtUt::AppendEx(&_results, XtUt::Pop(&r));
}

namespace
{
	class FinderSinkResults : public Xt::QueryElement
	{
		FinderDlg* _dlg;

	public:
		FinderSinkResults(FinderDlg* dlg) :
			_dlg(dlg)
		{
		}

	public:
		virtual void Process(Xt::QueryContext* context, const Xt::QueryResult& source)
		{
			_dlg->AddResult(&source);
		}
	};
}

void FinderDlg::ExecuteQuery()
{
	LarvaSchema* schema = Schema();

	Free(&_results);
	Free(&_columns);

	Xt::Query q;

	q.AddElement(new Xt::QeAllRecords());

	for(XtUt::List<FinderRow>::Iterator it(&_rows); it; ++it)
	{
		if (!it->IsEnabled())
			continue;

		it->BuildQuery(&q);
		it->BuildColumns(&_columns);
	}

	q.AddElement(new FinderSinkResults(this));

	q.Execute(Db());

	ListView_DeleteAllItems(_listWnd);
	while(ListView_DeleteColumn(_listWnd, 0)) { }

	LV_COLUMN col = { 0 };

	col.mask	= LVCF_TEXT | LVCF_WIDTH;
	col.cx		= 260;
	col.pszText	= "Record Name";

	ListView_InsertColumn(_listWnd, 0, &col);

	col.cx		= 100;
	col.pszText	= "Record Type";

	ListView_InsertColumn(_listWnd, 1, &col);

	col.cx		= 200;
	col.pszText	= "Path";

	ListView_InsertColumn(_listWnd, 2, &col);

	for(XtUt::List<FinderColumn>::Iterator it(&_columns); it; ++it)
	{
		col.cx		= 100;
		col.pszText	= (LPSTR)it->Title().Text();

		ListView_InsertColumn(_listWnd, 3 + it.index, &col);
	}

	for(XtUt::List<FinderResult>::Iterator it(&_results); it; ++it)
	{
		Xt::DomRecord*	record		= AsRecord(it->record);
		LtStruct*		recordType	= TypeOf(schema, record);

		LV_ITEM item = { 0 };

		item.mask	= LVIF_PARAM;
		item.iItem	= it.index;
		item.lParam	= it.index;

		ListView_InsertItem(_listWnd, &item);
	}

	RefreshQueryColumns();
}

void FinderDlg::RefreshQueryColumns()
{
	LarvaSchema* schema = Schema();

	int count = ListView_GetItemCount(_listWnd);

	for(int i = 0; i < count; i++)
	{
		FinderResult* result = At(&_results, ListView_GetItemParam(_listWnd, i));

		if (!result)
			continue;

		Xt::DomRecord*	record		= AsRecord(result->record);
		LtStruct*		recordType	= TypeOf(schema, record);

		ListView_SetItemText(_listWnd, i, 0, record ? record->name.Text() : "(null)");
		ListView_SetItemText(_listWnd, i, 1, recordType ? recordType->label.Text() : "");
		ListView_SetItemText(_listWnd, i, 2, record ? result->path.Text() : "");

		if (record)
		{
			for(XtUt::List<FinderColumn>::Iterator itCol(&_columns); itCol; ++itCol)
			{
				XtString text = itCol->Evaluate(result);

				ListView_SetItemText(_listWnd, i, 3 + itCol.index, (LPSTR)text.Text());
			}
		}
	}
}

int CALLBACK FinderDlg::ListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	FinderListCompareCtx*	ctx		= (FinderListCompareCtx*)lParamSort;
	LarvaSchema*			schema	= ctx->dlg->Schema();

	FinderResult* r0 = At(&ctx->dlg->_results, lParam1);
	FinderResult* r1 = At(&ctx->dlg->_results, lParam2);

	if (!r0 || !r1)
	{
		if (r0)
			return -1;
		else if (r1)
			return 1;

		return 0;
	}

	if (ctx->column == 0)
	{
		Xt::DomRecord* rec0 = AsRecord(r0->record);
		Xt::DomRecord* rec1 = AsRecord(r1->record);

		return _stricmp(rec0->name, rec1->name);
	}
	else if (ctx->column == 1)
	{
		Xt::DomRecord* rec0 = AsRecord(r0->record);
		Xt::DomRecord* rec1 = AsRecord(r1->record);

		LtStruct* type0 = TypeOf(schema, rec0);
		LtStruct* type1 = TypeOf(schema, rec1);

		return _stricmp(type0 ? type0->label : "", type1 ? type1->label : "");
	}
	else if (ctx->column == 2)
	{
		return _stricmp(r0->path, r1->path);
	}
	else if (ctx->column > 2)
	{
		FinderColumn* col = At(&ctx->dlg->_columns, ctx->column - 3);

		XtString text0 = col->Evaluate(r0);
		XtString text1 = col->Evaluate(r1);

		return _stricmp(text0, text1);
	}

	return 0;
}
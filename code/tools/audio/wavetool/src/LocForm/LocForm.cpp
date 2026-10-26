#include "Core/StdAfx.h"
#include "XtWin/Core.h"
#include "Application/SorceryApp.h"
#include "Application/SorceryView.h"
#include "Xt/XtTool.h"
#include "LocForm/LocForm.h"
#include "XtWin/LocTextDlg.h"
#include "Resource/Resource.h"

/*

	LocString		- Owned by record (can be set to act like a WeakLocString)
	WeakLocString	- Referenced into another record

	type Manufacturer
	{
		[Category("Manufacturer.Name")]
		LocString name;
	}

	Ui -> [LocStringContainingRecord] [Id]

	Records "own" their locstring, but do not contain them. They instead generate their locstrings in a nominated "container" record during export.

	A LocString in a record is just a *unique-id*, so it's easy to generate locstring automatically, for referencing them you just need to lookup a locstring and store that.

	Sorcery would need a tool to iterate all records looking for those with locstrings contained within them, then create new records, or new ui based upon where a record
	says its locdata should be stored.  This brings up the question whether a record should have any say where its locdata is stored, perhaps being able to say per folder, or
	per pack where the locstrings should be placed would be sufficent.

	Pack / Folder / Type with priorities?

	DLC1 Cutscene/Chapter1/ * -> DLC1-Chapter1-Cutscene-Text
	DLC1 * * -> DLC1-Misc-Text
	* * Vehicle -> Vehicle-Text
	* * * -> Misc-Text

*/

const int kLocForm_List		= 0x1001;
const int kLocForm_Combo	= 0x1002;
const int kLocForm_Add		= 0x1003;
const int kLocForm_Remove	= 0x1004;
const int kLocForm_Import	= 0x1005;
const int kLocForm_Export	= 0x1006;
const int kLocForm_Filter	= 0x1007;

static XtString kStrName	= "name";
static XtString kStrComment	= "comment";

XtString gRememberedFilterText;

struct LocFormCompareContext
{
	LocFormMdiWindow*	window;
	Xt::DomArray*		strings;

public:
	inline LocFormCompareContext(LocFormMdiWindow* window_, Xt::DomArray* strings_) :
		window(window_),
		strings(strings_)
	{
	}
};

int CALLBACK LocFormCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	LocFormCompareContext*	ctx			= (LocFormCompareContext*)lParamSort;
	Xt::DomBlock*			blockLhs	= AsBlock(At(ctx->strings, lParam1));
	Xt::DomBlock*			blockRhs	= AsBlock(At(ctx->strings, lParam2));

	return _stricmp(AsText(At(blockLhs, kStrName)), AsText(At(blockRhs, kStrName)));
}

//
// LocFormMdiWindow
//

LocFormMdiWindow::LocFormMdiWindow(SorceryView& view) :
	_view(view),
	_list(0),
	_combo(0),
	_add(0),
	_remove(0),
	_import(0),
	_export(0),
	_filter(0),
	_language(0)
{
	_filterText = gRememberedFilterText;
}

LocFormMdiWindow::~LocFormMdiWindow()
{
}

bool LocFormMdiWindow::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!MdiWindow::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_CLIPCHILDREN;
	cs->dwExStyle	&= ~WS_EX_CLIENTEDGE;

	return true;
}

LRESULT LocFormMdiWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			if ((_list = CreateWindowEx(
					0, WC_LISTVIEW, _T(""), LVS_REPORT | WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_SHOWSELALWAYS, 0, 0, 0, 0, *this, (HMENU)(intptr_t)kLocForm_List, 0, 0
				)) == 0)
			{
				return -1;
			}

			if ((_combo = CreateWindowEx(
					0, _T("XTWINCOMBOBOX"), _T(""), CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DISABLENOSCROLL, 0, 0, 0, 0, *this, (HMENU)(intptr_t)kLocForm_Combo, 0, 0
				)) == 0)
			{
				return -1;
			}

			Xt::DomDb* db = _view._app._db;

			int englishIndex = 0;

			for(XtUt::List<Xt::DomLanguage>::Iterator it(&db->languages); it; ++it)
			{
				SendMessage(_combo, CB_ADDSTRING, 0, (LPARAM)(const char*)it->name);

				if (_stricmp(it->name, "english") == 0)
					englishIndex = (int)it.index;
			}

			SendMessage(_combo, CB_SETCURSEL, englishIndex, 0);

			_language = englishIndex;

			if ((_add = CreateWindowEx(
					0, WC_BUTTON, _T("New"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, *this, (HMENU)(intptr_t)kLocForm_Add, 0, 0
				)) == 0)
			{
				return -1;
			}

			if ((_remove = CreateWindowEx(
					0, WC_BUTTON, _T("Remove"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, *this, (HMENU)(intptr_t)kLocForm_Remove, 0, 0
				)) == 0)
			{
				return -1;
			}

			if ((_import = CreateWindowEx(
					0, WC_BUTTON, _T("Import"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, *this, (HMENU)(intptr_t)kLocForm_Import, 0, 0
				)) == 0)
			{
				return -1;
			}

			if ((_export = CreateWindowEx(
					0, WC_BUTTON, _T("Export"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, *this, (HMENU)(intptr_t)kLocForm_Export, 0, 0
				)) == 0)
			{
				return -1;
			}

			if ((_filter = CreateWindowEx(
					WS_EX_CLIENTEDGE, WC_EDIT, _T(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0, 0, 0, 0, *this, (HMENU)(intptr_t)kLocForm_Filter, 0, 0
				)) == 0)
			{
				return -1;
			}

			SetWindowText(_filter, _filterText);

			SendMessage(_combo, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), FALSE);
			SendMessage(_add, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), FALSE);
			SendMessage(_remove, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), FALSE);
			SendMessage(_import, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), FALSE);
			SendMessage(_export, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), FALSE);
			SendMessage(_filter, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), FALSE);

			ListView_SetExtendedListViewStyleEx(_list,
					LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP,
					LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP
				);

			{
				LVCOLUMN col = { 0 };

				col.mask	= LVCF_TEXT | LVCF_WIDTH;
				col.pszText	= _T("Name");
				col.cx		= 150;

				ListView_InsertColumn(_list, 0, &col);
			}
			{
				LVCOLUMN col = { 0 };

				col.mask	= LVCF_TEXT | LVCF_WIDTH;
				col.pszText	= _T("String");
				col.cx		= 400;

				ListView_InsertColumn(_list, 1, &col);
			}
			{
				LVCOLUMN col = { 0 };

				col.mask	= LVCF_TEXT | LVCF_WIDTH;
				col.pszText	= _T("Comment");
				col.cx		= 150;

				ListView_InsertColumn(_list, 2, &col);
			}

			UpdateWindowTitle();

			PopulateFromDom();

			RegisterDialogWindow(this);
		}
		break;

		case WM_DESTROY:
			UnregisterDialogWindow(this);
		break;

		case WM_CLOSE:
			_view.Close();
		return 0;

		case WM_SIZE:
		{
			WinRect rcClient(*this);

			SetWindowPos(_combo, 0, rcClient.left + 4, rcClient.top + 4, 200, 20 + 200, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(_add, 0, rcClient.left + 200 + 12, rcClient.top + 4, 50, 21, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(_remove, 0, rcClient.left + 250 + 20, rcClient.top + 4, 70, 21, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(_import, 0, rcClient.left + 320 + 28, rcClient.top + 4, 60, 21, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(_export, 0, rcClient.left + 380 + 36, rcClient.top + 4, 60, 21, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(_filter, 0, rcClient.left + 440 + 44, rcClient.top + 4, 120, 21, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(_list, 0, rcClient.left, rcClient.top + 33, rcClient.Width(), rcClient.Height() - 33, SWP_NOACTIVATE | SWP_NOZORDER);
		}
		break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(_hwnd, &ps);
			SetBkColor(ps.hdc, GetSysColor(COLOR_APPWORKSPACE));
			ExtTextOut(ps.hdc, 0, 0, ETO_OPAQUE, &ps.rcPaint, 0, 0, 0);
			EndPaint(_hwnd, &ps);
		}
		return 0;

		case WM_COMMAND:
		{
			int id		= LOWORD(wParam);
			int code	= HIWORD(wParam);

			if ((id == kLocForm_Combo) && (code == CBN_SELCHANGE))
			{
				Xt::DomDb* db = _view._app._db;

				_language = Clamp((int)SendMessage(_combo, CB_GETCURSEL, 0, 0), 0, Max((int)db->languages.count - 1, 0));

				UpdateFromDom();
			}
			else if ((id == kLocForm_Add) && (code == BN_CLICKED))
			{
				Xt::DomDb* db = _view._app._db;

				LocTextData ldd;

				ldd.currentItem = _language;

				for(XtUt::List<Xt::DomLanguage>::Iterator it(&db->languages); it; ++it)
				{
					LocTextItem* item = XtUt::AppendExN(&ldd.items, 1);

					if (!item)
						continue;

					item->language = it->name;
				}

				for(;;)
				{
					if (!LocTextDlg(*this, &ldd))
						break;

					if (GetBlockByName(ldd.name, false))
					{
						MessageBox(*this, _T("A string of that name already exists"), _T("Sorcery"), MB_ICONERROR | MB_OK);
						continue;
					}

					Xt::DomBlock* block = GetBlockByName(ldd.name, true);

					if (!block)
					{
						MessageBox(*this, _T("Failed to create string"), _T("Sorcery"), MB_ICONERROR | MB_OK);
						continue;
					}

					Set(block, kStrName, ldd.name);

					for(XtUt::Array<LocTextItem>::Iterator it(&ldd.items); it; ++it)
						Set(block, it->language, it->text);

					PopulateFromDom();

					break;
				}
			}
			else if ((id == kLocForm_Remove) && (code == BN_CLICKED))
			{
				if (Xt::DomArray* strings = GetStrings())
				{
					int position = -1;

					XtUt::Array<int> toDelete;

					Allocate(&toDelete, 0, ListView_GetSelectedCount(_list));

					for(;;)
					{
						if ((position = ListView_GetNextItem(_list, position, LVNI_SELECTED)) == -1)
							break;

						LVITEM item = { 0 };

						item.mask		= LVIF_PARAM;
						item.iItem		= (int)position;
						item.iSubItem	= 0;

						if (ListView_GetItem(_list, &item) == FALSE)
							break;

						AppendEx(&toDelete, (int)item.lParam);
					}

					if (MergeSort(&toDelete, XtUt::GreaterThan<int>()))
					{
						for(int i = 0; i < (int)toDelete.count; i++)
							Remove(strings, toDelete[i]);
					}

					PopulateFromDom();
				}
			}
			else if ((id == kLocForm_Import) && (code == BN_CLICKED))
			{
				static int filterIndex = 1;

				TCHAR currentDir[MAX_PATH];
				GetCurrentDirectory(MAX_PATH, currentDir);

				TCHAR filename[MAX_PATH] = "";

				OPENFILENAME ofn = { 0 };

				ofn.lStructSize			= sizeof(ofn);
				ofn.hwndOwner			= *this;
				ofn.lpstrFilter			= _T("All supported files (*.ini; *.csv; *.txt)\0*.ini;*.csv;*.txt\0Csv files (*.csv)\0*.csv\0Tab delimited text files (*.txt)\0*.txt\0All files (*.*)\0*.*\0");
				ofn.nFilterIndex		= filterIndex;
				ofn.lpstrFile			= filename;
				ofn.nMaxFile			= MAX_PATH;
				//ofn.lpstrInitialDir	= pathRoot;
				ofn.lpstrTitle			= _T("Localisation import...");
				ofn.Flags				= OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

				if (GetOpenFileName(&ofn) != FALSE)
				{
					if (_stricmp(filename + ofn.nFileExtension, "ini") == 0)
						ImportIni(filename);
					else if (_stricmp(filename + ofn.nFileExtension, "csv") == 0)
						ImportCsv(filename);
					else if (_stricmp(filename + ofn.nFileExtension, "txt") == 0)
						ImportTxt(filename);
					else
					{
						MessageBox(*this, _T("Unsupported file type"), _T("Sorcery"), MB_ICONERROR | MB_OK);
					}
				}

				filterIndex = ofn.nFilterIndex;

				PopulateFromDom();

				SetCurrentDirectory(currentDir);
			}
			else if ((id == kLocForm_Export) && (code == BN_CLICKED))
			{
				TCHAR currentDir[MAX_PATH];
				GetCurrentDirectory(MAX_PATH, currentDir);

				TCHAR filename[MAX_PATH] = "";

				OPENFILENAME ofn = { 0 };

				ofn.lStructSize			= sizeof(ofn);
				ofn.hwndOwner			= *this;
				ofn.lpstrFilter			= _T("Ini files (*.ini)\0*.ini\0Csv files (*.csv)\0*.csv\0Tab delimited text files (*.txt)\0*.txt\0All files (*.*)\0*.*\0");
				ofn.lpstrFile			= filename;
				ofn.nMaxFile			= MAX_PATH;
				//ofn.lpstrInitialDir	= pathRoot;
				ofn.lpstrTitle			= _T("Localisation export...");
				ofn.lpstrDefExt			= _T("ini");
				ofn.Flags				= OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

				if (GetSaveFileName(&ofn) != FALSE)
				{
					if (_stricmp(filename + ofn.nFileExtension, "ini") == 0)
						ExportIni(filename);
					else if (_stricmp(filename + ofn.nFileExtension, "csv") == 0)
						ExportCsv(filename);
					else if (_stricmp(filename + ofn.nFileExtension, "txt") == 0)
						ExportTxt(filename);
					else
					{
						MessageBox(*this, _T("Unsupported file type"), _T("Sorcery"), MB_ICONERROR | MB_OK);
					}
				}

				SetCurrentDirectory(currentDir);
			}
			else if ((id == kLocForm_Filter) && (code == EN_CHANGE))
			{
				_filterText				= SzGetWindowText(_filter);
				gRememberedFilterText	= _filterText;

				PopulateFromDom();
			}
		}
		break;

		case WM_NOTIFY:
		{
			NMHDR* hdr = (NMHDR*)lParam;

			if (hdr->idFrom == kLocForm_List)
			{
				switch(hdr->code)
				{
					case LVN_COLUMNCLICK:
					{
						NMLISTVIEW* lvn = (NMLISTVIEW*)hdr;

						if (Xt::DomArray* strings = GetStrings())
						{
							LocFormCompareContext ctx(this, strings);

							ListView_SortItems(_list, LocFormCompare, &ctx);
						}
					}
					break;

					case LVN_ITEMACTIVATE:
					{
						NMLISTVIEW* lvn = (NMLISTVIEW*)hdr;

						if (Xt::DomArray* strings = GetStrings())
						{
							LVITEM item = { 0 };

							item.mask		= LVIF_PARAM;
							item.iItem		= (int)lvn->iItem;
							item.iSubItem	= 0;

							if (ListView_GetItem(_list, &item) != FALSE)
							{
								if (Xt::DomBlock* block = AsBlock(At(strings, item.lParam)))
								{
									Xt::DomDb* db = _view._app._db;

									LocTextData ldd;

									ldd.currentItem = _language;

									for(XtUt::List<Xt::DomLanguage>::Iterator it(&db->languages); it; ++it)
									{
										LocTextItem* item = XtUt::AppendExN(&ldd.items, 1);

										if (!item)
											continue;

										item->language	= it->name;
										item->text		= AsText(At(block, it->name));
									}

									ldd.name		= AsText(At(block, kStrName));
									ldd.comment		= AsText(At(block, kStrComment));

									if (LocTextDlg(*this, &ldd))
									{
										Set(block, kStrName, ldd.name);
										Set(block, kStrComment, ldd.comment);

										for(XtUt::Array<LocTextItem>::Iterator it(&ldd.items); it; ++it)
											Set(block, it->language, it->text);

										UpdateItemText(lvn->iItem, block);
										UpdateItemString(lvn->iItem, block);
									}
								}
							}
						}
					}
					break;
				}
			}
		}
		break;

	    case DM_GETDEFID: return 0;
	    case DM_SETDEFID: return 0;
	}

	return MdiWindow::WndProc(msg, wParam, lParam);
}

void LocFormMdiWindow::UpdateWindowTitle()
{
	Xt::DomRecord* record = AsRecord(_view._document->_dom);

	XtString title;

	if (record)
		title = record->name + "." + record->root.type;
	else
		title = "(unnamed)";

	if (record->dirty)
		title = title + "*";

	SetWindowText(*this, title);
}

void LocFormMdiWindow::AdjustSizeToForm()
{
	WinSize sz(600, 400);

	WinRect rc(0, 0, sz.cx, sz.cy);

	AdjustWindowRectEx(&rc, GetWindowLong(*this, GWL_STYLE), FALSE, GetWindowLong(*this, GWL_EXSTYLE));

	WINDOWPLACEMENT wp = { 0 };
	wp.length = sizeof(wp);

	if (GetWindowPlacement(*this, &wp) == FALSE)
		return;

	WinRect rcMain(_view._app._window.GetMdiWnd());

	int width	= Clamp(rc.Width() + 36, 64, Max<int>(rcMain.Width() - wp.rcNormalPosition.left, 64));
	int height	= Clamp(rc.Height() + 48, 64, Max<int>(rcMain.Height() - wp.rcNormalPosition.top, 64));

	wp.rcNormalPosition.right	= wp.rcNormalPosition.left + width;
	wp.rcNormalPosition.bottom	= wp.rcNormalPosition.top + height;

	SetWindowPlacement(*this, &wp);
}

void LocFormMdiWindow::PopulateFromDom()
{
	Xt::DomArray* strings = GetStrings();

	if (!strings)
		return;

	SetWindowRedraw(_list, FALSE);

	int topIndex = ListView_GetTopIndex(_list);

	ListView_DeleteAllItems(_list);

	XtString where = GetWhere();

	for(Xt::DomArray::IndexIterator it(strings); it; ++it)
	{
		Xt::DomBlock* block = AsBlock(it);

		if (!block)
			continue;

		XtString name		= AsText(At(block, kStrName));
		XtString text		= AsText(At(block, where));
		XtString comment	= AsText(At(block, kStrComment));

		if (*_filterText && (
				(IndexOfStringI<char>(name, _filterText) < 0) &&
				(IndexOfStringI<char>(text, _filterText) < 0) &&
				(IndexOfStringI<char>(comment, _filterText) < 0)
			))
		{
			continue;
		}

		LVITEM item = { 0 };

		item.mask		= LVIF_TEXT | LVIF_PARAM;
		item.iItem		= (int)it.index;
		item.iSubItem	= 0;
		item.lParam		= (LPARAM)it.index;
		item.pszText	= (LPSTR)(const char*)name;

		int index = ListView_InsertItem(_list, &item);

		if (index != -1)
		{
			ListView_SetItemText(_list, index, 2, (LPSTR)(const char*)comment);
		}
	}

	LocFormCompareContext ctx(this, strings);

	ListView_SortItems(_list, LocFormCompare, &ctx);

	ListView_EnsureVisible(_list, topIndex, TRUE);

	SetWindowRedraw(_list, TRUE);

	UpdateFromDom();
}

void LocFormMdiWindow::UpdateFromDom()
{
	Xt::DomArray* strings = GetStrings();

	if (!strings)
		return;

	SetWindowRedraw(_list, FALSE);

	int count = ListView_GetItemCount(_list);

	for(int i = 0; i < count; i++)
	{
		LVITEM item = { 0 };

		item.mask		= LVIF_PARAM;
		item.iItem		= (int)i;
		item.iSubItem	= 0;

		if (ListView_GetItem(_list, &item) != FALSE)
		{
			if (Xt::DomBlock* block = AsBlock(At(strings, (int)item.lParam)))
				UpdateItemString(i, block);
		}
	}

	SetWindowRedraw(_list, TRUE);
}

void LocFormMdiWindow::UpdateItemText(int index, Xt::DomBlock* block)
{
	XtString comment = AsText(At(block, kStrComment));

	ListView_SetItemText(_list, index, 0, (LPSTR)(const char*)AsText(At(block, kStrName)));
	ListView_SetItemText(_list, index, 2, (LPSTR)(const char*)comment);
}

void LocFormMdiWindow::UpdateItemString(int index, Xt::DomBlock* block)
{
	const char* where = GetWhere();

	Sz16 string = MbToWc(AsText(At(block, where)));

	LV_ITEMW lvi;
	lvi.iSubItem	= 1;
	lvi.pszText		= (wchar_t*)string.Text();
	SendMessage(_list, LVM_SETITEMTEXTW, (WPARAM)(index), (LPARAM)(LV_ITEMW*)&lvi);
}

Xt::DomArray* LocFormMdiWindow::GetStrings()
{
	if (Xt::DomRecord* record = AsRecord(_view._document->_dom))
		return AcquireArray(&record->root.data, "strings");

	return 0;
}

Xt::DomRecord* LocFormMdiWindow::GetRecord()
{
	if (Xt::DomRecord* record = AsRecord(_view._document->_dom))
		return record;

	return 0;
}

const char* LocFormMdiWindow::GetWhere()
{
	Xt::DomDb* db = _view._app._db;

	if ((_language < 0) || (_language >= (int)db->languages.count))
		return "english";

	return db->languages[_language]->name;
}

bool LocFormMdiWindow::SetPair(const XtString& name, const XtString& text, const char* language, bool createIfDoesntExist)
{
	Xt::DomBlock* block = GetBlockByName(name, createIfDoesntExist);

	if (!block)
		return false;

	const char* where = (language && (*language != '\0')) ? language : GetWhere();

	return Set(block, where, text) != 0;
} 

Xt::DomBlock* LocFormMdiWindow::GetBlockByName(const XtString& name, bool createIfDoesntExist)
{
	if (createIfDoesntExist)
		return Xt::AcquireLocEntryByName(GetRecord(), name);

	return Xt::FindLocEntryByName(GetRecord(), name);
}
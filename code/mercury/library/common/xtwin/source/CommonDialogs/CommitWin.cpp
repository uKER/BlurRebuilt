#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/WndCore.h"
#include "XtWin/CommitWin.h"
#include "XtWin/SendEmail.h"
#include "XtWin/VisualConflictResolver.h"
#include "XtWin/Resource.h"

#include <svn_wrapper.h>

#define IDC_COMMENT	1501
#define IDC_LIST	1502
#define IDC_STATUS	1503

class CommitWin : public DlgWindow
{
public:
	CommitWin(CommitParams* params) :
		_params(params),
		_result(kCrCancelled),
		_isDonePopulating(false),
		_comment(0),
		_list(0),
		_okay(0),
		_cancel(0)
	{
	}

	virtual ~CommitWin()
	{
	}

public:
	CommitResult Result()
	{
		return _result;
	}

private:
	enum VcsItemKind
	{
		kItemText,
		kItemFile,
		kItemUnversioned
	};

	struct VcsItem
	{
		VcsItemKind	kind;
		XtString	path;
		COLORREF	colour;
		XtString	name;
		XtString	textStatus;

	public:
		VcsItem() :
			kind(kItemText),
			colour(0)
		{
		}
	};

	XtUt::List<VcsItem> _items;

public:
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch(msg)
		{
			case WM_CREATE:
			{
				if (DlgWindow::WndProc(msg, wParam, lParam) < 0)
					return -1;

				// General window

				SetWindowText(*this, _T("Enter log message"));

				// Comment editbox

				if ((_comment = CreateWindowEx(
						WS_EX_CLIENTEDGE, WC_EDIT, _T(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | WS_VSCROLL, 0, 0, 0, 0, *this, (HMENU)(intptr_t)IDC_COMMENT, 0, 0
					)) == 0)
				{
					return -1;
				}

				SendMessage(_comment, WM_SETFONT, (WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT), FALSE);

				if (!*_params->comment)
					SetWindowText(_comment, "Enter comment here");
				else
					SetWindowText(_comment, _params->comment);

				Edit_SetSel(_comment, 0, -1);

				SetFocus(_comment);

				// List view

				if ((_list = CreateWindowEx(
						WS_EX_CLIENTEDGE, WC_LISTVIEW, _T(""), LVS_REPORT | WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_SHOWSELALWAYS, 0, 0, 0, 0, *this, (HMENU)(intptr_t)IDC_LIST, 0, 0
					)) == 0)
				{
					return -1;
				}

				ListView_SetExtendedListViewStyleEx(_list,
						LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP,
						LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP
					);

				/*HIMAGELIST images = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 8, 8);

				ImageList_AddMasked(images, (HBITMAP)LoadImage(
						GetXtWinIconDll(), MAKEINTRESOURCE(IDB_XTWIN_EMPTYBOX), IMAGE_BITMAP, 16, 16, LR_SHARED
					), RGB(255, 0, 255));

				ImageList_AddMasked(images, (HBITMAP)LoadImage(
						GetXtWinIconDll(), MAKEINTRESOURCE(IDB_XTWIN_CHECKBOX), IMAGE_BITMAP, 16, 16, LR_SHARED
					), RGB(255, 0, 255));

				ListView_SetImageList(_list, images, LVSIL_SMALL);*/

				{
					LVCOLUMN col = { 0 };

					col.mask	= LVCF_TEXT | LVCF_WIDTH;
					col.pszText	= _T("File");
					col.cx		= 500 - 80 - 36;

					ListView_InsertColumn(_list, 0, &col);
				}
				{
					LVCOLUMN col = { 0 };

					col.mask	= LVCF_TEXT | LVCF_WIDTH;
					col.pszText	= _T("Status");
					col.cx		= 80;

					ListView_InsertColumn(_list, 1, &col);
				}

				_isDonePopulating = false;

				if (!_thread.Create(PopulateList, this))
					PopulateList(this);

				// Okay button

				if ((_okay = CreateWindowEx(
						0, WC_BUTTON, _T("Okay"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, *this, (HMENU)(intptr_t)IDOK, 0, 0
					)) == 0)
				{
					return -1;
				}

				SendMessage(_okay, WM_SETFONT, (WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT), FALSE);

				EnableWindow(_okay, FALSE);

				// Cancel button

				if ((_cancel = CreateWindowEx(
						0, WC_BUTTON, _T("Cancel"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, *this, (HMENU)(intptr_t)IDCANCEL, 0, 0
					)) == 0)
				{
					return -1;
				}

				SendMessage(_cancel, WM_SETFONT, (WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT), FALSE);

				EnableWindow(_cancel, FALSE);

				// Status button

				if ((_status = CreateWindowEx(
						0, WC_STATIC, _T("Scanning for changes..."), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, *this, (HMENU)(intptr_t)IDC_STATUS, 0, 0
					)) == 0)
				{
					return -1;
				}

				SendMessage(_status, WM_SETFONT, (WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT), FALSE);

				// Disable window frame close button

				EnableMenuItem(GetSystemMenu(*this, FALSE), SC_CLOSE, MF_DISABLED | MF_BYCOMMAND);
			}
			return 0;

			case WM_SIZE:
			{
				WinRect rc(*this);

				int top		= rc.top + 6;
				int bottom	= rc.bottom - 30;
				int middle	= (top + bottom) / 3;

				int y0 = Max(top, middle - 3);
				int y1 = Min(bottom, middle + 3);

				SetWindowPos(_comment, 0, rc.left + 4, top, rc.Width() - 8, y0 - top, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
				SetWindowPos(_list, 0, rc.left + 4, y1, rc.Width() - 8, bottom - y1, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
				SetWindowPos(_okay, 0, rc.right - 132, rc.bottom - 24, 60, 20, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
				SetWindowPos(_cancel, 0, rc.right - 64, rc.bottom - 24, 60, 20, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
				SetWindowPos(_status, 0, rc.left + 6, rc.bottom - 21, (rc.right - 136) - (rc.left + 6), 17, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
			}
			break;

			case WM_COMMAND:
			{
				if (_isDonePopulating)
				{
					switch(LOWORD(wParam))
					{
						case IDOK:
							if (HIWORD(wParam) == BN_CLICKED)
							{
								Vcs::Svn svn;

								_params->comment = SzGetWindowText(GetDlgItem(*this, IDC_COMMENT));

								_result = kCrProceed;

								int count = ListView_GetItemCount(_list);

								XtUt::Free(&_params->files);

								for(int i = 0; i < count; i++)
								{
									LV_ITEM li = { 0 };

									li.mask	= LVIF_PARAM;
									li.iItem	= i;

									if (!ListView_GetItem(_list, &li))
										continue;

									VcsItem* item = At(&_items, li.lParam);

									if (!item)
										continue;

									if (item->kind == kItemText)
										continue;

									BOOL isChecked = ListView_GetCheckState(_list, i);

									if (!isChecked)
										continue;

									if (item->kind == kItemUnversioned)
										svn.Add(item->path);

									XtUt::AppendEx(&_params->files, item->path);
								}

								EndModal(IDOK);
							}
						return 0;

						case IDCANCEL:
							if (HIWORD(wParam) == BN_CLICKED)
								EndModal(IDCANCEL);
						return 0;

						case IDC_COMMENT:
							if (HIWORD(wParam) == EN_CHANGE)
							{
								Sz8 text = SzGetWindowText(_comment);

								EnableWindow(_okay, *text && (text != "Enter comment here"));

								_params->comment = text;
							}
						break;
					}
				}
			}
			break;

			case WM_NOTIFY:
			{
				NMHDR* hdr = (NMHDR*)lParam;

				if (_isDonePopulating)
				{
					switch(hdr->idFrom)
					{
						case IDC_LIST:
							if (hdr->code == NM_CUSTOMDRAW)
							{
								NMLVCUSTOMDRAW* cd = (NMLVCUSTOMDRAW*)hdr;

								switch(cd->nmcd.dwDrawStage)
								{
									case CDDS_PREPAINT:
									return CDRF_NOTIFYITEMDRAW;

									case CDDS_ITEMPREPAINT:
										if (VcsItem* item = At(&_items, cd->nmcd.lItemlParam))
											cd->clrText = item->colour;
									return 0;
								}
							}
							else if (hdr->code == LVN_ITEMCHANGED)
							{
								NMLISTVIEW* nm = (NMLISTVIEW*)hdr;

								if (VcsItem* it = At(&_items, nm->lParam))
								{
									//if (nm->uChanged & LVIS_STATEIMAGEMASK)
									{
										if (it->kind == kItemText)
										{
											LVITEM item = { 0 };

											item.mask		= LVIF_STATE;
											item.iItem		= nm->iItem;
											item.iSubItem	= 0;
											item.stateMask	= LVIS_STATEIMAGEMASK;
											item.state		= 0;

											ListView_SetItem(_list, &item);
										}
									}
								}
							}
							else if (hdr->code == NM_RCLICK)
							{
								NMITEMACTIVATE* ia = (NMITEMACTIVATE*)hdr;

								LVITEM item;

								item.mask	= LVIF_PARAM;
								item.iItem	= ia->iItem;

								if (ListView_GetItem(_list, &item))
								{
									if (VcsItem* it = At(&_items, item.lParam))
									{
										if (it->kind != kItemText)
										{
											HMENU menu = CreatePopupMenu();

											if (it->kind == kItemFile)
											{
												AppendMenu(menu, MF_STRING, 1, _T("Show Differences"));
												AppendMenu(menu, MF_SEPARATOR, 0, 0);
												AppendMenu(menu, MF_STRING, 2, _T("Revert"));
												AppendMenu(menu, MF_STRING, 3, _T("Delete Locally"));
											}
											else if (it->kind == kItemUnversioned)
											{
												AppendMenu(menu, MF_STRING, 4, _T("Add"));
												AppendMenu(menu, MF_STRING, 3, _T("Delete Locally"));
											}

											WinPoint pt;
											GetCursorPos(&pt);

											int result = TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, *this, 0);

											DestroyMenu(menu);

											bool refresh = false;

											switch(result)
											{
												case 1:
												{
													Vcs::Svn svn;

													Xt::DomDb schemaDb;

													schemaDb.path = _params->path;

													if (Xt::ReloadSchema(&schemaDb, 0))
													{
														Sz8 pristine;

														if (svn.GetPristineCopy(it->path, pristine))
														{
															XtUt::Ptr<Xt::DomPack> ancestor	= Xt::ImportPack(pristine, 0);
															XtUt::Ptr<Xt::DomPack> mine		= Xt::ImportPack(it->path, 0);

															bool mineResult = false;

															XtUt::Ptr<Xt::Change> rMine = Xt::Diff(ancestor, mine, &mineResult);

															if (mineResult)
																VisualConflictResolver(*this, schemaDb.schema, rMine, rMine, 0, ancestor, mine, ancestor);
														}
													}
												}
												break;

												case 2:
												{
													Vcs::Svn svn;

													svn.Revert(it->path.Text());

													refresh = true;
												}
												break;

												case 3:
												{
													RecycleFile(it->path.Text());

													refresh = true;
												}
												break;

												case 4:
												{
													Vcs::Svn svn;

													svn.Add(it->path.Text());

													refresh = true;
												}
												break;
											}

											if (refresh)
											{
												_isDonePopulating = false;

												ListView_DeleteAllItems(_list);

												XtUt::Free(&_items);

												if (!_thread.Create(PopulateList, this))
													PopulateList(this);
											}
										}
									}
								}

								return 0;
							}
						break;
					}
				}
			}
			break;
		}

		return DlgWindow::WndProc(msg, wParam, lParam);
	}

	virtual bool PreTranslateMessage(MSG& msg)
	{
		if ((msg.message == WM_KEYDOWN) && (msg.wParam == VK_RETURN))
		{
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				PostMessage(*this, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), (LPARAM)GetDlgItem(*this, IDOK));
				return true;
			}
		}

		return DlgWindow::PreTranslateMessage(msg);
	}

private:
	struct PopulateListCallbackContext
	{
		Vcs::Svn*	svn;
		CommitWin*	win;
	};

	static void CreateItem(CommitWin* win, VcsItemKind kind, const XtString& path, COLORREF colour, const XtString& name, const XtString& textStatus)
	{
		VcsItem* item = XtUt::AppendEx(&win->_items, new VcsItem);

		if (!item)
			return;

		item->kind			= kind;
		item->path			= path;
		item->colour		= colour;
		item->name			= name;
		item->textStatus	= textStatus;
	}

	static XtString Summarise(Xt::Change* chg)
	{
		if (!chg)
			return "(empty)";

		switch(chg->type)
		{
			case Xt::kChangeRemove:
			return "(remove)";

			case Xt::kChangeLeaf:
			return "(new)";

			case Xt::kChangeBlock:
			{
				Xt::ChangeBlock* change = (Xt::ChangeBlock*)chg;

				XtString result;

				for(Xt::ChangeBlock::Iterator it(&change->changes); it; ++it)
				{
					if (*result)
						result += ", ";

					result += SplitIntercaps(it.Key());
				}

				return result;
			}
			break;

			case Xt::kChangeArray:
			{
				Xt::ChangeArray* change = (Xt::ChangeArray*)chg;

				XtString result;

				for(Xt::ChangeArray::Iterator it(&change->changes); it; ++it)
				{
					if (*result)
						result += ", ";

					result += "[" + Summarise(it) + "]";
				}

				return result;
			}
			break;

			case Xt::kChangeComponent:
			{
				Xt::ChangeComponent* change = (Xt::ChangeComponent*)chg;

				return Summarise(change->data);
			}
			break;

			default:
			return "(data)";
		}
	}

	static void PopulateListCallback(const Vcs::StatusEntry& info, void* baton)
	{
		PopulateListCallbackContext*	ctx = (PopulateListCallbackContext*)baton;
		CommitWin*						win = ctx->win;

		const char* status = "";
		COLORREF colour = RGB(0, 0, 0);

		bool isModified = false;
		bool unversioned = false;

		if (info.textStatus == Vcs::kStatus_Ignored)
		{
			status = "ignored";
			colour = RGB(192, 192, 192);

			unversioned = true;
		}
		else if (info.textStatus == Vcs::kStatus_Unversioned)
		{
			status = "unversioned";
			colour = RGB(127, 127, 127);

			unversioned = true;
		}
		else if (info.textStatus == Vcs::kStatus_Conflicted)
		{
			status = "conflicted";
			colour = RGB(255, 0, 0);
		}
		else if (info.textStatus == Vcs::kStatus_Added)
		{
			status = "added";
			colour = RGB(160, 0, 160);
		}
		else if (info.textStatus == Vcs::kStatus_Deleted)
		{
			status = "deleted";
			colour = RGB(160, 0, 0);
		}
		else if (info.copied)
		{
			status = "copied";
			colour = RGB(255, 0, 255);
		}
		else if (info.incomplete)
		{
			status = "incomplete";
			colour = RGB(255, 0, 255);
		}
		else if (info.absent)
		{
			status = "absent";
			colour = RGB(255, 0, 255);
		}
		else if (info.textStatus == Vcs::kStatus_Modified)
		{
			status = "modified";
			colour = RGB(0, 0, 160);
			isModified = true;
		}
		else if (info.textStatus == Vcs::kStatus_Missing)
		{
			status = "missing";
			colour = RGB(255, 0, 0);
		}
		else if (info.textStatus == Vcs::kStatus_External)
		{
			//status = "external";
			//colour = RGB(0, 0, 0);

			//Vcs::Svn svn;
			//svn.Info(info.localPath, true, PopulateListCallback, ctx);

			return;
		}
		else if (info.textStatus != Vcs::kStatus_Normal)
		{
			status = "other";
			colour = RGB(255, 0, 0);
		}
		else
		{
			//status = "normal";
			//colour = RGB(0, 0, 0);
			return;
		}

		// Work out db relative path

		int pathLength = (int)win->_params->path.Length();

		const char* filename = info.path + (((int)strlen(info.path) > pathLength) ? pathLength : 0);

		// Skip files we don't care about

		if ((_stricmp(filename, ".db-lock") == 0) || (_stricmp(filename, ".lock") == 0))
			return;

		// Create item

		CreateItem(win, unversioned ? kItemUnversioned : kItemFile, info.path, colour, filename, status);

		// Diff stuff

		XtString path, file, ext;

		if (XtsSplitFileName(info.path, path, file, ext) && (_stricmp(ext, ".xtd") == 0))
		{
			if (isModified)
			{
				Sz8 pristine;

				if (ctx->svn->GetPristineCopy(info.path, pristine))
				{
					XtUt::Ptr<Xt::DomPack> oldPack = Xt::ImportPack(info.path, 0);

					if (oldPack)
					{
						XtUt::Ptr<Xt::DomPack> newPack = Xt::ImportPack(pristine, 0);

						if (newPack)
						{
							int changeCount = 0;

							if (XtUt::Ptr<Xt::Change> change = Xt::Diff(newPack, oldPack))
							{
								if (change->type == Xt::kChangePack)
								{
									Xt::ChangePack* pack = (Xt::ChangePack*)(Xt::Change*)change;

									if (pack->properties)
									{
										CreateItem(win, kItemText, info.path, RGB(0, 0, 80), "     Properties", "modified");
										changeCount++;
									}

									for(Xt::ChangePack::Iterator it(&pack->changes); it; ++it)
									{
										Xt::DomRecord* oldRecord = oldPack->records[it.Key()];
										Xt::DomRecord* newRecord = newPack->records[it.Key()];

										XtString name;

										if (newRecord)
											name = newRecord->name + "." + newRecord->root.type;
										else if (oldRecord)
											name = oldRecord->name + "." + oldRecord->root.type;
										else
											name = Format("#%08X", it.Key());

										changeCount++;

										if (it->type == Xt::kChangeRemove)
										{
											CreateItem(win, kItemText, info.path, RGB(80, 0, 0), "     " + name, "deleted");
										}
										else if (it->type == Xt::kChangeLeaf)
										{
											CreateItem(win, kItemText, info.path, RGB(80, 0, 80), "     " + name, "added");
										}
										else
										{
											CreateItem(win, kItemText, info.path, RGB(80, 0, 80), "     " + name, "modified");

											if (it->type == Xt::kChangeRecord)
											{
												Xt::ChangeRecord* record = (Xt::ChangeRecord*)(Xt::Change*)it;

												if (record->root)
													CreateItem(win, kItemText, info.path, RGB(0, 0, 0), "            " + Summarise(record->root), "");
											}
										}
									}
								}
							}

							if (changeCount <= 0)
								CreateItem(win, kItemText, info.path, RGB(0, 0, 0), "     No relevant changes", "");
						}
						else
						{
							CreateItem(win, kItemText, info.path, RGB(0, 0, 0), "     Failed to open: " + XtString(pristine), "");
						}
					}
					else
					{
						CreateItem(win, kItemText, info.path, RGB(0, 0, 0), "     Failed to open: " + XtString(info.path), "");
					}
				}
			}
		}
	}

private:
	static void PopulateList(void* context)
	{
		CommitWin* win = (CommitWin*)context;

		Vcs::Svn svn;

		if (svn)
		{
			Vcs::Status status;

			PopulateListCallbackContext ctx;

			ctx.win = win;
			ctx.svn = &svn;

			if (!(status = svn.Info(win->_params->path, Vcs::kDepth_Infinity, PopulateListCallback, &ctx)))
				CreateItem(win, kItemText, "", RGB(0, 0, 0), XtString("Error: ") + status.text, "");

			if (Count(&win->_items) <= 0)
				CreateItem(win, kItemText, "", RGB(0, 0, 0), "No relevant changes", "");
		}

		win->_isDonePopulating = true;

		for(XtUt::List<VcsItem>::Iterator it(&win->_items); it; ++it)
		{
			LVITEM item = { 0 };

			item.mask		= LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
			item.iItem		= (int)ListView_GetItemCount(win->_list);
			item.iSubItem	= 0;
			item.pszText	= (LPSTR)it->name.Text();
			item.lParam		= it.index;
			item.stateMask	= LVIS_STATEIMAGEMASK;
			item.state		= (it->kind != kItemText) ? INDEXTOSTATEIMAGEMASK(2) : 0;

			item.iItem = ListView_InsertItem(win->_list, &item);

			item.mask		= LVIF_TEXT;
			item.iSubItem	= 1;
			item.pszText	= (LPSTR)it->textStatus.Text();

			ListView_SetItem(win->_list, &item);

			if (it->kind == kItemFile)
				ListView_SetCheckState(win->_list, item.iItem, TRUE);
		}

		Sz8 text = SzGetWindowText(win->_comment);
		EnableWindow(win->_okay, *text && (text != "Enter comment here"));

		EnableWindow(win->_cancel, TRUE);
		SetWindowText(win->_status, "");
		GetSystemMenu(*win, TRUE);

		RedrawWindow(*win, 0, 0, RDW_FRAME | RDW_INVALIDATE);
	}

private:
	CommitParams* _params;

private:
	CommitResult _result;

private:
	ArteryThread	_thread;
	bool			_isDonePopulating;

private:
	HWND _comment;
	HWND _list;
	HWND _okay;
	HWND _cancel;
	HWND _status;
};

CommitResult SorceryCommit(CommitParams* params)
{
	assert(params != 0);
	assert(params->parentWindow != 0);

	CommitWin win(params);

	WinRect rc;
	GetWindowRect(params->parentWindow, &rc);

	int left	= rc.left + (rc.Width() / 2) - 250;
	int top		= rc.top + (rc.Height() / 2) - 250;

	if (win.DoModal(WinPoint(left, top), WinSize(500, 500), params->parentWindow) <= 0)
		return kCrCancelled;

	return win.Result();
}

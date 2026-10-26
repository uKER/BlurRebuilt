#include "Core/StdAfx.h"
#include "Application/SorceryApp.h"
#include "Application/SorceryView.h"
#include "Application/SorceryExport.h"
#include "Resource/Resource.h"
#include "DynamicForm/GridForm.h"
#include "DynamicForm/HyperView.h"
#include "Xt/XtTool.h"
#include "XtWin/DeskWindow.h"
#include "Application/GotoRecordDlg.h"
#include "XtWin/SendEmail.h"
#include <PsApi.h>

extern void BuildAllBanks(SorceryApp& app);
extern void BuildCurrentBank(SorceryApp& app);

/*
SetFloat(const char* key, XtF64 value);
SetInt(const char* key, XtI64 value);
SetString(const char* key, const char* value);
SetUnicode(const char* key, const wchar_t* value);
*/
/*

All dependencies are listed in the "References" section, so the scope is always local

<Class name="Emitter">
	<Field name="particle" display="combobox">
		<Data>
			<Source path="namespace/shooter.shared.particles/*" />
				<Where type="Particle" clause="object.flags.magic == this.flags.magic" />
			</Source>

			<Source path="this/*" />
				<Where type="Particle" clause="object.flags.magic == this.flags.magic" />
			</Source>
		</Data>
		<Label text="{&object.name} ({name:object})" />
		<Value field="object.id" />
	</Field>
</Class>

*/

SorceryWindowFdc::SorceryWindowFdc(SorceryWindow* window_) :
	window(window_)
{
}

Xt::DomDb* SorceryWindowFdc::Db()
{
	return window->App()._db;
}

void SorceryWindowFdc::OpenRecord(Xt::DomRecord* record)
{
	window->OpenRecord(record);
}

ConnectToData gConnectToData;

bool DoEnsureUniqueIds(AsyncOp* op, SorceryApp* app)
{
	op->SetTitle("Ensure Unique Ids...");

	op->NewMessage(kAoiInfo, "Processing...");

	EuuParams params;

	params.ensureGloballyUnique = true;

	for(Xt::DomDb::Iterator itPack(app->_db); itPack; ++itPack)
	{
		for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
		{
			if (LtStruct* type = TypeOf(app->_db->schema, itRecord))
			{
				params.missingCount		= 0;
				params.collisionCount	= 0;
				params.totalCount		= 0;

				EnsureUniqueUids(app->_db->schema, type, &itRecord->root, &params);

				if (params.totalCount > 0)
				{
					op->NewMessage(kAoiWarning, itRecord->name + " [" + itPack->name + "]");

					op->AppendMessage(Format<char>("Missing: %i", params.missingCount));
					op->AppendMessage(Format<char>("Collisions: %i", params.collisionCount));
					op->AppendMessage(Format<char>("Total: %i", params.totalCount));
				}
			}
		}
	}

	op->NewMessage(kAoiInfo, "Done...");

	return true;
}

//
// SorceryWindow
//

SorceryWindow::SorceryWindow(class SorceryApp& app) :
	_app(app),
	_statusHwnd(0),
	_explorer(*this),
	_isConnected(false),
	_monitor(DomCallback, this),
	_finderCtx(this),
	_finder(&_finderCtx)
{
	_menu = LoadMenu(GetModuleHandle(0), MAKEINTRESOURCE(IDM_SORCERY));
}

SorceryWindow::~SorceryWindow()
{
}

bool SorceryWindow::PreCreateMdiWindow(CLIENTCREATESTRUCT* ccs)
{
	ccs->hWindowMenu	= GetSubMenu(_menu, 5);
	ccs->idFirstChild	= ID_MDI_WINDOW;

	return true;
}

bool SorceryWindow::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!FrameWindow::PreCreateWindow(cs))
		return false;

	cs->hMenu = _menu;

	return true;
}

bool SorceryWindow::PreTranslateMessage(MSG& msg)
{
	if (TranslateAccelerator(*this, _accelerator, &msg) != FALSE)
		return true;

	return FrameWindow::PreTranslateMessage(msg);
}

LRESULT SorceryWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			_statusHwnd = CreateWindowEx(0, STATUSCLASSNAME, _T(""), SBARS_SIZEGRIP | SBARS_TOOLTIPS | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, *this, 0, 0, 0);

			_explorer.Create(WinPoint(0, 0), WinSize(0, 0), *this);

			if (FrameWindow::WndProc(msg, wParam, lParam) != 0)
				return -1;

			if ((_accelerator = LoadAccelerators(0, MAKEINTRESOURCE(IDA_SORCERY))) == NULL)
				return -1;

			LoadConnectToData(&gConnectToData); // TODO: Should be in SorceryApp

			SendMessage(*this, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_SORCERY)));

			//SetWindowLong(_mdiHwnd, GWL_EXSTYLE, GetWindowLong(_mdiHwnd, GWL_EXSTYLE) | WS_EX_CLIENTEDGE);

			SendMessage(_statusHwnd, SB_SETTEXT, SBT_NOBORDERS, (LPARAM)_T(""));

			_host.SetWindow(*this);

			DwSplit* sp = (DwSplit*)_host.Root()->SetChild(0, new DwSplit(&_host));
			sp->Create();
			sp->SetVertical(true);

			DwPane* pa0 = (DwPane*)sp->SetChild(0, new DwPane(&_host));
			DwPane* pa1 = (DwPane*)sp->SetChild(1, new DwPane(&_host));

			pa0->Add(new DwTab(&_host, _explorer));
			pa1->Add(new DwTab(&_host, _mdiWnd));

			sp->SetPosition(220);

			SetTimer(*this, WM_APP, 30, 0);

			SetWindowTitle();
		}
		return 0;

		case WM_DESTROY:
			_app._xtNet.Destroy();

			_host.SetWindow(0);
		break;

		case WM_CLOSE:
		{
			bool needSave = _app._db->dirty;

			for(Xt::DomDb::Iterator itPack(_app._db); itPack; ++itPack)
			{
				if (itPack->dirty)
				{
					needSave = true;
					break;
				}

				for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
				{
					if (itRecord->dirty)
					{
						needSave = true;
						break;
					}
				}
			}

			if (needSave)
			{
				switch(MessageBox(*this, _T("Would you like to save your changes to this database?"), _T("Sorcery"), MB_ICONQUESTION | MB_YESNOCANCEL))
				{
					case IDYES:
						if (!_app.SaveProject())
							return 0;
					break;

					case IDNO:
					break;

					default:
					return 0;
				}
			}

			// Cheat for fast shutdowns when not running under the debugger

			if (!IsDebuggerPresent())
				ExitProcess(0);

			// End cheat

			for(LinkIterator<SorceryView> it(_views); it; )
			{
				SorceryView* view = it;

				++it;

				view->Close();
			}

			PostQuitMessage(0);
		}
		return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(_hwnd, &ps);
			_host.Paint(ps.hdc, ps.rcPaint);
			EndPaint(_hwnd, &ps);
		}
		return 0;

		case WM_SIZE:
		{
			SendMessage(_statusHwnd, WM_SIZE, 0, 0);

			if (!IsMinimized(*this))
			{
				WinRect rcWin(*this);
				WinRect rcStatus(_statusHwnd);

				_host.SetLocation(WinRect(rcWin.left, rcWin.top, rcWin.right, rcWin.bottom - rcStatus.Height()));

				_host.PerformLayout(false);
			}
		}
		return 0;

		case WM_INITMENU:
		{
			Xt::DomDb* db = _app._db;

			HMENU menu = GetMenu(*this);

			// Platform menu
			{
				HMENU popup	= CreatePopupMenu();

				for(XtUt::List<Xt::BuildPlatform>::Iterator it(&db->build.platforms); it; ++it)
				{
					int checked = MFS_CHECKED;

					for(XtUt::Array<XtString>::Iterator itSkip(&_app._skipPlatforms); itSkip; ++itSkip)
					{
						if (*itSkip == it->name)
						{
							checked = 0;
							break;
						}
					}

					AppendMenu(popup, MF_STRING | checked, ID_TOOLS_PLATFORMFILTER_FIRST + it.index, SplitIntercaps(it->name));
				}

				MENUITEMINFO mii = { sizeof(mii) };

				mii.fMask		= MIIM_SUBMENU;
				mii.hSubMenu	= popup;

				SetMenuItemInfo(menu, ID_TOOLS_PLATFORMFILTER, FALSE, &mii);
			}

			// Work offline
			{
				MENUITEMINFO mii = { sizeof(mii) };

				mii.fMask	= MIIM_STATE;
				mii.fState	= _app._xtNet.IsWorkingOffline() ? MFS_CHECKED : 0;

				SetMenuItemInfo(menu, ID_TOOLS_WORKOFFLINE, FALSE, &mii);
			}
		}
		return 0;

		case WM_COMMAND:
		{
			int cmd = LOWORD(wParam);

			if ((cmd >= ID_TOOLS_PLATFORMFILTER_FIRST) && (cmd < (ID_TOOLS_PLATFORMFILTER_LAST + 20)))
			{
				Xt::DomDb*			db		= _app._db;
				Xt::BuildPlatform*	plat	= At(&db->build.platforms, cmd - ID_TOOLS_PLATFORMFILTER_FIRST);

				if (plat)
				{
					int found = -1;
				
					for(XtUt::Array<XtString>::Iterator itSkip(&_app._skipPlatforms); itSkip; ++itSkip)
					{
						if (itSkip == plat->name)
						{
							found = itSkip.index;
							break;
						}
					}

					if (found >= 0)
						XtUt::Remove(&_app._skipPlatforms, found);
					else
						XtUt::AppendEx(&_app._skipPlatforms, plat->name);
				}
			}

			switch(cmd)
			{
				case ID_FILE_CLOSE:
					OnFileClose();
				return 0;

				case ID_FILE_OPENPROJECT:
				return 0;

				case ID_FILE_SAVEPROJECT:
					_app.SaveProject();
				return 0;

				case ID_FILE_CLOSEPROJECT:
				return 0;

				case ID_FILE_EXIT:
					PostMessage(*this, WM_CLOSE, 0, 0);
				return 0;

				case ID_TOOLS_BUILDPROJECT:
					OnBuildProject(false);
				return 0;

				case ID_TOOLS_BUILDPROJECT_DATAONLY:
					OnBuildProject(true);
				return 0;

				case ID_TOOLS_BUILDPROJECT_NODEPLOY:
					SorceryExport(_app, false, true, 0);
				return 0;

				case ID_TOOLS_OPENDBFOLDER:
					ShellExecute(*this, _T("open"), _app._db->path, 0, 0, SW_SHOWDEFAULT);
				return 0;

				case ID_WINDOW_CASCADE:
					SendMessage(_mdiWnd, WM_MDICASCADE, 0, 0);
				return 0;

				case ID_WINDOW_TILE_HORZ:
					SendMessage(_mdiWnd, WM_MDITILE, MDITILE_HORIZONTAL, 0);
				return 0;

				case ID_WINDOW_TILE_VERT:
					SendMessage(_mdiWnd, WM_MDITILE, MDITILE_VERTICAL, 0);
				return 0;

				case ID_TOOLS_CONVERTOLDDATA:
				{
					void ConvertData(SorceryApp& app);
					ConvertData(_app);
				}
				return 0;

				case ID_TOOLS_SEARCH:
					OpenSearchWindow();
				break;

				case ID_SUBVERSION_UPDATE:
					_app.SvnUpdate();
				break;

				case ID_SUBVERSION_COMMIT:
					_app.SvnCommit();
				break;

				case ID_EDIT_FIND:
					OpenSearchWindow();
				break;

				case ID_TOOLS_CONNECTTO:
				{
					ConnectToDlg dlg(*this);

					dlg.Data() = gConnectToData;

					if (dlg.Modal())
					{
						gConnectToData = dlg.Data();

						_app._xtNet.Reconnect();

						SaveConnectToData(&gConnectToData);
					}
				}
				return 0;

				case ID_DEBUG_RECOVERDATABASE:
					_app.SvnRecover();
				break;

				case ID_TOOLS_BUILDWAVEBANKS:					
					BuildAllBanks(_app);				
					return 0;				
				case ID_TOOLS_BUILDCURRENTWAVEBANK:					
					BuildCurrentBank(_app);				
					return 0;				
				case ID_HELP_RELOADSCHEMA:
					_app.ReloadSchema();
				return 0;

				case ID_GOTO_RECORD:
				{
					GotoRecordDlg dlg(*this);

					if (dlg.Modal())
					{
						if (Xt::DomRecord* record = FindRecordById(_app._db, dlg.RecordId()))
						{
							OpenRecord(record);
						}
						else
						{
							if (Xt::DomRecord* record = FindRecordById(_app._db, dlg.AltRecordId()))
							{
								OpenRecord(record);
							}
							else
							{
								MessageBox(*this, _T("Record not found."), _T("Sorcery"), MB_ICONEXCLAMATION | MB_OK);
							}
						}
					}
				}
				return 0;

				case ID_DEBUG_ENSUREUNIQUEUIDS:
					AsyncInvoke(*this, DoEnsureUniqueIds, &_app);
				return 0;

				case ID_DEBUG_COMPARETARGETSCHEMA:
					void CompareTargetSchema(SorceryApp*);
					CompareTargetSchema(&_app);
				return 0;

				case ID_TOOLS_WORKOFFLINE:
				{
					if (_app._xtNet.IsWorkingOffline())
						_app._xtNet.WorkOnline();
					else
						_app._xtNet.WorkOffline();

					SetWindowTitle();
				}
				return 0;

				case ID_HELP_SENDEMAIL:
				{
					XtString body;

					body = "\r\n\r\n\r\nSorcery build date: " + GetExeBuildTime() + "\r\n";
					body += "Xt db version: " + Format("%i", XtGetDbVersion()) + "\r\n";
					body += "Db path: " + _app._dbPath + "\r\n\r\n";
					body += "Loaded modules:\r\n";

					HMODULE modules[2048];
					DWORD needed = 0;

					if (EnumProcessModules(GetCurrentProcess(), modules, sizeof(modules), &needed) != FALSE)
					{
						int count = needed / sizeof(HMODULE);

						for(int i = 0; i < count; i++)
						{
							char fname[MAX_PATH];

							if (GetModuleFileNameEx(GetCurrentProcess(), modules[i], fname, MAX_PATH) > 0)
							{
								body += XtString("\t") + fname + "\r\n";
							}
						}
					}

					SendEmail("Sorcery Issue -", body);
				}
				return 0;

				case ID_DEBUG_TRANSFEROPENRECORDS:
				{
					for(LinkIterator<SorceryDocument> itDoc(_app._documents); itDoc; ++itDoc)
					{
						if (itDoc->_kind != kSdkRecord)
							continue;

						Xt::DomRecord* record = AsRecord(itDoc->_dom);

						if (!record)
							continue;

						void NetUpdateHack(SorceryApp* app, Xt::DomRecord* record);
						NetUpdateHack(&_app, record);
					}
				}
				return 0;
			}
		}
		break;

		case WM_ACTIVATE:
		{
			HWND hwnd = (HWND)SendMessage(_mdiWnd, WM_MDIGETACTIVE, 0, 0);

			if (hwnd)
				SendMessage(hwnd, kWmNotifyActivate, wParam, lParam);
		}
		return 0;

		case WM_SETFOCUS:
		{
			HWND hwnd = (HWND)SendMessage(_mdiWnd, WM_MDIGETACTIVE, 0, 0);

			if (hwnd)
				SendMessage(hwnd, kWmNotifySetFocus, wParam, lParam);
		}
		return 0;

		case WM_TIMER:
		{
			if (wParam == WM_APP)
			{
				static int count = 0;
				static int count2 = 0;

				if (++count >= 100)
				{
					NetSendIdle(&_app);
					count = 0;
				}

				void NetSendHack(SorceryApp*);
				NetSendHack(&_app);

				if (++count2 >= 40)
				{
					_app._xtNet.Pump();
					count2 = 0;
				}

				return 0;
			}
		}
		break;

		case kWmNetworkNotify:
		{
			switch(wParam)
			{
				case kNetConnected:
				{
					_isConnected = true;

					SetWindowTitle();

					void NetSendHackAll(SorceryApp* app);
					NetSendHackAll(&_app);
				}
				break;

				case kNetDisconnected:
					_isConnected = false;

					SetWindowTitle();
				break;
			}

			_app._xtNet.Pump();
		}
		break;

		case kWmDbNotify:
		{
			switch(wParam)
			{
				case kNetConnected:
					if (!_app._xtNet.Create(&_app, *this, kWmNetworkNotify))
					{
						DebugLn("Failed to start XtNet");
					}
				break;

				case kNetDisconnected:
					_app._xtNet.Destroy();
				break;
			}
		}
		break;
	}

	return FrameWindow::WndProc(msg, wParam, lParam);
}

void SorceryWindow::SetWindowTitle()
{
	if (_isConnected)
	{
		XtString name = _app._xtNet.TargetName();

		SetWindowText(*this, "Bob - " + name);
	}
	else if (_app._xtNet.IsWorkingOffline())
		SetWindowText(*this, _T("Bob - (working offline)"));
	else
		SetWindowText(*this, _T("Bob - (disconnected)"));
}

void SorceryWindow::OnFileNew()
{
}

void SorceryWindow::OnFileOpen()
{
}

void SorceryWindow::OnFileClose()
{
	if (SorceryView* view = GetActiveView())
		SendMessage(*view->_window, WM_CLOSE, 0, 0);
}

void SorceryWindow::OnBuildProject(bool dataOnly)
{
	SorceryExport(_app, dataOnly, false, 0);
}

void SorceryWindow::OpenRecord(Xt::DomRecord* record)
{
	if (!record)
		return;

	for(LinkIterator<SorceryDocument> itDoc(_app._documents); itDoc; ++itDoc)
	{
		if (itDoc->_kind != kSdkRecord)
			continue;

		if (itDoc->_id != record->id)
			continue;

		for(LinkIterator<SorceryView> itView(itDoc->_views); itView; ++itView)
			SendMessage(_mdiWnd, WM_MDIACTIVATE, (WPARAM)(HWND)*itView->_window, 0);

		return;
	}

	SorceryDocument* doc = new SorceryDocument(_app);

	if (!doc)
		return;

	doc->_kind	= kSdkRecord;
	doc->_id	= record->id;

	Monitor(&doc->_dom, record);

	SorceryViewType type = kSvtScarab;

	if (record->root.type == "XtLocTable")
		type = kSvtLoc;
	else if (record->root.type == "WaveBank")		type = kSvtWaveBank;
	SorceryView* view = new SorceryView(_app, doc, type);

	if (!view)
	{
		delete doc;
		return;
	}

	doc->_link.AttachBefore(_app._documents);
	view->_linkWindow.AttachBefore(_app._window._views);
	view->_linkDocument.AttachBefore(doc->_views);

	view->Create();
}

void SorceryWindow::OpenPack(Xt::DomPack* pack)
{
	if (!pack)
		return;

	for(LinkIterator<SorceryDocument> itDoc(_app._documents); itDoc; ++itDoc)
	{
		if (itDoc->_kind != kSdkPack)
			continue;

		if (itDoc->_id != pack->id)
			continue;

		for(LinkIterator<SorceryView> itView(itDoc->_views); itView; ++itView)
			SendMessage(_mdiWnd, WM_MDIACTIVATE, (WPARAM)(HWND)*itView->_window, 0);

		return;
	}

	SorceryDocument* doc = new SorceryDocument(_app);

	if (!doc)
		return;

	doc->_kind	= kSdkPack;
	doc->_id	= pack->id;

	Monitor(&doc->_dom, pack);

	SorceryView* view = new SorceryView(_app, doc, kSvtScarab);

	if (!view)
	{
		delete doc;
		return;
	}

	doc->_link.AttachBefore(_app._documents);
	view->_linkWindow.AttachBefore(_app._window._views);
	view->_linkDocument.AttachBefore(doc->_views);

	view->Create();
}

void SorceryWindow::OpenDb(Xt::DomDb* db)
{
	if (!db)
		return;

	for(LinkIterator<SorceryDocument> itDoc(_app._documents); itDoc; ++itDoc)
	{
		if (itDoc->_kind != kSdkDb)
			continue;

		for(LinkIterator<SorceryView> itView(itDoc->_views); itView; ++itView)
			SendMessage(_mdiWnd, WM_MDIACTIVATE, (WPARAM)(HWND)*itView->_window, 0);

		return;
	}

	SorceryDocument* doc = new SorceryDocument(_app);

	if (!doc)
		return;

	doc->_kind = kSdkDb;

	Monitor(&doc->_dom, db);

	SorceryView* view = new SorceryView(_app, doc, kSvtScarab);

	if (!view)
	{
		delete doc;
		return;
	}

	doc->_link.AttachBefore(_app._documents);
	view->_linkWindow.AttachBefore(_app._window._views);
	view->_linkDocument.AttachBefore(doc->_views);

	view->Create();
}

void SorceryWindow::OpenGridView(const XtString& typeName)
{
	SorceryView* view = new SorceryView(_app, 0, kSvtGrid);

	if (!view)
		return;

	((GridMdiWindow*)view->_window)->SetType(typeName);

	view->_linkWindow.AttachBefore(_app._window._views);

	view->Create();
}

void SorceryWindow::OpenHyperView(Xt::DomRecord* record)
{
	SorceryView* view = new SorceryView(_app, 0, kSvtHyper);

	if (!view)
		return;

	((HyperViewMdiWindow*)view->_window)->SetRecord(record);

	view->_linkWindow.AttachBefore(_app._window._views);

	view->Create();
}

SorceryView* SorceryWindow::GetActiveView()
{
	HWND active = (HWND)SendMessage(_mdiWnd, WM_MDIGETACTIVE, 0, 0);

	for(LinkIterator<SorceryView> it(_views); it; ++it)
		if (*it->_window == active)
			return it;

	return 0;
}

void SorceryWindow::OpenSearchWindow()
{
	if ((HWND)_finder == 0)
		_finder.Create(200, 150, 700, 500, *this);
	else
		BringWindowToTop(_finder);
}

void SorceryWindow::FindDependencies(Xt::DomRecord* record)
{
	OpenSearchWindow();

	if ((HWND)_finder != 0)
	{
		_finder.SetupFindDependencies(record);
	}
}

void SorceryWindow::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SorceryWindow* window)
{
	if (event->code == Xt::kDomDbOpened)
	{
		PostMessage(*window, kWmDbNotify, kNetConnected, 0);
	}
	else if (event->code == Xt::kDomDbClosing)
	{
		PostMessage(*window, kWmDbNotify, kNetDisconnected, 0);
	}
}
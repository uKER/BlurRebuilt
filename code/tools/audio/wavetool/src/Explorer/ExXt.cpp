#include "Core/StdAfx.h"
#include "Application/SorceryApp.h"
#include "Explorer/Explorer.h"
#include "Xt/FindDependencies.h"
#include <XtWin/SendEmail.h>
#include <Xt/Impl/CryptRandom.h>

void NetUpdateHack(SorceryApp* app, Xt::DomRecord* record);

extern bool g_bAudioMode;
extern void BuildBanksSubset(SorceryApp& app, const char* pchFilter);

XtString MakeRecordId(Xt::DomRecord* record)
{
	return Format("1%s.%s-%08X", (const char*)record->name, (const char*)record->root.type, record->id);
}

//
// Lifetime
//

ExXt::ExXt(ExNode* node, Xt::DomRecord* record, const XtString& label) :
	ExItem(kEitRecord, node),
	_xtId(record->id)
{
	_dom.SetCallback(DomCallback, this);

	SetId(MakeRecordId(record));
	SetLabel(label);
	SetIcon(kEniFile, kEniFile);

	SorceryApp& app = Node()->Explorer()->Window().App();

	if (Xt::DomRecord* rec = FindRecordById(app._db, _xtId))
		Monitor(&_dom, rec);

	DomCallback(&_dom, &Xt::DomRecordChanged(RecordOf(_dom)), this);
}

ExXt::~ExXt()
{
}

bool ExXt::OnEnter()
{
	Open();

	return true;
}

bool ExXt::OnDoubleClick()
{
	Open();

	return true;
}

bool ExXt::OnRightClick()
{
	SorceryApp& app = Node()->Explorer()->Window().App();

	HMENU menu = CreatePopupMenu();

	AppendMenu(menu, MF_STRING, 1, _T("Open"));
	AppendMenu(menu, MF_STRING, 12, _T("Open in Grid View"));
	AppendMenu(menu, MF_STRING, 14, _T("Open in Hyper View"));
	AppendMenu(menu, MF_SEPARATOR, 0, 0);
	AppendMenu(menu, MF_STRING, 2, _T("Rename"));
	AppendMenu(menu, MF_STRING, 3, _T("Delete"));
	AppendMenu(menu, MF_STRING, 4, _T("Duplicate"));
	AppendMenu(menu, MF_SEPARATOR, 0, 0);
	AppendMenu(menu, MF_STRING, 8, _T("Find Dependencies"));
	AppendMenu(menu, MF_SEPARATOR, 0, 0);

	bool isDirty = false;

	if (Xt::DomRecord* record = FindRecordById(app._db, _xtId))
	{
		if (record->dirty)
			isDirty = true;
	}

	AppendMenu(menu, MF_STRING | (!isDirty ? (MF_GRAYED | MF_DISABLED) : 0), 5, _T("Reload"));

	if (g_bAudioMode)
	{
		if (Xt::DomRecord* record = FindRecordById(app._db, _xtId))
		{
			if (record->root.type == "WaveBank")
			{
				AppendMenu(menu, MF_SEPARATOR, 0, 0);
				AppendMenu(menu, MF_STRING, 9, _T("Build this WaveBank"));
			}
		}
	}

	AppendMenu(menu, MF_SEPARATOR, 0, 0);

	AppendMenu(menu, MF_STRING, 10, _T("Export..."));
	AppendMenu(menu, MF_STRING, 11, _T("Import.."));

	AppendMenu(menu, MF_SEPARATOR, 0, 0);

	AppendMenu(menu, MF_STRING, 13, Format<char>("Id = %08X", _xtId));

	SetMenuDefaultItem(menu, 1, 0);

	WinPoint pt;
	GetCursorPos(&pt);

	int result = TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, Node()->Explorer()->Window(), 0);

	DestroyMenu(menu);

	switch(result)
	{
		case 1:
			Open();
		break;

		case 2:
			TreeView_EditLabel(Node()->Explorer()->Tree(), Node()->TreeItem());
		break;

		case 3:
			OnDeletePressed();
		break;

		case 4:
			if (Node() && Node()->Container())
				OnDuplicateRecord(Node()->Container()->Item(), true);
		break;

		case 5:
		{
			bool reopen = false;

			for(LinkIterator<SorceryDocument> itDoc(app._documents); itDoc; ++itDoc)
			{
				if (itDoc->_id == _xtId)
				{
					for(LinkIterator<SorceryView> itView(itDoc->_views); itView; )
					{
						SorceryView* view = itView;

						++itView;

						bool gotNext = itView != 0;

						view->Close();

						reopen = true;

						if (!gotNext)
							break;
					}

					break;
				}
			}

			if (Xt::DomRecord* record = FindRecordById(app._db, _xtId))
			{
				if (!RevertToPristine(record))
					MessageBox(app._window, "Failed to reload:\r\n" + record->name, "Sorcery", MB_ICONERROR | MB_OK);

				NetUpdateHack(&app, record);

				if (reopen)
					app._window.OpenRecord(record);
			}
		}
		break;

		case 8:
		{
			if (Xt::DomRecord* record = FindRecordById(app._db, _xtId))
			{
				app._window.FindDependencies(record);
			}
		}
		break;

		case 9:
		{
			if (Xt::DomRecord* record = FindRecordById(app._db, _xtId))
				BuildBanksSubset(app, record->name);
		}
		break;

		case 10:
		{
			TCHAR currentDir[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, currentDir);

			TCHAR filename[MAX_PATH] = "";

			if (Xt::DomRecord* record = FindRecordById(app._db, _xtId))			
			{				
				_snprintf_s(filename, MAX_PATH, _TRUNCATE, "%s.%s.xtd", record->name.Text(), record->root.type.Text());			
			}

			OPENFILENAME ofn = { 0 };

			ofn.lStructSize			= sizeof(ofn);
			ofn.hwndOwner			= Node()->Explorer()->Window();
			ofn.lpstrFilter			= _T("Xt data files (*.xtd)\0*.xtd\0All files (*.*)\0*.*\0");
			ofn.lpstrFile			= filename;
			ofn.nMaxFile			= MAX_PATH;
			//ofn.lpstrInitialDir	= pathRoot;
			ofn.lpstrTitle			= _T("Record export...");
			ofn.lpstrDefExt			= _T("xtd");
			ofn.Flags				= OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

			if (GetSaveFileName(&ofn) != FALSE)
			{
				if (Xt::DomRecord* record = FindRecordById(app._db, _xtId))
				{
					if (!Xt::Export2(&record->root.data, filename))
					{
						MessageBox(Node()->Explorer()->Window(), _T("Export failed"), _T("Bob"), MB_ICONERROR | MB_OK);
					}
				}
			}

			SetCurrentDirectory(currentDir);
		}
		break;

		case 11:
		{
			TCHAR currentDir[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, currentDir);

			TCHAR filename[MAX_PATH] = "";

			OPENFILENAME ofn = { 0 };

			ofn.lStructSize			= sizeof(ofn);
			ofn.hwndOwner			= Node()->Explorer()->Window();
			ofn.lpstrFilter			= _T("Xt data files (*.xtd)\0*.xtd\0All files (*.*)\0*.*\0");
			ofn.lpstrFile			= filename;
			ofn.nMaxFile			= MAX_PATH;
			//ofn.lpstrInitialDir	= pathRoot;
			ofn.lpstrTitle			= _T("Record import...");
			ofn.Flags				= OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

			if (GetOpenFileName(&ofn) != FALSE)
			{
				XtUt::Ptr<Xt::DomElement> newDataEl = Xt::Import2(filename, 0, 0);

				if (Xt::DomBlock* newData = AsBlock(newDataEl))
				{
					if (Xt::DomRecord* record = FindRecordById(app._db, _xtId))
					{
						bool reopen = false;

						for(LinkIterator<SorceryDocument> itDoc(app._documents); itDoc; ++itDoc)
						{
							if (itDoc->_id == _xtId)
							{
								for(LinkIterator<SorceryView> itView(itDoc->_views); itView; )
								{
									SorceryView* view = itView;

									++itView;

									bool gotNext = itView != 0;

									view->Close();

									reopen = true;

									if (!gotNext)
										break;
								}

								break;
							}
						}

						Xt::Clone(&record->root.data, newData);

						Xt::Notify(record, &Xt::DomRecordChanged(record));

						NetUpdateHack(&app, record);

						if (reopen)
							app._window.OpenRecord(record);
					}
				}
				else
				{
					MessageBox(Node()->Explorer()->Window(), _T("Import failed"), _T("Bob"), MB_ICONERROR | MB_OK);
				}
			}

			SetCurrentDirectory(currentDir);
		}
		break;

		case 12:
			if (Xt::DomRecord* record = FindRecordById(app._db, _xtId))
			{
				app._window.OpenGridView(record->root.type);
			}
		break;

		case 13:
			SetClipboardText(app._window, Format("%08X", _xtId));
		break;

		case 14:
		{
			if (Xt::DomRecord* record = FindRecordById(app._db, _xtId))
			{
				app._window.OpenHyperView(record);
			}
		}
		break;
	}

	return true;
}

bool ExXt::OnBeginEditLabel()
{
	HWND tree = Node()->Explorer()->Tree();
	HWND edit = TreeView_GetEditControl(tree);

	SorceryApp&		app		= Node()->Explorer()->Window().App();
	Xt::DomRecord*	record	= FindRecordById(app._db, _xtId);

	if (!record)
		return false;

	const char* dot = record->name;

	for(const char* ptr = dot; *ptr != '\0'; ptr++)
		if (*ptr == '.')
			dot = ptr + 1;

	Edit_SetText(edit, dot);

	return true;
}

bool ExXt::OnEndEditLabel(const char* newTextArg)
{
	if (!newTextArg)
		return false;

	XtString newText = XtsStripBoth(newTextArg, ' ');

	if (!XtsIsName(newText))
		return false;

	SorceryApp&		app		= Node()->Explorer()->Window().App();
	Xt::DomRecord*	record	= FindRecordById(app._db, _xtId);

	if (!record)
		return false;

	SetLabel(XtString(newText) + "." + record->root.type);

	const char* start	= record->name;
	const char* dot		= start;

	for(const char* ptr = dot; *ptr != '\0'; ptr++)
		if (*ptr == '.')
			dot = ptr + 1;

	record->name = XtString(start, (int)(dot - start)) + newText;

	SetDirty(record, true);

	SetId(MakeRecordId(record));

	Node()->Explorer()->Root()->Populate(TVI_FIRST);
	SendMessage(Node()->Explorer()->Tree(), TVM_SELECTITEM, TVGN_CARET, (LPARAM)Node()->TreeItem());

	return false;
}

void ExXt::OnDeletePressed()
{
	XtString name = "(unknown record)";

	SorceryApp&		app		= Node()->Explorer()->Window().App();
	Xt::DomRecord*	record	= FindRecordById(app._db, _xtId);
	HWND			tree	= Node()->Explorer()->Tree();

	if (record)
		name = record->name + "." + record->root.type;

	if (MessageBox(Node()->Explorer()->Window(), "Are you sure you want to delete the record:\r\n\r\n" + name, "Bob", MB_ICONQUESTION | MB_OKCANCEL) == IDOK)
		Delete();

	SetFocus(tree);
}

void DuplicateRecordName(XtString& name, ExItem* item)
{
	if (!item || !item->Node())
		return;

	if (item->Node()->Container())
	{
		if (item->Node()->Container()->Item()->Type() == kEitFolder)
		{
			DuplicateRecordName(name, item->Node()->Container()->Item());

			name += ".";
		}
	}

	name += item->Node()->Label();
}

void ExXt::OnDuplicateRecord(ExItem* parent, bool renameRecord)
{
	SorceryApp& app = Node()->Explorer()->Window().App();

	if (Xt::DomRecord* oldRecord = FindRecordById(app._db, _xtId))
	{
		if (Xt::DomPack* oldPack = PackOf(oldRecord))
		{
			Xt::DomRecord* newRecord = Xt::DuplicateRecord(app._db->schema, oldRecord);

			if (!newRecord)
				return;

			if (renameRecord)
			{
				newRecord->name = Format("%s (2)", (const char*)oldRecord->name);
			}
			else
			{
				XtString newName;

				DuplicateRecordName(newName, parent);

				if (newName.Length() > 0)
					newName += ".";

				const char* dot = oldRecord->name;

				for(const char* ptr = dot; *ptr != '\0'; ptr++)
					if (*ptr == '.')
						dot = ptr + 1;

				newRecord->name = newName + dot;
			}

			ExNode* node = ExAcquireRecord(Node()->Explorer()->Root(), newRecord);

			if (!node)
				return;

			if (renameRecord)
			{
				Node()->Explorer()->Root()->Populate(TVI_FIRST);

				TreeView_Expand(Node()->Explorer()->Tree(), Node()->TreeItem(), TVE_EXPAND);
				Node()->UpdateIconLabel();

				TreeView_EditLabel(node->Explorer()->Tree(), node->TreeItem());
			}

			NetUpdateHack(&app, newRecord);
		}
	}
}

void ExXt::Selected()
{
}

void ExXt::Delete()
{
	SorceryApp& app = Node()->Explorer()->Window().App();

	for(LinkIterator<SorceryDocument> itDoc(app._documents); itDoc; ++itDoc)
	{
		if (itDoc->_id != _xtId)
			continue;

		for(LinkIterator<SorceryView> itView(itDoc->_views); itView; )
		{
			SorceryView* view = itView;

			++itView;

			view->Destroy(false);
		}

		delete itDoc;

		break;
	}

	if (Xt::DomRecord* record = FindRecordById(app._db, _xtId))
	{
		if (Xt::DomPack* pack = PackOf(record))
			RemoveRecord(pack, record->id);
	}
}

void ExXt::Repath(const XtString& oldPath, const XtString& newPath)
{
	SorceryApp& app = Node()->Explorer()->Window().App();

	if (Xt::DomRecord* rec = FindRecordById(app._db, _xtId))
	{
		int oldLen	= (int)oldPath.Length();
		int nameLen	= (int)rec->name.Length();

		if (nameLen > oldLen)
			rec->name = newPath + XtString((const char*)rec->name + oldLen, nameLen - oldLen);

		SetDirty(rec, true);
	}
}

void ExXt::MoveToPack(Xt::DomPack* pack)
{
	SorceryApp& app = Node()->Explorer()->Window().App();

	if (Xt::DomRecord* record = FindRecordById(app._db, _xtId))
	{
		Xt::MoveRecord(record, pack);

		SetDirty(record, true);
	}
}

void ExXt::Open()
{
	SorceryApp& app = Node()->Explorer()->Window().App();

	if (Xt::DomRecord* rec = FindRecordById(app._db, _xtId))
		app._window.OpenRecord(rec);
}

void ExXt::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, ExXt* xt)
{
	if (event->code == Xt::kDomRemoved)	
	{		
		delete xt->Node()->DetachSelf();		
		return;	
	}	

	if (*monitor == event->from)
	{
		if (Xt::DomRecord* record = AsRecord(xt->_dom))
		{
			if (Xt::DomDb* db = DbOf(record))
			{
				if (LarvaType* type = TypeOf(db->schema, record))
				{
					xt->SetIcon(type);
				}
				else					
					xt->SetIcon(kEniFile, kEniFile);			
			}
			else
				xt->SetIcon(kEniFile, kEniFile);

			xt->SetOverlayIcon(record->dirty ? kEnoiModified : kEnoiNone);
		}
	}
}
#include "stdafx.h"

#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Core.h>

#include <EditorApp/Components/BaseExplorer.h>

using namespace Editor;

C_BaseExplorer::C_BaseExplorer()
{
	LoadIcon("./icons/Project.bmp", kExItemTypeRoot);
	LoadIcon("./icons/FolderHiddenClosed.bmp", kExItemTypePack);
	LoadIcon("./icons/File.bmp", kExItemTypeRecord);
	LoadIcon("./icons/PackFolderClosed.bmp", kExItemTypeFilterPack);
	LoadIcon("./icons/FolderClosed.bmp", kExItemTypeFilterRecord);

	GetFlags().HasIcons = true;

	SetAllowRename(true);
}

void C_BaseExplorer::OnContextMenu(Explorer::ExplorerData* item, const WinPoint& pt)
{
	enum ExMenuItem
	{
		kExMenuItemNewPack = 1,
		kExMenuItemNewFolder,
		kExMenuItemRename,
		kExMenuItemDelete,
		kExMenuItemOpen,
		kExMenuItemId,
	};

	HMENU hContextMenu = CreatePopupMenu();

	switch (item->GetType())
	{
		case (kExItemTypeRoot):
		{
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemNewPack, "New Pack");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemNewFolder, "New Folder");
			break;
		}
		case (kExItemTypeFilterPack):
		{
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemNewPack, "New Pack");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemNewFolder, "New Folder");
			AppendMenu(hContextMenu, MF_SEPARATOR, 0, "");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemRename, "Rename");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemDelete, "Delete");
			break;
		}
		case (kExItemTypePack):
		{
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemOpen, "Open");
			AppendMenu(hContextMenu, MF_SEPARATOR, 0, "");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemNewFolder, "New Folder");
			AppendMenu(hContextMenu, MF_SEPARATOR, 0, "");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemRename, "Rename");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemDelete, "Delete");
			break;
		}
		case (kExItemTypeFilterRecord):
		{
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemNewFolder, "New Folder");
			AppendMenu(hContextMenu, MF_SEPARATOR, 0, "");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemRename, "Rename");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemDelete, "Delete");
			break;
		}
		case (kExItemTypeRecord):
		{
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemOpen, "Open");
			AppendMenu(hContextMenu, MF_SEPARATOR, 0, "");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemRename, "Rename");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemDelete, "Delete");
			AppendMenu(hContextMenu, MF_SEPARATOR, 0, "");

			AppendMenu(hContextMenu, MF_STRING, kExMenuItemId, Format("Id = %X", item->GetDomRecord()->id).Text());
			break;
		}
	}

	POINT winPt;
	winPt.x = pt.x;
	winPt.y = pt.y;
	ClientToScreen(_hwnd, &winPt);

	switch (TrackPopupMenuEx(hContextMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD, winPt.x, winPt.y, _hwnd, 0))
	{
	case kExMenuItemNewPack:
		{
			CreateNewPack(item);
			break;
		}
	case kExMenuItemNewFolder:
		{
			CreateNewFolder(item);
			break;
		}
	case kExMenuItemRename:
		{
			StartRename(item);
			break;
		}
	case kExMenuItemDelete:
		{
			Delete(item);
			break;
		}
	case kExMenuItemOpen:
		{
			OnActivateItem(item);
			break;
		}
	case kExMenuItemId:
		{
			break;
		}
	}	

	DestroyMenu(hContextMenu);
}

void C_BaseExplorer::CreateNewPack(Explorer::ExplorerData* pData)
{
	XtString filterName;

	XtString packName;
	XtString filteredName;

	for (int i=0;; i++) {
		packName = Format("New Pack %i", i);

		filteredName = packName;
		GetFilterName(pData, filteredName);

		if (FindPack(filteredName) == 0) {
			break;
		}
	}

	Xt::DomPack* pPack = Xt::NewPack(GetDb(), filteredName);
}

Btl::String C_BaseExplorer::GetUniqueFolderName(Explorer::ExplorerData* pData, const Btl::String& base)
{
	int number = 1;
	Btl::String folderName = base;

	bool nameCollision = false;
	do
	{
		nameCollision = false;
		ExplorerData* child = (ExplorerData*)pData->GetFirstChild();
		while (child)
		{
			if ((child->GetType() == kExItemTypeFilterPack || child->GetType() == kExItemTypeFilterRecord) && child->GetFilter() == folderName.Chars())
			{
				nameCollision = true;

				folderName = Btl::String::Format("%s %d", base.Chars(), number);
				number++;
			}

			child = (ExplorerData*)child->GetNextSibling();
		}

	} while (nameCollision == true);

	return folderName;
}

void C_BaseExplorer::CreateNewFolder(Explorer::ExplorerData* pData)
{
	if (pData->GetType() == kExItemTypeRoot || pData->GetType() == kExItemTypeFilterPack)
	{
		Btl::String folderName = GetUniqueFolderName(pData, "New Folder");
		ExplorerData* newFolder = new ExplorerData(this, folderName.Chars(), 0);
		newFolder->SetLabel(folderName.Chars());
		pData->AddChild(newFolder);
	}
	else if (pData->GetType() == kExItemTypePack || pData->GetType() == kExItemTypeFilterRecord)
	{
		Btl::String folderName = GetUniqueFolderName(pData, "New Folder");
		ExplorerData* newFolder = new ExplorerData(this, folderName.Chars(), pData->GetDomPack());
		newFolder->SetLabel(folderName.Chars());
		pData->AddChild(newFolder);
	}
}

void C_BaseExplorer::CreateNewRecord(Explorer::ExplorerData* pData, Btl::String type)
{
	XtString filter;
	if (pData->GetType() == kExItemTypeFilterRecord)
		GetFilterName(pData, filter);

	Xt::DomPack* pPack = pData->GetDomPack();
	BIZ_ASSERT(pPack);

	XtId recordId = EditorLib::C_Core::Instance().GetXtManager().CreateRecord(pPack->id, type);

	Xt::DomRecord* pRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(recordId);
	BIZ_ASSERT(pRecord);

	Xt::SetName(pRecord, filter + pRecord->name);
}

void C_BaseExplorer::DuplicateItem(Explorer::ExplorerData* data)
{
	if (data->GetType() == kExItemTypeRecord)
	{
		Xt::DuplicateRecord(GetDb()->schema, data->GetDomRecord());
	}
}
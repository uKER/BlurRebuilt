#include "stdafx.h"

#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Core.h>

#include <EditorApp/Components/LevelExplorer.h>

using namespace Editor;

C_LevelExplorer::C_LevelExplorer()
{
}

void C_LevelExplorer::OnContextMenu(Explorer::ExplorerData* item, const WinPoint& pt)
{
	enum ExMenuItem
	{
		kExMenuItemNewPack = 1,
		kExMenuItemNewFolder,
		kExMenuItemNewLevelActorList,
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
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemNewLevelActorList, "New Level Actor List");
			AppendMenu(hContextMenu, MF_SEPARATOR, 0, "");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemNewFolder, "New Folder");
			AppendMenu(hContextMenu, MF_SEPARATOR, 0, "");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemRename, "Rename");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemDelete, "Delete");
			break;
		}
		case (kExItemTypeFilterRecord):
		{
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemNewLevelActorList, "New Level Actor List");
			AppendMenu(hContextMenu, MF_SEPARATOR, 0, "");
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
	case kExMenuItemNewLevelActorList:
		{
			CreateNewRecord(item, EditorLib::kXtLevelRecordType);
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

bool C_LevelExplorer::OnFilterItem(Explorer::ExplorerData* pData)
{
	if (pData->GetType() == kExItemTypeRecord)
	{
		if (Xt::TypeOf(GetDb()->schema, pData->GetDomRecord())->name == "LevelActorList")
			return true;
		else
			return false;
	}
	return true;
}


#include "Core/StdAfx.h"
#include "Application/SorceryApp.h"
#include "Explorer/Explorer.h"

//
// Lifetime
//

ExRoot::ExRoot(ExNode* node) :
	ExItem(kEitRoot, node)
{
	SetId(Node()->Explorer()->Window().App()._db->name);
	SetLabel(Node()->Explorer()->Window().App()._db->name);
	SetIcon(kEniProject, kEniProject);
}

ExRoot::~ExRoot()
{
}

bool ExRoot::OnRightClick()
{
	SorceryApp&	app		= Node()->Explorer()->Window().App();
	HMENU		menu	= CreatePopupMenu();

	AppendMenu(menu, MF_STRING, 1, "New pack");
	AppendMenu(menu, MF_STRING, 2, "New folder");
	AppendMenu(menu, MF_SEPARATOR, 0, 0);
	AppendMenu(menu, MF_STRING, 3, "Properties");

	WinPoint pt;
	GetCursorPos(&pt);

	int result = TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, Node()->Explorer()->Window(), 0);

	DestroyMenu(menu);

	switch(result)
	{
		case 1:
		{
			Xt::DomPack* pack = NewPack(app._db, kXtNullString);

			if (!pack)
				return true;

			for(int i = 1;; i++)
			{
				XtString packName;

				packName = Format("New Pack %i", i);

				bool failed = false;

				for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator it(&app._db->packs); it; ++it)
				{
					if (_stricmp(it->name, packName) == 0)
					{
						failed = true;
						break;
					}
				}

				if (!failed)
				{
					pack->name = packName;
					break;
				}
			}

			ExNode* node = new ExNode(Node()->Explorer());

			if (!node)
				return true;

			ExPack* ex = new ExPack(node, pack, pack->name);

			if (!ex)
			{
				delete node;
				return true;
			}

			Node()->Add(node);

			Node()->Explorer()->Root()->Populate(TVI_FIRST);

			TreeView_Expand(Node()->Explorer()->Tree(), Node()->TreeItem(), TVE_EXPAND);
			Node()->UpdateIconLabel();

			TreeView_EditLabel(node->Explorer()->Tree(), node->TreeItem());

			SetDirty(app._db, false);
		}
		break;

		case 2:
		{
			ExNode* node = new ExNode(Node()->Explorer());

			if (!node)
				return true;

			for(int i = 1;; i++)
			{
				XtString name = Format("New Folder %i", i);

				if (Node()->Get(MakeFolderId(name)))
					continue;

				ExPackFolder* ex = new ExPackFolder(node, name);

				if (!ex)
				{
					delete node;
					return true;
				}

				Node()->Add(node);

				Node()->Explorer()->Root()->Populate(TVI_FIRST);

				TreeView_Expand(Node()->Explorer()->Tree(), Node()->TreeItem(), TVE_EXPAND);
				Node()->UpdateIconLabel();

				TreeView_EditLabel(node->Explorer()->Tree(), node->TreeItem());

				SetDirty(app._db, false);

				break;
			}
		}
		break;

		case 3:
			app._window.OpenDb(app._db);
		break;
	}

	return true;
}

void ExRoot::GenerateFilters(XtUt::Set<XtString>* filters, const XtString& path)
{
	for(int i = 0; i < Node()->Length(); i++)
	{
		ExNode* node = Node()->Get(i);

		if (!node || !node->Item())
			continue;

		node->Item()->GenerateFilters(0, "");
	}
}
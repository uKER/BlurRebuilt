#include "Core/StdAfx.h"
#include "Application/SorceryApp.h"
#include "Explorer/Explorer.h"

extern bool g_bAudioMode;
extern void BuildBanksSubset(SorceryApp& app, const char* pchFilter);

/*

Rename(Xt::DomRecord* record, const char* name);
ChangeType(Xt::DomRecord* record, const char* type);

*/

XtString MakeFolderId(const XtString& name){	return Format("0%s", name.Text());}//
// Lifetime
//

ExFolder::ExFolder(ExNode* node, XtString name) :
	ExItem(kEitFolder, node),
	_name(name)
{
	SetId(MakeFolderId(name));
	SetLabel(name);
	SetIcon(kEniFolderOpen, kEniFolderClosed);
}

ExFolder::~ExFolder()
{
}

namespace
{
	bool SortTypes(LtStruct* lhs, LtStruct* rhs)
	{
		return _stricmp(lhs->label, rhs->label) < 0;
	}

	XtString GetCategory(LtStruct* type)
	{
		if (!type)
			return kXtNullString;

		if (*type->category)
			return type->category;

		if (type->base)
		{
			XtString result = GetCategory(type->base->type);

			if (*result)
				return result;
		}

		return kXtNullString;
	}
}

bool RecursiveSearchForType(ExNode* pNode, XtString& strType, SorceryApp* pApp)
{
	bool bFound = false;

	if (!pNode)
		return false;

	//if (pNode->Item()->Type() == kEitFolder)
	//{
	//	OutputDebugString(">>> ");
	//	OutputDebugString((const char*)pNode->Label());
	//	OutputDebugString("\n");
	//}

	for(int i = 0; i < pNode->Length(); i++)
	{
		ExNode* pCurNode = pNode->Get(i);
		ExItem* pItem = pCurNode->Item();

		if (!pCurNode || !pItem)
			continue;
		
		if (pCurNode->Item()->Type() == kEitRecord)
		{
			ExXt* record = (ExXt*)pItem;
			Xt::DomRecord* dom = FindRecordById(pApp->_db, record->Id());

			//OutputDebugString((const char*)dom->name);

			if (dom->root.type == strType)
			{
				//OutputDebugString("################ FOUND!");
				return true;
			}
			//OutputDebugString("\n");
		}

		if (pItem->Type() == kEitFolder)
		{	
			bFound = RecursiveSearchForType(pCurNode, strType, pApp);

			if (bFound)
				break;
		}
	}

	return bFound;
}

bool ExFolder::OnRightClick()
{
	SorceryApp& app = Node()->Explorer()->Window().App();
	LarvaSchema* schema = app._db->schema;

	ExNode* parent = Node();

	XtUt::Array<LtStruct*> types;
	XtUt::Array<LtStruct*> customTypes;

	for(XtUt::Dictionary<XtString, LarvaType>::Iterator it(&app._db->schema->types); it; ++it)
	{
		if (it->kind != kLtStruct)
			continue;

		LtStruct* type = (LtStruct*)(LarvaType*)it;

		if (!type->isRecord)
			continue;

		AppendEx(&types, (LtStruct*)type);
	}

	for(int i = 0; i < parent->Length(); i++)
	{
		ExNode* node = parent->Get(i);
		ExItem* item = node->Item();

		if (!item)
			continue;

		if (item->Type() != kEitRecord)
			continue;

		ExXt* record = (ExXt*)item;

		Xt::DomRecord* dom = FindRecordById(app._db, record->Id());

		if (!dom)
			continue;

		LarvaType* lt = TypeOf(app._db->schema, dom);

		if (!lt)
			continue;

		if (lt->kind != kLtStruct)
			continue;

		bool addIt = true;

		for(XtUt::Array<LtStruct*>::Iterator it(&customTypes); it; ++it)
		{
			if (lt == it)
			{
				addIt = false;
				break;
			}
		}

		if (addIt)
			AppendEx(&customTypes, (LtStruct*)lt);
	}

	XtUt::MergeSort(&types, SortTypes);
	XtUt::MergeSort(&customTypes, SortTypes);

	// Categories

	XtUt::Array<XtString> categories;

	bool emptyCat = false;

	for(XtUt::Array<LtStruct*>::Iterator itType(&types); itType; ++itType)
	{
		XtString category = GetCategory(*itType);

		if (!*category)
		{
			emptyCat = true;
			continue;
		}

		bool present = false;

		for(XtUt::Array<XtString>::Iterator itCat(&categories); itCat; ++itCat)
		{
			if (itCat == category)
			{
				present = true;
				break;
			}
		}

		if (!present)
			XtUt::AppendEx(&categories, category);
	}

	XtUt::MergeSort(&categories, XtUt::LessThan<XtString>());

	if (emptyCat)
		XtUt::AppendEx(&categories, kXtNullString);

	// Menu

	HMENU typeMenu = CreatePopupMenu();

	AppendMenu(typeMenu, MF_STRING, 100, _T("Folder"));

	AppendMenu(typeMenu, MF_SEPARATOR, 0, 0);

	for(XtUt::Array<XtString>::Iterator itCat(&categories); itCat; ++itCat)
	{
		HMENU catMenu = typeMenu;

		if (categories.count > 1)
		{
			catMenu = CreatePopupMenu();

			XtString label = CleanUpLabel(*itCat);

			if (!*label)
				label = "(misc)";

			AppendMenu(typeMenu, MF_STRING | MF_POPUP, (UINT_PTR)catMenu, label);
		}

		int count = 0;

		for(XtUt::Array<LtStruct*>::Iterator it(&types); it; ++it)
		{
			if (GetCategory(*it) ==  itCat)
			{
				int flags = MF_STRING;

				if (count > 40)
				{
					flags |= MF_MENUBARBREAK;
					count = 0;
				}

				AppendMenu(catMenu, flags, 1000 + it.index, (*it)->label);

				count++;
			}
		}
	}

	HMENU menu = CreatePopupMenu();

	AppendMenu(menu, MF_STRING | MF_POPUP, (UINT_PTR)typeMenu, _T("New"));

	for(XtUt::Array<LtStruct*>::Iterator it(&customTypes); it; ++it)
		AppendMenu(menu, MF_STRING, 2000 + it.index, XtString("New ") + (*it)->label);

	AppendMenu(menu, MF_SEPARATOR, 0, 0);
	AppendMenu(menu, MF_STRING, 2, _T("Rename"));
	AppendMenu(menu, MF_STRING, 3, _T("Delete"));
	AppendMenu(menu, MF_STRING, 4, _T("Duplicate"));

	if (g_bAudioMode)
	{
		// only bring up the "Build Banks" option if there are wavebanks in the folders
		if (RecursiveSearchForType( Node(), XtString("WaveBank"), &app))
		{
			AppendMenu(menu, MF_SEPARATOR, 0, 0);
			AppendMenu(menu, MF_STRING, 5, _T("Build these Wave Banks"));
		}
	}

	WinPoint pt;
	GetCursorPos(&pt);

	int result = TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, Node()->Explorer()->Window(), 0);

	DestroyMenu(menu);

	ExNode* packNode = Node();

	XtString nodePath;

	while(packNode && packNode->Item() && (packNode->Item()->Type() != kEitPack))
	{
		if (packNode->Item()->Type() == kEitFolder)
		{
			ExFolder* item = (ExFolder*)packNode->Item();

			nodePath = item->_name + "." + nodePath;
		}

		packNode = packNode->Container();
	}

	ExPack* exPack = 0;

	if (packNode && packNode->Item())
		exPack = (ExPack*)packNode->Item();

	Xt::DomPack* pack = app._db->packs[exPack->_packId];

	if (pack && (result >= 1000) && (result < 3000))
	{
		LtStruct* type = 0;

		if ((result >= 1000) && (result < 1000 + (int)types.count))
			type = types[result - 1000];
		else if ((result >= 2000) && (result < 2000 + (int)customTypes.count))
			type = customTypes[result - 2000];

		if (type)
		{
			Xt::DomRecord* newRecord = Xt::NewRecord(pack, Format("%sNew Record", (const char*)nodePath), type->name);

			SetDefaults(schema, type, &newRecord->root);

			ExNode* node = ExAcquireRecord(Node()->Explorer()->Root(), newRecord);

			Node()->Explorer()->Root()->Populate(TVI_FIRST);

			TreeView_Expand(Node()->Explorer()->Tree(), Node()->TreeItem(), TVE_EXPAND);
			Node()->UpdateIconLabel();

			Xt::SetDirty(newRecord, true);

			app._window.OpenRecord(newRecord);

			TreeView_EditLabel(node->Explorer()->Tree(), node->TreeItem());
		}
	}
	else
	{
		switch(result)
		{
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
				BuildBanksSubset(app, (const char*)nodePath);
			break;

			case 100:
			{
				for(int i = 1;; i++)
				{
					XtString name = Format("New Folder %i", i);

					if (Node()->Get(MakeFolderId(name)))
						continue;

					ExNode* node = new ExNode(Node()->Explorer());

					if (!node)
						break;

					ExFolder* ex = new ExFolder(node, name);

					if (!ex)
					{
						delete node;
						break;
					}

					if (!Node()->Add(node))
						break;

					Node()->Explorer()->Root()->Populate(TVI_FIRST);

					TreeView_Expand(Node()->Explorer()->Tree(), Node()->TreeItem(), TVE_EXPAND);
					Node()->UpdateIconLabel();

					TreeView_EditLabel(node->Explorer()->Tree(), node->TreeItem());

					break;
				}
			}
			break;
		}
	}

	return true;
}

void ExFolder::Delete()
{
	SorceryApp& app = Node()->Explorer()->Window().App();

	while(Node()->Length() > 0)
	{
		ExNode* node = Node()->Get(0);

		if (!node)
			break;

		node->Item()->Delete();
	}

	ExNode* parent = Node()->Container();

	delete Node()->DetachSelf();

	if (parent)
		parent->UpdateIconLabel();

	SetDirty(app._db, false);
}

bool ExFolder::OnBeginEditLabel()
{
	return true;
}

bool ExFolder::OnEndEditLabel(const char* newText)
{
	if (newText)
		Rename(newText);

	return false;
}

void ExFolder::OnDeletePressed()
{
	HWND tree = Node()->Explorer()->Tree();

	XtString path;

	for(ExNode* packNode = Node()->Container(); packNode && packNode->Item() && (packNode->Item()->Type() != kEitPack); packNode = packNode->Container())
	{
		if (packNode->Item()->Type() == kEitFolder)
		{
			ExFolder* item = (ExFolder*)packNode->Item();

			path = item->_name + "." + path;
		}
	}

	if (MessageBox(Node()->Explorer()->Window(), "Are you sure you want to delete the folder:\r\n\r\n" + path + _name, _T("Bob"), MB_ICONQUESTION | MB_OKCANCEL) == IDOK)
		Delete();

	SetFocus(tree);
}

void ExFolder::OnDuplicateRecord(ExItem* parent, bool renameRecord)
{
	if (!parent || !parent->Node())
		return;

	ExNode* newNode = new ExNode(parent->Node()->Explorer());

	if (!newNode)
		return;

	XtString newName = Node()->Label();

	if (renameRecord)
		newName += " (2)";

	ExFolder* ex = new ExFolder(newNode, newName);

	if (!ex)
	{
		delete newNode;
		return;
	}

	if (!parent->Node()->Add(newNode))
		return;

	for(int i = 0; i < Node()->Length(); i++)
	{
		ExNode* node = Node()->Get(i);

		if (!node || !node->Item())
			continue;

		node->Item()->OnDuplicateRecord(newNode->Item(), false);
	}

	if (renameRecord)
	{
		Node()->Explorer()->Root()->Populate(TVI_FIRST);

		//TreeView_Expand(Node()->Explorer()->Tree(), Node()->TreeItem(), TVE_EXPAND);
		Node()->UpdateIconLabel();

		TreeView_EditLabel(newNode->Explorer()->Tree(), newNode->TreeItem());
	}

	SetDirty(Node()->Explorer()->Window().App()._db, false);
}

void ExFolder::Rename(const char* newNameArg)
{
	XtString newName = XtsStripBoth(newNameArg, ' ');

	if (!XtsIsName(newName))
		return;

	if (ExNode* node = Node()->Container()->Get(MakeFolderId(newName)))
	{
		if (node != Node())
			return;
	}

	XtString nodePath;

	for(ExNode* packNode = Node()->Container(); packNode && packNode->Item() && (packNode->Item()->Type() != kEitPack); packNode = packNode->Container())
	{
		if (packNode->Item()->Type() == kEitFolder)
		{
			ExFolder* item = (ExFolder*)packNode->Item();

			nodePath = item->_name + "." + nodePath;
		}
	}

	XtString oldNodePath = nodePath + _name + ".";
	XtString newNodePath = nodePath + newName + ".";

	for(int i = 0; i < Node()->Length(); i++)
	{
		ExNode* node = Node()->Get(i);

		if (!node || !node->Item())
			continue;

		node->Item()->Repath(oldNodePath, newNodePath);
	}

	_name = newName;

	SetId(MakeFolderId(_name));
	SetLabel(_name);

	Node()->Explorer()->Root()->Populate(TVI_FIRST);

	SetDirty(Node()->Explorer()->Window().App()._db, false);

	SendMessage(Node()->Explorer()->Tree(), TVM_SELECTITEM, TVGN_CARET, (LPARAM)Node()->TreeItem());
}

void ExFolder::Repath(const XtString& oldPath, const XtString& newPath)
{
	for(int i = 0; i < Node()->Length(); i++)
	{
		ExNode* node = Node()->Get(i);

		if (!node || !node->Item())
			continue;

		node->Item()->Repath(oldPath + _name + ".", newPath + _name + ".");
	}	
}

void ExFolder::MoveToPack(Xt::DomPack* pack)
{
	for(int i = 0; i < Node()->Length(); i++)
	{
		ExNode* node = Node()->Get(i);

		if (!node || !node->Item())
			continue;

		node->Item()->MoveToPack(pack);
	}	
}

void ExFolder::GenerateFilters(XtUt::Set<XtString>* filters, const XtString& path)
{
	if (!filters)
		return;

	XtString newPath = path + _name;

	if (Node()->Length() == 0)
		Insert(filters, newPath);
	else
	{
		for(int i = 0; i < Node()->Length(); i++)
		{
			ExNode* node = Node()->Get(i);

			if (!node || !node->Item())
				continue;

			node->Item()->GenerateFilters(filters, newPath + ".");
		}
	}
}

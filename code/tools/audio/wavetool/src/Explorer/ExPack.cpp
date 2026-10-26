 #include "Core/StdAfx.h"
#include "Application/SorceryApp.h"
#include "Application/SorceryExport.h"
#include "Explorer/Explorer.h"

XtString MakePackId(Xt::DomPack* pack)
{
	return Format("1%s-%08X", (const char*)pack->name, pack->id);
}

//
// Lifetime
//

ExPack::ExPack(ExNode* node, Xt::DomPack* pack, const XtString& label) :
	ExItem(kEitPack, node),
	_packId(pack->id)
{
	SetId(MakePackId(pack));
	SetLabel(label);
	SetIcon(kEniFolderOpenHidden, kEniFolderClosedHidden);
}

ExPack::~ExPack()
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

bool ExPack::OnRightClick()
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
	AppendMenu(menu, MF_SEPARATOR, 0, 0);
	AppendMenu(menu, MF_STRING, 5, _T("Build " + Node()->Label()));
	AppendMenu(menu, MF_STRING, 6, _T("Build " + Node()->Label()) + " (Data Only)");
	AppendMenu(menu, MF_STRING, 7, _T("Build " + Node()->Label()) + " (No Deploy)");
	AppendMenu(menu, MF_SEPARATOR, 0, 0);
	AppendMenu(menu, MF_STRING, 4, _T("Properties"));

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

			if (newRecord)
			{
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
			{
				if (Xt::DomPack* pack = Xt::FindPackById(app._db, _packId))
					app._window.OpenPack(pack);
			}	
			break;

			case 5:
			{
				if (Xt::DomPack* pack = Xt::FindPackById(app._db, _packId))
					SorceryExport(app, false, false, pack);
			}
			break;

			case 6:
			{
				if (Xt::DomPack* pack = Xt::FindPackById(app._db, _packId))
					SorceryExport(app, true, false, pack);
			}
			break;

			case 7:
			{
				if (Xt::DomPack* pack = Xt::FindPackById(app._db, _packId))
					SorceryExport(app, false, true, pack);
			}
			break;

			case 100:
			{
				for(int i = 1;; i++)
				{
					XtString name = Format("New Folder %i", i);

					if (Node()->Get(MakePackFolderId(name)))
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

					SetDirty(Node()->Explorer()->Window().App()._db, false);

					break;
				}
			}
			break;
		}
	}

	return true;
}

bool ExPack::OnBeginEditLabel()
{
	return true;
}

bool ExPack::OnEndEditLabel(const char* newTextArg)
{
	if (!newTextArg)
		return false;

	XtString newText = XtsStripBoth(newTextArg, ' ');

	if (!XtsIsName(newText))
		return false;

	SorceryApp&		app		= Node()->Explorer()->Window().App();
	Xt::DomPack*	pack	= app._db->packs[_packId];

	if (!pack)
		return false;

	const char* start	= pack->name;
	const char* dot		= start;

	for(const char* ptr = dot; *ptr != '\0'; ptr++)
		if (*ptr == '.')
			dot = ptr + 1;

	XtString newName = XtString(start, (int)(dot - start)) + newText;

	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator it(&app._db->packs); it; ++it)
	{
		if ((_stricmp(it->name, newName) == 0) && (pack != it))
			return false;
	}

	pack->name = newName;

	SetId(MakePackId(pack));
	SetLabel(newText);

	SetDirty(pack, true);

	Node()->Explorer()->Root()->Populate(TVI_FIRST);

	SendMessage(Node()->Explorer()->Tree(), TVM_SELECTITEM, TVGN_CARET, (LPARAM)Node()->TreeItem());

	return false;
}

void ExPack::OnDeletePressed()
{
	SorceryApp&		app		= Node()->Explorer()->Window().App();
	Xt::DomPack*	pack	= app._db->packs[_packId];
	HWND			tree	= Node()->Explorer()->Tree();

	XtString name = "(unknown pack)";

	if (pack)
		name = pack->name;

	if (MessageBox(Node()->Explorer()->Window(), "Are you sure you want to delete the pack:\r\n\r\n" + name, "Sorcery", MB_ICONQUESTION | MB_OKCANCEL) == IDOK)
		Delete();

	SetFocus(tree);
}

void ExPack::Delete()
{
	SorceryApp& app = Node()->Explorer()->Window().App();

	while(Node()->Length() > 0)
	{
		ExNode* node = Node()->Get(0);

		if (!node)
			break;

		node->Item()->Delete();
	}

	RemovePack(app._db, _packId);

	SetDirty(Node()->Explorer()->Window().App()._db, false);

	ExNode* parent = Node()->Container();

	delete Node()->DetachSelf();

	if (parent)
		parent->UpdateIconLabel();
}

void ExPack::GenerateFilters(XtUt::Set<XtString>*, const XtString& path)
{
	SorceryApp&		app		= Node()->Explorer()->Window().App();
	Xt::DomPack*	pack	= app._db->packs[_packId];

	if (!pack)
		return;

	XtUt::Set<XtString> newFilters;

	for(int i = 0; i < Node()->Length(); i++)
	{
		ExNode* node = Node()->Get(i);

		if (!node || !node->Item())
			continue;

		node->Item()->GenerateFilters(&newFilters, "");
	}

	if (Xt::DomArray* filters = AcquireArray(&pack->properties, "filters"))
	{
		XtUt::Set<XtString> oldFilters;

		for(Xt::DomArray::IndexIterator itFilter(filters); itFilter; ++itFilter)
			XtUt::Insert(&oldFilters, AsText(itFilter));

		XtUt::Array<XtId> toRemove;

		for(Xt::DomArray::IndexIterator itFilter(filters); itFilter; ++itFilter)
		{
			if (!XtUt::Exists(&newFilters, AsText(itFilter)))
				XtUt::AppendEx(&toRemove, itFilter.Key());
		}

		for(XtUt::Array<XtId>::Iterator it(&toRemove); it; ++it)
			RemoveById(filters, it);

		for(XtUt::Set<XtString>::Iterator it(&newFilters); it; ++it)
		{
			if (!XtUt::Exists(&oldFilters, *it))
			{
				if (Xt::DomField* field = Xt::Append(filters, new Xt::DomField))
					field->value = *it;
			}
		}
	}
}

void ExPack::RepathPack(const XtString& oldPath, const XtString& newPath)
{
	SorceryApp& app = Node()->Explorer()->Window().App();

	if (Xt::DomPack* pack = FindPackById(app._db, _packId))
	{
		int oldLen	= (int)oldPath.Length();
		int nameLen	= (int)pack->name.Length();

		if (nameLen > oldLen)
			pack->name = newPath + XtString((const char*)pack->name + oldLen, nameLen - oldLen);

		SetDirty(pack, true);
	}
}
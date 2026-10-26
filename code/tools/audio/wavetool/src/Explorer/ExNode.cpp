#include "Core/StdAfx.h"
#include "Explorer/Explorer.h"

//
// Lifetime
//

ExNode::ExNode(ExplorerView* explorer) :
	_item(0),
	_container(0),
	_explorer(explorer),
	_ti(0),
	_isOpen(false),
	_iconOpen(kEniFile),
	_iconClosed(kEniFile),
	_iconOverlay(kEnoiNone)
{
}

ExNode::~ExNode()
{
	Clear();

	if (_ti != 0)
		TreeView_DeleteItem(_explorer->_tree, _ti);

	delete _item;
}

//
// Build tree
//

bool ExNode::Populate(HTREEITEM insertAfter)
{
	if (_ti == 0)
	{
		DWORD state = _container ? 0 : TVIS_EXPANDED;

		TVINSERTSTRUCT is = { 0 };

		is.hParent					= _container ? _container->_ti : TVI_ROOT;
		is.hInsertAfter				= insertAfter;
		is.itemex.mask				= TVIF_TEXT | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE;
		is.itemex.stateMask			= state | TVIS_OVERLAYMASK;
		is.itemex.state				= state | INDEXTOOVERLAYMASK(_iconOverlay);
		is.itemex.pszText			= (LPSTR)(const char*)_label;
		is.itemex.lParam			= (LPARAM)this;
		is.itemex.iImage			= _iconClosed;
		is.itemex.iSelectedImage	= _iconClosed;

		if ((_ti = TreeView_InsertItem(_explorer->_tree, &is)) == 0)
			return false;
	}

	HTREEITEM childInsertAfter = TVI_FIRST;

	for(int i = 0; i < _items.Length(); i++)
	{
		if (!_items[i].Populate(childInsertAfter))
			return false;

		childInsertAfter = _items[i]._ti;
	}

	return true;
}

void ExNode::UpdateIconLabel()
{
	if (_ti == 0)
		return;

	TVITEMEX tv = { 0 };

	tv.mask				= TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
	tv.hItem			= _ti;
	tv.pszText			= (LPSTR)(const char*)_label;
	tv.iImage			= (_isOpen && (_items.Length() > 0)) ? _iconOpen : _iconClosed;
	tv.iSelectedImage	= tv.iImage;
	tv.stateMask		= TVIS_OVERLAYMASK;
	tv.state			= INDEXTOOVERLAYMASK(_iconOverlay);

	TreeView_SetItem(_explorer->_tree, &tv);
}

//
// Container management
//

ExNode* ExNode::Add(ExNode* item)
{
	if (!item)
		return 0;

	if (item->_container && !item->_container->Detach(item->_container->Find(item->_id)))
	{
		assert(0);
		return 0;
	}

	int low		= 0;
	int high	= _items.Length();

	while(low < high)
	{
		int mid		= (low + high) / 2;
		int result	= _stricmp(item->_id, _items[mid]._id);

		if (result < 0)
			high = mid;
		else if (result > 0)
			low = mid + 1;
		else if (result == 0)
		{
			if (!_items.Replace(mid, item))
				return 0;

			item->_container = this;

			return item;
		}
	}

	if (!_items.Insert(low, item))
		return 0;

	item->_container = this;

	return item;
}

int ExNode::Find(const char* id)
{
	int low		= 0;
	int high	= _items.Length();

	while(low < high)
	{
		int mid		= (low + high) / 2;
		int result	= _stricmp(id, _items[mid]._id);

		if (result < 0)
			high = mid;
		else if (result > 0)
			low = mid + 1;
		else if (result == 0)
			return mid;
	}

	return -1;
}

ExNode* ExNode::Detach(int index)
{
	ExNode* item = _items.Detach(index);

	if (item)
	{
		assert(item->_container == this);

		item->_container = 0;

		if (item->_ti != 0)
		{
			TreeView_DeleteItem(_explorer->_tree, item->_ti);
			item->_ti = 0;

			item->ClearChildTreeItems();
		}
	}

	return item;
}

void ExNode::Remove(int index)
{
	_items.Remove(index);
}

void ExNode::Clear()
{
	_items.Clear();
}

ExNode* ExNode::DetachSelf()
{
	if (!_container)
		return this;

	return _container->Detach(_container->Find(_id));
}

bool ExNode::IsParentOf(ExNode* item)
{
	for(; item; item = item->_container)
	{
		if (item == this)
			return true;
	}

	return false;
}

void ExNode::ClearChildTreeItems()
{
	for(int i = 0; i < _items.Length(); i++)
	{
		_items[i]._ti = 0;
		_items[i].ClearChildTreeItems();
	}
}
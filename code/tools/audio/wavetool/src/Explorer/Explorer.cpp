#include "Core/StdAfx.h"
#include "Application/SorceryApp.h"
#include "Explorer/Explorer.h"
#include "Resource/Resource.h"
#include "XtWin/SendEmail.h"

extern XtString MakePackId(Xt::DomPack* pack);
extern XtString MakeFolderId(const XtString& name);
extern XtString MakeRecordId(Xt::DomRecord* record);
extern XtString MakePackFolderId(const XtString& name);

void SplitXtPath(const XtString& path, XtString& folder, XtString& file)
{
	int index = LastIndexOf(path, ".");

	if (index < 0)
	{
		folder.Clear();
		file = path;
	}
	else
	{
		folder	= Substring(path, 0, index);
		file	= Substring(path, index + 1);
	}
}

ExNode* ExAcquirePackFolder(ExNode* parent, const char* path)
{
	if (!parent || !path)
		return 0;

	switch(parent->ItemType())
	{
		case kEitRoot:
		case kEitPackFolder:
		break;

		default:
		return 0;
	}

	const char* start = path;

	while((*path != '.') && (*path != '\0'))
		path++;

	if (start == path)
		return parent;

	XtString name(start, path - start);

	ExNode* node = parent->Get(MakePackFolderId(name));

	if (!node)
	{
		if ((node = new ExNode(parent->Explorer())) == 0)
			return 0;

		ExPackFolder* ex = new ExPackFolder(node, name);

		if (!ex)
			return 0;

		if (!parent->Add(node))
			return 0;
	}

	if (*path != '\0')
		return ExAcquirePackFolder(node, path + 1);

	return node;
}

ExNode* ExAcquirePack(ExNode* parent, Xt::DomPack* pack)
{
	if (!parent || !pack)
		return 0;

	XtString folder;
	XtString file;

	SplitXtPath(pack->name, folder, file);

	if ((parent = ExAcquirePackFolder(parent, folder)) == 0)
		return 0;

	ExNode* node = parent->Get(MakePackId(pack));

	if (!node)
	{
		if ((node = new ExNode(parent->Explorer())) == 0)
			return 0;

		ExPack* ex = new ExPack(node, pack, file);

		if (!ex)
			return 0;

		if (!parent->Add(node))
			return 0;
	}

	return node;
}

ExNode* ExAcquireFolder(ExNode* parent, const char* path)
{
	if (!parent)
		return 0;

	switch(parent->ItemType())
	{
		case kEitPack:
		case kEitFolder:
		break;

		default:
		return 0;
	}

	const char* start = path;

	while((*path != '.') && (*path != '\0'))
		path++;

	if (start == path)
		return parent;

	XtString name(start, path - start);

	ExNode* node = parent->Get(MakeFolderId(name));

	if (!node)
	{
		if ((node = new ExNode(parent->Explorer())) == 0)
			return 0;

		ExFolder* ex = new ExFolder(node, name);

		if (!ex)
			return 0;

		if (!parent->Add(node))
			return 0;
	}

	if (*path != '\0')
		return ExAcquireFolder(node, path + 1);

	return node;
}

ExNode* ExAcquireRecord(ExNode* parent, Xt::DomRecord* record)
{
	if (!parent || !record)
		return 0;

	Xt::DomDb*		db		= parent->Explorer()->Window().App()._db;
	Xt::DomPack*	pack	= PackOf(record);

	if (!pack)
		return 0;

	if ((parent = ExAcquirePack(parent, pack)) == 0)
		return 0;

	XtString folder;
	XtString file;

	SplitXtPath(record->name, folder, file);

	if ((parent = ExAcquireFolder(parent, folder)) == 0)
		return 0;

	ExNode* node = parent->Get(MakeRecordId(record));

	if (!node)
	{
		if ((node = new ExNode(parent->Explorer())) == 0)
			return 0;

		XtString label;

		if (LtStruct* type = TypeOf(db->schema, record))
			label = Format("%s.%s", file.Text(), type->label.Text());
		else
			label = Format("%s.%s", file.Text(), SplitIntercaps(record->root.type).Text());

		ExXt* ex = new ExXt(node, record, label);

		if (!ex)
			return 0;

		if (!parent->Add(node))
			return 0;
	}

	return node;
}

//
// ExplorerView
//

ExplorerView::ExplorerView(SorceryWindow& window) :
	_window(window),
	_tree(0),
	_images(0),
	_root(0),
	_dragging(false),
	_dragNode(0)
{
}

ExplorerView::~ExplorerView()
{
}

LRESULT ExplorerView::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			if (ChildWindow::WndProc(msg, wParam, lParam) != 0)
				return -1;

			if (!InitialiseTree())
				return -1;

			SetWindowText(*this, "Project Explorer");
		}
		return 0;

		case WM_DESTROY:
			delete _root;
			_root = 0;

			ImageList_Destroy(_images);
			_images = 0;
		break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(_hwnd, &ps);
			ExtTextOut(ps.hdc, 0, 0, ETO_OPAQUE, &ps.rcPaint, 0, 0, 0);
			EndPaint(_hwnd, &ps);
		}
		return 0;

		case WM_SIZE:
		{
			WinRect rcWin;
			GetClientRect(*this, &rcWin);

			SetWindowPos(_tree, 0, 0, 0, rcWin.Width(), rcWin.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
		}
		return 0;

		case WM_NOTIFY:
		return OnNotify((NMHDR*)lParam);

		case WM_MOUSEMOVE:
			Drag_Update(LOWORD(lParam), HIWORD(lParam));
		return 0;

		case WM_LBUTTONUP:
			Drag_End(true, LOWORD(lParam), HIWORD(lParam));
		return 0;

		case WM_CAPTURECHANGED:
			Drag_End(false, 0, 0);
		return 0;
	}

	return ChildWindow::WndProc(msg, wParam, lParam);
}

int ExplorerView::OnNotify(NMHDR* hdr)
{
	if (hdr->hwndFrom == _tree)
	{
		switch(hdr->code)
		{
			case TVN_BEGINDRAG:
				Drag_Begin((NMTREEVIEW*)hdr);
			return 1;

			case TVN_BEGINRDRAG:
			return 1;

			case TVN_BEGINLABELEDIT:
			{
				NMTVDISPINFO* di = (NMTVDISPINFO*)hdr;

				ExNode* node = (ExNode*)di->item.lParam;

				if (node && node->_item)
					return node->_item->OnBeginEditLabel() ? FALSE : TRUE;
			}
			return TRUE;

			case TVN_ENDLABELEDIT:
			{
				NMTVDISPINFO* di = (NMTVDISPINFO*)hdr;

				ExNode* node = (ExNode*)di->item.lParam;

				if (node && node->_item)
					return node->_item->OnEndEditLabel(di->item.pszText) ? TRUE : FALSE;
			}
			return FALSE;

			case NM_RCLICK:
			{
				TVHITTESTINFO ht = { 0 };

				DWORD pos = GetMessagePos();

				ht.pt.x = GET_X_LPARAM(pos);
				ht.pt.y = GET_Y_LPARAM(pos);

				ScreenToClient(_tree, &ht.pt);

				TVITEMEX ti = { 0 };

				ti.mask = TVIF_HANDLE | TVIF_PARAM;

				if ((ti.hItem = TreeView_HitTest(_tree, &ht)) == 0)
					return 0;

				TreeView_SelectItem(_tree, ti.hItem);

				if (TreeView_GetItem(_tree, &ti) == FALSE)
					return 0;

				ExNode* node = (ExNode*)ti.lParam;

				if (!node || !node->_item)
					return 0;

				if (node->_item->OnRightClick())
					return 1;
			}
			return 1;

			case NM_DBLCLK:
			{
				TVHITTESTINFO ht = { 0 };

				DWORD pos = GetMessagePos();

				ht.pt.x = GET_X_LPARAM(pos);
				ht.pt.y = GET_Y_LPARAM(pos);

				ScreenToClient(_tree, &ht.pt);

				TVITEMEX ti = { 0 };

				ti.mask = TVIF_HANDLE | TVIF_PARAM;

				if ((ti.hItem = TreeView_HitTest(_tree, &ht)) == 0)
					return 0;

				if (TreeView_GetItem(_tree, &ti) == FALSE)
					return 0;

				ExNode* node = (ExNode*)ti.lParam;

				if (!node || !node->_item)
					return 0;

				if (node->_item->OnDoubleClick())
					return 1;
			}
			return 0;

			case NM_RDBLCLK:
			return 1;

			case NM_RETURN:
			{
				TVITEMEX ti = { 0 };

				ti.mask		= TVIF_HANDLE | TVIF_PARAM;
				ti.hItem	= TreeView_GetSelection(_tree);

				if (TreeView_GetItem(_tree, &ti) == FALSE)
					return 0;

				ExNode* node = (ExNode*)ti.lParam;

				if (!node || !node->_item)
					return 0;

				if (node->_item->OnEnter())
					return 1;

				if (TreeView_GetChild(_tree, node->_ti) != NULL)
				{
					if ((TreeView_GetItemState(_tree, node->_ti, TVIS_EXPANDED) & TVIS_EXPANDED) != 0)
						SendMessage(_tree, WM_KEYDOWN, VK_LEFT, 0);
					else
						SendMessage(_tree, WM_KEYDOWN, VK_RIGHT, 0);

					return 1;
				}
			}
			return 0;

			case TVN_KEYDOWN:
			{
				NMTVKEYDOWN* tv = (NMTVKEYDOWN*)hdr;

				if (tv->wVKey == VK_APPS)
				{
					TVITEMEX ti = { 0 };

					ti.mask		= TVIF_HANDLE | TVIF_PARAM;
					ti.hItem	= TreeView_GetSelection(_tree);

					if (TreeView_GetItem(_tree, &ti) == FALSE)
						return 1;

					ExNode* node = (ExNode*)ti.lParam;

					if (!node || !node->_item)
						return 1;

					if (node->_item->OnRightClick())
						return 1;

					return 1;
				}
				else if (tv->wVKey == VK_F2)
				{
					TVITEMEX ti = { 0 };

					ti.mask		= TVIF_HANDLE | TVIF_PARAM;
					ti.hItem	= TreeView_GetSelection(_tree);

					if (TreeView_GetItem(_tree, &ti) == FALSE)
						return 1;

					ExNode* node = (ExNode*)ti.lParam;

					if (!node || !node->_item)
						return 1;

					TreeView_EditLabel(_tree, node->_ti);

					return 1;
				}
				else if (tv->wVKey == VK_DELETE)
				{
					TVITEMEX ti = { 0 };

					ti.mask		= TVIF_HANDLE | TVIF_PARAM;
					ti.hItem	= TreeView_GetSelection(_tree);

					if (TreeView_GetItem(_tree, &ti) == FALSE)
						return 1;

					ExNode* node = (ExNode*)ti.lParam;

					if (!node || !node->_item)
						return 1;

					node->_item->OnDeletePressed();

					return 1;
				}
			}
			return 0;

			case TVN_ITEMEXPANDING:
			{
				NMTREEVIEW*	tv		= (NMTREEVIEW*)hdr;
				ExNode*		node	= (ExNode*)tv->itemNew.lParam;

				if (node && node->_item && node->_item->Type() == kEitRoot)
					return TRUE;
			}
			return 0;

			case TVN_ITEMEXPANDED:
			{
				NMTREEVIEW*	tv		= (NMTREEVIEW*)hdr;
				ExNode*		node	= (ExNode*)tv->itemNew.lParam;

				if (node)
				{
					node->_isOpen = (tv->action == TVE_EXPAND);

					TVITEMEX ie = { 0 };

					ie.mask				= TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
					ie.hItem			= tv->itemNew.hItem;
					ie.iImage			= node->_isOpen ? node->_iconOpen : node->_iconClosed;
					ie.iSelectedImage	= ie.iImage;

					TreeView_SetItem(_tree, &ie);
				}
			}
			return 0;

			case TVN_SELCHANGING:
			{
			}
			return 0;

			case TVN_SELCHANGED:
			{
				NMTREEVIEW*	tv		= (NMTREEVIEW*)hdr;
				ExNode*		node	= (ExNode*)tv->itemNew.lParam;

				if (node && node->Item())
					node->Item()->Selected();
			}
			return 0;

			case NM_SETFOCUS:
				InvalidateWindow(_tree);
			return 0;

			case NM_KILLFOCUS:
				InvalidateWindow(_tree);
			return 0;
		}
	}

	return 0;
}

bool ExplorerView::InitialiseTree()
{
	if ((_tree = CreateWindow("XTWINTREEVIEW", "",
			WS_VISIBLE | WS_CHILD | TVS_HASLINES | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | TVS_EDITLABELS,
		0, 0, 0, 0, *this, 0, 0, 0)) == 0)
	{
		return false;
	}

	if ((_images = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 8, 8)) == 0)
	{
		return false;
	}

	ImageList_AddMasked(_images, (HBITMAP)LoadImage(
			GetModuleHandle(0), MAKEINTRESOURCE(IDB_FILE), IMAGE_BITMAP, 16, 16, LR_SHARED
		), RGB(255, 0, 255));

	ImageList_AddMasked(_images, (HBITMAP)LoadImage(
			GetModuleHandle(0), MAKEINTRESOURCE(IDB_HIDDEN_FILE), IMAGE_BITMAP, 16, 16, LR_SHARED
		), RGB(255, 0, 255));

	ImageList_AddMasked(_images, (HBITMAP)LoadImage(
			GetModuleHandle(0), MAKEINTRESOURCE(IDB_MODIFIED_FILE), IMAGE_BITMAP, 16, 16, LR_SHARED
		), RGB(255, 0, 255));

	ImageList_AddMasked(_images, (HBITMAP)LoadImage(
			GetModuleHandle(0), MAKEINTRESOURCE(IDB_CHECKED_OUT_FILE), IMAGE_BITMAP, 16, 16, LR_SHARED
		), RGB(255, 0, 255));

	ImageList_AddMasked(_images, (HBITMAP)LoadImage(
			GetModuleHandle(0), MAKEINTRESOURCE(IDB_FOLDER_OPEN), IMAGE_BITMAP, 16, 16, LR_SHARED
		), RGB(255, 0, 255));

	ImageList_AddMasked(_images, (HBITMAP)LoadImage(
			GetModuleHandle(0), MAKEINTRESOURCE(IDB_FOLDER_CLOSED), IMAGE_BITMAP, 16, 16, LR_SHARED
		), RGB(255, 0, 255));

	ImageList_AddMasked(_images, (HBITMAP)LoadImage(
			GetModuleHandle(0), MAKEINTRESOURCE(IDB_FOLDER_HIDDEN_OPEN), IMAGE_BITMAP, 16, 16, LR_SHARED
		), RGB(255, 0, 255));

	ImageList_AddMasked(_images, (HBITMAP)LoadImage(
			GetModuleHandle(0), MAKEINTRESOURCE(IDB_FOLDER_HIDDEN_CLOSED), IMAGE_BITMAP, 16, 16, LR_SHARED
		), RGB(255, 0, 255));

	ImageList_AddMasked(_images, (HBITMAP)LoadImage(
			GetModuleHandle(0), MAKEINTRESOURCE(IDB_PROJECT), IMAGE_BITMAP, 16, 16, LR_SHARED
		), RGB(255, 0, 255));

	ImageList_AddMasked(_images, (HBITMAP)LoadImage(
			GetModuleHandle(0), MAKEINTRESOURCE(IDB_PACK_FOLDER_OPEN), IMAGE_BITMAP, 16, 16, LR_SHARED
		), RGB(255, 0, 255));

	ImageList_AddMasked(_images, (HBITMAP)LoadImage(
			GetModuleHandle(0), MAKEINTRESOURCE(IDB_PACK_FOLDER_CLOSED), IMAGE_BITMAP, 16, 16, LR_SHARED
		), RGB(255, 0, 255));

	int modImg = ImageList_AddMasked(_images, (HBITMAP)LoadImage(
			GetModuleHandle(0), MAKEINTRESOURCE(IDB_MODIFIED), IMAGE_BITMAP, 16, 16, LR_SHARED
		), RGB(255, 0, 255));

	ImageList_SetOverlayImage(_images, modImg, 1);

	TreeView_SetImageList(_tree, _images, TVSIL_NORMAL);

	return ResetView();
}

void ExplorerView::ClearTree()
{
	SetWindowRedraw(_tree, FALSE);

	delete _root;

	if ((_root = new ExNode(this)) != 0)
	{
		ExRoot* exRoot = new ExRoot(_root);

		if (exRoot == 0)
		{
			delete _root;
			_root = 0;
		}
	}

	SetWindowRedraw(_tree, TRUE);
}

bool ExplorerView::ResetView()
{
	SetWindowRedraw(_tree, FALSE);

	bool result = ResetViewSub();

	SetWindowRedraw(_tree, TRUE);

	return result;
}

bool ExplorerView::ResetViewSub()
{
	Xt::DomDb& db = *_window._app._db;

	delete _root;

	ExRoot* exRoot = new ExRoot(_root = new ExNode(this));

	if (exRoot == 0)
	{
		delete _root;
		_root = 0;
		return false;
	}

	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator it(&db.packs); it; ++it)
	{
		ExNode* exPack = ExAcquirePack(_root, it);

		if (Xt::DomArray* filters = AsArray(At(&it->properties, "filters")))
		{
			for(Xt::DomArray::IndexIterator itFilter(filters); itFilter; ++itFilter)
				ExAcquireFolder(exPack, AsText(itFilter));
		}
	}

	for(Xt::DomDb::Iterator itPack(&db); itPack; ++itPack)
	{
		for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
		{
			if (!ExAcquireRecord(_root, itRecord))
				return false;
		}
	}

	return _root->Populate(TVI_FIRST);
}

ExNode* ExplorerView::ResolveDragTarget(ExNode* sourceNode, int x, int y)
{
	if (!sourceNode)
		return 0;

	TVHITTESTINFO ht;

	ht.pt.x = x;
	ht.pt.y = y;

	HTREEITEM htiTarget;

	if ((htiTarget = TreeView_HitTest(_tree, &ht)) == NULL)
		return 0;

	TVITEMEX ti = { 0 };

	ti.mask		= TVIF_HANDLE | TVIF_PARAM;
	ti.hItem	= htiTarget;

	if (TreeView_GetItem(_tree, &ti) == FALSE)
		return 0;

	ExNode* node = (ExNode*)ti.lParam;

	switch(sourceNode->_item->Type())
	{
		case kEitPack:
		case kEitPackFolder:
			while(node && node->_item && ((node->_item->Type() == kEitRecord) || (node->_item->Type() == kEitFolder) || (node->_item->Type() == kEitPack)))
				node = node->_container;
		break;

		case kEitFolder:
		case kEitRecord:
			while(node && node->_item && (node->_item->Type() == kEitRecord))
				node = node->_container;
		break;
	}

	if (node && sourceNode->IsParentOf(node))
		node = sourceNode;

	if (!node || !node->_item)
		return 0;

	switch(sourceNode->_item->Type())
	{
		case kEitPack:
		case kEitPackFolder:
			if ((node->_item->Type() == kEitRoot) || (node->_item->Type() == kEitPackFolder))
				return node;
		break;

		case kEitFolder:
		case kEitRecord:
			if ((node->_item->Type() == kEitPack) || (node->_item->Type() == kEitFolder))
				return node;
		break;
	}

	return 0;
}

void ExplorerView::Drag_Begin(NMTREEVIEW* tv)
{
	if (_dragging)
		return;

	TVITEMEX ti = { 0 };

	ti.mask		= TVIF_HANDLE | TVIF_PARAM;
	ti.hItem	= tv->itemNew.hItem;

	if (TreeView_GetItem(_tree, &ti) == FALSE)
		return;

	ExNode* node = (ExNode*)ti.lParam;

	if (!node || !node->_item)
		return;

	switch(node->_item->Type())
	{
		case kEitPack:
		case kEitFolder:
		case kEitRecord:
		case kEitPackFolder:
		break;

		default:
		return;
	}

	_dragging	= true;
	_dragNode	= node;

	// Select the node we're going to drag

	TreeView_SelectItem(_tree, node->_ti);

	// Tell the tree-view control to create an image to use for dragging

	HIMAGELIST himl = TreeView_CreateDragImage(_tree, tv->itemNew.hItem);
	
	// Get the bounding rectangle of the item being dragged.

	RECT rcItem;
	TreeView_GetItemRect(_tree, tv->itemNew.hItem, &rcItem, TRUE);

	// Get the heading level and the amount that the child items are indented.

	DWORD dwLevel = (DWORD)tv->itemNew.lParam; 
	DWORD dwIndent = (DWORD)SendMessage(_tree, TVM_GETINDENT, 0, 0); 

	// Start the drag operation.

	ImageList_BeginDrag(himl, 0, 0, 0);
	ImageList_DragEnter(_tree, 0, 0);

	// Hide the mouse pointer, and direct mouse input to the parent window

	SetCapture(*this);

	ImageList_DragShowNolock(FALSE);
	TreeView_SelectDropTarget(_tree, 0);
	ImageList_DragShowNolock(TRUE);
}

void ExplorerView::Drag_Update(int x, int y)
{
	if (!_dragging)
		return;

	ImageList_DragMove(x, y);

	ExNode* target = ResolveDragTarget(_dragNode, x, y);
	
	ImageList_DragShowNolock(FALSE);

	TreeView_SelectDropTarget(_tree, target ? target->_ti : 0);

	ImageList_DragShowNolock(TRUE);
}

void ExplorerView::Drag_End(bool accepted, int x, int y)
{
	if (!_dragging)
		return;

	ExNode* sourceNode = _dragNode;

	_dragging = false;
	_dragNode = 0;

    ImageList_EndDrag();
    ReleaseCapture();
	TreeView_SelectDropTarget(_tree, 0);

	if (!sourceNode)
		return;

	ExNode* node = ResolveDragTarget(sourceNode, x, y);

	if (!node || sourceNode->IsParentOf(node) || (node == sourceNode->Container()))
		return;

	if (sourceNode->Item()->Type() == kEitRecord)
	{
		Xt::DomRecord* record = AsRecord(((ExXt*)sourceNode->Item())->_dom);

		if (!record)
			return;

		// Create path to new node and find target pack

		XtString nodePath;
		Xt::DomPack* pack = 0;

		for(ExNode* packNode = node; packNode && packNode->Item() && (packNode->Item()->Type() != kEitRoot); packNode = packNode->Container())
		{
			if (packNode->Item()->Type() == kEitFolder)
			{
				ExFolder* item = (ExFolder*)packNode->Item();

				nodePath = item->_name + "." + nodePath;
			}
			else if (packNode->Item()->Type() == kEitPack)
			{
				pack = _window.App()._db->packs[((ExPack*)packNode->Item())->_packId];

				break;
			}
		}

		if (!pack)
			return;

		// Move record

		const char* dot = record->name;

		for(const char* ptr = dot; *ptr != '\0'; ptr++)
			if (*ptr == '.')
				dot = ptr + 1;

		if (pack != PackOf(record))
			MoveRecord(record, pack);

		record->name = nodePath + dot;

		SetDirty(record, true);

		// Move ExNodes

		sourceNode->DetachSelf();
		sourceNode->_id = MakeRecordId(record);
		node->Add(sourceNode);

		_root->Populate(TVI_FIRST);
	}
	else if (sourceNode->Item()->Type() == kEitFolder)
	{
		ExNode* srcPackNode = sourceNode->_container;
		ExNode* dstPackNode = node;

		XtString srcPath;
		XtString dstPath;

		while(srcPackNode && srcPackNode->_item && (srcPackNode->_item->Type() == kEitFolder))
		{
			srcPath		= ((ExFolder*)srcPackNode->Item())->_name + "." + srcPath;
			srcPackNode	= srcPackNode->_container;
		}

		while(dstPackNode && dstPackNode->_item && (dstPackNode->_item->Type() == kEitFolder))
		{
			dstPath		= ((ExFolder*)dstPackNode->Item())->_name + "." + dstPath;
			dstPackNode = dstPackNode->_container;
		}

		if (!srcPackNode || (srcPackNode->Item()->Type() != kEitPack))
			return;

		if (!dstPackNode || (dstPackNode->Item()->Type() != kEitPack))
			return;

		// At this point we have a valid move

		// Move pack

		Xt::DomPack* srcPack = _window.App()._db->packs[((ExPack*)srcPackNode->Item())->_packId];
		Xt::DomPack* dstPack = _window.App()._db->packs[((ExPack*)dstPackNode->Item())->_packId];

		if (!srcPack || !dstPack)
			return;

		if (srcPack != dstPack)
		{
			SetDirty(srcPack, false);
			SetDirty(dstPack, false);

			sourceNode->Item()->MoveToPack(dstPack);
		}

		// Move folders

		((ExFolder*)sourceNode->Item())->Repath(srcPath, dstPath);

		// Move ExNodes

		sourceNode->DetachSelf();
		node->Add(sourceNode);

		_root->Populate(TVI_FIRST);
 	}
	else if (sourceNode->Item()->Type() == kEitPack)
	{
		Xt::DomDb*		db		= _window.App()._db;
		Xt::DomPack*	pack	= Xt::FindPackById(db, ((ExPack*)sourceNode->Item())->_packId);

		if (!db || !pack)
			return;

		// Create path to new node and find target pack

		XtString nodePath;

		for(ExNode* ex = node; ex && ex->Item() && (ex->Item()->Type() != kEitRoot); ex = ex->Container())
		{
			if (ex->Item()->Type() == kEitPackFolder)
			{
				ExPackFolder* item = (ExPackFolder*)ex->Item();

				nodePath = item->_name + "." + nodePath;
			}
		}

		// Move pack

		const char* dot = pack->name;

		for(const char* ptr = dot; *ptr != '\0'; ptr++)
			if (*ptr == '.')
				dot = ptr + 1;

		pack->name = nodePath + dot;

		SetDirty(pack, true);

		// Move ExNodes

		sourceNode->DetachSelf();
		sourceNode->_id = MakePackId(pack);
		node->Add(sourceNode);

		_root->Populate(TVI_FIRST);
	}
	else if (sourceNode->Item()->Type() == kEitPackFolder)
	{
		ExNode* srcPackFolderNode = sourceNode->_container;
		ExNode* dstPackFolderNode = node;

		XtString srcPath;
		XtString dstPath;

		while(srcPackFolderNode && srcPackFolderNode->_item && (srcPackFolderNode->_item->Type() == kEitPackFolder))
		{
			srcPath				= ((ExPackFolder*)srcPackFolderNode->Item())->_name + "." + srcPath;
			srcPackFolderNode	= srcPackFolderNode->_container;
		}

		while(dstPackFolderNode && dstPackFolderNode->_item && (dstPackFolderNode->_item->Type() == kEitPackFolder))
		{
			dstPath				= ((ExPackFolder*)dstPackFolderNode->Item())->_name + "." + dstPath;
			dstPackFolderNode	= dstPackFolderNode->_container;
		}

		if (!srcPackFolderNode || (srcPackFolderNode->Item()->Type() != kEitRoot))
			return;

		if (!dstPackFolderNode || (dstPackFolderNode->Item()->Type() != kEitRoot))
			return;

		// At this point we have a valid move

		// Move folders

		((ExFolder*)sourceNode->Item())->RepathPack(srcPath, dstPath);

		// Move ExNodes

		sourceNode->DetachSelf();
		node->Add(sourceNode);

		_root->Populate(TVI_FIRST);
	}
}

ExNodeIcon ExplorerView::RegisterTypeForIcon(LarvaType* type)
{
	if (!type || !*type->icon)
		return kEniFile;

	if (int* i = _iconMap[type->icon])
		return (ExNodeIcon)*i;

	int newIndex = kEniFile;

	if (Gdiplus::Bitmap* bmp = CacheBitmap(type->icon))
	{
		Gdiplus::Bitmap redrawn(16, 16);

		Gdiplus::Graphics(&redrawn).DrawImage(bmp, 0, 0, 16, 16);

		HBITMAP hbmp = 0;

		if (redrawn.GetHBITMAP(Gdiplus::Color::White, &hbmp) == Gdiplus::Ok)
		{
			if ((newIndex = ImageList_Add(_images, hbmp, 0)) < 0)
				newIndex = kEniFile;

			DeleteObject(hbmp);
		}
	}

	if (newIndex == kEniFile)
		DebugLn("Failed to load icon for %s: %s", type->name.Text(), type->icon.Text());

	XtUt::Insert(&_iconMap, type->icon, new int(newIndex));

	return (ExNodeIcon)newIndex;
}
#ifndef EXPLORER_H
#define EXPLORER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin/WndCore.h"
#include "Core/List.h"
#include "Xt/XtTool.h"

//
// Prototypes
//

class ExNode;
class ExItem;

//
// ExItemIcon
//

enum ExNodeIcon
{
	kEniFile,
	kEniFileHidden,
	kEniFileModified,
	kEniFileCheckedOut,
	kEniFolderOpen,
	kEniFolderClosed,
	kEniFolderOpenHidden,
	kEniFolderClosedHidden,
	kEniProject,
	kEniPackFolderOpen,
	kEniPackFolderClosed
};

enum ExNodeOverlayIcon
{
	kEnoiNone,
	kEnoiModified
};

//
// ExplorerView
//

class ExplorerView : public ChildWindow
{
public:
	ExplorerView(class SorceryWindow& window);
	~ExplorerView();

public:
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

private:
	int OnNotify(NMHDR* hdr);

private:
	bool InitialiseTree();

public:
	void ClearTree();
	bool ResetView();

private:
	bool ResetViewSub();

public:
	ExNode* ResolveDragTarget(ExNode* sourceNode, int x, int y);
	void Drag_Begin(NMTREEVIEW* tv);
	void Drag_Update(int x, int y);
	void Drag_End(bool accepted, int x, int y);

public:
	ExNodeIcon RegisterTypeForIcon(LarvaType* type);

public:
	inline SorceryWindow& Window()
	{
		return _window;
	}

	inline ExNode* Root()
	{
		return _root;
	}

	inline HWND Tree()
	{
		return _tree;
	}

private:
	class SorceryWindow& _window;

private:
	HWND		_tree;
	HIMAGELIST	_images;
	ExNode*		_root;

private:
	bool	_dragging;
	ExNode*	_dragNode;

private:
	XtUt::Dictionary<XtString, int> _iconMap;

friend class ExNode;
};

//
// ExItemType
//

enum ExItemType
{
	kEitNone,
	kEitRoot,
	kEitPack,
	kEitFolder,
	kEitRecord,
	kEitPackFolder
};

//
// ExNode
//

class ExNode
{
public:
	ExNode(ExplorerView* explorer);
	virtual ~ExNode();

public:
	bool Populate(HTREEITEM insertAfter);

public:
	void UpdateIconLabel();

public:
	ExNode*	Add(ExNode* item);
	int		Find(const char* id);
	ExNode*	Detach(int index);
	void	Remove(int index);
	void	Clear();
	ExNode*	DetachSelf();
	bool	IsParentOf(ExNode* item);

public:
	inline int Length()
	{
		return _items.Length();
	}

	inline ExNode* Get(int index)
	{
		return _items.Get(index);
	}

	inline ExNode* Get(const char* id)
	{
		return _items.Get(Find(id));
	}

	inline ExItem* Item()
	{
		return _item;
	}

	inline ExItemType ItemType();

	inline ExNode* Container()
	{
		return _container;
	}

	inline ExplorerView* Explorer()
	{
		return _explorer;
	}

	inline const XtString& Id() const
	{
		return _id;
	}

	inline const XtString& Label() const
	{
		return _label;
	}

	HTREEITEM TreeItem() const
	{
		return _ti;
	}

private:
	void ClearChildTreeItems();

private:
	ExItem*			_item;
	ExNode*			_container;
	ExplorerView*	_explorer;
	List<ExNode>	_items;
	HTREEITEM		_ti;
	bool			_isOpen;

private:
	XtString			_id;
	XtString			_label;
	ExNodeIcon			_iconOpen;
	ExNodeIcon			_iconClosed;
	ExNodeOverlayIcon	_iconOverlay;

friend class ExItem;
friend class ExplorerView;
};

//
// ExItem
//

class ExItem
{
public:
	ExItem(ExItemType type, ExNode* node);
	virtual ~ExItem();

public:
	void SetId(const XtString& id);
	void SetLabel(const XtString& label);
	void SetIcon(ExNodeIcon open, ExNodeIcon closed);
	void SetIcon(LarvaType* type);
	void SetOverlayIcon(ExNodeOverlayIcon icon);

public:
	virtual bool OnEnter();
	virtual bool OnDoubleClick();
	virtual bool OnRightClick();
	virtual bool OnBeginEditLabel();
	virtual bool OnEndEditLabel(const char* newText);
	virtual void OnDeletePressed();
	virtual void OnDuplicateRecord(ExItem* parent, bool renameRecord);
	virtual void Selected();

public:
	virtual void Delete();
	virtual void Repath(const XtString& oldPath, const XtString& newPath);
	virtual void MoveToPack(Xt::DomPack* pack);
	virtual void GenerateFilters(XtUt::Set<XtString>* filters, const XtString& path);
	virtual void RepathPack(const XtString& oldPath, const XtString& newPath);

public:
	inline ExItemType Type()
	{
		return _type;
	}

	inline ExNode* Node()
	{
		return _node;
	}

private:
	ExItemType	_type;
	ExNode*		_node;

friend class ExNode;
friend class PiItem;
};

inline ExItemType ExNode::ItemType()
{
	return _item ? _item->Type() : kEitNone;
}

//
// ExRoot
//

class ExRoot : public ExItem
{
public:
	ExRoot(ExNode* node);
	virtual ~ExRoot();

public:
	virtual bool OnRightClick();
	virtual void GenerateFilters(XtUt::Set<XtString>* filters, const XtString& path);
};

//
// ExPack
//

class ExPack : public ExItem
{
public:
	ExPack(ExNode* node, Xt::DomPack* pack, const XtString& label);
	virtual ~ExPack();

public:
	virtual bool OnRightClick();
	virtual bool OnBeginEditLabel();
	virtual bool OnEndEditLabel(const char* newText);
	virtual void OnDeletePressed();

public:
	virtual void Delete();
	virtual void GenerateFilters(XtUt::Set<XtString>* filters, const XtString& path);
	virtual void RepathPack(const XtString& oldPath, const XtString& newPath);

public:
	XtId _packId;
};

//
// ExFolder
//

class ExFolder : public ExItem
{
public:
	ExFolder(ExNode* node, XtString name);
	virtual ~ExFolder();

public:
	virtual bool OnRightClick();
	virtual bool OnBeginEditLabel();
	virtual bool OnEndEditLabel(const char* newText);
	virtual void OnDeletePressed();
	virtual void OnDuplicateRecord(ExItem* parent, bool renameRecord);

public:
	virtual void Delete();
	virtual void Repath(const XtString& oldPath, const XtString& newPath);
	virtual void MoveToPack(Xt::DomPack* pack);
	virtual void GenerateFilters(XtUt::Set<XtString>* filters, const XtString& path);

public:
	void Rename(const char* newName);

public:
	XtString _name;
};

//
// ExXt
//

class ExXt : public ExItem
{
public:
	ExXt(ExNode* node, Xt::DomRecord* record, const XtString& label);
	virtual ~ExXt();

public:
	virtual bool OnEnter();
	virtual bool OnDoubleClick();
	virtual bool OnRightClick();
	virtual bool OnBeginEditLabel();
	virtual bool OnEndEditLabel(const char* newText);
	virtual void OnDeletePressed();
	virtual void OnDuplicateRecord(ExItem* parent, bool renameRecord);
	virtual void Selected();

public:
	virtual void Delete();
	virtual void Repath(const XtString& oldPath, const XtString& newPath);
	virtual void MoveToPack(Xt::DomPack* pack);

public:
	void Open();

public:
	XtId Id()
	{
		return _xtId;
	}

public:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, ExXt* xt);

private:
	XtId			_xtId;
	Xt::DomMonitor	_dom;

friend class ExplorerView;
};

//
// ExPackFolder
//

class ExPackFolder : public ExItem
{
public:
	ExPackFolder(ExNode* node, XtString name);
	virtual ~ExPackFolder();

public:
	virtual bool OnRightClick();
	virtual bool OnBeginEditLabel();
	virtual bool OnEndEditLabel(const char* newText);
	virtual void OnDeletePressed();

public:
	virtual void Delete();
	virtual void GenerateFilters(XtUt::Set<XtString>* filters, const XtString& path);
	virtual void RepathPack(const XtString& oldPath, const XtString& newPath);

public:
	void Rename(const char* newName);

public:
	XtString _name;
};

XtString MakePackId(Xt::DomPack* pack);
XtString MakeFolderId(const XtString& name);
XtString MakeRecordId(Xt::DomRecord* record);
XtString MakePackFolderId(const XtString& name);

ExNode* ExAcquirePackFolder(ExNode* parent, const char* path);
ExNode* ExAcquirePack(ExNode* parent, Xt::DomPack* pack);
ExNode* ExAcquireFolder(ExNode* parent, const char* path);
ExNode* ExAcquireRecord(ExNode* parent, Xt::DomRecord* record);

#endif // EXPLORER_H
#ifndef XTWIN_TREE_CTRL__H
#define XTWIN_TREE_CTRL__H

#ifdef _MSC_VER
#pragma once
#endif

#include <xtwin/Core.h>
#include <xtwin/WndCore.h>

// Undefine unnecessary windows.h macros
#undef GetFirstChild
#undef GetNextSibling
#undef GetPrevSibling
#undef LoadIcon

class TreeCtrl;
class TreeItem;

enum TcHitKind
{
	kTcHitNone,
	kTcHitRowButton,
	kTcHitRowIcon,
	kTcHitRowLabel,
	kTcHitRowEmpty,	
};

enum TcSelectionType
{
	kTcSelectionClean,
	kTcSelectionToggle,
};

struct TcHitResult
{
	TreeItem* Item;
	TcHitKind HitType;
};

enum TcSourceType
{
	kTcSourceSelection,
	kTcSourceItem,
	kTcSourceUnknown,
};

enum TcDestinationType
{
	kTcDestinationBetween,
	kTcDestinationOver,
	kTcDestinationUnknown,
};

struct TcDragInfo
{
	bool IsDragging;

	TcSourceType SourceType;
	TcDestinationType DestinationType;

	TreeItem* SourceItem;
	TreeItem* NewParent;
	int NewIndex;
};

class TreeItem
{
public:
	TreeItem();
	virtual ~TreeItem();
	
	const TreeItem* GetParent() const;
	TreeItem* GetParent();
	void SetParent(TreeItem* parent);

	int GetNumChildren() const;
	TreeItem* GetChild(int num);
	const TreeItem* GetChild(int num) const;
	void AddChild(TreeItem* child, TreeItem* before=0);
	bool RemoveChild(TreeItem* child, bool freeMemory=true, bool force=true, bool skipEvents=false);

	void SetIcon(int iconId);
	void SetColour(int r, int g, int b);

private:
	void ResetChildIndices();
	void SetTreeCtrl(TreeCtrl* tree);
	bool UnlinkChild(TreeItem* child);
	TreeItem* _GetNext();
	TreeItem* _GetPrev();

public:	
	const Sz16& GetLabel() const;
	void SetLabel(const Sz16& label);
	void SetLabel(const XtString& label);

	bool IsSelected() const;
	void SetSelected(bool isSelected, bool force=false);

	bool IsHidden() const;
	void SetHidden(bool isHidden);

	bool IsExpanded() const;
	void SetExpanded(bool isExpanded, bool recursive=false);

	bool IsVisible() const;

	int GetItemWidth();
	int GetItemHeight();
	int GetTotalHeight();
	int GetMaxWidth();
	
	int GetIndex();

	TcHitResult HitTest(const WinPoint& point);

	void Draw(Gdiplus::Graphics* g, int offsetX, int offsetY);

	void SortChildren(bool recursive=false);
	
	TreeItem* GetNextSibling();
	TreeItem* GetPrevSibling();

	TreeItem* GetFirstChild();
	TreeItem* GetLastChild();

	bool IsChildOf(TreeItem* item);
	
public:
	virtual void OnContextMenu(const WinPoint& pt) {}
	virtual bool OnItemDeleting() { return true; }
	virtual void OnItemDeleted() {}
	virtual bool OnItemSelectionChanged(bool selected) { return true; }
	virtual bool OnReparent(TreeItem* parent) { return true; }
	virtual bool OnReorder(int newIndex) { return true; }
	virtual bool OnStartRename(Sz16& newName) { return true; }
	virtual bool OnEndRename(Sz16& newName) { return true; }
	virtual bool OnActivate() { return false; }

private:
	TreeCtrl* _TreeCtrl;
	
	TreeItem* _Parent;
	int _Index;

	XtUt::Array<TreeItem*> _Children;

	Sz16 _Label;
	bool _IsHidden;
	bool _IsExpanded;
	bool _IsSelected;

	int _TotalHeight;
	bool _TotalHeightDirty;

	int _IconId;
	Gdiplus::Color _Colour;

	const int _MinItemWidth;

friend class TreeCtrl;
};

class TreeCtrl : public ChildWindow
{
public:
	TreeCtrl();
	virtual ~TreeCtrl();
	
	struct TreeCtrlFlags
	{
		bool AllowMultipleSelection : 1;
		bool HasIcons : 1;
		bool HighlightSelectedRow : 1;
		bool ToggleSelection : 1;
	};

	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT EditWndProc(UINT Message,WPARAM wParam, LPARAM lParam);

	TreeCtrlFlags& GetFlags();
	const TreeCtrlFlags& GetFlags() const;

	void SetRootNode(TreeItem* data, bool takeOwnership=true);
	const TreeItem* GetRootNode() const;
	TreeItem* GetRootNode();

	int GetIndentSize() const;
	void SetIndentSize(int indentSize);

	int GetButtonSize() const;
	void SetButtonSize(int buttonSize);

	int GetIconSize() const;
	void SetIconSize(int iconSize);

	bool IsSorted() const;
	void SetSorted(bool sort);

	const XtUt::Array<TreeItem*>& GetSelection() const;
	void ClearSelection(bool supressSelectionChangedEvent=false);
	void ClearSelectionRange(TreeItem* start, TreeItem* end, bool supressSelectionChangedEvent=false);
	void SetSelection(TreeItem* item);
	void SetSelection(const XtUt::Array<TreeItem*>& selection);
	void SetSelectionRange(TreeItem* start, TreeItem* end);

	virtual void OnPrePaintWindow();
	virtual void OnAddItem(TreeItem* item);
	virtual void OnRemoveItem(TreeItem* item);
	virtual bool OnRemoveItems(const XtUt::Array<TreeItem*>& items);
	virtual void OnHideItem(TreeItem* item);
	virtual void OnLabelChanged(TreeItem* item);
	virtual bool OnCompareItems(TreeItem* a, TreeItem* b);
	virtual void OnSelectionChanged(const XtUt::Array<TreeItem*>& selection, TcSelectionType selectionType) {};

	int LoadIcon(const Sz8& location, int id=-1);
	void RemoveIcon(int iconId);

	const TreeItem* GetFocusItem() const;

	TcHitResult HitTest(const WinPoint& point);

	bool RemoveItem(TreeItem* item);

	void EnsureVisible(TreeItem* item);

	void Freeze();
	void Thaw();

	void StartRename(TreeItem* item);

private:
	void EndRename(bool apply);

	Gdiplus::Font* GetDefaultFont();
	Gdiplus::SolidBrush* GetDefaultBrush();
	WinRect& GetDisplayRect();

	void AddToSelection(TreeItem* item, int index=-1);
	void RemoveFromSelection(TreeItem* item);

	void PaintWindow();
	void DrawTree(Gdiplus::Graphics* g);

	void LMouseDown(const WinPoint &pt, WPARAM wParam, bool isDoubleClick);
	void MMouseDown(const WinPoint &pt, WPARAM wParam);
	void RMouseDown(const WinPoint &pt, WPARAM wParam);

	void DragReparent(TreeItem* item);
	void MMouseUp(const WinPoint &pt, WPARAM wParam);

	void MouseMove(const WinPoint &pt, WPARAM wParam);

	bool KeyDown(LPARAM lParam, WPARAM wParam);

	int GetItemOffsetX(TreeItem* item);
	int GetItemOffsetXInternal(TreeItem* item);
	int GetItemOffsetY(TreeItem* item);

	void UpdateBounds();

	Gdiplus::Bitmap* GetIconImage(int iconId);

	bool _OwnTreeData;
	TreeItem* _TreeData;
	XtUt::Array<TreeItem*> _Selection;

	XtUt::Dictionary<int, Gdiplus::Bitmap> _Icons;

	bool _SelectDown;
	TreeItem* _FocusStart;
	TreeItem* _FocusItem;
	TreeItem* _RenameItem;

	int _TreeHeight;
	int _TreeWidth;
	int _ScrollX;
	int _ScrollY;

	int _IndentSize;
	int _ButtonSize;
	int _IconSize;

	bool _IsSorted;
	bool _CacheDirty;
	bool _BoundsDirty;

	int _IsFrozen;

	TcDragInfo _DragInfo;	

	TreeCtrlFlags _Flags;

	Gdiplus::Font* _DefaultFont;
	Gdiplus::SolidBrush* _DefaultBrush;

	WinRect _DisplayRect;

	WNDPROC _EditWndProc;
	HWND _EditControl;

friend class TreeItem;
};

#endif

#ifndef XTWIN_DOPESHEET_H
#define XTWIN_DOPESHEET_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin/WndCore.h"
#include "XtWin/NgMaths.h"

class DsKey;
class DsRow;
class DsDiagram;
class DopeSheetWnd;

//
// DopeSheet enums
//

enum DsSelectMode
{
	kDsSelectSingle,
	kDsSelectAdd,
	kDsSelectRemove,
	kDsSelectToggle
};

enum DsHitResult
{
	kDsHrNone,
	kDsHrHeader,
	kDsHrRowLabel,
	kDsHrRowContent,
	kDsHrRowExpand
};

//
// DopeSheetContext
//

class DopeSheetContext
{
public:
	DopeSheetContext();
	virtual ~DopeSheetContext();

public:
	virtual void SelectionChanged();
};

//
// DsKey
//

class DsKey
{
public:
	DsKey();
	virtual ~DsKey();

public:
	virtual void OnTimeChanged();

public:
	void SetTime(float time);
	void SetDuration(float duration);
	void SetColour(XtU32 colour);

public:
	void Detach();

public:
	DsDiagram* Diagram() const;
	DsRow* Row() const					{ return _row;			}
	XtUt::WeakAnchor<DsKey>& Anchor()	{ return _anchor;		}
	float Time() const					{ return _time;			}
	float Duration() const				{ return _duration;		}
	XtU32 Colour() const				{ return _colour;		}
	bool IsSelected() const				{ return _isSelected;	}
	const NgRect& HitRect() const		{ return _hitRect;		}
	int SubRow() const					{ return _subRow;		}

public:
	void MoveBy(const NgVector& offset);

public:
	void Select();
	void Deselect();

private:
	void SetDirty(bool redoLayout);

private:
	XtUt::WeakAnchor<DsKey>	_anchor;
	DsRow*					_row;
	XtUt::Link<DsKey>		_linkRow;

private:
	bool				_isSelected;
	XtUt::Link<DsKey>	_linkSelection;

private:
	NgRect	_hitRect;
	int		_subRow;

private:
	float _time;
	float _duration;
	XtU32 _colour;

friend class DsRow;
friend class DsDiagram;
};

//
// DsRow
//

class DsRow
{
public:
	DsRow();
	virtual ~DsRow();

public:
	void SetLabel(const Sz16& label);
	void SetColour(XtU32 colour);
	void SetHeight(float height);
	void SetExpanded(bool expanded);
	void SetBlocks(bool blocks);

public:
	DsRow* AddRow(DsRow* row);
	DsRow* InsertRow(DsRow* row, DsRow* after);
	void Detach();
	void Clear();

public:
	DsKey* AddKey(DsKey* key);
	DsKey* InsertKey(DsKey* key, DsKey* after);

public:
	DsDiagram* Diagram() const			{ return _diagram;		}
	XtUt::WeakAnchor<DsRow>& Anchor()	{ return _anchor;		}
	DsRow* Parent() const				{ return _parent;		}
	const Sz16& Label() const			{ return _label;		}
	XtU32 Colour() const				{ return _colour;		}
	float Height() const				{ return _height;		}
	float Indent() const				{ return _indent;		}
	const NgRect& HitRect() const		{ return _hitRect;		}
	bool IsSelected() const				{ return _isSelected;	}
	bool IsExpanded() const				{ return _isExpanded;	}
	bool IsBlocks() const				{ return _isBlocks;		}
	int SubRowCount() const				{ return _subRowCount;	}
	bool CanExpand() const;

public:
	XtUt::LinkList<DsRow>::Iterator Rows()							{ return XtUt::LinkList<DsRow>::Iterator(&_rows);				}
	XtUt::LinkList<DsRow>::ConstIterator Rows() const				{ return XtUt::LinkList<DsRow>::ConstIterator(&_rows);			}
	XtUt::LinkList<DsKey>::Iterator Keys()							{ return XtUt::LinkList<DsKey>::Iterator(&_keys);				}
	XtUt::LinkList<DsKey>::ConstIterator Keys() const				{ return XtUt::LinkList<DsKey>::ConstIterator(&_keys);			}
	XtUt::LinkList<DsKey>::ReverseIterator ReverseKeys()			{ return XtUt::LinkList<DsKey>::ReverseIterator(&_keys);		}
	XtUt::LinkList<DsKey>::ConstReverseIterator ReverseKeys() const	{ return XtUt::LinkList<DsKey>::ConstReverseIterator(&_keys);	}

public:
	void Select();
	void Deselect();

public:
	virtual DsHitResult HitTest(const NgVector& pt);
	virtual DsRow* HitTestRow(const NgVector& pt);
	virtual DsKey* HitTestKey(const NgVector& pt);

public:
	void LayoutKeys();

private:
	void AssignSubRow(XtUt::Array<DsKey*>* subRows, DsKey* key);
	void SetDirty(bool redoLayout);

private:
	XtUt::WeakAnchor<DsRow>	_anchor;
	DsDiagram*				_diagram;
	DsRow*					_parent;
	XtUt::Link<DsRow>		_linkParent;

private:
	XtUt::LinkList<DsRow> _rows;
	XtUt::LinkList<DsKey> _keys;

private:
	bool				_isSelected;
	XtUt::Link<DsRow>	_linkSelection;

private:
	NgRect _hitRect;
	NgRect _childRect;
	NgRect _expandRect;
	float _indent;
	int _subRowCount;

private:
	Sz16	_label;
	XtU32	_colour;
	float	_height;
	bool	_isExpanded;
	bool	_isBlocks;

friend class DsKey;
friend class DsDiagram;
};

//
// DsDiagram
//

struct DsRenderContext;

class DsDiagram
{
public:
	DsDiagram(DopeSheetWnd* dopesheet);
	~DsDiagram();

public:
	void SetClientRect(const NgRect& client);
	void Layout();
	void Render(Gdiplus::Graphics* g);
	void SetScrollY(float y);
	void Pan(const NgVector& delta);
	void Zoom(const NgVector& pt, float factor);
	void SetRange(float low, float high);
	void SetQuantisation(bool quantise, float q);
	void BeginMoveKeys();
	void MoveSelectedKeysBy(const NgVector& offset);
	void EndMoveKeys();

public:
	DsHitResult HitTest(const NgVector& pt);
	DsRow* HitTestRow(const NgVector& pt);
	void ClearSelection();
	void Select(DsRow* row, DsSelectMode mode);
	void Select(DsKey* key, DsSelectMode mode);
	void SelectRect(const NgRect& rect, DsSelectMode mode);
	int RowSelectionCount() const;
	int KeySelectionCount() const;

public:
	void RectSelectionBegin(const NgVector& pt);
	void RectSelectionUpdate(const NgVector& pt);
	void RectSelectionEnd();

public:
	DopeSheetWnd*	DopeSheet()		{ return _dopesheet;		}
	DsRow*			Root()			{ return &_root;		}
	float			HeaderY()		{ return _headerY;		}
	float			SplitterX()		{ return _splitterX;	}
	float			TotalHeight()	{ return _totalHeight;	}
	float			ZoomX()			{ return _zoomX;	}
	float			ScrollX()		{ return _scrollX;	}
	float			ScrollY()		{ return _scrollY;	}
	NgRect RectSelection() const	{ return Normalise(NgRect(_rectSelectionStart, _rectSelectionEnd - _rectSelectionStart)); }

public:
	XtUt::LinkList<DsRow>::Iterator SelectionRows()				{ return XtUt::LinkList<DsRow>::Iterator(&_selectionRows); }
	XtUt::LinkList<DsRow>::ConstIterator SelectionRows() const	{ return XtUt::LinkList<DsRow>::ConstIterator(&_selectionRows); }
	XtUt::LinkList<DsKey>::Iterator SelectionKeys()				{ return XtUt::LinkList<DsKey>::Iterator(&_selectionKeys); }
	XtUt::LinkList<DsKey>::ConstIterator SelectionKeys() const	{ return XtUt::LinkList<DsKey>::ConstIterator(&_selectionKeys); }

public:
	NgVector ToRowArea(const NgVector& pt);
	NgVector ToContentArea(const NgVector& pt);

private:
	void DoRowLayout(DsRow* row, float indent, float& rowY, float width);
	void DoRenderBackground(DsRenderContext* ctx, DsRow* row);
	void DoRenderLabel(DsRenderContext* ctx, DsRow* row, float oldBoxY);
	void DoRenderRowShadow(DsRenderContext* ctx, DsRow* parent, DsRow* row, bool selected);
	void DoRenderRow(DsRenderContext* ctx, DsRow* row);
	void SetDirty(bool redoLayout);

private:
	DopeSheetWnd*			_dopesheet;
	DsRow					_root;
	XtUt::LinkList<DsRow>	_selectionRows;
	XtUt::LinkList<DsKey>	_selectionKeys;

private:
	float	_headerY;
	float	_splitterX;
	float	_zoomX;
	float	_scrollX;
	float	_scrollY;
	float	_rangeLow;
	float	_rangeHigh;
	bool	_quantise;
	float	_quantisation;

private:
	float _moveKeysOffset;

private:
	NgRect	_client;
	bool	_redoLayout;
	float	_totalHeight;

private:
	bool		_rectSelectionActive;
	NgVector	_rectSelectionStart;
	NgVector	_rectSelectionEnd;

friend class DsKey;
friend class DsRow;
};

//
// DopeSheet
//

class DopeSheetWnd : public ChildWindow
{
public:
	DopeSheetWnd(DopeSheetContext* context);
	virtual ~DopeSheetWnd();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	DsDiagram* Diagram() { return _diagram; }
	DopeSheetContext* Context() { return _context; }

private:
	void DoDrag(const WinPoint& pt);
	void DoSetCapture();
	void DoReleaseCapture();
	void DoScrollBars();
	void DoScrollBarPos();
	void DoCursor();

private:
	int SelectionCount() const;

private:
	enum DragState
	{
		kDragNone,
		kDragIgnore,
		kDragHeader,
		kDragRowLabel,
		kDragRowContent,
		kDragMoveKeys,
		kDragRectSelection
	};

private:
	DsDiagram*			_diagram;
	DopeSheetContext*	_context;

private:
	DragState	_dragging;
	WinPoint	_dragPt;
	WinPoint	_dragStart;
	DWORD		_dragTime;
	bool		_dragCommitted;
	bool		_panning;
	WinPoint	_panPt;
	bool		_adjusting;
	WinPoint	_adjustPt;

friend class DsDiagram;
};

#endif // XTWIN_DOPESHEET_H

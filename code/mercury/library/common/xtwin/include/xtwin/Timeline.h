#ifndef XTWIN_TIMELINE_H
#define XTWIN_TIMELINE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin/WndCore.h"
#include "XtWin/NgMaths.h"

class TlKey;
class TlRow;
class TlDiagram;
class TimelineWnd;

//
// Timeline enums
//

enum TlSelectMode
{
	kTlSelectSingle,
	kTlSelectAdd,
	kTlSelectRemove,
	kTlSelectToggle
};

//
// HitTest
//

enum TlHitKind
{
	kTlHitNone,
	kTlHitHeader,
	kTlHitSplitter,
	kTlHitRowLabel,
	kTlHitRowContent,
	kTlHitRowExpander,
	kTlHitKey
};

struct TlHitResult
{
	TlHitKind	kind;
	TlRow*		row;
	TlKey*		key;

public:
	TlHitResult(TlHitKind kind_ = kTlHitNone, TlRow* row_ = 0, TlKey* key_ = 0) :
		kind(kind_),
		row(row_),
		key(key_)
	{
	}
};

//
// TimelineContext
//

class TimelineContext
{
public:
	TimelineContext();
	virtual ~TimelineContext();

public:
	virtual void SelectionChanged();
};

//
// TlKey
//

class TlKey
{
public:
	TlKey();
	virtual ~TlKey();

public:
	virtual void OnTimeChanged();

public:
	void SetTime(float time);
	void SetDuration(float duration);
	void SetLabel(const Sz16& label);
	void SetColour(XtU32 colour);

public:
	void Detach();

public:
	TlDiagram* Diagram() const;
	TlRow* Row() const					{ return _row;			}
	XtUt::WeakAnchor<TlKey>& Anchor()	{ return _anchor;		}
	float Time() const					{ return _time;			}
	float Duration() const				{ return _duration;		}
	const Sz16& Label() const			{ return _label;		}
	XtU32 Colour() const				{ return _colour;		}
	bool IsSelected() const				{ return _isSelected;	}

public:
	void MoveBy(const NgVector& offset);

public:
	void Select();
	void Deselect();

private:
	void SetDirty(bool redoLayout);

private:
	XtUt::WeakAnchor<TlKey>	_anchor;
	TlRow*					_row;
	XtUt::Link<TlKey>		_linkRow;

private:
	bool				_isSelected;
	XtUt::Link<TlKey>	_linkSelection;

private:
	float	_time;
	float	_duration;
	Sz16	_label;
	XtU32	_colour;

friend class TlRow;
friend class TlGroup;
friend class TlDiagram;
};

//
// TlRow
//

class TlRow
{
public:
	TlRow();
	virtual ~TlRow();

public:
	void SetLabel(const Sz16& label);
	void SetColour(XtU32 colour);

public:
	void Detach();

public:
	TlKey* AddKey(TlKey* key);
	TlKey* InsertKey(TlKey* key, TlKey* after);
	void Clear();

public:
	TlDiagram* Diagram() const			{ return _diagram;		}
	XtUt::WeakAnchor<TlRow>& Anchor()	{ return _anchor;		}
	const Sz16& Label() const			{ return _label;		}
	XtU32 Colour() const				{ return _colour;		}
	bool IsSelected() const				{ return _isSelected;	}

public:
	XtUt::LinkList<TlKey>::Iterator Keys()							{ return XtUt::LinkList<TlKey>::Iterator(&_keys);				}
	XtUt::LinkList<TlKey>::ConstIterator Keys() const				{ return XtUt::LinkList<TlKey>::ConstIterator(&_keys);			}
	XtUt::LinkList<TlKey>::ReverseIterator ReverseKeys()			{ return XtUt::LinkList<TlKey>::ReverseIterator(&_keys);		}
	XtUt::LinkList<TlKey>::ConstReverseIterator ReverseKeys() const	{ return XtUt::LinkList<TlKey>::ConstReverseIterator(&_keys);	}

public:
	void Select();
	void Deselect();

private:
	void SetDirty(bool redoLayout);

private:
	XtUt::WeakAnchor<TlRow>	_anchor;
	TlDiagram*				_diagram;
	XtUt::Link<TlRow>		_linkDiagram;

private:
	XtUt::LinkList<TlKey> _keys;

private:
	TlGroup*			_group;
	XtUt::Link<TlRow>	_linkGroup;

private:
	bool				_isSelected;
	XtUt::Link<TlRow>	_linkSelection;

private:
	Sz16	_label;
	XtU32	_colour;

private:
	float	_y;
	float	_height;

friend class TlKey;
friend class TlGroup;
friend class TlDiagram;
};

//
// TlGroup
//

class TlGroup
{
public:
	TlGroup();
	virtual ~TlGroup();

public:
	void SetLabel(const Sz16& label);
	void SetColour(XtU32 colour);

public:
	void Detach();

public:
	TlRow* AddRow(TlRow* row);
	TlRow* InsertRow(TlRow* row, TlRow* after);
	void Clear();

public:
	TlDiagram* Diagram() const			{ return _diagram;		}
	XtUt::WeakAnchor<TlGroup>& Anchor()	{ return _anchor;		}
	const Sz16& Label() const			{ return _label;		}
	XtU32 Colour() const				{ return _colour;		}

public:
	XtUt::LinkList<TlRow>::Iterator Rows()				{ return XtUt::LinkList<TlRow>::Iterator(&_rows);		}
	XtUt::LinkList<TlRow>::ConstIterator Rows() const	{ return XtUt::LinkList<TlRow>::ConstIterator(&_rows);	}

private:
	void SetDirty(bool redoLayout);

private:
	XtUt::WeakAnchor<TlGroup>	_anchor;
	TlDiagram*					_diagram;
	XtUt::Link<TlGroup>			_linkDiagram;

private:
	XtUt::LinkList<TlRow> _rows;

private:
	Sz16	_label;
	XtU32	_colour;

private:
	float	_y;
	float	_height;

friend class TlKey;
friend class TlRow;
friend class TlDiagram;
};

//
// TlDiagram
//

struct TlRenderContext;

class TlDiagram
{
public:
	TlDiagram(TimelineWnd* timeline);
	~TlDiagram();

public:
	TlRow* AddRow(TlRow* row);
	TlRow* InsertRow(TlRow* row, TlRow* after);
	void ClearRows();

public:
	TlGroup* AddGroup(TlGroup* group);
	TlGroup* InsertGroup(TlGroup* group, TlGroup* after);
	void ClearGroups();

public:
	void ClearAll();

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
	TlHitResult HitTest(const NgVector& pt);
	void ClearSelection();
	void Select(TlRow* row, TlSelectMode mode);
	void Select(TlKey* key, TlSelectMode mode);
	void SelectRect(const NgRect& rect, TlSelectMode mode);
	int RowSelectionCount() const;
	int KeySelectionCount() const;

public:
	void RectSelectionBegin(const NgVector& pt);
	void RectSelectionUpdate(const NgVector& pt);
	void RectSelectionEnd();

public:
	TimelineWnd*	Timeline()		{ return _timeline;		}
	float			HeaderY()		{ return _headerY;		}
	float			SplitterX()		{ return _splitterX;	}
	float			TotalHeight()	{ return _totalHeight;	}
	float			ZoomX()			{ return _zoomX;	}
	float			ScrollX()		{ return _scrollX;	}
	float			ScrollY()		{ return _scrollY;	}
	NgRect RectSelection() const	{ return Normalise(NgRect(_rectSelectionStart, _rectSelectionEnd - _rectSelectionStart)); }

public:
	XtUt::LinkList<TlRow>::Iterator Rows()						{ return XtUt::LinkList<TlRow>::Iterator(&_rows); }
	XtUt::LinkList<TlRow>::ConstIterator Rows() const			{ return XtUt::LinkList<TlRow>::ConstIterator(&_rows); }
	XtUt::LinkList<TlGroup>::Iterator Groups()					{ return XtUt::LinkList<TlGroup>::Iterator(&_groups); }
	XtUt::LinkList<TlGroup>::ConstIterator Groups() const		{ return XtUt::LinkList<TlGroup>::ConstIterator(&_groups); }
	XtUt::LinkList<TlRow>::Iterator SelectionRows()				{ return XtUt::LinkList<TlRow>::Iterator(&_selectionRows); }
	XtUt::LinkList<TlRow>::ConstIterator SelectionRows() const	{ return XtUt::LinkList<TlRow>::ConstIterator(&_selectionRows); }
	XtUt::LinkList<TlKey>::Iterator SelectionKeys()				{ return XtUt::LinkList<TlKey>::Iterator(&_selectionKeys); }
	XtUt::LinkList<TlKey>::ConstIterator SelectionKeys() const	{ return XtUt::LinkList<TlKey>::ConstIterator(&_selectionKeys); }

public:
	NgVector ToRowArea(const NgVector& pt);
	NgVector ToContentArea(const NgVector& pt);

private:
	void DoRowLayout(TlRow* row, float indent, float& rowY, float width);
	void DoRenderBackground(TlRenderContext* ctx, TlRow* row);
	void DoRenderLabel(TlRenderContext* ctx, TlRow* row, float oldBoxY);
	void DoRenderRowShadow(TlRenderContext* ctx, TlRow* parent, TlRow* row, bool selected);
	void DoRenderRow(TlRenderContext* ctx, TlRow* row);
	void SetDirty(bool redoLayout);

private:
	TimelineWnd*			_timeline;
	XtUt::LinkList<TlRow>	_rows;
	XtUt::LinkList<TlGroup>	_groups;
	XtUt::LinkList<TlRow>	_selectionRows;
	XtUt::LinkList<TlKey>	_selectionKeys;

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

friend class TlKey;
friend class TlRow;
friend class TlGroup;
};

//
// Timeline
//

class TimelineWnd : public ChildWindow
{
public:
	TimelineWnd(TimelineContext* context);
	virtual ~TimelineWnd();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	TlDiagram* Diagram() { return _diagram; }
	TimelineContext* Context() { return _context; }

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
	TlDiagram*			_diagram;
	TimelineContext*	_context;

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

friend class TlDiagram;
};

#endif // XTWIN_TIMELINE_H
#ifndef XTWIN_PROPERTYSHEET_H
#define XTWIN_PROPERTYSHEET_H

#ifdef _MSC_VER
#pragma once
#endif

#undef PropertySheet

#include "XtWin/WndCore.h"

class PsControl;
class PsRow;
class PsDiagram;
class PropertySheet;
struct PropertySheetDrawContext;

struct PsHitResult
{
	enum Kind
	{
		kNone,
		kMargin,
		kExpander,
		kLabel,
		kSplitter,
		kContent
	};

public:
	Kind		kind;
	PsRow*		row;
	PsControl*	control;

public:
	PsHitResult(Kind kind_ = kNone, PsRow* row_ = 0, PsControl* control_ = 0) :
		kind(kind_),
		row(row_),
		control(control_)
	{
	}

public:
	operator bool()
	{
		return kind != kNone;
	}
};

class PsControl
{
public:
	PsControl();
	virtual ~PsControl();

public:
	void SetDesiredWidth(int desiredWidth);

public:
	virtual void Clicked();

public:
	virtual void Draw(Gdiplus::Graphics* g, PropertySheetDrawContext* dc, const XwRect& rect);

public:
	PsRow*							Row()			{ return _row;			}
	XtUt::WeakAnchor<PsControl>&	Anchor()		{ return _anchor;		}
	int								DesiredWidth()	{ return _desiredWidth;	}
	int								Width()			{ return _width;		}

protected:
	void SetDirty(bool performLayout);

private:
	XtUt::WeakAnchor<PsControl>	_anchor;
	PsRow*						_row;
	XtUt::Link<PsControl>		_linkRow;

private:
	int _desiredWidth;

private:
	int _x;
	int _width;

friend class PsRow;
friend class PsDiagram;
friend class PropertySheet;
};

class PsRow
{
public:
	PsRow();
	virtual ~PsRow();

public:
	void SetLabel(const Sz16& label);
	void SetHeight(int height);
	void SetExpanded(bool expanded);

public:
	void Detach();

public:
	PsRow* AddRow(PsRow* row);
	PsRow* InsertRow(PsRow* row, PsRow* after);
	void ClearRows();

public:
	PsControl* AddControl(PsControl* control);
	PsControl* InsertControl(PsControl* control, PsControl* after);
	void ClearControls();
	
public:
	virtual void DrawLabel(Gdiplus::Graphics* g, PropertySheetDrawContext* dc, const XwRect& rect);
	virtual void DrawContent(Gdiplus::Graphics* g, PropertySheetDrawContext* dc, const XwRect& rect);

public:
	typedef XtUt::LinkList<PsRow>::Iterator				RowIterator;
	typedef XtUt::LinkList<PsRow>::ReverseIterator		RowReverseIterator;
	typedef XtUt::LinkList<PsControl>::Iterator			ControlIterator;
	typedef XtUt::LinkList<PsControl>::ReverseIterator	ControlReverseIterator;

public:
	PsDiagram*					Diagram()		{ return _diagram;		}
	PsRow*						Parent()		{ return _parent;		}
	XtUt::WeakAnchor<PsRow>&	Anchor()		{ return _anchor;		}
	const Sz16&					Label()			{ return _label;		}
	int							Height()		{ return _height;		}
	int							Indent()		{ return _indent;		}
	RowIterator					Rows()			{ return &_rows;		}
	ControlIterator				Controls()		{ return &_controls;	}
	bool						IsExpanded()	{ return _expanded;		}
	bool						CanExpand();

private:
	void SetDirty(bool performLayout);

private:
	XtUt::WeakAnchor<PsRow>	_anchor;
	PsDiagram*				_diagram;
	PsRow*					_parent;
	XtUt::Link<PsRow>		_linkParent;

private:
	XtUt::LinkList<PsRow>		_rows;
	XtUt::LinkList<PsControl>	_controls;

private:
	Sz16	_label;
	bool	_expanded;

private:
	int		_y;
	int		_height;
	int		_indent;
	int		_bottom;

friend class PsControl;
friend class PsDiagram;
friend class PropertySheet;
};

class PsDiagram
{
public:
	PsDiagram(PropertySheet* sheet);
	~PsDiagram();

public:
	PsRow* SetRootEx(PsRow* root);
	void SetScrollY(int offset);

	template<typename Type> Type* SetRoot(Type* root) { return (Type*)SetRootEx(root); }

public:
	PropertySheet*	Sheet()			{ return _sheet;		}	
	PsRow*			Root()			{ return _root;			}
	XwVector		ClientSize()	{ return _clientSize;	}
	int				SplitterX()		{ return _splitterX;	}
	int				ScrollY()		{ return _scrollY;		}

private:
	PsHitResult HitTestSub(PsRow* row, const XwVector& pt);

public:
	PsHitResult HitTest(const XwVector& pt);

public:
	XwVector ToCanvas(const XwVector& pt);
	XwVector ToClient(const XwVector& pt);

private:
	void LayoutSub(PsRow* row, int& y, int indent);
	void Layout();
	void RenderSub(Gdiplus::Graphics* g, PropertySheetDrawContext* dc, PsRow* parent);
	void Render(Gdiplus::Graphics* g, PropertySheetDrawContext* dc);
	void SetDirty(bool performLayout);

private:
	PropertySheet* _sheet;

private:
	PsRow*		_root;
	XwVector	_clientSize;
	int			_splitterX;
	bool		_layoutDirty;
	int			_scrollY;

friend class PsControl;
friend class PsRow;
friend class PropertySheet;
};

class PropertySheet : public ChildWindow
{
public:
	PropertySheet();
	virtual ~PropertySheet();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	PsDiagram* Diagram() { return &_diagram; }

private:
	void DoDrag(const WinPoint& pt);
	void DoSetCapture();
	void DoReleaseCapture();
	void DoCursor(const XwVector& pt);
	void DoScrollBars();
	void DoScrollPos();

private:
	PsDiagram _diagram;

private:
	enum DragState
	{
		kDragNone,
		kDragIgnore,
		kDragLabel,
		kDragSplitter
	};

private:
	DragState	_dragging;
	WinPoint	_dragPt;
	WinPoint	_dragStart;
	DWORD		_dragTime;
	bool		_dragCommitted;

friend class PsControl;
friend class PsRow;
friend class PsDiagram;
};

class PsEdit : public PsControl
{
public:
	PsEdit();
	virtual ~PsEdit();

public:
	void SetText(const Sz16& text);

public:
	const Sz16& Text() const { return _text; }

public:
	virtual void Clicked();

public:
	virtual void Draw(Gdiplus::Graphics* g, PropertySheetDrawContext* dc, const XwRect& rect);

private:
	Sz16 _text;
};

class PsButton : public PsControl
{
public:
	PsButton();
	virtual ~PsButton();

public:
	void SetText(const Sz16& text);

public:
	const Sz16& Text() const { return _text; }

public:
	virtual void Clicked();

public:
	virtual void Draw(Gdiplus::Graphics* g, PropertySheetDrawContext* dc, const XwRect& rect);

private:
	Sz16 _text;
};

#endif // XTWIN_PROPERTYSHEET_H

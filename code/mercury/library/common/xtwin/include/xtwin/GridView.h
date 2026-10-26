#ifndef XTWIN_GRIDVIEW_H
#define XTWIN_GRIDVIEW_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin\WndCore.h"
#include "XtWin\NgMaths.h"

class GridColumn;
class GridRow;
class GridView;

struct GridViewDrawContext;

//
// GridColumn
//

class GridColumn
{
public:
	GridColumn();
	virtual ~GridColumn();

public:
	void SetWidth(float width);
	void SetLabel(const Sz16& label);

public:
	GridView* View()			{ return _view;		}
	float Width() const			{ return _width;	}
	const Sz16& Label() const	{ return _label;	}

private:
	GridView*	_view;

private:
	float	_width;
	Sz16	_label;

private:
	float _x;
	float _cx;

friend class GridRow;
friend class GridView;
};

//
// GridRow
//

class GridRow
{
public:
	GridRow();
	virtual ~GridRow();

public:
	virtual Sz16 Content(GridColumn* column);

public:
	virtual void KeyPressed(GridColumn* column, int key);
	virtual void DoubleClicked(GridColumn* column);

public:
	void SetHeight(float height);
	void SetLabel(const Sz16& label);

public:
	GridView* View()			{ return _view;		}
	float Height() const		{ return _height;	}
	const Sz16& Label() const	{ return _label;	}

private:
	GridView*	_view;

private:
	float	_height;
	Sz16	_label;

private:
	float _y;
	float _cy;

friend class GridColumn;
friend class GridView;
};

//
// GridViewContext
//

class GridViewContext
{
public:
	GridViewContext();
	virtual ~GridViewContext();

public:
	virtual void ContextMenu(const WinPoint& pt);

protected:
	GridView* _grid;

friend class GridView;
};

//
// GridHitResult
//

struct GridHitResult
{
	enum Kind
	{
		kNone,
		kColumnHeader,
		kColumnSplitter,
		kRowHeader,
		kRowEdge,
		kCells
	};

public:
	Kind	kind;
	int		column;
	int		row;

public:
	GridHitResult(Kind kind_, int column_, int row_) : kind(kind_), column(column_), row(row_) { }
};

//
// GridView
//

class GridView : public ChildWindow
{
public:
	GridView();
	virtual ~GridView();

public:
	void SetContext(GridViewContext* context);

public:
	void EnsureVisibleColumn(int x);
	void EnsureVisibleRow(int y);
	void EnsureVisible(int x, int y);

public:
	void Clear();

public:
	int			ColumnCount();
	GridColumn*	ColumnAt(int x);
	GridColumn*	ColumnAppend(GridColumn* column);
	GridColumn*	ColumnInsert(int before, GridColumn* column);
	void		ColumnRemove(int x);

public:
	int			RowCount();
	GridRow*	RowAt(int y);
	GridRow*	RowAppend(GridRow* row);
	GridRow*	RowInsert(int before, GridRow* row);
	void		RowRemove(int y);

public:
	void SetColumnHeaderWidth(float width);
	void SetRowHeaderHeight(float height);

public:
	int FocusX() const		{ return _focusX; }
	int FocusY() const		{ return _focusY; }
	int SelectX0() const	{ return _selectX0; }
	int SelectY0() const	{ return _selectY0; }
	int SelectX1() const	{ return _selectX1; }
	int SelectY1() const	{ return _selectY1; }

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void DoVirtualLayout();
	void DoLayout();
	void DoScrollBars();

private:
	void OnPaint(Gdiplus::Graphics* g);
	void DrawHeader(Gdiplus::Graphics* g, GridViewDrawContext* dc, const Sz16& label, const NgRect& rc, bool selected);
	void DrawCell(Gdiplus::Graphics* g, GridViewDrawContext* dc, GridRow* row, GridColumn* column, const NgRect& rc);

private:
	void DoDrag(const WinPoint& pt);
	void DoPan(const WinPoint& pt);
	void DoSetCapture();
	void DoReleaseCapture();

private:
	enum DragState
	{
		kDragNone,
		kDragIgnore,
		kDragSelectColumns,
		kDragSelectRows,
		kDragSelectCells,
		kDragRowEdge,
		kDragColumnSplitter
	};

private:
	GridHitResult HitTest(const WinPoint& pt);
	int HitTestColumn(int x);
	int HitTestRow(int y);

private:
	void DoCursor();

private:
	GridViewContext* _context;

private:
	XtUt::List<GridColumn>	_columns;
	XtUt::List<GridRow>		_rows;

private:
	float _widthHeader;
	float _widthTotal;
	float _heightHeader;
	float _heightTotal;

private:
	float _scrollX;
	float _scrollY;

private:
	int _selectX0;
	int _selectX1;
	int _selectY0;
	int _selectY1;

private:
	int _focusX;
	int _focusY;

private:
	DragState	_dragState;
	WinPoint	_dragStart;
	WinPoint	_dragLast;
	DWORD		_dragTime;
	bool		_dragCommitted;
	int			_dragColumn;

private:
	bool		_panning;
	WinPoint	_panStart;
	WinPoint	_panLast;
	DWORD		_panTime;
	bool		_panCommitted;

friend class GridRow;
friend class GridColumn;
};

#endif // XTWIN_GRIDVIEW_H
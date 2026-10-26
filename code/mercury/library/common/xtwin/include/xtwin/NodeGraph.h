#ifndef XTWIN_NODEGRAPH_H
#define XTWIN_NODEGRAPH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin/WndCore.h"
#include "XtWin/NgMaths.h"

class NgConnection;
class NgPin;
class NgNode;
class NgDiagram;
class NodeGraph;

struct NgRenderContext;

//
// NgSelectMode
//

enum NgSelectMode
{
	kNgSelectSingle,
	kNgSelectAdd,
	kNgSelectRemove,
	kNgSelectToggle
};

//
// NgAlign
//

enum NgAlign
{
	kNgAlignLeft,
	kNgAlignRight
};

// NgVirtual

enum NgVirtual
{
	kNgVirtualNone,
	kNgVirtualFrom,
	kNgVirtualTo,
	kNgVirtualBoth
};

//
// NgConnection
//

class NgConnection
{
public:
	NgConnection();
	virtual ~NgConnection();

public:
	virtual bool IsConflicted();
	virtual bool IsHot();

public:
	XtUt::WeakAnchor<NgConnection>& Anchor() { return _anchor; }
	NgDiagram* Diagram()	{ return _diagram;	}
	NgPin* From()			{ return _from;		}
	NgPin* To()				{ return _to;		}
	NgVirtual Virtual()		{ return _virtual;	}

public:
	void SetColour(XtU32 colour);
	void SetVirtual(NgVirtual v);

public:
	bool SetFrom(NgPin* from);
	bool SetTo(NgPin* to);

public:
	XtU32 Colour() const { return _colour; }

private:
	XtUt::WeakAnchor<NgConnection>	_anchor;
	NgDiagram*						_diagram;
	XtUt::Link<NgConnection>		_linkDiagram;
	NgPin*							_from;
	XtUt::Link<NgConnection>		_linkTo;
	NgPin*							_to;
	XtUt::Link<NgConnection>		_linkFrom;

private:
	XtU32		_colour;
	NgVirtual	_virtual;

friend class NgPin;
friend class NgNode;
friend class NgDiagram;
};

//
// NgPin
//

class NgPin
{
public:
	NgPin();
	virtual ~NgPin();

public:
	virtual NgPin* OnDrag();
	virtual bool OnTryConnect(NgPin* to);
	virtual void OnConnect(NgPin* to);

public:
	XtUt::WeakAnchor<NgPin>& Anchor()	{ return _anchor;	}
	NgNode* Node()						{ return _node;		}
	NgDiagram* Diagram();

public:
	void SetLabel(const Sz16& label);
	void SetColour(XtU32 colour);
	void SetAlign(NgAlign align);
	void SetFromArrow(bool enabled);
	void SetToArrow(bool enabled);

public:
	const Sz16& Label() const	{ return _label;		}
	XtU32 Colour() const		{ return _colour;		}
	NgAlign Align() const		{ return _align;		}
	bool FromArrow() const		{ return _fromArrow;	}
	bool ToArrow() const		{ return _toArrow;		}
	int ConnectionCount() const;

public:
	XtUt::LinkList<NgConnection>::Iterator Connections()			{ return XtUt::LinkList<NgConnection>::Iterator(&_connections);			}
	XtUt::LinkList<NgConnection>::ConstIterator Connections() const	{ return XtUt::LinkList<NgConnection>::ConstIterator(&_connections);	}

private:
	void SetDirty(bool redoLayout);

private:
	void Measure(NgRenderContext* context);
	NgAlign NodeAlign() const;

private:
	XtUt::WeakAnchor<NgPin>	_anchor;
	NgNode*					_node;
	XtUt::Link<NgPin>		_link;

private:
	XtUt::LinkList<NgConnection> _connections;

private:
	NgRect		_location;
	Sz16		_label;
	XtU32		_colour;
	NgAlign		_align;
	bool		_fromArrow;
	bool		_toArrow;

private:
	NgVector	_labelSize;
	NgVector	_centre;
	NgRect		_selectRect;

friend class NgConnection;
friend class NgNode;
friend class NgDiagram;
};

//
// NgNode
//

class NgNode
{
public:
	NgNode();
	virtual ~NgNode();

public:
	virtual void OnPositionChanged();
	virtual void OnDoubleClicked();

public:
	XtUt::WeakAnchor<NgNode>& Anchor()	{ return _anchor;	}
	NgDiagram* Diagram()				{ return _diagram;	}

public:
	void SetPosition(const NgVector& pt);
	void SetLabel(const Sz16& label);
	void SetColour(XtU32 colour);
	void SetMirrored(bool mirrored);
	void SetCompact(bool compact);

public:
	NgPin* AddPin(NgPin* pin);
	NgPin* InsertPin(NgPin* pin, NgPin* after);

public:
	void Select();
	void Deselect();

public:
	void BringToTop();
	void SendToBottom();
	void MoveBy(const NgVector& offset);

public:
	bool IsBehind(NgNode* node);

public:
	const NgRect&	Location() const	{ return _location;		}
	const Sz16&		Label() const		{ return _label;		}
	bool			IsSelected() const	{ return _isSelected;	}
	XtU32			Colour() const		{ return _colour;		}
	bool			Mirrored() const	{ return _mirrored;		}
	bool			Compact() const		{ return _compact;		}
	int				PinCount() const;

public:
	XtUt::LinkList<NgPin>::Iterator Pins()				{ return XtUt::LinkList<NgPin>::Iterator(&_pins);		}
	XtUt::LinkList<NgPin>::ConstIterator Pins() const	{ return XtUt::LinkList<NgPin>::ConstIterator(&_pins);	}

private:
	void SetDirty(bool redoLayout);
	void Layout(NgRenderContext* context);

private:
	XtUt::WeakAnchor<NgNode>	_anchor;
	NgDiagram*					_diagram;
	XtUt::Link<NgNode>			_linkDiagram;

private:
	bool				_isSelected;
	XtUt::Link<NgNode>	_linkSelection;

private:
	bool	_isDirty;
	NgRect	_location;
	Sz16	_label;
	XtU32	_colour;
	bool	_mirrored;
	bool	_compact;

private:
	bool	_drawSplit;
	float	_splitAt;

private:
	XtUt::LinkList<NgPin> _pins;

friend class NgConnection;
friend class NgPin;
friend class NgDiagram;
};

//
// NgDiagram
//

class NgDiagram
{
public:
	NgDiagram(NodeGraph* graph);
	virtual ~NgDiagram();

public:
	NodeGraph* Graph();

public:
	NgNode* AddNode(NgNode* node);
	NgConnection* AddConnection(NgConnection* connection);
	void Clear();

public:
	void Pan(NgVector delta);
	void Zoom(NgVector pt, float factor);
	void Frame();

public:
	void SetClientRect(const NgRect& rect);
	void SetTransformOffset(const NgVector& offset);
	void SetTransformScale(const NgVector& scale);
	void SetHotPin(NgPin* hot);
	void SetWatermark(const Sz16& watermark);
	void SetDirty(bool redoLayout);

public:
	void Layout();
	void Render(class Gdiplus::Graphics* g);

public:
	NgVector ToCanvas(const NgVector& clientPt);
	NgVector ToClient(const NgVector& canvasPt);

public:
	NgNode* HitTestNode(const NgVector& canvasPt);
	NgPin* HitTestPin(const NgVector& canvasPt);

public:
	void ClearSelection();
	void Select(NgNode* node, NgSelectMode mode);
	void SelectRect(const NgRect& rect, NgSelectMode mode);

public:
	void MoveSelectedBy(const NgVector& offset);

public:
	void RectSelectionBegin(const NgVector& pt);
	void RectSelectionUpdate(const NgVector& pt);
	void RectSelectionEnd();

public:
	void ConnectionDragBegin(NgPin* from, NgPin* to);
	void ConnectionDragUpdate(NgVector& pt);
	void ConnectionDragUpdate(NgPin* to);
	void ConnectionDragEnd();

public:
	const NgVector& TransformOffset() const	{ return _transformOffset;		}
	const NgVector& TransformScale() const	{ return _transformScale;		}
	bool IsRectSelectionActive() const		{ return _rectSelectionActive;	}
	NgRect RectSelection() const			{ return Normalise(NgRect(_rectSelectionStart, _rectSelectionEnd - _rectSelectionStart)); }
	bool IsConnectionDragActive() const		{ return _connectionDragActive;	}
	NgPin* ConnectionDragFrom()				{ return _connectionDragFrom;	}
	NgPin* ConnectionDragTo()				{ return _connectionDragTo;		}
	NgPin* Hot()							{ return _hot;					}
	int SelectionCount() const;
	XtUt::LinkList<NgNode>::Iterator Selection()			{ return XtUt::LinkList<NgNode>::Iterator(&_selection); }
	XtUt::LinkList<NgNode>::ConstIterator Selection() const	{ return XtUt::LinkList<NgNode>::ConstIterator(&_selection); }

private:
	NodeGraph*				_graph;
	XtUt::LinkList<NgNode>	_nodes;
	XtUt::LinkList<NgNode>	_selection;

private:
	XtUt::LinkList<NgConnection> _connections;

private:
	NgRect		_clientRect;
	NgVector	_clientCentre;

private:
	NgVector	_transformOffset;
	NgVector	_transformScale;

private:
	bool		_rectSelectionActive;
	NgVector	_rectSelectionStart;
	NgVector	_rectSelectionEnd;

private:
	bool					_connectionDragActive;
	XtUt::WeakRef<NgPin>	_connectionDragFrom;
	XtUt::WeakRef<NgPin>	_connectionDragTo;
	NgVector				_connectionDragPt;

private:
	XtUt::WeakRef<NgPin>	_hot;
	Sz16					_watermark;

private:
	bool _isDirty;

friend class NgConnection;
friend class NgPin;
friend class NgNode;
};

//
// NodeGraphContext
//

class NodeGraphContext
{
public:
	virtual ~NodeGraphContext();

public:
	virtual void ContextMenu(const WinPoint& pt);

public:
	virtual void SelectionChanged();
	virtual void CheckPoint();

public:
	virtual void Undo();
	virtual void Redo();
	virtual void Delete();
	virtual void Cut();
	virtual void Copy();
	virtual void Paste(const NgVector& targetPt);
};

//
// NodeGraph window
//

class NodeGraph : public ChildWindow
{
public:
	NodeGraph(NodeGraphContext* context);
	virtual ~NodeGraph();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	inline NgDiagram* Diagram()
	{
		return &_diagram;
	}

	inline NodeGraphContext* Context()
	{
		return _context;
	}

private:
	void DoPan(const WinPoint& pt);
	void DoDrag(const WinPoint& pt);

public:
	void DoSetCapture();
	void DoReleaseCapture();

private:
	enum DragState
	{
		kDragNone,
		kDragIgnore,
		kDragSelection,
		kDragMoveNodes,
		kDragPin,
		kDragIgnorePin
	};

private:
	NgDiagram			_diagram;
	NodeGraphContext*	_context;

private:
	bool		_panning;
	bool		_panCommitted;
	DWORD		_panTime;
	DragState	_dragging;
	bool		_dragCommitted;
	DWORD		_dragTime;
	bool		_zoomIn;
	bool		_zoomOut;

private:
	WinPoint _panStart;
	WinPoint _panLast;
	WinPoint _dragStart;
	WinPoint _dragLast;
};

#endif // XTWIN_NODEGRAPH_H
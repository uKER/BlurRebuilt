#ifndef NODE_GRAPH_H
#define NODE_GRAPH_H

#define wxUSE_GENERIC_DRAGIMAGE 1

class C_NodeGraph;
class I_Node;
class wxTreeItemId;

typedef Ut::UtVector<Ut::DynamicVector<I_Node*> > Nodes;

// use wxGenericeDragImage on Windows
#if wxUSE_GENERIC_DRAGIMAGE
	#include "wx/generic/dragimgg.h"
	#define wxDragImage wxGenericDragImage
	class DragImage : public wxDragImage
	{
	public:
		DragImage(C_NodeGraph *pSchematic, I_Node *node, const wxBitmap& image, const wxCursor& cursor = wxNullCursor)
			: wxGenericDragImage(image, cursor), m_pSchematic(pSchematic), m_pNode(node) {};

		bool UpdateBackingFromWindow(wxDC& windowDC, wxMemoryDC& destDC, const wxRect& sourceRect, const wxRect& destRect) const;

		bool ForceShow();
	private:
		C_NodeGraph	*m_pSchematic;
		I_Node	*m_pNode;
	};
#else
	#include "wx/frame.h"
#endif

// C_NodeGraph - the main gui area for nodes of a particle event

enum E_DragMode
{
	E_DragMode_None,
	E_DragMode_ShapeStart,
	E_DragMode_ShapeDragging,
	E_DragMode_CanvasStart,
	E_DragMode_CanvasDragging
};

class C_NodeGraph : public wxWindow
{
public:
	C_NodeGraph( wxWindow *parent, wxWindowID, const wxPoint &pos, const wxSize &size);
    ~C_NodeGraph();

	// Event functions

    void			OnPaint( wxPaintEvent &event );
    void			OnEraseBackground(wxEraseEvent& event);
	void			OnSize(wxSizeEvent& event);

	void			OnMouseEvent(wxMouseEvent& event);
	void			OnMouseCaptureLostEvent(wxMouseCaptureLostEvent& event);

	void			Clean(I_Node* pNode);
	virtual void	Initialise();
	virtual void	LayoutNodes();

	bool			SelectNode(I_Node* pSelect);

	friend class DragImage;

protected:
	// Drawing and erasing commands
	void			DrawBuffered();
	void			DrawNode(const I_Node* pNode);

	void			Paint(wxDC& dc);
	void			DrawGrid(wxDC& dc) const;
	void			DrawNodes(wxDC& dc, bool bDrawDragSrc) const;

	// virtuals for dervied class to override

	virtual	void	Draw(wxDC& dc) const;
	virtual void	SelectionChanged(I_Node* pOld, I_Node* pNew);
	
	

	// network manipulation commands
	void			MoveNodes(wxPoint move);
	void			EndDrag(wxPoint logicalPosition);
	void			BringToFront(I_Node* pNode);
	void			DeleteNodes();

	// Utility functions

	I_Node*			FindDraggable(const wxPoint& pt, I_Node* exclude = NULL);
	
	// Menus
	void			ShowMenu(const wxPoint& pt);
	    
	// Variables

	E_DragMode		m_dragMode;
    I_Node*			m_pDragNode;
	I_Node*			m_pSelectedNode;
    I_Node*			m_pTargetNode;
    
	wxPoint			m_MousePosition;
	wxPoint			m_DragPosition;
	
	wxPoint			m_gridOffset;
    DragImage*		m_dragImage;

	Nodes			m_Nodes;

    DECLARE_CLASS(C_NodeGraph)
    DECLARE_EVENT_TABLE()
};

#endif // NODE_GRAPH_H

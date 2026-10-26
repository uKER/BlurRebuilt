#ifndef NODE_GRAPH_NODE_INTERFACE_H
#define NODE_GRAPH_NODE_INTERFACE_H

#pragma once

class I_Node;
class C_NodeGraph;
class I_NodeVisitor;


class I_Node : public wxObject
{
public:

	I_Node();
	I_Node(S32 type);
	virtual ~I_Node();

	// virtual interface
	virtual I_Node*		HitTest(wxPoint pt, I_Node* pExclude);
	virtual bool		MouseClick() { return false; }
	virtual void		PrepareDC(wxDC& dc) const {}
    virtual void		Draw(wxDC& dc) const = 0;
	virtual bool		EndDrag(I_Node* target, wxPoint point) { return false; }
	virtual	bool		SetSelected(const bool bSelected);
	virtual bool		SetDragSrc(const bool bDragSrc);
	virtual bool		SetDragDest(const bool bDragDest);
	virtual void		BringToFront(I_Node* pNode) {};
	virtual	void		Close();

	S32					GetType() const { return m_iType; }
	bool				GetSelected() const { return m_bSelected; }
	bool				GetDragSrc() const { return m_bDragSrc; }
	bool				GetDragDest() const { return m_bDragDest; }
	bool				GetDragRedraw() const { return m_bDragRedraw; }
	
	wxPoint				GetPosition() const;
	wxPoint				GetPositionAbsolute() const;
	wxSize				GetSize() const { return m_Size; }
	wxRect				GetRect() const { return wxRect(m_Position, m_Size); }
   
    void				SetPosition(const wxPoint& position) { m_Position = position; }
	void				SetPositionAbsolute(const wxPoint& position);
	void				SetSize(const wxSize& size) { m_Size = size; }

	 
	void				GenerateBitmap();
	wxBitmap&			GetBitmap() const { return (wxBitmap&) m_Bitmap; }

	void				SetParent(I_Node* pParent) { m_pParent = pParent; }
	const I_Node*		GetParent() const { return m_pParent; };
	
	void				ToggleVisible() { m_bVisible = !m_bVisible; }
	bool				IsVisible() const { return m_bVisible; }
	bool				IsDraggable() const { return m_bDraggable; }
	bool				GetDrawConnection() const { return m_bDrawConnection; }

	C_NodeGraph*		GetGraph() { return m_pGraph; }

	virtual void		Accept(I_NodeVisitor* pVisitor);

	template<class T> friend class I_ParentNode;

protected:
	void				Clear();

	wxPoint				m_Position;
	wxSize				m_Size;
	wxBitmap			m_Bitmap;

	const S32			m_iType;

	bool				m_bVisible;
	bool				m_bDraggable;
	bool				m_bDrawConnection;
	bool				m_bDrawChildren;

	bool				m_bDragSrc;
	bool				m_bDragDest;
	bool				m_bDragRedraw;
	bool				m_bSelected;

	C_NodeGraph*		m_pGraph;
	I_Node*				m_pParent;
};

#endif // NODE_GRAPH_NODE_INTERFACE_H

#ifndef RECORD_GRAPH_GENERIC_NODE_H
#define RECORD_GRAPH_GENERIC_NODE_H

#pragma once

#include "src/ParentNodeInterface.h"
#include "Xt/XtTool.h"
#include "Type.h"


namespace RecordGraph
{

class I_RecordGraphVisitor;

enum E_NodeAlignment
{
	E_NodeAlignment_Vertical,
	E_NodeAlignment_Horizontal
};

// similar to ScarabNode
class C_GenericNode : public I_ParentNode<C_GenericNode>
{
public:

	C_GenericNode() : I_ParentNode(E_NodeType_Generic), m_bBuilt(false), m_Alignment(E_NodeAlignment_Vertical) { };
	C_GenericNode(S32 type) : I_ParentNode(type), m_bBuilt(false), m_Alignment(E_NodeAlignment_Vertical) { };
	virtual ~C_GenericNode() {};

	virtual void			PrepareDC(wxDC& dc) const;
    virtual void			Draw(wxDC& dc) const;
	
	virtual bool			EndDrag(I_Node* pTarget, wxPoint point);

	C_GenericNode*			AddChild(C_GenericNode* pNode);
	C_GenericNode*			GetChild(U32 index);

	C_GenericNode*			GetParent() { return (C_GenericNode*)m_pParent; }
	//S32						GetNumChildren() const { return I_ParentNode<C_GenericNode>::GetNumChildren(); } 
	

	void					SetColour(const wxColour& colour) { m_Colour = colour; }
	E_NodeAlignment			GetAlignment() const { return m_Alignment; }

	virtual void			Accept(I_RecordGraphVisitor* pVisitor);

public:
	bool Populate();
	bool Build();
	void Bind(Xt::DomField* parentDom);
	void Bind(Xt::DomBlock* parentDom);
	void Bind(Xt::DomArray* parentDom, int index);
	void Layout(S32 index);
	void Layout2(S32 index);

protected:

	virtual bool OnPopulate();
	virtual bool OnBuild();
	virtual void OnBind(Xt::DomField* parentDom);
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual void OnLayout(S32 index);
	virtual void OnLayout2(S32 index);

	wxColour				m_Colour;
	E_NodeAlignment			m_Alignment;
	bool					m_bBuilt;
	
};

bool BuildNode(C_GenericNode* pParentNode, LarvaField* pField);

} // RecordGraph

#endif // RECORD_GRAPH_GENERIC_NODE_H

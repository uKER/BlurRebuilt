#ifndef NODE_GRAPH_NODE_EXAMPLE_H
#define NODE_GRAPH_NODE_EXAMPLE_H

#pragma once

#include "ParentNodeInterface.h"

class C_NodeExample : public I_ParentNode<C_NodeExample>
{
public:

	C_NodeExample() : I_ParentNode(0) { };
	C_NodeExample(S32 type) : I_ParentNode(type) { };
    virtual ~C_NodeExample();

	virtual void			PrepareDC(wxDC& dc) const;
    virtual void			Draw(wxDC& dc) const;
	
	virtual bool			EndDrag(I_Node* pTarget, wxPoint point);

	void					SetPositions();

	
protected:
	wxString				m_Title;

private:

};

#endif // NODE_GRAPH_NODE_EXAMPLE_H


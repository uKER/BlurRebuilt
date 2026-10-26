#ifndef RECORD_GRAPH_RECORD_GRAPH_H
#define RECORD_GRAPH_RECORD_GRAPH_H

#include "../NodeGraph.h"

namespace Xt
{
	struct DomRecord;
	struct DomBlock;
}

struct LarvaType;

class C_Record;

namespace RecordGraph
{

class C_GenericNode;

class C_Graph : public C_NodeGraph
{
public:
	C_Graph( wxWindow *parent, wxWindowID, const wxPoint &pos, const wxSize &size, C_Record* pRecord);
    ~C_Graph();

	virtual	void		Draw(wxDC& dc) const;

	C_Record*			GetRecord() { return m_pRecord; }

	void				Initialise();
	void				LoadRecord(Xt::DomRecord* pRecord);
	void				LoadBlock(Xt::DomBlock* pBlock, LarvaType* pType);
	void				LayoutNodes();

	C_GenericNode*		GetRootNode();

private:

	C_Record*			m_pRecord;
	I_Node*				m_pSelection;

    DECLARE_CLASS(C_Graph)
    DECLARE_EVENT_TABLE()
};

} // RecordGraph

#endif // RECORD_GRAPH_RECORD_GRAPH_H

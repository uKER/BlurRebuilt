#ifndef XT_EDITOR_EVENT_RECORD_H
#define XT_EDITOR_EVENT_RECORD_H

#include "Record.h"

class wxAuiNotebook;

class wxTreeItemId;

namespace Xt
{
	struct DomElement;
}

namespace RecordGraph
{
	class C_Graph;
}

// generic record is a record with an item in the tree view, a record graph and form
class C_GenericRecord : public C_Record
{
public:
	C_GenericRecord(Xt::DomRecord* pDomRecord, DynamicForm::C_Form* pForm) : C_Record(pDomRecord, pForm), m_pExplorerItemId(NULL), m_pGraph(NULL) {}
	~C_GenericRecord();

	bool						Create();
	void						Destroy();
	
	bool						Open();
	void						Close();
	void						Select();

protected:

	void						OpenGraph();
	void						CloseGraph();

	wxTreeItemId*				m_pExplorerItemId;
	RecordGraph::C_Graph*		m_pGraph;
	
};

#endif // XT_EDITOR_EVENT_RECORD_H
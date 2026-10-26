#ifndef RECORD_GRAPH_STRUCT_NODE_H
#define RECORD_GRAPH_STRUCT_NODE_H

#pragma once

#include "Generic.h"
#include "../../Common/WeakInterface.h"

class C_NodeGraph;

namespace RecordGraph
{

class C_StructNode : public C_GenericNode, public I_WeakInterface
{
public:

	C_StructNode() : C_GenericNode(E_NodeType_Struct) {};
	C_StructNode(LfStruct* pStruct, C_NodeGraph* pGraph = 0);
	virtual ~C_StructNode();

	virtual bool SetSelected(const bool bSelected);

	const LfStruct*		GetLfStruct() const { return m_pStruct; }
	Xt::DomMonitor&		GetDomMonitor() { return m_DomMonitor; }

	void			RefreshControls();

protected:
	bool			OnPopulate();
	void			OnBind(Xt::DomBlock* parentDom);
	void			OnBind(Xt::DomArray* parentDom, int index);
	void			OnLayout(S32 index);
	

	void			AddWeakPtr(LfWeakPtr* pWeakPtr, Xt::DomField* field);

	virtual bool	EndDrag(I_Node* pTarget, wxPoint point);

	Xt::DomMonitor		m_DomMonitor;
	LfStruct*			m_pStruct;
};

} // RecordGraph

#endif // RECORD_GRAPH_STRUCT_NODE_H

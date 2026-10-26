#ifndef RECORD_GRAPH_ARRAY_NODE_H
#define RECORD_GRAPH_ARRAY_NODE_H

#pragma once

#include "Generic.h"

namespace RecordGraph
{

enum E_ArrayLayout
{
	E_ArrayLayout_Horizontal,
	E_ArrayLayout_Vertical
};

enum E_ArrayAttachment
{
	E_ArrayAttachment_Floating,
	E_ArrayAttachment_Fixed
};

enum E_ArrayItemConnections
{
	E_ArrayItemConnections_Enabled,
	E_ArrayItemConnections_Disabled
};

class C_ArrayNode : public C_GenericNode
{
public:

	C_ArrayNode() : C_GenericNode(E_NodeType_Array) {};
	C_ArrayNode(LfArray* pLfArray);
	virtual ~C_ArrayNode();

	
	friend class C_ArrayItemNode;

	void					Add();
	void					Remove(S32 index);
	void					ToggleHideChildren();
	static void				DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_ArrayNode* pNode);

	E_ArrayLayout			GetLayout() const { return m_Layout; }
	E_ArrayAttachment		GetAttachment() const { return m_Attachment; }
	E_ArrayItemConnections	GetItemConnections() const { return m_ItemConnections; }

protected:
	bool					OnPopulate();
	void					OnBind(Xt::DomBlock* parentDom);
	void					OnBind(Xt::DomArray* parentDom, S32 iIndex);
	void					OnLayout(S32 index);

	void					SetDom(Xt::DomArray* pArray);
	void					PopulateBody();
	void					RefreshControls();

	virtual bool			EndDrag(I_Node* pTarget, wxPoint point);

	Xt::DomMonitor			m_DomMonitor;
	LfArray*				m_pLfArray;
	E_ArrayLayout			m_Layout;
	E_ArrayAttachment		m_Attachment;
	E_ArrayItemConnections	m_ItemConnections;
	bool					m_Binding;
};

} // RecordGraph

#endif // RECORD_GRAPH_ARRAY_NODE_H

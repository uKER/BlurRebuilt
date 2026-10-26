
#include "stdafx.h"
#include "Xt/XtTool.h"
#include "Nodes/Nodes.h"
#include "Nodes/Custom/Factory.h"

namespace RecordGraph
{

struct C_GraphBuilder : public LvField
{
	C_GenericNode*	m_pParentNode;

public:
	C_GraphBuilder(C_GenericNode* pParentNode);
	virtual ~C_GraphBuilder();

public:
	virtual bool Visit(struct LfStruct* field);
	virtual bool Visit(struct LfInt* field);
	virtual bool Visit(struct LfFloat* field);
	virtual bool Visit(struct LfString* field);
	virtual bool Visit(struct LfBool* field);
	virtual bool Visit(struct LfEnum* field);
	virtual bool Visit(struct LfFlags* field);
	virtual bool Visit(struct LfPtr* field);
	virtual bool Visit(struct LfWeakPtr* field);
	virtual bool Visit(struct LfHandle* field);
	virtual bool Visit(struct LfArray* field);
	virtual bool Visit(struct LfLocString* field);
	virtual bool Visit(struct LfControl* field);
};

//
// C_GraphBuilder
//

C_GraphBuilder::C_GraphBuilder(C_GenericNode* pParentNode) :
	m_pParentNode(pParentNode)
{
}

C_GraphBuilder::~C_GraphBuilder()
{
}

bool C_GraphBuilder::Visit(struct LfStruct* field)
{
	if(m_pParentNode->GetType() != E_NodeType_Struct)
	{
		C_StructNode* pStruct = C_CustomNodeFactory::Instance().GetNode(field);

		wxPoint point;
		point.x = m_pParentNode->GetNumChildren() * (64 + 10);
		point.y = m_pParentNode->GetSize().GetHeight() + 40;
		pStruct->SetPosition(point);

		m_pParentNode->AddChild(pStruct);
	}

	return true;
}

bool C_GraphBuilder::Visit(struct LfInt* field)
{
	return true;
}

bool C_GraphBuilder::Visit(struct LfFloat* field)
{
	return true;
}

bool C_GraphBuilder::Visit(struct LfString* field)
{
	return true;
}

bool C_GraphBuilder::Visit(struct LfBool* field)
{
	return true;
}

bool C_GraphBuilder::Visit(struct LfEnum* field)
{
	return true;
}

bool C_GraphBuilder::Visit(struct LfFlags* field)
{
	return true;
}

bool C_GraphBuilder::Visit(struct LfPtr* field)
{
	return true;
}

bool C_GraphBuilder::Visit(struct LfWeakPtr* field)
{
	C_WeakPtrNode* pWeakPtr = new C_WeakPtrNode(field);
	m_pParentNode->AddChild(pWeakPtr);

	return true;
}

bool C_GraphBuilder::Visit(struct LfHandle* field)
{
	return true;
}

bool C_GraphBuilder::Visit(struct LfArray* field)
{
	C_ArrayNode* pArray = new C_ArrayNode(field);

	wxPoint point;
	point.x = m_pParentNode->GetNumChildren() * (64 + 10);
	point.y = m_pParentNode->GetSize().GetHeight() + 40;
	pArray->SetPosition(point);

	m_pParentNode->AddChild(pArray);

	return true;
}

bool C_GraphBuilder::Visit(struct LfLocString* field)
{
	return true;
}

bool C_GraphBuilder::Visit(struct LfControl* field)
{
	return true;
}

//
// BuildNode
//

bool BuildNode(C_GenericNode* pParentNode, LarvaField* pField)
{
	C_GraphBuilder build(pParentNode);
	return pField->Accept(&build);
}

}
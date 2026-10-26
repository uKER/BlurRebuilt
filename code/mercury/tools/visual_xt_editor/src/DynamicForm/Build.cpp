
#include "stdafx.h"
#include "Xt/XtTool.h"
#include "Nodes/Nodes.h"

namespace DynamicForm
{

struct C_FormBuilder : public LvField
{
	C_Node*		m_pParentNode;

public:
	C_FormBuilder(C_Node* pParentNode);
	virtual ~C_FormBuilder();

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
// C_FormBuilder
//

C_FormBuilder::C_FormBuilder(C_Node* pParentNode) :
	m_pParentNode(pParentNode)
{
}

C_FormBuilder::~C_FormBuilder()
{
}

bool C_FormBuilder::Visit(struct LfStruct* field)
{
	C_StructNode* pStruct = new C_StructNode(field);
	m_pParentNode->AddChild(pStruct);

	return true;
}

bool C_FormBuilder::Visit(struct LfInt* field)
{
	C_IntNode* pInt = new C_IntNode(field);
	m_pParentNode->AddChild(pInt);
	return true;
}

bool C_FormBuilder::Visit(struct LfFloat* field)
{
	C_FloatNode* pFloat = new C_FloatNode(field);
	m_pParentNode->AddChild(pFloat);
	return true;
}

bool C_FormBuilder::Visit(struct LfString* field)
{
	C_StringNode* pString = new C_StringNode(field);
	m_pParentNode->AddChild(pString);
	return true;
}

bool C_FormBuilder::Visit(struct LfBool* field)
{
	C_BoolNode* pBool = new C_BoolNode(field);
	m_pParentNode->AddChild(pBool);
	return true;
}

bool C_FormBuilder::Visit(struct LfEnum* field)
{
	C_EnumNode* pEnum = new C_EnumNode(field);
	m_pParentNode->AddChild(pEnum);
	return true;
}

bool C_FormBuilder::Visit(struct LfFlags* field)
{
	C_FlagsNode* pFlags= new C_FlagsNode(field);
	m_pParentNode->AddChild(pFlags);
	return true;
}

bool C_FormBuilder::Visit(struct LfPtr* field)
{
	C_PtrNode* pPtr = new C_PtrNode(field);
	m_pParentNode->AddChild(pPtr);
	return true;
}

bool C_FormBuilder::Visit(struct LfWeakPtr* field)
{
	C_WeakPtrNode* pPtr = new C_WeakPtrNode(field);
	m_pParentNode->AddChild(pPtr);
	return true;
}

bool C_FormBuilder::Visit(struct LfHandle* field)
{
	C_HandleNode* pHandle = new C_HandleNode(field);
	m_pParentNode->AddChild(pHandle);
	return true;
	return true;
}

bool C_FormBuilder::Visit(struct LfArray* field)
{
	C_ArrayNode* pArray = new C_ArrayNode(field);
	m_pParentNode->AddChild(pArray);
	return true;
}

bool C_FormBuilder::Visit(struct LfLocString* field)
{
	return true;
}

bool C_FormBuilder::Visit(struct LfControl* field)
{
	return true;
}

//
// BuildNode
//

bool BuildNode(C_Node* pParentNode, LarvaField* pField)
{
	C_FormBuilder build(pParentNode);
	return pField->Accept(&build);
}

}
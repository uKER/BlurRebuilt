#include <stdafx.h>

using namespace Aftk;

C_ShaderNode::C_ShaderNode() : C_Node(0)
{
	OneTimeInit();
}

C_Node* C_ShaderNode::Clone(C_Scene* pScene) const
{
	return new C_ShaderNode(*this);
}

C_ShaderNode::C_ShaderNode(const C_ShaderNode &rhs) : C_Node(0)
{
	OneTimeInit();
	*this = rhs;
}

C_ShaderNode::~C_ShaderNode()
{
	for (U32 i = 0; i < (U32)m_dynamicPlugs.Size(); i++)
		delete m_dynamicPlugs[i];
}

const C_ShaderNode& C_ShaderNode::operator=(const C_ShaderNode &rhs)
{
	C_Node::operator =(rhs);

	ClearThis();

	m_pParentMaterial = rhs.m_pParentMaterial;
	m_nodeInstance = rhs.m_nodeInstance;

	m_pParentMaterial = rhs.m_pParentMaterial;
	m_originalNodeTypeName = rhs.m_originalNodeTypeName;

	//Copy dynamic plugs
	m_dynamicPlugs.Resize(rhs.m_dynamicPlugs.Size());
	for (U32 i = 0; i < (U32)rhs.m_dynamicPlugs.Size(); i++)
	{
		m_dynamicPlugs[i] = new C_DynamicPlug(*((C_DynamicPlug*)rhs.m_dynamicPlugs[i]));
		m_dynamicPlugs[i]->SetLogicalIndex(rhs.m_dynamicPlugs[i]->GetLogicalIndex());
		m_dynamicPlugs[i]->SetParentPtr(this);
	}

	return *this;
}
	
bool C_ShaderNode::operator==(const C_ShaderNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_ShaderNode::operator!=(const C_ShaderNode &rhs) const
{
	return !((*this)==rhs);
}

bool C_ShaderNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbortTest, flags);
}

bool C_ShaderNode::IsEqualToAll(const I_Object& rhs, E_ObjType classToAbortTest, U32 flags) const
{
	if (classToAbortTest == E_ObjType_Shader)
		return true;
	return (IsEqualToThis(rhs, flags) && C_Node::IsEqualToAll(rhs, classToAbortTest, flags));
}

bool C_ShaderNode::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_Shader))
		return false;

	const C_ShaderNode& rhs = (const C_ShaderNode&)_rhs;

	//Note: do not compare parent material pointers, otherwise two materials can never be the same.
	if ((m_nodeInstance != rhs.m_nodeInstance) 
		||
		(m_dynamicPlugs.Size() != rhs.m_dynamicPlugs.Size()
		||
		(m_originalNodeTypeName != rhs.m_originalNodeTypeName)
		)
	)
		return false;
	
	for (U32 i = 0; i < (U32)m_dynamicPlugs.Size(); i++)
		if (!m_dynamicPlugs[i]->IsEqualTo(*rhs.m_dynamicPlugs[i]))
			return false;

	return true;
}

C_MaterialNode* C_ShaderNode::GetMaterialNodePtr()
{
	return m_pParentMaterial;
}

const C_MaterialNode* C_ShaderNode::GetMaterialNodePtr() const
{
	return m_pParentMaterial;
}

S32 C_ShaderNode::GetNodeIndex() const
{
	return m_nodeInstance;
}

S32 C_ShaderNode::AddPlugPtr(C_DynamicPlug* pNewPlug)
{
	if (!pNewPlug)
		return -1;
	m_dynamicPlugs.PushBack(pNewPlug);
	S32 logicalIndex = GetNumPlugs()-1;
	pNewPlug->SetLogicalIndex(logicalIndex);
	pNewPlug->SetParentPtr(this);
	return logicalIndex;
}

void C_ShaderNode::SetOriginalNodeTypeName(const std::string& name)
{
	m_originalNodeTypeName = name;
}

const std::string& C_ShaderNode::GetOriginalNodeTypeName() const
{
	return m_originalNodeTypeName;
}

U32 C_ShaderNode::GetNumDynamicPlugs() const
{
	return (U32)m_dynamicPlugs.Size();
}

C_DynamicPlug* C_ShaderNode::GetDynamicPlugPtr(U32 instance)
{
	return m_dynamicPlugs[instance];
}

const C_DynamicPlug* C_ShaderNode::GetDynamicPlugPtr(U32 instance) const
{
	return m_dynamicPlugs[instance];
}

C_DynamicPlug* C_ShaderNode::GetDynamicPlugPtr(const std::string& name)
{
	return const_cast<C_DynamicPlug*>(((const C_ShaderImageNode*)this)->GetDynamicPlugPtr(name));
}

const C_DynamicPlug* C_ShaderNode::GetDynamicPlugPtr(const std::string& name) const
{
	for (U32 i = 0; i < (U32)m_dynamicPlugs.Size(); i++)
		if (m_dynamicPlugs[i]->GetName() == name)
			return m_dynamicPlugs[i];

	return 0;
}

bool C_ShaderNode::DeletePlug(U32 plugIndex)
{
	U32 nStatic = GetNumStaticPlugs();
	if (plugIndex < nStatic)
		//We cannot delete static plugs.
		return false;

	U32 dynIndex = plugIndex - nStatic;
	if (dynIndex >= (U32)m_dynamicPlugs.Size())
	{
		//The specified dynamic plug does not exist.
		AFTK_ASSERT(false);
		return false;
	}

	//Free memory for plug.
	delete m_dynamicPlugs[dynIndex];
	//Remove plug pointer from array.
	m_dynamicPlugs.EraseByIndex(dynIndex);

	for (U32 i = dynIndex; i < (U32)m_dynamicPlugs.Size(); i++)
	{
		//Decrement logical index.
		m_dynamicPlugs[i]->SetLogicalIndex(
			m_dynamicPlugs[i]->GetLogicalIndex()-1);
	}

	return true;
}

E_ObjType C_ShaderNode::GetType() const
{
	return E_ObjType_Shader;
}

bool C_ShaderNode::HasType(E_ObjType type) const
{
	if (C_Node::HasType(type))
		return true;
	if (type == C_ShaderNode::GetType()) 
		return true;

	return false;
}

C_ShaderNode* C_ShaderNode::Clone() const
{
	return new C_ShaderNode(*this);
}

std::string C_ShaderNode::GetPath() const
{
	std::string finalString;
	if (m_pParentMaterial)
	{
		finalString = m_pParentMaterial->GetPath();
		finalString += "|";
	}
	finalString += m_name;
	return finalString;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_ShaderNode::SetMaterialNodePtr(C_MaterialNode* pMaterial)
{
	m_pParentMaterial = pMaterial;
}

void C_ShaderNode::SetParentPtr(C_MaterialNode* pParent)
{
	m_pParentMaterial = pParent;
}

void C_ShaderNode::SetNodeInstance(S32 instance)
{
	m_nodeInstance = instance;
}

void C_ShaderNode::OneTimeInit()
{
	m_pParentMaterial = 0;
	m_nodeInstance = -1;
}

void C_ShaderNode::Clear()
{
	ClearThis();
	C_Node::Clear();
}

void C_ShaderNode::ClearThis()
{
	m_pParentMaterial = 0;

	for (U32 i = 0; i < (U32)m_dynamicPlugs.Size(); i++)
		delete m_dynamicPlugs[i];

	m_dynamicPlugs.Clear();
	m_originalNodeTypeName.clear();
}

bool C_ShaderNode::IsManagedResource() const
{
	return true;
}

#endif


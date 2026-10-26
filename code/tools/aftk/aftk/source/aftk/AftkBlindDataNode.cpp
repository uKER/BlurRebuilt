#include <stdafx.h>

using namespace Aftk;

C_BlindDataNode::C_BlindDataNode() : C_Node(0)
{
	OneTimeInit();
}

C_Node* C_BlindDataNode::Clone(C_Scene* pScene) const
{
	return new C_BlindDataNode(*this);
}

C_BlindDataNode::~C_BlindDataNode()
{
	ClearThis();
}

C_BlindDataNode::C_BlindDataNode(const C_BlindDataNode &rhs) : C_Node(0)
{
	OneTimeInit();

	*this = rhs;
}

const C_BlindDataNode& C_BlindDataNode::operator=(const C_BlindDataNode &rhs)
{
	// copy base class info
	C_Node::operator=(rhs);

	ClearThis();

	m_attributes = rhs.m_attributes;

	return *this;
}

E_ObjType C_BlindDataNode::GetType() const
{
	return E_ObjType_BlindData;
}

bool C_BlindDataNode::HasType(E_ObjType type) const
{
	if (C_Node::HasType(type))
		return true;
	if (type == C_BlindDataNode::GetType()) 
		return true;

	return false;
}

bool C_BlindDataNode::operator==(const C_BlindDataNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_BlindDataNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbortTest, flags);
}

bool C_BlindDataNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	if (classToAbortTest == E_ObjType_BlindData)
		return IsEqualToThis(rhs, flags);
	return (IsEqualToThis(rhs, flags) && C_Node::IsEqualToAll(rhs, classToAbortTest, flags));
}

bool C_BlindDataNode::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_BlindData))
		return false;

	const C_BlindDataNode& rhs = (C_BlindDataNode&)_rhs;

	if (m_attributes.Size() != rhs.m_attributes.Size()) 
		return false;
	else 
	{
		//Each blind data node has the same number of attributes
		for (U32 i = 0; i < (U32)m_attributes.Size(); i++) 
		{
			// Use the comparison operator of the C_GenericAttribute class
			if (m_attributes[i] != rhs.m_attributes[i])
				return false;
		}
	}
	return true;
}


bool C_BlindDataNode::operator!=(const C_BlindDataNode &rhs) const 
{
	if ( *this == rhs ) 
		return false;
	else 
		return true;
}

void C_BlindDataNode::SetAttributeInitialCapacity(U32 capacity)
{
	if (capacity > (U32)m_attributes.Capacity())
	{
		m_attributes.Reserve(capacity);
	}
}

U32 C_BlindDataNode::AddAttribute(const C_GenericAttribute &attribute)
{
	m_attributes.PushBack(attribute);

	return (U32)(m_attributes.Size() -1);
}

void C_BlindDataNode::DeleteAttribute(U32 instance)
{
	m_attributes.EraseByIndex(instance);
}

void C_BlindDataNode::SetAttribute(U32 instance, const C_GenericAttribute &attribute)
{
	m_attributes[instance] = attribute;
}

const C_GenericAttribute& C_BlindDataNode::GetAttribute(U32 instance) const
{
	return m_attributes[instance];
}

C_GenericAttribute& C_BlindDataNode::GetAttribute(U32 instance)
{
	return m_attributes[instance];
}

U32	C_BlindDataNode::GetNumAttributes() const
{
	return (U32)m_attributes.Size();
}

U32 C_BlindDataNode::GetHash(E_ObjType classToAbort) const
{
	if (classToAbort == E_ObjType_BlindData)
		return 0;
	return C_BlindDataNode::GetHashOfThis() + C_Node::GetHash(classToAbort);
}

void C_BlindDataNode::Output(const C_BlindDataNode& blindData)
{
	AFTK_DO_BEGIN << "--- BLIND DATA BEGIN ---\n" AFTK_DO_INFO_END;
	AFTK_DO_BEGIN << "Name: " << blindData.GetName() << "\n" AFTK_DO_INFO_END;

	for (U32 i = 0; i <  blindData.GetNumAttributes(); i++)
	{
		C_GenericAttribute::Output(blindData.GetAttribute(i));
	}
	AFTK_DO_BEGIN << "--- BLIND DATA END ---\n" AFTK_DO_INFO_END;
}

void* C_BlindDataNode::operator new (size_t size)
{
	return C_MemoryManager::PoolAllocate((U32)size);
}

void C_BlindDataNode::operator delete(void* p)
{
	C_MemoryManager::PoolFree(sizeof(C_BlindDataNode), p);
}

// Protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

U32 C_BlindDataNode::GetHashOfThis() const
{
	U32 hash = 0;
	for (U32 i = 0; i < (U32)m_attributes.Size(); i++)
	{
		hash += m_attributes[i].GetHash();
	}
	return hash;
}

void C_BlindDataNode::OneTimeInit()
{
}

void C_BlindDataNode::Clear()
{
	ClearThis();
	C_Node::Clear();
}

void C_BlindDataNode::ClearThis()
{
	m_attributes.Clear();
}



#endif

#include <stdafx.h>

using namespace Aftk;

C_Node::C_Node(C_Scene* pScene)
{
	OneTimeInit(pScene);
}

C_Node* C_Node::Clone(C_Scene* pScene) const
{
	C_Node* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_Node(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_Node(pScene);
		*pNewNode = *this;
	}

	return pNewNode;
}

C_Node::~C_Node()
{
	Clear();
	if (m_pScene)
		m_pScene->OnDestroyUnmanagedDgNode(this);
}

C_Node::C_Node(const C_Node& rhs)
{
	OneTimeInit(0);

	*this = rhs;
}

C_Node& C_Node::operator =(const C_Node& rhs)
{
	Clear();

	if (!m_pScene && rhs.m_pScene)
	{
		m_pScene = rhs.m_pScene;
		//Associate with rhs scene.
		m_pScene->OnCreateUnmanagedDgNode(this);
	}


	m_name = rhs.m_name;

	//Note that we do not copy the scene pointer, this is set during construction only.

	if (rhs.m_pAttachedData != 0)
		m_pAttachedData = rhs.m_pAttachedData->CreateClone();
	else
		m_pAttachedData = 0;
	return *this;
}

E_ObjType C_Node::GetType() const
{
	return E_ObjType_Node;
}

bool C_Node::HasType(E_ObjType type) const
{
	if (type == E_ObjType_Node) 
		return true;

	return false;
}

bool C_Node::IsEqualTo(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbort, flags);
}

bool C_Node::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	if (!IsEqualToThis(rhs, flags))
		return false;

	return true;
}

bool C_Node::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_Node))
		return false;

	const C_Node& rhs = (const C_Node&)_rhs;

	if (GetType() == rhs.GetType() &&
		m_name == rhs.m_name) 
	{
		return true;
	} 
	else 
	{
		return false;
	}	
}

bool C_Node::operator==(const C_Node &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_Node::operator!=(const C_Node &rhs) 
{
	return !((*this)==rhs);
}

U32 C_Node::GetHashOfThis() const
{
	return Ut::HashKeyOpString::GetHash(m_name.c_str());
}

U32 C_Node::GetHash(E_ObjType classToAbort) const
{
	return C_Node::GetHashOfThis();
}

void C_Node::SetName(const std::string &name)
{
	m_name = name;
}

void C_Node::SetAttachData(const C_AttachData& attachData)
{
	m_pAttachedData = attachData.CreateClone();
}

const std::string& C_Node::GetName() const
{
	return m_name;
}

const C_AttachData* C_Node::GetAttachDataPtr() const
{
	return m_pAttachedData;
}

C_AttachData* C_Node::GetAttachDataPtr()
{
	return m_pAttachedData;
}

void C_Node::DeleteAttachData()
{
	if (m_pAttachedData != 0)
	{
		delete m_pAttachedData;
		m_pAttachedData = 0;
	}
}

std::string C_Node::GetPath() const
{
	return m_name;
}

const C_Scene* C_Node::GetScenePtr() const
{
	return m_pScene;
}

C_Scene* C_Node::GetScenePtr()
{
	return m_pScene;
}

bool C_Node::DeletePlug(U32 plugIndex)
{
	return false;
}

bool C_Node::IsUsed() const
{
	if ((U32)m_connectedPlugs.Size() > 0)
		return true;

	return false;
}

const I_Plug* C_Node::GetStaticPlugPtr(U32 instance) const
{
	return 0;
}

I_Plug* C_Node::GetStaticPlugPtr(U32 instance)
{
	return const_cast<I_Plug*>(((const C_Node*)this)->GetStaticPlugPtr(instance));
}

U32	C_Node::GetNumStaticPlugs() const
{
	return 0;
}

const C_DynamicPlug* C_Node::GetDynamicPlugPtr(U32 instance) const
{
	return 0;
}

C_DynamicPlug* C_Node::GetDynamicPlugPtr(U32 instance)
{
	return const_cast<C_DynamicPlug*>(((const C_Node*)this)->GetDynamicPlugPtr(instance));
}

U32	C_Node::GetNumDynamicPlugs() const
{
	return 0;
}

const I_Plug* C_Node::GetPlugPtr(U32 instance) const
{
	U32 nStatic = GetNumStaticPlugs();
	if (instance < nStatic)
		return GetStaticPlugPtr(instance);

	return GetDynamicPlugPtr(instance - nStatic);
}

const I_Plug* C_Node::GetPlugPtr(const std::string& name) const
{
	for (U32 i = 0; i < GetNumPlugs(); i++)
	{
		const I_Plug* pPlug = GetPlugPtr(i);
		if (pPlug->GetName() == name)
			return pPlug;
	}

	return 0;
}

const I_Plug* C_Node::GetPlugPtr(const C_PlugLogicalIndexList& locationIndices) const
{
	if (locationIndices.nIndices < 1)
		return 0;

	U32 indexIndex = 0;
	S32 index = locationIndices.indices[indexIndex];
	if (index < 0)
		return 0;
	
	const I_Plug* pPlug = GetPlugPtr(index);
	indexIndex++;
	for(; indexIndex < locationIndices.nIndices;	indexIndex++)
	{
		index = locationIndices.indices[indexIndex];
		if (index < 0)
			return 0;
		pPlug = pPlug->GetChildPtr(index);
	}

	//We've found our plug.
	return pPlug;
}

I_Plug* C_Node::GetPlugPtr(U32 instance)
{
	return const_cast<I_Plug*>(((const C_Node*)this)->GetPlugPtr(instance));
}

I_Plug* C_Node::GetPlugPtr(const std::string& name)
{
	return const_cast<I_Plug*>(((const C_Node*)this)->GetPlugPtr(name));
}

I_Plug* C_Node::GetPlugPtr(const C_PlugLogicalIndexList& locationIndices)
{
	return const_cast<I_Plug*>(((const C_Node*)this)->GetPlugPtr(locationIndices));
}

U32	C_Node::GetNumPlugs() const
{
	return GetNumStaticPlugs() + GetNumDynamicPlugs();
}
		
void C_Node::ClearThis()
{
	if (m_pAttachedData != 0)
	{
		delete m_pAttachedData;
		m_pAttachedData = 0;
	}
	m_name.clear();
}

void C_Node::Clear()
{
	ClearThis();
}

// Protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_Node::OneTimeInit(C_Scene* pScene)
{
	m_pScene = pScene;

	//If we're to be associated with a scene, and if we're not managed, attach to our unmanged dg node list.
	if (m_pScene)
		m_pScene->OnCreateUnmanagedDgNode(this);

	m_pAttachedData = 0;
}

void C_Node::SetScenePtr(C_Scene* pScene)
{
	m_pScene = pScene;
}

#endif


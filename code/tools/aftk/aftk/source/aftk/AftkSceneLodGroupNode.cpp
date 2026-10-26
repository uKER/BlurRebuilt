#include <stdafx.h>

using namespace Aftk;

#define AFTK_RESIZE_LG_INFO_LEVELS_IF_NEEDED if (lodLevel >= (U32)m_infoList.Size()) {	m_infoList.Resize(lodLevel+1); }

C_SceneLodGroupNode::C_SceneLodGroupNode(C_Scene* pScene) : C_SceneNode(pScene)
{
	OneTimeInit();
}

C_SceneLodGroupNode::C_SceneLodGroupNode(const C_SceneLodGroupNode& rhs) : C_SceneNode(0)
{
	OneTimeInit();

	*this = rhs;
}

C_Node* C_SceneLodGroupNode::Clone(C_Scene* pScene) const
{
	C_SceneLodGroupNode* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_SceneLodGroupNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_SceneLodGroupNode(pScene);
		(C_SceneLodGroupNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

C_SceneLodGroupNode::~C_SceneLodGroupNode()
{
	ClearThis();
}

const C_SceneLodGroupNode& C_SceneLodGroupNode::operator=(const C_SceneLodGroupNode& rhs)
{
	C_SceneNode::operator=(rhs);

	ClearThis();

	//Don't copy info list between scenes, such relationships must be seperately resolved.
	if (m_pScene == rhs.m_pScene)
	{
		m_infoList = rhs.m_infoList;
	}

	return *this;
}

S32 C_SceneLodGroupNode::AddLodLevelInfo(const C_GroupLodLevelInfo& info)
{
	m_infoList.PushBack(info);
	return m_infoList.Size() -1;
}

void C_SceneLodGroupNode::InsertLodLevelInfo(U32 lodLevel, const C_GroupLodLevelInfo& info)
{
	if (lodLevel >= (U32)m_infoList.Size()) 
	{	
		m_infoList.Resize(lodLevel+1); 
		m_infoList[lodLevel] = info;
	}
	else
	{
		m_infoList.InsertBefore(info, m_infoList.Begin() + lodLevel);
	}
}

const C_GroupLodLevelInfo& C_SceneLodGroupNode::GetLodLevelInfo(U32 lodLevel) const
{
	return m_infoList[lodLevel];
}

C_GroupLodLevelInfo& C_SceneLodGroupNode::GetLodLevelInfo(U32 lodLevel)
{
	return m_infoList[lodLevel];
}

U32 C_SceneLodGroupNode::GetNumLodLevels() const
{
	return (U32)m_infoList.Size();
}

S32 C_SceneLodGroupNode::GetLodLevelIndex(const C_SceneNode* pNode) const
{
	for (U32 i = 0; i < (U32)m_infoList.Size(); i++)
		if (m_infoList[i].GetSceneNodePtr() == pNode)
			return i;

	return -1;
}

void C_SceneLodGroupNode::DeleteLodLevelInfo(U32 lodLevel)
{
	AFTK_ASSERT(lodLevel < (U32)m_infoList.Size());
	m_infoList.EraseByIndex(lodLevel);
}

void C_SceneLodGroupNode::DeleteLodLevelInfo(C_SceneNode* pNode)
{
	for (U32 i = 0; i < (U32)m_infoList.Size(); i++)
	{
		if (m_infoList[i].GetSceneNodePtr() == pNode)
		{
			m_infoList.EraseByIndex(i);
			return;
		}
	}
}

E_ObjType C_SceneLodGroupNode::GetType() const
{
	return E_ObjType_SceneLodGroup;
}

bool C_SceneLodGroupNode::HasType(E_ObjType type) const
{
	if (C_SceneNode::HasType(type))
		return true;
	if (type == C_SceneLodGroupNode::GetType()) 
		return true;

	return false;
}

bool C_SceneLodGroupNode::operator== (const C_SceneLodGroupNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_SceneLodGroupNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbort, flags);
}

bool C_SceneLodGroupNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	if (classToAbort == E_ObjType_SceneLodGroup)
		return IsEqualToThis(rhs, flags);
	return (IsEqualToThis(rhs, flags) && C_SceneNode::IsEqualToAll(rhs, classToAbort, flags));
}

bool C_SceneLodGroupNode::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	if (!_rhs.HasType(GetType()))
	{
		return false;
	}

	C_SceneLodGroupNode& rhs = (C_SceneLodGroupNode&) _rhs;

	if (m_infoList.Size() != rhs.m_infoList.Size())
		return false;

	if (m_infoList.Size() > 0)
		if (memcmp(&m_infoList[0], &rhs.m_infoList[0], sizeof(C_GroupLodLevelInfo)*m_infoList.Size()) != 0)
			return false;

	return true;
}

void C_SceneLodGroupNode::Clear()
{
	ClearThis();
	C_SceneNode::Clear();
}

// protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_SceneLodGroupNode::ClearThis()
{
	m_infoList.Clear();
}

void C_SceneLodGroupNode::OneTimeInit()
{
	ClearThis();
}

#endif

#include <stdafx.h>
#include <aftk/AftkStringTable.h>

using namespace Aftk;

C_SceneXRefNode::C_SceneXRefNode(C_Scene* pScene) : C_SceneNode(pScene)
{
	m_pSourceScene = 0;
	m_pSourceSceneNode = 0;
}

C_SceneXRefNode::~C_SceneXRefNode()
{
	Clear();
}

void C_SceneXRefNode::SetSourceSceneFileName(const std::string& fileName)
{
	m_sourceSceneFileName = fileName;
}

void C_SceneXRefNode::SetSourceNodePath(const std::string& path)
{
	m_sourceSceneNodePath = path;
}

C_Scene* C_SceneXRefNode::GetSourceScenePtr()
{
	return m_pSourceScene;
}

const C_Scene* C_SceneXRefNode::GetSourceScenePtr() const
{
	return m_pSourceScene;
}

const C_SceneNode* C_SceneXRefNode::GetSourceNodePtr() const
{
	return m_pSourceSceneNode;
}

C_SceneNode* C_SceneXRefNode::GetSourceNodePtr()
{
	return m_pSourceSceneNode;
}

const std::string& C_SceneXRefNode::GetSourceSceneFileName() const
{
	return m_sourceSceneFileName;
}

const std::string& C_SceneXRefNode::GetSourceNodePath() const
{
	return m_sourceSceneNodePath;
}

E_ObjType C_SceneXRefNode::GetType() const
{
	return E_ObjType_SceneXRef;
}

bool C_SceneXRefNode::HasType(E_ObjType type) const
{
	if (C_SceneNode::HasType(type))
		return true;
	if (type == C_SceneXRefNode::GetType()) 
		return true;

	return false;
}

// protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_SceneXRefNode::Clear()
{
	m_pSourceScene = 0;
	m_pSourceSceneNode = 0;
	m_sourceSceneFileName.clear();
	m_sourceSceneNodePath.clear();
	C_SceneNode::Clear();
}

C_Status C_SceneXRefNode::SetSourceScenePtr(C_Scene* pSourceScene)
{
	m_pSourceScene = pSourceScene;
	if (pSourceScene)
	{
		m_pSourceSceneNode = m_pSourceScene->GetSceneNodePtrByPartialPath(m_sourceSceneNodePath);
		if (!m_pSourceSceneNode)
		{
#ifdef _DEBUG
			AFTK_ASSERT(false);
#endif
			return C_Status(C_Status::E_Status_Failure, AFTK_E_FAILED_TO_FIND_SCENE_NODE);
		}
	}
	else
	{
		m_pSourceSceneNode = 0;
	}

	return C_Status::E_Status_Success;
}

/*
C_SceneXRefNode::C_SceneXRefNode(const C_SceneXRefNode& rhs)
{
	*this = rhs;
}

const C_SceneXRefNode& C_SceneXRefNode::operator =(const C_SceneXRefNode& rhs)
{
	C_SceneNode::operator=(rhs);

	m_pXRefEntry		= rhs.m_pXRefEntry;

	return *this;
}
*/

#endif

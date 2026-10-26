#include "stdafx.h"

using namespace MayaIo;

C_AftkMayaMapping::C_AftkMayaMapping()
{
	m_pNameStringMayaSceneNodeMapping = 0;
	m_pDagPathStringMayaSceneNodeMapping = 0;
	m_pMayaAftkSceneNodeMapping = 0;
	m_pAftkMayaSceneNodeMapping = 0;
	m_created = false;
}

Aftk::C_Status C_AftkMayaMapping::Create()
{
	AFTK_ASSERT(!m_created);
	if (m_created)
	{
		return Aftk::C_Status::E_Status_Success;
	}
	m_pNameStringMayaSceneNodeMapping = new Ut::UtMap<std::string, MDagPath>;
	if (!m_pNameStringMayaSceneNodeMapping)	
		return Aftk::C_Status::E_Status_OutOfMemory;

	m_pDagPathStringMayaSceneNodeMapping = new Ut::UtMap<std::string, MDagPath>;
	if (!m_pDagPathStringMayaSceneNodeMapping)
		return Aftk::C_Status::E_Status_OutOfMemory;

	m_pMayaAftkSceneNodeMapping = new Ut::UtMap<std::string, Aftk::C_SceneNode*>;
	if (!m_pMayaAftkSceneNodeMapping)
		return Aftk::C_Status::E_Status_OutOfMemory;

	m_pAftkMayaSceneNodeMapping = new Ut::UtMap<Aftk::C_SceneNode*, MDagPath>;
	if (!m_pAftkMayaSceneNodeMapping)
		return Aftk::C_Status::E_Status_OutOfMemory;

	m_created = true;
	return Aftk::C_Status::E_Status_Success;
}

void C_AftkMayaMapping::Destroy()
{
	if (m_created)
	{
		delete m_pNameStringMayaSceneNodeMapping;
		delete m_pDagPathStringMayaSceneNodeMapping;
		delete m_pMayaAftkSceneNodeMapping;
		delete m_pAftkMayaSceneNodeMapping;
	}
	m_created = false;
}

void C_AftkMayaMapping::Clear()
{
	AFTK_ASSERT(m_created);
	//Remove our store of mappings between the Aftk and Maya.
	m_pDagPathStringMayaSceneNodeMapping->Clear();
	m_pMayaAftkSceneNodeMapping->Clear();
	m_pAftkMayaSceneNodeMapping->Clear();
	m_pNameStringMayaSceneNodeMapping->Clear();
}

void C_AftkMayaMapping::DeleteAftkMayaSceneNodeMapping(
	Aftk::C_SceneNode* pAftkSceneNode)
{
	Ut::UtMap<Aftk::C_SceneNode*, MDagPath>::Iterator it = 
		m_pAftkMayaSceneNodeMapping->Find(pAftkSceneNode);
	if (it == m_pAftkMayaSceneNodeMapping->End())
		return;

	MDagPath& path = it->GetValue();
	std::string sPath = path.fullPathName().asChar();
	Ut::UtMap<std::string, Aftk::C_SceneNode*>::Iterator it2 = 
		m_pMayaAftkSceneNodeMapping->Find(sPath);
	if (it2 != m_pMayaAftkSceneNodeMapping->End())
		m_pMayaAftkSceneNodeMapping->Erase(it2);

	m_pAftkMayaSceneNodeMapping->Erase(it);
}

bool C_AftkMayaMapping::GetAftkSceneNodeFromDagPathString(
	const std::string& dagPathString,
	Aftk::C_SceneNode** ppAftkSceneNode)
{
	MStatus status;
	MDagPath dagPath;
	if (!GetMayaSceneNodeFromDagPathString(dagPathString, dagPath))
	{
		return false;
	}
	if (!GetAftkSceneNodeFromMayaDagPath(dagPath, ppAftkSceneNode))
	{
		return false;
	}

	return true;
}

bool C_AftkMayaMapping::GetMayaSceneNodeFromNameString(
	const std::string& name,
	MDagPath& dagPath)
{
	Ut::UtMap<std::string, MDagPath>::ConstIterator it;

	it = m_pNameStringMayaSceneNodeMapping->Find(name);
	if (it == m_pNameStringMayaSceneNodeMapping->End())
		return false;

	dagPath = it->GetValue();
	return true;
}

bool C_AftkMayaMapping::GetMayaSceneNodeFromDagPathString(
	const std::string& dagPathString,
	MDagPath& dagPath)
{
	Ut::UtMap<std::string, MDagPath>::ConstIterator it;

	it = m_pDagPathStringMayaSceneNodeMapping->Find(dagPathString);

	if (it == m_pDagPathStringMayaSceneNodeMapping->End())
	{
		return false;
	}

	dagPath = it->GetValue();
	return true;
}

void C_AftkMayaMapping::SetNameStringMayaSceneNodeMapping(
	const std::string& nodeName,
	const MDagPath& mayaDagPath)
{
	(*m_pNameStringMayaSceneNodeMapping)[nodeName] = mayaDagPath;
}

void C_AftkMayaMapping::SetDagPathStringMayaSceneNodeMapping(
	const std::string& dagPathString,
	const MDagPath& mayaDagPath)
{
	(*m_pDagPathStringMayaSceneNodeMapping)[dagPathString] = mayaDagPath;
}

void C_AftkMayaMapping::SetMayaAftkSceneNodeMapping(
	const MDagPath& mayaDagPath,
	Aftk::C_SceneNode* pAftkSceneNode)
{
	std::string dagPathString = mayaDagPath.fullPathName().asChar();
	(*m_pMayaAftkSceneNodeMapping)[dagPathString] = pAftkSceneNode;
}

void C_AftkMayaMapping::SetMayaAftkSceneNodeMapping(
	const MDagPath& mayaDagPath,
	const Aftk::C_SceneNode* pAftkSceneNode)
{
	std::string dagPathString = mayaDagPath.fullPathName().asChar();
	Aftk::C_SceneNode* pNonConst = const_cast<Aftk::C_SceneNode*>(pAftkSceneNode);
	(*m_pMayaAftkSceneNodeMapping)[dagPathString] = pNonConst;
}

void C_AftkMayaMapping::SetAftkMayaSceneNodeMapping(
	const Aftk::C_SceneNode* pAftkSceneNode,
	const MDagPath& mayaDagPath)
{
	Aftk::C_SceneNode* pNonConst = const_cast<Aftk::C_SceneNode*>(pAftkSceneNode);
	(*m_pAftkMayaSceneNodeMapping)[pNonConst] = MDagPath(mayaDagPath);
}

void C_AftkMayaMapping::SetAftkMayaSceneNodeMapping(
	Aftk::C_SceneNode* pAftkSceneNode,
	const MDagPath& mayaDagPath)
{
	(*m_pAftkMayaSceneNodeMapping)[pAftkSceneNode] = MDagPath(mayaDagPath);
}

bool C_AftkMayaMapping::GetAftkSceneNodeFromMayaDagPath(
	const MDagPath& mayaDagPath, 
	Aftk::C_SceneNode** ppSceneNode)
{
	std::string dagPathString = mayaDagPath.fullPathName().asChar();
	Ut::UtMap<std::string, Aftk::C_SceneNode*>::ConstIterator it;

	it = m_pMayaAftkSceneNodeMapping->Find(dagPathString);

	if (it == m_pMayaAftkSceneNodeMapping->End())
	{
		return false;
	}

	*ppSceneNode = it->GetValue();
	return true;
}

bool C_AftkMayaMapping::GetMayaDagPathFromAftkSceneNode(
	Aftk::C_SceneNode* pSceneNode,
	MDagPath* pDagPath)
{
	Ut::UtMap<Aftk::C_SceneNode*, MDagPath>::ConstIterator it;

	it = m_pAftkMayaSceneNodeMapping->Find(pSceneNode);

	if (it == m_pAftkMayaSceneNodeMapping->End())
	{
		return false;
	}

	*pDagPath = it->GetValue();
	return true;
}

bool C_AftkMayaMapping::GetMayaDagPathFromAftkSceneNode(
	const Aftk::C_SceneNode* pSceneNode,
	MDagPath* pDagPath)
{
	Aftk::C_SceneNode* pNonConst = const_cast<Aftk::C_SceneNode*>(pSceneNode);
	Ut::UtMap<Aftk::C_SceneNode*, MDagPath>::ConstIterator it;

	it = m_pAftkMayaSceneNodeMapping->Find(pNonConst);

	if (it == m_pAftkMayaSceneNodeMapping->End())
	{
		return false;
	}

	*pDagPath = it->GetValue();
	return true;
}


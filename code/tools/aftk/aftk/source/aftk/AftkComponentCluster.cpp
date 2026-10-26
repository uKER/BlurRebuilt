#include <stdafx.h>

using namespace Aftk;

C_ComponentCluster::C_ComponentCluster(C_SceneMeshNode* pMesh)
{
	OneTimeInit();
	m_pMeshNode = pMesh;
}
/*
C_ComponentCluster::C_ComponentCluster(const C_ComponentCluster& rhs)
{
	OneTimeInit();
	*this = rhs;
}
*/
C_ComponentCluster::~C_ComponentCluster()
{
}

const C_ComponentCluster& C_ComponentCluster::operator=(const C_ComponentCluster& rhs)
{
	m_name = rhs.m_name;
	return *this;
}

C_SceneMeshNode* C_ComponentCluster::GetMeshNodePtr()
{
	return m_pMeshNode;
}

const C_SceneMeshNode* C_ComponentCluster::GetMeshNodePtr() const
{
	return m_pMeshNode;
}

void C_ComponentCluster::SetName(const std::string& name)
{
	m_name = name;
}

const std::string& C_ComponentCluster::GetName() const
{
	return m_name;
}

void C_ComponentCluster::Clear()
{
	ClearThis();
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_ComponentCluster::OneTimeInit()
{
}

void C_ComponentCluster::ClearThis()
{
	m_name.clear();
}

/*
void C_ComponentCluster::AttachToMesh(C_SceneMeshNode* pSceneMeshNode)
{
	m_pMeshNode = pSceneMeshNode;
}
*/

#endif

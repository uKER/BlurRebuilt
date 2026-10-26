#include "stdafx.h"
#include "AftkRender.h"

using namespace AftkRender;

C_RenderMeshInstanceNode::C_RenderMeshInstanceNode(
	Aftk::C_SceneNode *pAftkNode,
	Aftk::C_SceneMeshRemappableNode *pAftkMaterialMappingSource,
	C_SceneRenderer *pRenderer)
:
	C_RenderMeshRemappableNode(pAftkNode, pAftkMaterialMappingSource, pRenderer),
	m_pSourceNode(0)
{
}


C_RenderMeshInstanceNode::~C_RenderMeshInstanceNode()
{
}


Aftk::C_Status C_RenderMeshInstanceNode::Rebuild()
{
	Aftk::C_Status status;
	status = C_RenderMeshRemappableNode::Rebuild();
	if (!status)
		return status;

	if (!m_pSourceNode)
	{
		LOGWARN(g_Log, "Instance node does not have a source %s\n", m_pAftkSceneNode->GetName().c_str());
		return Aftk::C_Status::E_Status_Failure;
	}
	const Aftk::C_SceneNode* pSourceAftkSceneNode = m_pSourceNode->GetAftkSceneNodePtr();

	ASSERT((pSourceAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMesh) ||
		(pSourceAftkSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance)));

	return Aftk::C_Status::E_Status_Success;

}


Aftk::C_Status C_RenderMeshInstanceNode::Render(
	U32 lodLevel)
{
	if (m_pSourceNode)
		return m_pSourceNode->Render(lodLevel);
	else
		return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_RenderMeshInstanceNode::Render(
	U32 lodLevel,
	U32 clusterIndex)
{
	if (m_pSourceNode)
		return m_pSourceNode->Render(lodLevel, clusterIndex);
	else
		return Aftk::C_Status::E_Status_Success;

}


U32 C_RenderMeshInstanceNode::GetNumClusters(U32 lod) const
{
	if (m_pSourceNode)
		return m_pSourceNode->GetNumClusters(lod);
	else
		return 0;
}


const C_RenderMeshCluster* C_RenderMeshInstanceNode::GetMeshClusterPtr(
	U32 lodLevel,
	U32 clusterIndex) const
{
	if (m_pSourceNode)
		return m_pSourceNode->GetMeshClusterPtr(lodLevel, clusterIndex);
	else
		return 0;
}


C_RenderMeshCluster* C_RenderMeshInstanceNode::GetMeshClusterPtr(
	U32 lodLevel,
	U32 clusterIndex)
{
	if (m_pSourceNode)
		return m_pSourceNode->GetMeshClusterPtr(lodLevel, clusterIndex);
	else
		return 0;
}

Display::C_VertexDeclaration* C_RenderMeshInstanceNode::GetVertexDeclaration() const
{
	if (m_pSourceNode)
		return m_pSourceNode->GetVertexDeclaration();
	else
		return 0;
}


I_RenderMeshNode::E_EffectMode C_RenderMeshInstanceNode::GetEffectMode() const
{
	if (m_pSourceNode)
		return m_pSourceNode->GetEffectMode();
	else
		return I_RenderMeshNode::E_EffectMode_None;
}

U8 C_RenderMeshInstanceNode::GetNumUVSets() const
{
	return m_pSourceNode->GetNumUVSets();
}

const C_RenderMeshNode* C_RenderMeshInstanceNode::GetSourceNodePtr() const
{
	return m_pSourceNode;
}

C_RenderMeshNode* C_RenderMeshInstanceNode::GetSourceNodePtr()
{
	return m_pSourceNode;
}

void C_RenderMeshInstanceNode::SetSourceNodePtr(C_RenderMeshNode* pRenderMeshNode)
{
	m_pSourceNode = pRenderMeshNode;
}

E_RenderNodeType C_RenderMeshInstanceNode::GetType() const
{
	return E_RenderNodeType_Mesh_Instance;
}

bool C_RenderMeshInstanceNode::HasType(E_RenderNodeType type) const
{
	return (type == C_RenderMeshInstanceNode::GetType()
			|| C_RenderMeshRemappableNode::HasType(type));
}


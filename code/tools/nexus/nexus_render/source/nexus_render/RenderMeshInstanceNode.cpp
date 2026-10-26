#include <nexus_render/stdafx.h>
#include <nexus_render/RenderMeshInstanceNode.h>
#include <tools_common/Utility.h>

using namespace NexusRender;

RenderMeshInstanceNode::RenderMeshInstanceNode(
	SceneRenderer* pSceneRenderer, 
	Aftk::C_SceneNode* pAftkNode,
	Aftk::C_SceneMeshRemappableNode* pAftkMaterialMappingSource) :
	RenderMeshRemappableNode(
		pSceneRenderer,
		pAftkNode,
		pAftkMaterialMappingSource)
{
	m_pSource = 0;
	m_pComponentDisplay = 0;
}
		
RenderMeshInstanceNode::~RenderMeshInstanceNode()
{
	if (m_pComponentDisplay)
	{
		m_pComponentDisplay->Destroy();
		delete m_pComponentDisplay;
		m_pComponentDisplay = 0;
	}
}

void RenderMeshInstanceNode::Clear()
{
	__super::Clear();
}

Tu::Status RenderMeshInstanceNode::Rebuild()
{
	Tu::Status status;
	status = RenderMeshRemappableNode::Rebuild();
	if (!status) return status;

	if (!m_pSource)
	{
		ASSERT(m_pSource);
		std::stringstream ss;
		ss << "Instance " << m_pAftkSceneNode->GetName() << " has an invalid source.";
		return Tu::Status(Tu::Status::FAILURE, ss.str());
	}

	//Get the source node...
	const Aftk::C_SceneNode* pSourceAftkSceneNode = m_pSource->GetAftkSceneNodePtr();
	//Check our source is a mesh.
	ASSERT((pSourceAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMesh) ||
		(pSourceAftkSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance)));

	m_nodeFlags.isBuilt = true;
	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshInstanceNode::Evaluate()
{
	return m_pSource->Evaluate();
}

Tu::Status RenderMeshInstanceNode::OnSelected()
{
	Tu::Status status;
	status = RenderNode::OnSelected();
	if (!status) return status;

	if (!m_pSource)
		return Tu::Status::SUCCESS;

	if (m_nodeFlags.isSelected && !m_pComponentDisplay)
	{
		m_pComponentDisplay = new MeshComponentDisplay();
		if (!m_pComponentDisplay)
			return Tu::Status::OUT_OF_MEMORY;
		status = m_pComponentDisplay->Create(this);
		if (!status) return status;
		if (GetType() == RENDER_NODE_TYPE_MESH_BONED_INSTANCE)
			m_pComponentDisplay->Update(true);
		else
			m_pComponentDisplay->Update(m_pSceneRenderer->GetSettingsPtr()->skinningMode != 0);
	}

	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshInstanceNode::OnDeselected()
{
	Tu::Status status;
	status = RenderNode::OnDeselected();
	if (!status) return status;

	if (!m_pSource)
		return Tu::Status::SUCCESS;

	if (!m_nodeFlags.isSelected && m_pComponentDisplay)
	{
		m_pComponentDisplay->Destroy();
		delete m_pComponentDisplay;
		m_pComponentDisplay = 0;
	}

	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshInstanceNode::Update(
	bool timeChanged,
	bool scaleChanged,
	bool skinningModeChanged) 
{
	Tu::Status status;
	//If time has changed and we're not static, then evaluate. Also evaluate if we've not performed our
	//initial update.
	if ((timeChanged && (!m_nodeFlags.isStatic)) || !m_nodeFlags.hasInitialUpdate)
	{
		status = Evaluate();
		if (!status) return status;
	}

	status = RenderNode::Update(
		timeChanged,
		scaleChanged,
		skinningModeChanged);
	if (!status) return status;

	if (m_nodeFlags.isCulled)
		return Tu::Status::SUCCESS;

	//Determine if we're hidden or not by node type.
	m_nodeFlags.isHidden = 
		(!m_pSceneRenderer->GetSettingsPtr()->renderInstances) ||
		(!m_pSceneRenderer->GetSettingsPtr()->renderMeshes);

	if (scaleChanged && m_pComponentDisplay)
	{
		if (GetType() == RENDER_NODE_TYPE_MESH_BONED_INSTANCE)
			m_pComponentDisplay->Update(true);
		else
			m_pComponentDisplay->Update(m_pSceneRenderer->GetSettingsPtr()->skinningMode != 0);
	}

	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshInstanceNode::DrawNormals(U32 lodLevel)
{
	if (m_nodeFlags.isSelected && m_pSource)
	{
		Tu::Status status;
		if (!m_pComponentDisplay)
		{
			status = OnSelected();
			if (!status) return status;
		}

		ASSERT(m_pComponentDisplay);
		m_pComponentDisplay->DrawNormals(lodLevel);
	}
	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshInstanceNode::DrawTangentSpace(U32 lodLevel, U16 set)
{
	if (m_nodeFlags.isSelected && m_pSource)
	{
		Tu::Status status;
		if (!m_pComponentDisplay)
		{
			status = OnSelected();
			if (!status) return status;
		}

		ASSERT(m_pComponentDisplay);
		m_pComponentDisplay->DrawTangentSpace(lodLevel, set);
	}
	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshInstanceNode::DrawSelectedComponents(U32 lodLevel)
{
	if (!m_pComponentDisplay)
		return Tu::Status::SUCCESS;
	
	m_pComponentDisplay->DrawSelectedFaces(lodLevel);
	m_pComponentDisplay->DrawSelectedPoints(lodLevel);

	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshInstanceNode::Draw()
{
	return m_pSource->Draw(m_activeLod, RenderMeshNode::SPECIAL_CLUSTER_ENTIRE_MESH);
}

//Draw the entire mesh without any batching (default), or 
//draw only the portion of this mesh that uses the material at the given index.
Tu::Status RenderMeshInstanceNode::Draw(U32 lodLevel, U32 clusterIndex)
{
	//Draw the correct mesh cluster.
	return m_pSource->Draw(lodLevel, clusterIndex);
}

Tu::Status RenderMeshInstanceNode::SetSourcePtr(RenderMeshNode* pRenderMeshNode)
{
	m_pSource = pRenderMeshNode;
	return Tu::Status::SUCCESS;
}

RenderMeshNode* RenderMeshInstanceNode::GetSourcePtr() const
{
	return m_pSource;
}

bool RenderMeshInstanceNode::IsSkinned() const
{
	return m_pSource->IsSkinned();
}

bool RenderMeshInstanceNode::IsSkinAnimated() const
{
	return m_pSource->IsSkinAnimated();
}

void RenderMeshInstanceNode::SetActiveLodLevel()
{
	if (m_pSource)
		m_activeLod = m_pSource->GetActiveLodLevel(this);
}

D3DManager::EVertexDecl RenderMeshInstanceNode::GetVertexDecl() const
{
	return m_pSource->GetVertexDecl();
}

IRenderMeshNode::EEffectMode RenderMeshInstanceNode::GetEffectMode() const
{
	return m_pSource->GetEffectMode();
}

U8 RenderMeshInstanceNode::GetNumUVSets() const
{
	return m_pSource->GetNumUVSets();
}

RenderNode::ERenderNodeType RenderMeshInstanceNode::GetType() const
{
	return RenderNode::RENDER_NODE_TYPE_MESH_INSTANCE;
}

bool RenderMeshInstanceNode::HasType(RenderNode::ERenderNodeType type) const
{
	if (__super::HasType(type))
		return true;
	if (type == RenderMeshInstanceNode::GetType()) 
		return true;

	return false;
}

U32 RenderMeshInstanceNode::GetNumClusters(U32 lod) const
{
	return (U32)m_pSource->GetNumClusters(lod);
}

const MeshCluster* RenderMeshInstanceNode::GetClusterPtr(U32 lod, U32 clusterIndex) const
{
	return m_pSource->GetClusterPtr(lod, clusterIndex);
}

MeshCluster* RenderMeshInstanceNode::GetClusterPtr(U32 lod, U32 clusterIndex)
{
	return m_pSource->GetClusterPtr(lod, clusterIndex);
}

void RenderMeshInstanceNode::SelectFaces(const Ut::UtSet<Nexus::C_ComponentSelection>& selected)
{
	ASSERT(m_pComponentDisplay);

	m_pComponentDisplay->SelectFaces(
		selected);

	m_pComponentDisplay->UpdateFaces(
		m_pSceneRenderer->GetSettingsPtr()->skinningMode != 0);
}

void RenderMeshInstanceNode::SelectPoints(const Ut::UtSet<Nexus::C_ComponentSelection>& selected)
{
	ASSERT(m_pComponentDisplay);
	m_pComponentDisplay->SelectFaces(selected);
}

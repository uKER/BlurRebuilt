#include <nexus_render/stdafx.h>
#include <tools_common/Utility.h>

using namespace NexusRender;

RenderMeshBonedInstanceNode::RenderMeshBonedInstanceNode(
	SceneRenderer* pSceneRenderer, 
	Aftk::C_SceneNode* pAftkNode,
	Aftk::C_SceneMeshRemappableNode* pAftkMaterialMappingSource) :
	RenderMeshInstanceNode(
		pSceneRenderer, 
		pAftkNode, 
		pAftkMaterialMappingSource)
{
}
		
RenderMeshBonedInstanceNode::~RenderMeshBonedInstanceNode()
{
	Clear();
}

void RenderMeshBonedInstanceNode::Clear()
{
	__super::Clear();
}

Tu::Status RenderMeshBonedInstanceNode::Rebuild()
{
	Tu::Status status;

	status = RenderMeshInstanceNode::Rebuild();
	if (!status) return status;

	m_nodeFlags.isBuilt = true;
	ASSERT(m_pSource);

	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshBonedInstanceNode::Evaluate()
{
	ASSERT(m_pSource);
	Tu::Status status;
	status = m_pSource->Evaluate();
	if (!status) return status;

	Aftk::C_SceneMeshBonedInstanceNode* pAftkBonedInstance = 
		(Aftk::C_SceneMeshBonedInstanceNode* )m_pAftkSceneNode;

	if (!pAftkBonedInstance->Evaluate())
		if (!status) return Tu::Status::FAILURE;

	//Make our own local copy of the matrix palette for this boned instance.
	m_matrixPalette[0] = 
		Tu::TemplateToNightmareMatrix44(pAftkBonedInstance->GetPaletteEntry(0).matrix);
	m_matrixPalette[1] = 
		Tu::TemplateToNightmareMatrix44(pAftkBonedInstance->GetPaletteEntry(1).matrix);

	m_matrixPaletteInverseTranspose[0] = 
		Tu::TemplateToNightmareMatrix44(pAftkBonedInstance->GetPaletteEntry(0).matrixInverseTranspose);
	m_matrixPaletteInverseTranspose[1] = 
		Tu::TemplateToNightmareMatrix44(pAftkBonedInstance->GetPaletteEntry(1).matrixInverseTranspose);

	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshBonedInstanceNode::Update(
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

	status = RenderMeshInstanceNode::Update(
		timeChanged,
		scaleChanged,
		skinningModeChanged);
	if (!status) return status;

	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshBonedInstanceNode::Draw()
{
	return m_pSource->Draw(m_activeLod, RenderMeshNode::SPECIAL_CLUSTER_ENTIRE_MESH);
}
//Draw the entire mesh without any batching (default), or 
//draw only the portion of this mesh that uses the material at the given index.
Tu::Status RenderMeshBonedInstanceNode::Draw(U32 lodLevel, U32 iCluster)
{
	return m_pSource->Draw(lodLevel, iCluster);
}

D3DManager::EVertexDecl RenderMeshBonedInstanceNode::GetVertexDecl() const
{
	ASSERT(m_pSource);
	return m_pSource->GetVertexDecl();
}

IRenderMeshNode::EEffectMode RenderMeshBonedInstanceNode::GetEffectMode() const
{
	ASSERT(m_pSource);
	if (m_pSource->GetEffectMode() == IRenderMeshNode::EFFECT_MODE_SKINNED)
		return IRenderMeshNode::EFFECT_MODE_BONED_INSTANCE;

//	If we get here, then this boned instance is invalid.
//	ASSERT(false);
	return IRenderMeshNode::EFFECT_MODE_STATIC;
}

const Maths::HMat44* RenderMeshBonedInstanceNode::GetPaletteMatricesPtr() const
{
	return m_matrixPalette;
}

const Maths::HMat44* RenderMeshBonedInstanceNode::GetPaletteMatricesTransposedInversePtr() const
{
	return m_matrixPaletteInverseTranspose;
}

RenderNode::ERenderNodeType RenderMeshBonedInstanceNode::GetType() const
{
	return RenderNode::RENDER_NODE_TYPE_MESH_BONED_INSTANCE;
}

bool RenderMeshBonedInstanceNode::HasType(RenderNode::ERenderNodeType type) const
{
	if (__super::HasType(type))
		return true;
	if (type == RenderMeshBonedInstanceNode::GetType()) 
		return true;

	return false;
}

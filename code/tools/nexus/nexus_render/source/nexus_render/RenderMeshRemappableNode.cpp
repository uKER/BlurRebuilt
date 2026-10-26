#include <nexus_render/stdafx.h>
#include <tools_common/Utility.h>

using namespace NexusRender;

RenderMeshRemappableNode::RenderMeshRemappableNode(
	SceneRenderer* pSceneRenderer, 
	Aftk::C_SceneNode* pAftkNode,
	Aftk::C_SceneMeshRemappableNode* pAftkMaterialMappingSource) :
	IRenderMeshNode(pSceneRenderer, pAftkNode)
{
	m_lightmapUVOffsets[0] = 0;
	m_lightmapUVOffsets[1] = 0;
	m_pAftkMaterialMappingSource = pAftkMaterialMappingSource;
}
		
RenderMeshRemappableNode::~RenderMeshRemappableNode()
{
}

Tu::Status RenderMeshRemappableNode::Rebuild()
{
	if (m_pAftkSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappable))
	{
		Aftk::C_SceneMeshRemappableNode* pAftkMesh = 
			(Aftk::C_SceneMeshRemappableNode*)m_pAftkSceneNode;
		if (pAftkMesh->GetNumTextureCoordinateSets() > 1)
		{
			Aftk::TextureCoordinate4 uvwx = pAftkMesh->GetTextureCoordinateOffset(1);
			m_lightmapUVOffsets[0] = uvwx[0];
			m_lightmapUVOffsets[1] = uvwx[1];
		}
	}
	else
	{
		m_lightmapUVOffsets[0] = 0;
		m_lightmapUVOffsets[1] = 0;		
	}
	return Tu::Status::SUCCESS;
}

U32 RenderMeshRemappableNode::GetActiveLodLevel(const RenderNode* pRenderNode) const
{
	//The node whoes lod level's we're querying (this node).
	const Aftk::C_SceneMeshNode* pAftkMesh = (Aftk::C_SceneMeshNode*)m_pAftkSceneNode;

	//The node whoes current level of detail we're trying to determine.
	const Aftk::C_SceneNode* pAftkNode = pRenderNode->GetAftkSceneNodePtr();

	if (pAftkMesh->GetNumLodLevels() > 1)
	{
		Maths::HVec3 cameraPosWorld(m_pSceneRenderer->GetCameraPtr()->GetPosition());

		Aftk::Position3 temp = pAftkNode->GetWorldBounds().GetCentre();
		Maths::HVec3 sceneNodePosWorld(Tu::TemplateToNightmare(temp));

		F32 distance = sceneNodePosWorld.Distance(cameraPosWorld);

		for (U32 iLodLevel = 0; iLodLevel < pAftkMesh->GetNumLodLevels(); iLodLevel++)
		{
			if (iLodLevel == pAftkMesh->GetNumLodLevels()-1)
			{
				//We've reached the lowest LOD
				return iLodLevel;
			}

			const Aftk::C_MeshLodLevelInfo& info = pAftkMesh->GetLodLevelInfo(iLodLevel);
			if (distance < info.GetThreshold())
			{
				return iLodLevel;
			}
		}
	}

	return 0;
}

const F32* RenderMeshRemappableNode::GetLightmapUVOffsets() const
{
	return m_lightmapUVOffsets;
}

RenderNode::ERenderNodeType RenderMeshRemappableNode::GetType() const
{
	return RenderNode::RENDER_NODE_TYPE_MESH_REMAPPABLE;
}

bool RenderMeshRemappableNode::HasType(RenderNode::ERenderNodeType type) const
{
	if (__super::HasType(type))
		return true;
	if (type == RenderMeshRemappableNode::GetType()) 
		return true;

	return false;
}


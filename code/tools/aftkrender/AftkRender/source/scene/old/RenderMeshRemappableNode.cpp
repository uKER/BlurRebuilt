#include "stdafx.h"
#include "AftkRender.h"

using namespace AftkRender;

C_RenderMeshRemappableNode::C_RenderMeshRemappableNode(
	Aftk::C_SceneNode* pAftkSceneNode,
	Aftk::C_SceneMeshRemappableNode* pAftkMaterialMappingSource,
	C_SceneRenderer *pRenderer)
:	I_RenderMeshNode(pAftkSceneNode, pRenderer)
{
	m_LightMapUVOffsets[0] = 0;
	m_LightMapUVOffsets[1] = 0;
	m_pAftkMaterialMappingSource = pAftkMaterialMappingSource;
}

C_RenderMeshRemappableNode::~C_RenderMeshRemappableNode()
{
}

Aftk::C_Status C_RenderMeshRemappableNode::Rebuild()
{
	if (m_pAftkSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappable))
	{
		Aftk::C_SceneMeshRemappableNode* pAftkMesh = 
			(Aftk::C_SceneMeshRemappableNode*)m_pAftkSceneNode;
		if (pAftkMesh->GetNumTextureCoordinateSets() > 1)
		{
			Aftk::TextureCoordinate4 uvwx = pAftkMesh->GetTextureCoordinateOffset(1);
			m_LightMapUVOffsets[0] = uvwx[0];
			m_LightMapUVOffsets[1] = uvwx[1];
		}
	}
	else
	{
		m_LightMapUVOffsets[0] = 0.0f;
		m_LightMapUVOffsets[1] = 0.0f;		
	}
	return Aftk::C_Status::E_Status_Success;
}

const F32* C_RenderMeshRemappableNode::GetLightMapUVOffsets() const
{
	return m_LightMapUVOffsets;
}

E_RenderNodeType C_RenderMeshRemappableNode::GetType() const
{
	return E_RenderNodeType_Mesh_Remappable;
}

bool C_RenderMeshRemappableNode::HasType(E_RenderNodeType type) const
{
	return (type == C_RenderMeshRemappableNode::GetType()
			|| I_RenderMeshNode::HasType(type));
}
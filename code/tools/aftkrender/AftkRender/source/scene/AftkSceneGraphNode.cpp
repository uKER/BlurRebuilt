#include "AftkRenderPch.h"

#include "common/ShaderConstant.h"
#include "common/RenderContext.h"
#include "common/Renderer.h"
#include "common/Scene.h"
#include "common/SceneGraphNode.h"
#include "common/GeometryNode.h"

#include "scene/AftkSceneGraphNode.h"

#include "render/SceneRenderer.h"

#include "shaders/vsConstants.h"
#include "shaders/psConstants.h"

#include "Utility.h"

using namespace AftkRender;

__forceinline Maths::HMat44 ConvertToHMat44(
	const Aftk::Matrix44& rhs)
{
	return Maths::HMat44(
		Maths::HVec4(rhs[0][0], rhs[0][1], rhs[0][2], rhs[0][3]),
		Maths::HVec4(rhs[1][0], rhs[1][1], rhs[1][2], rhs[1][3]),
		Maths::HVec4(rhs[2][0], rhs[2][1], rhs[2][2], rhs[2][3]),
		Maths::HVec4(rhs[3][0], rhs[3][1], rhs[3][2], rhs[3][3]));
}

__forceinline Maths::HVec3 ConvertToHVec3( 
	const UtMaths::MPoint< UtMaths::VectorXYZ<F32> >& rhs)
{
	return Maths::HVec3(rhs[0], rhs[1], rhs[2]);
}



C_AftkSceneGraphNode::C_AftkSceneGraphNode(C_Scene *pRenderScene, Aftk::C_SceneNode* pNode)
: C_SceneGraphNode(pRenderScene, pNode->GetName().c_str(), pNode->GetUniqueId())
, m_pAftkSceneNode(pNode)
{
	m_LocalMatrix = ConvertToHMat44(m_pAftkSceneNode->GetLocalMatrix());
	m_WorldMatrix = ConvertToHMat44(m_pAftkSceneNode->GetWorldMatrix());

	Aftk::C_Bounds bounds = pNode->GetWorldBounds();
	m_WorldAABBox.Min(ConvertToHVec3(bounds.GetMin()));
	m_WorldAABBox.Max(ConvertToHVec3(bounds.GetMax()));
}

C_AftkSceneGraphNode::~C_AftkSceneGraphNode()
{
}

Aftk::C_SceneNode* C_AftkSceneGraphNode::GetAftkSceneNodePtr()
{
	return m_pAftkSceneNode;
}

const Aftk::C_SceneNode* C_AftkSceneGraphNode::GetAftkSceneNodePtr() const
{
	return m_pAftkSceneNode;
}



void C_AftkSceneGraphNode::AttachToRenderer(U8 lod,  E_RendererPass pass, bool recurse)
{
	AftkRender::C_Renderer *pRenderer = GetSceneRenderer();
	assert(pRenderer && "Failed to obtain a valid renderer");

	//FIXME: lod switching
	if (m_pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneLodGroup)
	{
		const Aftk::C_SceneLodGroupNode* pLodGroup = (const Aftk::C_SceneLodGroupNode*)m_pAftkSceneNode;
		if (pLodGroup->GetNumLodLevels())
		{
			const Aftk::C_GroupLodLevelInfo* pInfo = &pLodGroup->GetLodLevelInfo(0);

			C_SceneGraphNode *pChild = m_pChild;
			while (pChild)
			{
				if (((C_AftkSceneGraphNode*)pChild)->GetAftkSceneNodePtr() == pInfo->GetSceneNodePtr())
					pChild->AttachToRenderer(lod, pass, recurse);

				pChild = pChild->GetNextSiblingPtr();
			}
		}
	}
	else
	{
		if (recurse)
		{
			C_SceneGraphNode *pChild = m_pChild;
			while (pChild)
			{
				pChild->AttachToRenderer(lod, pass, true);
				pChild = pChild->GetNextSiblingPtr();
			}
		}
	}
}





C_AftkMeshSceneGraphNode::C_AftkMeshSceneGraphNode(C_Scene *pRenderScene, Aftk::C_SceneNode* pNode)
: C_AftkSceneGraphNode(pRenderScene, pNode)
, m_pGeometryNode(0)
{
}

C_AftkMeshSceneGraphNode::~C_AftkMeshSceneGraphNode()
{
}

void C_AftkMeshSceneGraphNode::SetGeometryNode(C_ResourcePtr<C_GeometryNode> pGeometry)
{
	m_pGeometryNode = pGeometry;
}

C_ResourcePtr<C_GeometryNode> C_AftkMeshSceneGraphNode::GetGeometryNode()
{
	return m_pGeometryNode;
}

const C_ResourcePtr<C_GeometryNode> C_AftkMeshSceneGraphNode::GetGeometryNode() const
{
	return m_pGeometryNode;
}

void C_AftkMeshSceneGraphNode::AttachToRenderer(U8 lod,  E_RendererPass pass, bool recurse)
{
	AftkRender::C_SceneRenderer *pRenderer = GetSceneRenderer();
	assert(pRenderer && "Failed to obtain a valid renderer");
	if (m_pGeometryNode && !m_Flags.hidden)
	{
		for (U32 i=0; i<m_pGeometryNode->GetNumClusters(); ++i)
		{
			const C_GeometryCluster *pCluster = m_pGeometryNode->GetClusterPtr(i);
			if (pCluster)
			{
				for (U32 j=0; j<pCluster->GetNumPrimitiveBatches(); ++j)
				{
					C_RenderContext renderContext(pCluster, j);
					renderContext.AddShaderConstant(C_ShaderConstant(C_ShaderConstant::E_ShaderType_Vertex, CV_DISPLAY_WORLD_MATRIX, &m_WorldMatrix));

					if (pass != E_RendererPass_VisibilityTest)
					{
						const Aftk::C_SceneMeshRemappableNode *pAftkNode = 0;

						if (m_pAftkSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappable))
							pAftkNode = (const Aftk::C_SceneMeshRemappableNode*) m_pAftkSceneNode;
						else if (m_pAftkSceneNode->HasType(Aftk::E_ObjType_SceneInstance))
							pAftkNode = (const Aftk::C_SceneMeshRemappableNode*) ((Aftk::C_SceneInstanceNode*)m_pAftkSceneNode)->GetSourceNodePtr();

						assert(pAftkNode!=0 && "Failed to obtain aftk mesh node");

						S16 materialIndex = pCluster->GetPrimitiveBatch(j).material;
						if (materialIndex != -1)
						{
							const Aftk::C_MaterialResource *pMaterial = pAftkNode->GetPolygonMaterialMappingPtr(materialIndex);
							U32 remappedIndex = pMaterial->GetIndex();
							renderContext.SetRemappedMaterialIndex((S32)remappedIndex);
							renderContext.GetFlags().isTextured = true;

							if (pMaterial->GetContent().GetMaterialTypeId() == CGFX_SPEEDTREE_LEAF)
								renderContext.GetFlags().isBillboard = true;
						}
					}
					pRenderer->AddRenderContext(renderContext, this);
				}
			}
		}
	}

	if (recurse)
	{
		C_SceneGraphNode *pChild = m_pChild;
		while (pChild)
		{
			pChild->AttachToRenderer(lod, pass, true);
			pChild = pChild->GetNextSiblingPtr();
		}
	}
}



const U32 C_AftkMeshSkinnedSceneGraphNode::m_MaxMatrixPaletteSize = 2;

C_AftkMeshSkinnedSceneGraphNode::C_AftkMeshSkinnedSceneGraphNode(C_Scene *pRenderScene, Aftk::C_SceneNode* pNode)
: C_AftkMeshSceneGraphNode(pRenderScene, pNode)
{
	assert(pNode->GetType() == Aftk::E_ObjType_SceneMeshBonedInstance && "GPU skinning is only supported for boned instances");

	m_pMatrixPalette = new Maths::HMat44[m_MaxMatrixPaletteSize];

	Aftk::C_SceneMeshBonedInstanceNode* pAftkBonedInstance = (Aftk::C_SceneMeshBonedInstanceNode*) pNode;
	m_pMatrixPalette[0] = ConvertToHMat44(pAftkBonedInstance->GetPaletteEntry(0).matrix * pAftkBonedInstance->GetWorldMatrix());
	m_pMatrixPalette[1] = ConvertToHMat44(pAftkBonedInstance->GetPaletteEntry(1).matrix * pAftkBonedInstance->GetWorldMatrix());
}

C_AftkMeshSkinnedSceneGraphNode::~C_AftkMeshSkinnedSceneGraphNode()
{
	delete[] m_pMatrixPalette;
}

void C_AftkMeshSkinnedSceneGraphNode::AttachToRenderer(U8 lod,  E_RendererPass pass, bool recurse)
{
	AftkRender::C_SceneRenderer *pRenderer = GetSceneRenderer();
	assert(pRenderer && "Failed to obtain a valid renderer");
	if (m_pGeometryNode && !m_Flags.hidden)
	{
		for (U32 i=0; i<m_pGeometryNode->GetNumClusters(); ++i)
		{
			const C_GeometryCluster *pCluster = m_pGeometryNode->GetClusterPtr(i);
			if (pCluster)
			{
				for (U32 j=0; j<pCluster->GetNumPrimitiveBatches(); ++j)
				{
					C_RenderContext renderContext(pCluster, j);
					renderContext.GetFlags().isSkinned = true;

					renderContext.AddShaderConstant(C_ShaderConstant(C_ShaderConstant::E_ShaderType_Vertex, CV_DISPLAY_WORLD_MATRIX, &m_WorldMatrix));

					for (U32 k=0; k<m_MaxMatrixPaletteSize; ++k)
						renderContext.AddShaderConstant(C_ShaderConstant(C_ShaderConstant::E_ShaderType_Vertex, CV_AFTKRENDER_MATRIX_PALETTE + k*4, &m_pMatrixPalette[k]));
					
					if (pass != E_RendererPass_VisibilityTest)
					{
						const Aftk::C_SceneMeshRemappableNode *pAftkNode = 
							(const Aftk::C_SceneMeshRemappableNode*) m_pAftkSceneNode;

						S16 materialIndex = pCluster->GetPrimitiveBatch(j).material;
						if (materialIndex != -1)
						{
							U32 remappedIndex = pAftkNode->GetPolygonMaterialMappingPtr(materialIndex)->GetIndex();
							renderContext.SetRemappedMaterialIndex((S32)remappedIndex);
							renderContext.GetFlags().isTextured = true;
						}
					}
					pRenderer->AddRenderContext(renderContext, this);
				}
			}
		}
	}

	if (recurse)
	{
		C_SceneGraphNode *pChild = m_pChild;
		while (pChild)
		{
			pChild->AttachToRenderer(lod, pass, true);
			pChild = pChild->GetNextSiblingPtr();
		}
	}
}
#include "AftkRenderPch.h"

#include "common/TextureManager.h"
#include "common/TextureState.h"
#include "common/Material.h"
#include "common/ShaderConstant.h"
#include "common/RenderContext.h"
#include "common/Renderer.h"
#include "common/Scene.h"
#include "common/SceneGraphNode.h"
#include "common/GeometryNode.h"

#include "mesh/MeshElements.h"
#include "mesh/Mesh.h"
#include "mesh/MeshSceneGraphNode.h"

#include "render/SceneRenderer.h"

#include "shaders/vsConstants.h"
#include "shaders/psConstants.h"

#include "display/shaders/psConstants.h"

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



C_MeshSceneGraphNode::C_MeshSceneGraphNode(C_Scene *pRenderScene, const char* pName, S32 id)
: C_SceneGraphNode(pRenderScene, pName, id)
, m_pMeshResource(0)
, m_pGeometryNode(0)
{

}

C_MeshSceneGraphNode::~C_MeshSceneGraphNode()
{
}

void C_MeshSceneGraphNode::UpdateCache() const
{
	C_SceneGraphNode::UpdateCache();

	if (m_pMeshResource)
	{
		//update bounding box
		m_WorldAABBox.SetInvalidExtents();
		
		for (U32 i=0; i<m_pMeshResource->GetNumClusters(); ++i)
		{
			const C_MeshClusterResource *pCluster = m_pMeshResource->GetCluster(i);

			const F32* pVtxPositions = (const F32*)pCluster->GetVertexDataElement(AftkRender::E_ElementType_Position);
			assert(pVtxPositions);

			for (U32 j=0; j<pCluster->GetNumVertices(); ++j)
			{
				Maths::HVec4 p(pVtxPositions[0],pVtxPositions[1],pVtxPositions[2], 1.f);
				p = p * m_WorldMatrix;

				m_WorldAABBox.UpdateExtents(Maths::C_Vertex(p[0], p[1], p[2]));
				pVtxPositions += 3;
			}
		}
	}
}

void C_MeshSceneGraphNode::SetMeshResource(AftkRender::C_ResourcePtr<C_MeshResource> pMesh)
{
	m_pMeshResource = pMesh;
}

C_ResourcePtr<C_MeshResource> C_MeshSceneGraphNode::GetMeshResource()
{
	return m_pMeshResource;
}

const C_ResourcePtr<C_MeshResource> C_MeshSceneGraphNode::GetMeshResource() const
{
	return m_pMeshResource;
}

void C_MeshSceneGraphNode::SetGeometryNode(C_ResourcePtr<C_GeometryNode> pGeometry)
{
	m_pGeometryNode = pGeometry;
}

C_ResourcePtr<C_GeometryNode> C_MeshSceneGraphNode::GetGeometryNode()
{
	return m_pGeometryNode;
}

const C_ResourcePtr<C_GeometryNode> C_MeshSceneGraphNode::GetGeometryNode() const
{
	return m_pGeometryNode;
}

void C_MeshSceneGraphNode::SetMaterialMapping(U32 index, S16 mapping)
{
	U32 size = m_MaterialIndexMap.Size();
	if (index >= size)
	{
		m_MaterialIndexMap.Resize(index+1);
		for (U32 i=size; i<index; ++i)
			m_MaterialIndexMap[i] = -1;

		m_MaterialIndexMap[index] = mapping;
	}
}

const Ut::UtVector< Ut::DynamicVector<S16> >& C_MeshSceneGraphNode::GetMaterialMapping() const
{
	return m_MaterialIndexMap;
}

void C_MeshSceneGraphNode::AttachToRenderer(U8 lod, E_RendererPass pass, bool recurse)
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
					renderContext.AddShaderConstant(
						C_ShaderConstant(C_ShaderConstant::E_ShaderType_Vertex, CV_DISPLAY_WORLD_MATRIX, &m_WorldMatrix));

					S16 materialIndex = pCluster->GetPrimitiveBatch(j).material;
					S16 remappedIndex = (materialIndex<0) ? -1 : m_MaterialIndexMap[materialIndex];
					S16 actualRemappedIndex = m_pScene->GetMaterialIndexMapping(remappedIndex);
					const AftkRender::C_Material* pMaterial = m_pScene->GetMaterial(actualRemappedIndex);
					if ((pMaterial && pMaterial->GetMaterialTypeId() == CGFX_SPEEDTREE_LEAF) || materialIndex == -2)
						renderContext.GetFlags().isBillboard = true;

					if (pass != E_RendererPass_VisibilityTest)
					{
						if (remappedIndex!=-1)
						{
							renderContext.GetFlags().isTextured = true;
							renderContext.SetRemappedMaterialIndex((S32)remappedIndex);
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


const U32 C_MeshSkinnedSceneGraphNode::m_MaxMatrixPaletteSize = 2;

C_MeshSkinnedSceneGraphNode::C_MeshSkinnedSceneGraphNode(C_Scene *pRenderScene, const char* pName, S32 id)
: C_MeshSceneGraphNode(pRenderScene, pName, id)
{
	m_pMatrixPalette = new Maths::HMat44[m_MaxMatrixPaletteSize];
}

C_MeshSkinnedSceneGraphNode::~C_MeshSkinnedSceneGraphNode()
{
	delete[] m_pMatrixPalette;
}

void C_MeshSkinnedSceneGraphNode::UpdateCache() const
{
	C_SceneGraphNode::UpdateCache();
	//todo.. bounding box calculation.
	m_WorldAABBox.UpdateExtents(Maths::C_Sphere(Maths::C_Vertex((Maths::HVec3&)m_WorldMatrix[3]), 10.f));
}

void C_MeshSkinnedSceneGraphNode::SetMatrixPaletteEntry(U32 index, const Maths::HMat44& matrix)
{
	assert(index < m_MaxMatrixPaletteSize);
	m_pMatrixPalette[index] = matrix;
}

void C_MeshSkinnedSceneGraphNode::AttachToRenderer(U8 lod, E_RendererPass pass, bool recurse)
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

					for (U32 k=0; k<m_MaxMatrixPaletteSize; ++k)
						renderContext.AddShaderConstant(C_ShaderConstant(C_ShaderConstant::E_ShaderType_Vertex, CV_AFTKRENDER_MATRIX_PALETTE + k*4, &m_pMatrixPalette[k]));

					if (pass != E_RendererPass_VisibilityTest)
					{
						S16 materialIndex = pCluster->GetPrimitiveBatch(j).material;
						S16 remappedIndex = (materialIndex==-1) ? -1 : m_MaterialIndexMap[materialIndex];
						if (remappedIndex!=-1)
						{
							renderContext.GetFlags().isTextured = true;
							renderContext.SetRemappedMaterialIndex((S32)remappedIndex);
						}
					}
					renderContext.GetFlags().isSkinned = true;
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
#ifndef AFTK_RENDER_MESH_INSTANCE_INCLUDE
#define AFTK_RENDER_MESH_INSTANCE_INCLUDE

#ifdef _MSC_VER
#pragma once
#endif

namespace Aftk
{
	class C_Status;
}

namespace AftkRender
{
	class C_RenderMeshInstanceNode : public C_RenderMeshRemappableNode
	{
	public:
		C_RenderMeshInstanceNode(
			Aftk::C_SceneNode *pAftkNode,
			Aftk::C_SceneMeshRemappableNode *pAftkMaterialMappingSource,
			C_SceneRenderer *pRenderer);

		~C_RenderMeshInstanceNode();

		Aftk::C_Status Rebuild();

		Aftk::C_Status Render(U32 lodLevel=0);

		Aftk::C_Status Render(U32 lodLevel, U32 clusterIndex);

		U32 GetNumClusters(U32 lod) const;

		const C_RenderMeshCluster* GetMeshClusterPtr(U32 lodLevel, U32 clusterIndex) const;

		C_RenderMeshCluster* GetMeshClusterPtr(U32 lodLevel, U32 clusterIndex);

		Display::C_VertexDeclaration* GetVertexDeclaration() const;

		I_RenderMeshNode::E_EffectMode GetEffectMode() const;

		U8 GetNumUVSets() const;

		E_RenderNodeType GetType() const;

		bool HasType(E_RenderNodeType type) const;

		void SetSourceNodePtr(C_RenderMeshNode* pRenderMeshNode);

		const C_RenderMeshNode* GetSourceNodePtr() const;

		C_RenderMeshNode* GetSourceNodePtr();

	private:
		C_RenderMeshNode* m_pSourceNode;
	
	
	};

}
#endif
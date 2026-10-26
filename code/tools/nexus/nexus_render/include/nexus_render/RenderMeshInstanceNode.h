#pragma once

namespace NexusRender
{
	class SceneRenderer;

	class RenderMeshInstanceNode : public RenderMeshRemappableNode
	{
	public:
		RenderMeshInstanceNode(
			SceneRenderer* pSceneRenderer, 
			Aftk::C_SceneNode* pAftkNode,
			Aftk::C_SceneMeshRemappableNode* pAftkMaterialMappingSource);
		
		virtual ~RenderMeshInstanceNode();

		virtual RenderNode::ERenderNodeType GetType() const;

		virtual bool HasType(RenderNode::ERenderNodeType type) const;

		U32 GetNumClusters(U32 lod) const;

		const MeshCluster* GetClusterPtr(U32 lodLevel, U32 iMeshCluster) const;

		MeshCluster* GetClusterPtr(U32 lodLevel, U32 iMeshCluster);

		virtual void Clear();

		virtual Tu::Status Rebuild();

		virtual Tu::Status Update(
			bool timeChanged,
			bool scaleChanged,
			bool skinningModeChanged);

		virtual Tu::Status OnSelected();

		virtual Tu::Status OnDeselected();

		virtual Tu::Status DrawNormals(U32 lodLevel);

		virtual Tu::Status DrawTangentSpace(U32 lodLevel, U16 set);

		virtual Tu::Status DrawSelectedComponents(U32 lodLevel);

		virtual Tu::Status Draw();

		virtual Tu::Status Draw(U32 lodLevel, U32 clusterIndex);

		virtual Tu::Status SetSourcePtr(RenderMeshNode* pRenderMeshNode);

		RenderMeshNode* GetSourcePtr() const;

		virtual bool IsSkinned() const;

		virtual bool IsSkinAnimated() const;

		///\brief Returns an enum indicating which vertex declaration should be used to render this mesh.
		///
		///@return The vertex declaration enum.
		D3DManager::EVertexDecl GetVertexDecl() const;

		IRenderMeshNode::EEffectMode GetEffectMode() const;

		U8 GetNumUVSets() const;

		virtual void SelectFaces(const Ut::UtSet<Nexus::C_ComponentSelection>& selection);

		virtual void SelectPoints(const Ut::UtSet<Nexus::C_ComponentSelection>& selection);

	protected:
		Tu::Status Evaluate();

		void SetActiveLodLevel();

		RenderMeshNode*	m_pSource;

		MeshComponentDisplay* m_pComponentDisplay;
	};
}
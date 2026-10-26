#pragma once

namespace NexusRender
{
	class SceneRenderer;

	class RenderMeshRemappableNode : public IRenderMeshNode
	{
	public:
		RenderMeshRemappableNode(
			SceneRenderer* pSceneRenderer, 
			Aftk::C_SceneNode* pAftkNode,
			Aftk::C_SceneMeshRemappableNode* pAftkMaterialMappingSource);

		virtual ~RenderMeshRemappableNode();

		Tu::Status Rebuild();

		const F32* GetLightmapUVOffsets() const;

		virtual RenderNode::ERenderNodeType GetType() const;

		virtual bool HasType(RenderNode::ERenderNodeType type) const;

		virtual void SelectFaces(const Ut::UtSet<Nexus::C_ComponentSelection>& selection) = 0;

		virtual void SelectPoints(const Ut::UtSet<Nexus::C_ComponentSelection>& selection) = 0;

	protected:
		U32 GetActiveLodLevel(const RenderNode* pRenderNode) const;

		//Texture coordinate offsets
		F32 m_lightmapUVOffsets[2];
		Aftk::C_SceneMeshRemappableNode*	m_pAftkMaterialMappingSource;
	};
}

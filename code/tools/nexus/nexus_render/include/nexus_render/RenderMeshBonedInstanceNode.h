#pragma once

namespace NexusRender
{
	class SceneRenderer;

	class RenderMeshBonedInstanceNode : public RenderMeshInstanceNode
	{
	public:
		RenderMeshBonedInstanceNode(
			SceneRenderer* pSceneRenderer, 
			Aftk::C_SceneNode* pAftkNode,
			Aftk::C_SceneMeshRemappableNode* pAftkMaterialMappingSource);
		
		~RenderMeshBonedInstanceNode();

		virtual RenderNode::ERenderNodeType GetType() const;

		virtual bool HasType(RenderNode::ERenderNodeType type) const;

		void Clear();

		Tu::Status Rebuild();

		Tu::Status Draw();

		//Draw the entire mesh without any batching (default), or 
		//draw only the portion of this mesh that uses the material at the given index.
		Tu::Status Draw(U32 lodLevel, U32 clusterIndex);

		Tu::Status Update(
			bool timeChanged,
			bool scaleChanged,
			bool skinningModeChanged);

		const Maths::HMat44* GetPaletteMatricesPtr() const;

		const Maths::HMat44* GetPaletteMatricesTransposedInversePtr() const;

		///\brief Returns an enum indicating which vertex declaration should be used to render this mesh.
		///
		///@return The vertex declaration enum.
		D3DManager::EVertexDecl GetVertexDecl() const;

		IRenderMeshNode::EEffectMode GetEffectMode() const;

	private:
		Tu::Status EvaluateMatrixPalette();

		Tu::Status Evaluate();

		Maths::HMat44 m_matrixPalette[2];
		Maths::HMat44 m_matrixPaletteInverseTranspose[2];
	};
}
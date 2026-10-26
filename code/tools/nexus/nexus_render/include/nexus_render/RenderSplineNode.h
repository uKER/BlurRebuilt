#pragma once

namespace NexusRender
{
	class SceneRenderer;

	class RenderSplineNode : public RenderNode
	{
	public:
		RenderSplineNode(
			SceneRenderer* pSceneRenderer, 
			Aftk::C_SceneNode* pAftkSceneNode);

		~RenderSplineNode();

		virtual RenderNode::ERenderNodeType GetType() const;

		virtual bool HasType(RenderNode::ERenderNodeType type) const;

		void Clear();

		Tu::Status Rebuild();

		Tu::Status Update(
			bool timeChanged,
			bool scaleChanged,
			bool skinningModeChanged);

		Tu::Status Draw(U32 lodLevel);

	protected:
		RenderSplineNode();
		RenderSplineNode(const RenderSplineNode& rhs);
		const RenderSplineNode& operator=(const RenderSplineNode& rhs);

		bool GetSplineParamFromIter(
			F32& u,
			const Aftk::C_SceneSplineNode* pSpline,
			const U32 iter) const;

		///
		///\brief Evaluate the current local matrix, world matrix and world-view-projection matrix.
		Tu::Status Evaluate();

		VBWrapper m_splineVBWrapper;
	};
}
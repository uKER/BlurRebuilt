#pragma once

namespace NexusRender
{
	class SceneRenderer;

	class RenderSpotLightNode : public RenderNonAmbientLightNode
	{
	public:
		RenderSpotLightNode(
			SceneRenderer* pSceneRenderer,
			Aftk::C_SceneNode* pAftkSceneNode);

		~RenderSpotLightNode();

		virtual RenderNode::ERenderNodeType GetType() const;

		virtual bool HasType(RenderNode::ERenderNodeType type) const;

		Tu::Status Rebuild();

		Tu::Status Update(
			bool timeChanged,
			bool scaleChanged,
			bool skinningModeChanged);

		Tu::Status Draw();

	protected:
		RenderSpotLightNode();
		RenderSpotLightNode(const RenderNode& rhs);
		const RenderSpotLightNode& operator=(const RenderSpotLightNode& rhs);

		///
		///\brief Evaluate the current local matrix, world matrix and world-view-projection matrix.
		Tu::Status Evaluate();
	};
}
#pragma once

namespace NexusRender
{
	class SceneRenderer;

	class RenderPointLightNode : public RenderNonAmbientLightNode
	{
	public:
		RenderPointLightNode(
			SceneRenderer* pSceneRenderer, 
			Aftk::C_SceneNode* pAftkSceneNode);
		
		~RenderPointLightNode();

		virtual RenderNode::ERenderNodeType GetType() const;

		virtual bool HasType(RenderNode::ERenderNodeType type) const;

		void Clear();

		Tu::Status Rebuild();

		Tu::Status Update(
			bool timeChanged,
			bool scaleChanged,
			bool skinningModeChanged);

		Tu::Status Draw();

	protected:
		RenderPointLightNode();
		RenderPointLightNode(const RenderPointLightNode& rhs);
		const RenderPointLightNode& operator=(const RenderPointLightNode& rhs);

		///
		///\brief Evaluate the current local matrix, world matrix and world-view-projection matrix.
		virtual Tu::Status Evaluate();
	};
}
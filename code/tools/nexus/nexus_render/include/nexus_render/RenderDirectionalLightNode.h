#ifndef NEXUS_RENDER_RENDER_DIRECTIONAL_LIGHT_NODE_H
#define NEXUS_RENDER_RENDER_DIRECTIONAL_LIGHT_NODE_H

namespace NexusRender
{
	class SceneRenderer;

	class RenderDirectionalLightNode : public RenderNonAmbientLightNode
	{
	public:
		RenderDirectionalLightNode(
			SceneRenderer* pSceneRenderer, 
			Aftk::C_SceneNode* pAftkSceneNode);

		~RenderDirectionalLightNode();

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
		RenderDirectionalLightNode();
		RenderDirectionalLightNode(const RenderDirectionalLightNode& rhs);
		const RenderDirectionalLightNode& operator=(const RenderDirectionalLightNode& rhs);

		Tu::Status Evaluate();
	};
}

#endif
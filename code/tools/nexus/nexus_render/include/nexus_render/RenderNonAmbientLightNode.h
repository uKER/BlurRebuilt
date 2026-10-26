#ifndef NEXUS_RENDER_RENDER_NON_AMBIENT_LIGHT_NODE_H
#define NEXUS_RENDER_RENDER_NON_AMBIENT_LIGHT_NODE_H

namespace NexusRender
{
	class SceneRenderer;

	class RenderNonAmbientLightNode : public RenderLightNode
	{
	public:
		RenderNonAmbientLightNode(
			SceneRenderer* pSceneRenderer,
			Aftk::C_SceneNode* pAftkSceneNode);
		
		virtual ~RenderNonAmbientLightNode();

		virtual RenderNode::ERenderNodeType GetType() const;

		virtual bool HasType(RenderNode::ERenderNodeType type) const;

		virtual void Clear();

	protected:
		RenderNonAmbientLightNode();
		RenderNonAmbientLightNode(const RenderNonAmbientLightNode& rhs);
		const RenderNonAmbientLightNode& operator=(const RenderNonAmbientLightNode& rhs);
	};
}

#endif

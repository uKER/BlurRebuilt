#ifndef NEXUS_RENDER_RENDER_AREA_LIGHT_NODE_H
#define NEXUS_RENDER_RENDER_AREA_LIGHT_NODE_H

namespace NexusRender
{
	class SceneRenderer;

	class RenderAreaLightNode : public RenderNonAmbientLightNode
	{
	public:
		RenderAreaLightNode(
			SceneRenderer* pSceneRenderer, 
			Aftk::C_SceneNode* pAftkSceneNode);
		
		~RenderAreaLightNode();

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
		///
		///\brief Evaluate the current local matrix, world matrix and world-view-projection matrix.
		virtual Tu::Status Evaluate();
	};
}

#endif

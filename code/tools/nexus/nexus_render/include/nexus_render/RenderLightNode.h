#pragma once

namespace NexusRender
{
	class SceneRenderer;

	class RenderLightNode : public RenderNode
	{
	public:
		RenderLightNode(
			SceneRenderer* pSceneRenderer,
			Aftk::C_SceneNode* pAftkSceneNode);

		virtual ~RenderLightNode();

		void Clear();

	protected:
		RenderLightNode();
		RenderLightNode(const RenderLightNode& rhs);
		const RenderLightNode& operator=(const RenderLightNode& rhs);
	};
}
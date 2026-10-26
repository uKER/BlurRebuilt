#pragma once

namespace NexusRender
{
	class SceneRenderer;

	class RenderInstanceNode : public RenderNode
	{
	public:
		RenderInstanceNode(
			SceneRenderer* pSceneRenderer,
			Aftk::C_SceneNode* pAftkSceneNode);

		~RenderInstanceNode();

		virtual RenderNode::ERenderNodeType GetType() const;

		virtual bool HasType(RenderNode::ERenderNodeType type) const;

		void Clear();

		Tu::Status Rebuild();

		Tu::Status Update(
			bool timeChanged,
			bool scaleChanged,
			bool skinningModeChanged);

		Tu::Status Draw(U32 lodLevel);

		void SetSourcePtr(RenderNode* pSource);

		RenderNode* GetSourcePtr();

		const RenderNode* GetSourcePtr() const;

	protected:
		RenderInstanceNode();
		RenderInstanceNode(const RenderInstanceNode& rhs);
		const RenderInstanceNode& operator=(const RenderInstanceNode& rhs);

		Tu::Status Evaluate();

		void SetActiveLodLevel();

		RenderNode* m_pSource;
	};
}
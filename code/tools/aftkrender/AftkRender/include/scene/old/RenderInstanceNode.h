#ifndef AFTK_RENDER_NODE_INSTANCE_INCLUDE
#define AFTK_RENDER_NODE_INSTANCE_INCLUDE

#ifdef _MSC_VER
#pragma once
#endif


namespace AftkRender
{
	class C_RenderNode;

	class C_RenderInstanceNode : public C_RenderNode
	{
	public:
		C_RenderInstanceNode(
			Aftk::C_SceneNode *pAftkNode,
			C_SceneRenderer *pRenderer);

		virtual ~C_RenderInstanceNode();

		virtual E_RenderNodeType GetType() const;

		virtual bool HasType(
			E_RenderNodeType type) const;

		void SetSourceNodePtr(C_RenderNode *pRenderNode);

		const C_RenderNode* GetSourceNodePtr() const;
		C_RenderNode* GetSourceNodePtr();

	private:
		C_RenderNode* m_pSourceNode;
	};
}

#endif
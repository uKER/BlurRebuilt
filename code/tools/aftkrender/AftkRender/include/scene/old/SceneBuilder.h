#ifndef AFTK_RENDER_SCENE_BUILDER_INCLUDE
#define AFTK_RENDER_SCENE_BUILDER_INCLUDE

#ifdef _MSC_VER
#pragma once
#endif

namespace Aftk
{
	class C_Status;
	class C_SceneNode;
}

namespace AftkRender
{

	template <typename NodeType, typename MemManagerType = C_NullMemoryManager>
	class I_SceneNodeVisitor
	{
	public:
		virtual ~I_SceneNodeVisitor(){};

		// Always return true at the moment - used by Aftk::C_SceneNodeVisit
		// we need to process all nodes in order to reconstruct the scene hierarchy.
		virtual bool IsInteresting(NodeType *pNode) = 0;

		virtual Aftk::C_Status Visit(
			NodeType *node,
			MemManagerType &mmgr) = 0;
	};



	class C_RenderNodeBuilder : public I_SceneNodeVisitor<Aftk::C_SceneNode>
	{
	public:
		C_RenderNodeBuilder(
			C_SceneRenderer *pRenderer,
			Ut::UtMap<const Aftk::C_SceneNode*, C_RenderNode*> *pAftkRenderNodeMap);

		~C_RenderNodeBuilder();

		bool IsInteresting(Aftk::C_SceneNode *pNode);

		Aftk::C_Status Visit(
			Aftk::C_SceneNode *pNode,
			C_NullMemoryManager& memoryManager);

	private:
		C_SceneRenderer *m_pRenderer;
		Ut::UtMap<const Aftk::C_SceneNode*, C_RenderNode*> *m_pAftkRenderNodeMap;

	};


	class C_RenderNodeHierarchyBuilder : public I_SceneNodeVisitor<Aftk::C_SceneNode>
	{
	public:
		C_RenderNodeHierarchyBuilder(
			Ut::UtMap<const Aftk::C_SceneNode*, C_RenderNode*> *pAftkRenderNodeMap);

		bool IsInteresting(Aftk::C_SceneNode *pNode);

		Aftk::C_Status Visit(
			Aftk::C_SceneNode *pNode,
			C_NullMemoryManager& memoryManager);

	private:
		Ut::UtMap<const Aftk::C_SceneNode*, C_RenderNode*> *m_pAftkRenderNodeMap;
	};


	class C_RenderNodeInstanceBuilder : public I_SceneNodeVisitor<Aftk::C_SceneNode>
	{
	public:
		C_RenderNodeInstanceBuilder(
			Ut::UtMap<const Aftk::C_SceneNode*, C_RenderNode*> *pAftkRenderNodeMap);

		bool IsInteresting(Aftk::C_SceneNode *pNode);

		Aftk::C_Status Visit(
			Aftk::C_SceneNode *pNode,
			C_NullMemoryManager& memoryManager);

	private:
		Ut::UtMap<const Aftk::C_SceneNode*, C_RenderNode*> *m_pAftkRenderNodeMap;
	};
}

#endif
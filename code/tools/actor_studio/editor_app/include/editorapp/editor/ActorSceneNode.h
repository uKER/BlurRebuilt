#ifndef EDITOR_ACTOR_SCENE_GRAPH_NODE_H
#define EDITOR_ACTOR_SCENE_GRAPH_NODE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace EditorLib
{
	struct XtVisualisationDef;
}

namespace Editor
{
	class C_SimpleRenderer;
	class C_ActorScene;
	class C_DgNodeSelection;

	enum E_CustomNodeType
	{
		E_CustomNodeType_Actor=1,
		E_CustomNodeType_Property
	};

	class C_DgNodeBase : public AftkRender::C_SceneGraphNode
	{
	public:
		C_DgNodeBase(C_ActorScene *pScene = 0, const EditorLib::I_DgNode* dgNode = 0);
		~C_DgNodeBase();

		const EditorLib::I_DgNode* GetDgNode() const;

		//for self-drawn nodes
		virtual bool GetVisible(const RECT& region, C_DgNodeSelection *pResult);

		virtual void AttachToRenderer(
			AftkRender::E_RendererPass pass=AftkRender::E_RendererPass_Normal);

		void SetDgNode(const EditorLib::I_DgNode* pDgNode);

		virtual void OnNodeDeserialised(const EditorLib::I_DgNode* dgNode=0);

		virtual void OnNodeResolved(const EditorLib::I_DgNode* dgNode=0);

		virtual Maths::C_AxisBox GetSelectionAABBox() const;

	protected:
		const EditorLib::I_DgNode* m_pDgNode;
	};


	class C_ActorSceneNode : public C_DgNodeBase
	{
	public:
		C_ActorSceneNode(C_ActorScene *pScene, 
			const EditorLib::C_SceneXtNodeBase* pDgNode);
		~C_ActorSceneNode();

		virtual AftkRender::E_SceneGraphNodeType GetType() const
		{
			return AftkRender::E_SceneGraphNode_Custom;
		}

		virtual bool IsA(AftkRender::E_SceneGraphNodeType type) const
		{
			return (type==AftkRender::E_SceneGraphNode_Custom ||
				AftkRender::C_SceneGraphNode::IsA(type));
		}

		virtual void OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode);

		void UpdateCache() const;

		virtual void AttachToRenderer(
			AftkRender::E_RendererPass pass=AftkRender::E_RendererPass_Normal);

		const char* GetPrimitiveTypeName();

	protected:
		void AttachToRenderer(U8 lod=0, 
			AftkRender::E_RendererPass pass=AftkRender::E_RendererPass_Normal,
			bool recurse=true)
		{ return AttachToRenderer(pass); }

		void RefreshProperties();

	private:

		mutable struct CacheFlags
		{
			U32 dirty : 1;
		}	m_CacheFlags;

		mutable const EditorLib::XtVisualisationDef *m_pDef;
		mutable const char* m_pPrimitiveTypeName;
	};

	class C_GroupSceneNode  : public C_DgNodeBase
	{
	public:
		C_GroupSceneNode(C_ActorScene *pScene, 
			const EditorLib::C_SceneXtNode* pDgNode);
		~C_GroupSceneNode();

		virtual AftkRender::E_SceneGraphNodeType GetType() const
		{
			return AftkRender::E_SceneGraphNode_Custom;
		}

		virtual bool IsA(AftkRender::E_SceneGraphNodeType type) const
		{
			return (type==AftkRender::E_SceneGraphNode_Custom ||
				AftkRender::C_SceneGraphNode::IsA(type));
		}

		virtual void OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode);

		void UpdateCache() const;

		virtual void AttachToRenderer(
			AftkRender::E_RendererPass pass=AftkRender::E_RendererPass_Normal);

	private:
		void AttachToRenderer(U8 lod=0, 
			AftkRender::E_RendererPass pass=AftkRender::E_RendererPass_Normal,
			bool recurse=true)
		{ return AttachToRenderer(pass); }
	};

}
#endif

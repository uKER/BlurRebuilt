#ifndef EDITOR_ACTOR_SCENE_H
#define EDITOR_ACTOR_SCENE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_DgNodeBase;

	class C_ActorScene : public AftkRender::C_Scene
	{
	public:
		typedef Ut::UtMap<const EditorLib::I_DgNode*, C_DgNodeBase*> DgRenderMap;

		C_ActorScene();
		~C_ActorScene();

		void Initialise();

		void Clear();

		AftkRender::C_SceneGraphNode* Find(const EditorLib::I_DgNode*);
		const AftkRender::C_SceneGraphNode* Find(const EditorLib::I_DgNode*) const;

		//visibility of self-drawn nodes
		void GetVisible(const RECT& region,	C_DgNodeSelection& result);

		AftkRender::C_SceneGraphNode* AddActorSceneNode(const EditorLib::I_DgNode& dgNode);
		void RemoveActorSceneNode(const EditorLib::I_DgNode& dgNode);

		S32 OnCreateSceneGraphNode(AftkRender::C_SceneGraphNode *pSgNode, S32 preferredId);
		void OnDestroySceneGraphNode(AftkRender::C_SceneGraphNode *pSgNode);

		void OnNodeDeserialised(const EditorLib::I_DgNode *pNode);
		void OnNodeResolved(const EditorLib::I_DgNode *pNode);

		const DgRenderMap& GetDgRenderMap();

	private:
		void RebuildDgRenderNodeMap() const;

		mutable bool m_DgRenderNodeMapDirty;
		mutable DgRenderMap m_DgRenderNodeMap;
	};
}

#endif
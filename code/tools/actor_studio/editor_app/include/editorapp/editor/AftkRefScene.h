#ifndef EDITOR_AFTK_REFSCENE_H
#define EDITOR_AFTK_REFSCENE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	typedef Ut::UtMap<AftkRender::C_SceneGraphNode*, EditorLib::I_DgNode*>	RenderDgNodeMap;
	typedef Ut::UtMap<EditorLib::I_DgNode*, AftkRender::C_SceneGraphNode*>	DgRenderNodeMap;

	class C_AftkRefScene : public AftkRender::C_Scene
	{
	public:
		C_AftkRefScene(const char* pOriginalFilePath, const EditorLib::AftkDgMap& dgMap);
		~C_AftkRefScene();

		void Clear();

		const Btl::String& GetOriginalFilePath() const;
		
		AftkRender::C_SceneGraphNode* Find(const EditorLib::I_DgNode *pNode);
		const AftkRender::C_SceneGraphNode* Find(const EditorLib::I_DgNode *pNode) const;

		EditorLib::I_DgNode* Find(const AftkRender::C_SceneGraphNode *pNode);
		const EditorLib::I_DgNode* Find(const AftkRender::C_SceneGraphNode *pNode) const;

		virtual S32 OnCreateSceneGraphNode(AftkRender::C_SceneGraphNode *pSgNode, S32 preferredId);
		virtual void OnDestroySceneGraphNode(AftkRender::C_SceneGraphNode *pSgNode);

	private:

		RenderDgNodeMap		m_RenderDgNodeMap;
		DgRenderNodeMap		m_DgRenderNodeMap;

		Btl::String		m_OriginalFilePath;

		const EditorLib::AftkDgMap& m_AftkDgMap;
	};
}

#endif
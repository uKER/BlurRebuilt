#include "stdafx.h"

#include <EditorApp/Editor/AftkRefScene.h>

namespace Editor
{

C_AftkRefScene::C_AftkRefScene(const char* pOriginalFilePath, const EditorLib::AftkDgMap &dgMap)
: AftkRender::C_Scene()
, m_OriginalFilePath(pOriginalFilePath)
, m_AftkDgMap(dgMap)
{
}

C_AftkRefScene::~C_AftkRefScene()
{
	m_RenderDgNodeMap.Clear();
	m_DgRenderNodeMap.Clear();
}

void C_AftkRefScene::Clear()
{
	m_RenderDgNodeMap.Clear();
	m_DgRenderNodeMap.Clear();
	AftkRender::C_Scene::Clear();
}

const Btl::String& C_AftkRefScene::GetOriginalFilePath() const
{
	return m_OriginalFilePath;
}

AftkRender::C_SceneGraphNode* C_AftkRefScene::Find(const EditorLib::I_DgNode* pNode)
{
	DgRenderNodeMap::ConstIterator it = m_DgRenderNodeMap.Find(const_cast<EditorLib::I_DgNode*>(pNode));
	if (it != m_DgRenderNodeMap.End())
		return it->GetValue();

	return 0;
}

const AftkRender::C_SceneGraphNode* C_AftkRefScene::Find(const EditorLib::I_DgNode* pNode) const
{
	DgRenderNodeMap::ConstIterator it = m_DgRenderNodeMap.Find(const_cast<EditorLib::I_DgNode*>(pNode));
	if (it != m_DgRenderNodeMap.End())
		return it->GetValue();

	return 0;
}

EditorLib::I_DgNode* C_AftkRefScene::Find(const AftkRender::C_SceneGraphNode *pNode)
{
	RenderDgNodeMap::ConstIterator it = m_RenderDgNodeMap.Find(const_cast<AftkRender::C_SceneGraphNode*>(pNode));
	if (it != m_RenderDgNodeMap.End())
		return it->GetValue();

	return 0;
}

const EditorLib::I_DgNode* C_AftkRefScene::Find(const AftkRender::C_SceneGraphNode *pNode) const
{
	RenderDgNodeMap::ConstIterator it = m_RenderDgNodeMap.Find(const_cast<AftkRender::C_SceneGraphNode*>(pNode));
	if (it != m_RenderDgNodeMap.End())
		return it->GetValue();

	return 0;
}

S32 C_AftkRefScene::OnCreateSceneGraphNode(AftkRender::C_SceneGraphNode *pSgNode, S32 preferredId)
{
	EditorLib::AftkDgMap::ConstIterator it = m_AftkDgMap.Begin();
	while (it != m_AftkDgMap.End())
	{
		if (it->GetKey()->GetUniqueId() == preferredId)
		{
			m_RenderDgNodeMap[pSgNode] = it->GetValue();
			m_DgRenderNodeMap[it->GetValue()] = pSgNode;

			break;
		}
		it++;
	};

	assert(it != m_AftkDgMap.End());

	return AftkRender::C_Scene::OnCreateSceneGraphNode(pSgNode, preferredId);
}

void C_AftkRefScene::OnDestroySceneGraphNode(AftkRender::C_SceneGraphNode *pSgNode)
{

	RenderDgNodeMap::Iterator it = m_RenderDgNodeMap.Find(pSgNode);
	if (it != m_RenderDgNodeMap.End())
	{
		m_DgRenderNodeMap.Erase(it->GetValue());
		m_RenderDgNodeMap.Erase(it);
	}

	return AftkRender::C_Scene::OnDestroySceneGraphNode(pSgNode);
}

}

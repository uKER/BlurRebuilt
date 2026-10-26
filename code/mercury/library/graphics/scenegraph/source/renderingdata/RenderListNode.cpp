#include "stdafx.h"

#include "SceneGraph/RenderingData/RenderListNode.h"
#include "Renderingdata\target.h"

namespace SceneGraph
{

C_RenderListNodeBase::C_RenderListNodeBase(const RenderingData::Target::Scene* pScene, const RenderingData::Target::RenderListNode* pRenderListNode) :
	m_pScene(pScene),
	m_pRenderListNode(pRenderListNode)
{
	if (m_pRenderListNode)
		m_pNodeDmaData = m_pRenderListNode->GetDmaData(); 
};

} // namespace SceneGraph


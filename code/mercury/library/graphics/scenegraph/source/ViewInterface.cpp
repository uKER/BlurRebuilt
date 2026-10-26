#include "stdafx.h"

#include <SceneGraph/ViewInterface.h>
#include <SceneGraph/GatherBatcher.h>
#include <SceneGraph/RenderingData/CachedRenderListNode.h>

namespace SceneGraph
{

I_View::I_View(C_SceneGraph* pSceneGraph) :	m_pSceneGraph(pSceneGraph),
					m_PvsCell(-1),
					m_futurePvsCell(-1),
					m_pBatcher(0),
					m_Gathering(false),
					m_RenderPassFlags(0),
					m_RenderNodeFlags(0xFFFFFFFF),
					m_VolatileInfoFlags(0)
{
	m_pBatcher = new C_CachedRenderListNodeGatherBatcher(this);
}

I_View::~I_View()
{
	if(m_pBatcher)
		delete m_pBatcher;
}

void I_View::AddDeferredGatherNode(const I_GatherNode* pGatherNode, const void* pData) const
{
	m_DeferredGatherNodes.PushBack();

	DeferredGatherNode& deferredGatherNode = m_DeferredGatherNodes.Back();
	deferredGatherNode.pGatherNode = pGatherNode;
	deferredGatherNode.pData = pData;
}

void I_View::EndGather() const
{
	// flush all jobs
	Scheduler::Flush(m_JobQueueGroup);

	if(m_pBatcher)
		m_pBatcher->EndGather();

	// gather deferred nodes
	for(S32 i = 0; i < m_DeferredGatherNodes.Size(); ++i)
	{
		DCBT(0, (U8*)m_DeferredGatherNodes[i].pData);
		m_DeferredGatherNodes[i].pGatherNode->GatherDeferred(m_DeferredGatherNodes[i].pData);
	}
	m_DeferredGatherNodes.Clear();
}

} // namespace SceneGraph


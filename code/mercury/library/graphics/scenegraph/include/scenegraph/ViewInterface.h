#ifndef SCENE_GRAPH_VIEW_INTERFACE_H
#define SCENE_GRAPH_VIEW_INTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/scheduler/jobqueue.h>

namespace SceneGraph
{

class I_GatherNode;
class C_SceneGraph;
class I_GatherBatcher;

class I_View
{
public:
	I_View(C_SceneGraph* pSceneGraph);
	~I_View();

	// these vectors are populated during Gather
	struct DeferredGatherNode
	{
		const I_GatherNode*						pGatherNode;
		const void*								pData;
	};
	typedef Btl::Vector<DeferredGatherNode>		DeferredGatherNodes;

	Scheduler::GroupHandle						GetJobQueueGroup() const;
	const C_SceneGraph*							GetSceneGraph() const;
	// enable or disable which renderpasses exist in this view. eg shadows would only be Depth/Solid
	void				EnableRenderPass(const U32 renderPassFlag);
	void				DisableRenderPass(const U32 renderPassFlag);
	const U32			RenderPassEnabled(const U32 renderPassFlag) const;
	const U32			RenderPasses() const { return m_RenderPassFlags; }

	void				EnableRenderNode(const U32 renderNode);
	void				DisableRenderNode(const U32 renderNode);
	const U32			RenderNodeEnabled(const U32 renderNode) const;
	const U32			RenderNodes() const { return m_RenderNodeFlags; }

	U32					GetVolatileInfoFlags() const { return m_VolatileInfoFlags; }

	void				EndGather() const;

	// deferred gather nodes
	void				AddDeferredGatherNode(const I_GatherNode* pGatherNode, const void* pData) const;
	DeferredGatherNode&	GetDeferredGatherNode(S32 index) const { return m_DeferredGatherNodes[index]; }
	S32					GetNumDeferredGatherNodes() const { return m_DeferredGatherNodes.Size(); }

	// todo: abstract this?
	void				SetBatcher(I_GatherBatcher* pBatcher) const { m_pBatcher = pBatcher; }
	I_GatherBatcher*	GetBatcher() const { return m_pBatcher; }

	void				SetPvsCell(const S32 index, const S32 future = -1) const { m_PvsCell = index; m_futurePvsCell = future;}
	const S32			GetPvsCell() const { return m_PvsCell; }
	const S32			GetFuturePvsCell() const { return m_futurePvsCell; }

protected:
	C_SceneGraph*								m_pSceneGraph;
	Scheduler::GroupHandle						m_JobQueueGroup;
	mutable S32									m_PvsCell;
	mutable S32									m_futurePvsCell;
	mutable DeferredGatherNodes					m_DeferredGatherNodes;
	mutable I_GatherBatcher*					m_pBatcher;
	mutable bool								m_Gathering;
	
	// flags
	U32											m_RenderPassFlags;
	U32											m_RenderNodeFlags;

	// what volatile info needs to be generated to go with this view
	U32											m_VolatileInfoFlags;	
};

INLINE const C_SceneGraph* I_View::GetSceneGraph() const
{
	return m_pSceneGraph;
}

INLINE Scheduler::GroupHandle I_View::GetJobQueueGroup() const
{
	return m_JobQueueGroup;
}

INLINE void I_View::EnableRenderPass(const U32 passFlag)
{
	m_RenderPassFlags |= passFlag;
}

INLINE void I_View::DisableRenderPass(const U32 passFlag)
{
	m_RenderPassFlags &= ~passFlag;
}

INLINE const U32 I_View::RenderPassEnabled(const U32 renderPassFlag) const
{
	return m_RenderPassFlags & renderPassFlag;
}

INLINE void I_View::EnableRenderNode(const U32 renderNode)
{
	m_RenderNodeFlags |= renderNode;
}

INLINE void I_View::DisableRenderNode(const U32 renderNode)
{
	m_RenderNodeFlags &= ~renderNode;
}

INLINE const U32 I_View::RenderNodeEnabled(const U32 renderNode) const
{
	return m_RenderNodeFlags & renderNode;
}

} // namespace SceneGraph

#endif // SCENE_GRAPH_VIEW_INTERFACE

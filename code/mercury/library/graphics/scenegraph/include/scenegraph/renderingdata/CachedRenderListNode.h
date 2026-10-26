#ifndef SCENE_GRAPH_CACHEDRENDERLISTNODE_H
#define SCENE_GRAPH_CACHEDRENDERLISTNODE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <SceneGraph/CullNode.h>
#include <SceneGraph/RenderNode.h>
#include <SceneGraph/GatherNode.h>
#include <RenderingData/RenderingData.h>

namespace Scheduler
{
	struct Task;
}

namespace SceneGraph
{

class C_MultiView;
class C_LevelInstance;
class C_StaticRenderListNode;
struct CachedRenderListNodeData;

// C_CachedRenderListNode - this is constructed from a C_RenderListNode based on PVS information

class C_CachedRenderListNode :	public C_BoundsCull,
								public C_PassCull,
								public I_GatherNode
{
public:
	C_CachedRenderListNode(const C_SceneGraph* pSceneGraph, const C_LevelInstance* pStaticScene, const C_StaticRenderListNode* pRenderListNode);
	~C_CachedRenderListNode();

	void			Cache();
	void			Build();

	void			Gather(const C_MultiView& multiView) const;
	void			Gather(const C_View& view) const;
	void			DebugRender() const;
	
	void			Accept(I_Visitor& visitor) const;
	
	const C_StaticRenderListNode*	GetRenderListNode() const { return m_pRenderListNode; }

	// cached data for each renderpass which nodes exist in
	struct CachedData
	{
		U32						renderPassType;
		U32						renderPassFlag;
		
		Btl::Vector<C_Bounds>	bounds;			// bounds per cached primitive
		Btl::Vector<U16>		indices;
		Btl::Vector<U16>		flags;			// per view culling flags
	};

	const CachedData&	GetPrimitiveCachedData(S32 index) const { return m_PrimitiveCachedData[index]; }
	S32					GetNumPrimitiveCachedData() const { return m_PrimitiveCachedData.Size(); }
	const CachedData&	GetInstanceCachedData(S32 index) const { return m_InstanceCachedData[index]; }
	S32					GetNumInstanceCachedData() const { return m_InstanceCachedData.Size(); }

	friend void CachedRenderListNodeGather(Scheduler::Task* pTask);
	friend void CachedRenderListNodePushResults(Scheduler::Task* pTask);
	friend class C_CachedRenderListNodeGatherBatcher;

private:
	// virtuals
	void	OnGather(const C_View& view) const { Gather(view); }
	void	OnGather(const C_MultiView& multiView) const { Gather(multiView); }
	
	static void	GatherDeferred(CachedRenderListNodeData* pData);
	
	const C_LevelInstance*							m_pStaticScene; 
	const C_StaticRenderListNode*					m_pRenderListNode;
	const RenderingData::Target::RenderListNode*	m_pRenderingDataNode;

	// this cached data allows us to do quicker culling etc compared to non cached version of render list node
	Btl::Vector<CachedData>					m_PrimitiveCachedData;
	Btl::Vector<CachedData>					m_InstanceCachedData;
	S32										m_NodeCount;
	bool									m_MultiThreadedGather;
};

}

// C_CachedRenderListNodeGatherBatcher
#include <SceneGraph/GatherBatcher.h>

namespace SceneGraph
{

struct CachedRenderListNodeData;
struct CachedRenderListNodeDataBatch;

class C_CachedRenderListNodeGatherBatcher : public I_GatherBatcher
{
public:
	C_CachedRenderListNodeGatherBatcher(const I_View* pView) : I_GatherBatcher(pView) {}
	~C_CachedRenderListNodeGatherBatcher() {}

	void	BeginGather1();
	void	BeginGather2();
	
	virtual void	EndGather();

	void	AddRenderListNode(CachedRenderListNodeData* pData);

	Btl::Vector<CachedRenderListNodeDataBatch*>		m_CachedRenderListNodeBatches;
	
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_RENDERLISTNODE_H

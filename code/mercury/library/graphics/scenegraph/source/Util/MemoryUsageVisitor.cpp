#include "stdafx.h"
#include "SceneGraph/Util/MemoryUsageVisitor.h"

#include "SceneGraph/GatherNode.h"
#include "SceneGraph/SceneGraphImpl.h"
#include "SceneGraph/RenderingData/StaticScene.h"
#include "SceneGraph/RenderingData/StaticRenderListNode.h"
#include "SceneGraph/RenderingData/CachedRenderListNode.h"
#include "SceneGraph/RenderingData/ModelInstanceFlatListGather.h"
#include "SceneGraph/RenderingData/ModelInstanceNode.h"

namespace SceneGraph
{

static Debug::Logger &g_MemoryLog = Debug::GetLogger("graphics.scenegraph.memory");

C_MemoryUsageVisitor::C_MemoryUsageVisitor() : I_Visitor(0), m_MemoryUsed(0)
{
	LOGINFO(g_MemoryLog, "\nSceneGraph Memory Usage Estimate");
}

C_MemoryUsageVisitor::~C_MemoryUsageVisitor()
{
	LOGINFO(g_MemoryLog, "Total: %d bytes", m_MemoryUsed);
}

void C_MemoryUsageVisitor::Visit(const I_GatherNode& node)
{
	U32 memUsed = sizeof(I_GatherNode);
	m_MemoryUsed += memUsed;
	
	LOGINFO(g_MemoryLog, "GatherNode: %d Total: %d", memUsed, m_MemoryUsed);
}

void C_MemoryUsageVisitor::Visit(const C_ModelInstance& node)
{
	U32 memUsed = sizeof(C_ModelInstance);
	memUsed += node.GetPrimitiveNodes().Size() * sizeof(C_ModelInstancePrimitive);
	m_MemoryUsed += memUsed;

	LOGINFO(g_MemoryLog, "ModelInstanceNode: %d Total: %d", memUsed, m_MemoryUsed);
}

void C_MemoryUsageVisitor::Visit(const C_LevelInstance& node)
{
	U32 memUsed = sizeof(C_LevelInstance);
	m_MemoryUsed += memUsed;
	
	LOGINFO(g_MemoryLog, "StaticScene: %d Total: %d", memUsed, m_MemoryUsed);

	const NodeCache& nodeCache = node.GetNodeCache();
	if(!nodeCache.Empty())
	{
		LOGINFO(g_MemoryLog, "StaticScene: NodeCache Count: %d", nodeCache.Size());
		U32 cacheMem = m_MemoryUsed;

		for(NodeCache::ConstIterator pIterator = nodeCache.Begin(); pIterator != nodeCache.End(); pIterator++)
		{
			const I_GatherNode* pGatherNode = pIterator->GetValue();
			pGatherNode->Accept(*this);
		}
		LOGINFO(g_MemoryLog, "StaticScene: NodeCache Usage: %d", m_MemoryUsed - cacheMem);
	}

	m_MemoryUsed += node.m_CachedRenderListNodeData.SizeInBytes();
	m_MemoryUsed += node.m_CachedRenderListNodeDataPass.SizeInBytes();
	m_MemoryUsed += node.m_CachedRenderListNodeDataPassOutput.SizeInBytes();

	LOGINFO(g_MemoryLog, "StaticScene: m_CachedRenderListNodeData : %d", node.m_CachedRenderListNodeData.SizeInBytes());
	LOGINFO(g_MemoryLog, "StaticScene: m_CachedRenderListNodeDataPass : %d", node.m_CachedRenderListNodeDataPass.SizeInBytes());
	LOGINFO(g_MemoryLog, "StaticScene: m_CachedRenderListNodeDataPassOutput : %d", node.m_CachedRenderListNodeDataPassOutput.SizeInBytes());
}

void C_MemoryUsageVisitor::Visit(const C_CachedStaticScene& node)
{
	U32 memUsed = sizeof(C_CachedStaticScene);
	m_MemoryUsed += memUsed;
	
	LOGINFO(g_MemoryLog, "CachedStaticScene: %d Total: %d", memUsed, m_MemoryUsed);

	const CachedNodeCache& nodeCache = node.GetNodeCache();
	if(!nodeCache.Empty())
	{
		LOGINFO(g_MemoryLog, "CachedStaticScene: CachedNodeCache Count: %d", nodeCache.Size());
		U32 cacheMem = m_MemoryUsed;

		for(CachedNodeCache::ConstIterator pIterator = nodeCache.Begin(); pIterator != nodeCache.End(); pIterator++)
		{
			const I_GatherNode* pGatherNode = pIterator->GetValue();
			pGatherNode->Accept(*this);
		}
		LOGINFO(g_MemoryLog, "CachedStaticScene: CachedNodeCache Usage: %d", m_MemoryUsed - cacheMem);
	}
}


void C_MemoryUsageVisitor::Visit(const C_CachedRenderListNode& node)
{
	U32 memUsed = sizeof(C_StaticRenderListNode);

	for(S32 i = 0; i < node.GetNumPrimitiveCachedData(); ++i)
	{
		const C_CachedRenderListNode::CachedData& cachedData = node.GetPrimitiveCachedData(i);
		memUsed += sizeof(C_CachedRenderListNode::CachedData);

		memUsed += cachedData.bounds.Size() * sizeof(C_Bounds);
		memUsed += cachedData.indices.Size() * sizeof(U16);
		memUsed += cachedData.flags.Size() * sizeof(U16);
	}

	for(S32 i = 0; i < node.GetNumInstanceCachedData(); ++i)
	{
		const C_CachedRenderListNode::CachedData& cachedData = node.GetInstanceCachedData(i);
		memUsed += sizeof(C_CachedRenderListNode::CachedData);

		memUsed += cachedData.bounds.Size() * sizeof(C_Bounds);
		memUsed += cachedData.indices.Size() * sizeof(U16);
		memUsed += cachedData.flags.Size() * sizeof(U16);
	}

	m_MemoryUsed += memUsed;

	LOGINFO(g_MemoryLog, "CachedRenderListNode: %d Total: %d", memUsed, m_MemoryUsed);
}

void C_MemoryUsageVisitor::Visit(const C_StaticRenderListNode& node)
{
	U32 memUsed = sizeof(C_StaticRenderListNode);
	memUsed += node.GetNumPrimitives() * sizeof(C_StaticPrimitive);
	memUsed += node.GetNumInstances() * sizeof(C_StaticInstance);
	memUsed += node.GetNumInstanceBatches() * sizeof(C_StaticInstanceBatch);
	m_MemoryUsed += memUsed;

	LOGINFO(g_MemoryLog, "StaticRenderListNode: %d Total: %d", memUsed, m_MemoryUsed);
}

void C_MemoryUsageVisitor::Visit(const C_LightManager& node)
{
}

extern Btl::DynamicPool<ModelInstanceFlatListGatherData>			g_GatherData;
extern Btl::DynamicPool<ModelInstanceFlatListGatherData::Output>	g_GatherDataOutput;

void C_MemoryUsageVisitor::Visit(const C_SceneGraph& node)
{
	U32 memUsed = sizeof(C_SceneGraph);

	LOGINFO(g_MemoryLog, "SceneGraph: %d", memUsed);
	LOGINFO(g_MemoryLog, "SceneGraph: ModelInstanceFlatListGatherData : %d", g_GatherData.SizeInBytes());
	LOGINFO(g_MemoryLog, "SceneGraph: ModelInstanceFlatListGatherData::Output : %d", g_GatherDataOutput.SizeInBytes());

	m_MemoryUsed += memUsed;
	m_MemoryUsed += g_GatherData.SizeInBytes();
	m_MemoryUsed += g_GatherDataOutput.SizeInBytes();
}

} // namespace SceneGraph


#include "stdafx.h"

#include "SceneGraph/RenderingData/StaticScene.h"

#include "SceneGraph/RenderingData/StaticRenderListNode.h"
#include "SceneGraph/RenderingData/CachedRenderListNode.h"

#include "SceneGraph/Visitor.h"
#include "SceneGraph/ViewInterface.h"

#include "RenderingData/Node.h"
#include "RenderingData/RenderingData.h"

namespace SceneGraph
{

#ifdef USE_DYNAMIC_RINGBUFFER
SceneGraph::DynamicRingBuffer<CachedRenderListNodeDataBatch>					C_LevelInstance::m_CachedRenderListNodeDataBatch;
SceneGraph::DynamicRingBuffer<CachedRenderListNodeData>							C_LevelInstance::m_CachedRenderListNodeData;
SceneGraph::DynamicRingBuffer<CachedRenderListNodeData::PassData>				C_LevelInstance::m_CachedRenderListNodeDataPass;
SceneGraph::DynamicRingBuffer<CachedRenderListNodeData::PassData::Output>		C_LevelInstance::m_CachedRenderListNodeDataPassOutput;
#else
Btl::DynamicPool<CachedRenderListNodeDataBatch>					C_LevelInstance::m_CachedRenderListNodeDataBatch;
Btl::DynamicPool<CachedRenderListNodeData>						C_LevelInstance::m_CachedRenderListNodeData;
Btl::DynamicPool<CachedRenderListNodeData::PassData>			C_LevelInstance::m_CachedRenderListNodeDataPass;
Btl::DynamicPool<CachedRenderListNodeData::PassData::Output>	C_LevelInstance::m_CachedRenderListNodeDataPassOutput;
#endif

Debug::Bool	g_StaticUseCache("Graphics.SceneGraph.Static.UseCache", true);
Debug::Bool	g_StaticUsePvs("Graphics.SceneGraph.Static.UsePvs", true);
Debug::Bool g_StaticUsePortals("Graphics.SceneGraph.Static.UsePortals", true);

// Static Scene

C_LevelInstance::C_LevelInstance(const C_SceneGraph* pSceneGraph, const RenderingData::Target::Scene* pScene) :
	I_GatherNode(-1, pSceneGraph),
	m_pSceneGraph(pSceneGraph),
	m_pScene(pScene),
	m_pRootNode(NULL),
	m_CachedScene(pSceneGraph),
	m_groupMask(0xffffffff)
{
}

C_LevelInstance::~C_LevelInstance()
{
	Destruct();

	while(m_NodeCache.Begin() != m_NodeCache.End())
	{
		NodeCache::Iterator it = m_NodeCache.Begin();
		I_GatherNode* pGatherNode = it->GetValue();
		delete pGatherNode;
		m_NodeCache.Erase(it);
	}
}

void C_LevelInstance::Build()
{
	MEMORY_TRACKER_PUSH("graphics.scenegraph");

	PERF_EVENT("StaticScene::Build");

	{
		PERF_EVENT("Destroy");
		Destruct();
	}

	{
		PERF_EVENT("Build Root");
		
		// create root node
		m_pRootNode = new C_StaticSceneRootNode(m_pSceneGraph, this);
		
		// pull out RenderListNodes from scene root node
		m_RenderListNodes = 0;
		const RenderingData::Node& rootNode = m_pScene->GetRootNode();
		Build(m_pRootNode, &rootNode);

	#ifdef USE_DYNAMIC_RINGBUFFER
		if(m_RenderListNodes > m_CachedRenderListNodeData.Size())
		{
			m_CachedRenderListNodeData.Reserve(m_RenderListNodes);
			m_CachedRenderListNodeDataPass.Reserve(m_RenderListNodes * 5);
			m_CachedRenderListNodeDataPassOutput.Reserve(m_RenderListNodes * 5);
		}
	#endif

		// remove all cached renderlist nodes
		/*while(m_StaticRenderListNodeCache.Begin() != m_StaticRenderListNodeCache.End())
		{
			StaticRenderListNodeCache::Iterator it = m_StaticRenderListNodeCache.Begin();
			C_StaticRenderListNode* pStaticRenderListNode = it->GetValue();
			delete pStaticRenderListNode;
			m_StaticRenderListNodeCache.Erase(it);
		}*/
	}

	{
		PERF_EVENT("Build Cache");
		m_CachedScene.Build(this);
	}

	MEMORY_TRACKER_POP("graphics.scenegraph");
}

void C_LevelInstance::Destruct()
{
	if(m_pRootNode)
	{
		delete m_pRootNode;
		m_pRootNode = NULL;
	}
}
	
void C_LevelInstance::DeleteNode(I_GatherNode* pGatherNode)
{
	switch(pGatherNode->GetType())
	{
		case E_GatherNodeType_StaticRenderListNode:
		{
			C_StaticRenderListNode* pStaticRenderListNode = (C_StaticRenderListNode*)pGatherNode;
			m_NodeCache.Insert(pStaticRenderListNode->GetLocation(), pStaticRenderListNode);
		}
		break;
	default:
		delete pGatherNode;
	}
}

void C_LevelInstance::SetGroupMask(U32 mask)
{
	m_groupMask = mask;
}

// this potentially needs to be redone whenever the scene streaming hierarchy changes?
void C_LevelInstance::Build(I_GatherNode* pParent, const RenderingData::Node* pNode)
{
	assert(pNode);
	RenderingData::NodeType nodeType = pNode->GetNodeType();

	if(nodeType == RenderingData::NODE_TYPE_RENDER_LIST_PLATFORM)
	{
		++m_RenderListNodes;
		const RenderingData::Target::RenderListNode* pRenderListNode = (const RenderingData::Target::RenderListNode*)pNode;
		NodeCache::Iterator it = m_NodeCache.Find(pRenderListNode->GetLocation());

		C_StaticRenderListNode* pStaticRenderListNode;

		// search for node in cache
		if(it != m_NodeCache.End())
		{
			// grab node from cache and erase from tree
			pStaticRenderListNode = (C_StaticRenderListNode*)it->GetValue();
			m_NodeCache.Erase(it);
		}
		else
		{
			pStaticRenderListNode = new C_StaticRenderListNode(m_pSceneGraph);
		}
		pStaticRenderListNode->Build(this, pRenderListNode);
		
		// add renderlist node as a child of the parent
		pParent->AddChild(pStaticRenderListNode);
	}
	else if(nodeType == RenderingData::NODE_TYPE_CULL)
	{
		// ignore node if child of root node, and not static scene (ie model)
		if(pNode->GetName() != "Static" && pNode->GetParent().GetBlock() == -1)
			return;

		const RenderingData::CullNode* pCullNode = (const RenderingData::CullNode*) pNode;
		S32 groupIndex = pCullNode->GetGroupIndex();

		if (groupIndex > -1)
		{
			if ((m_groupMask & (1 << groupIndex)) == 0)
				return;
		}
	}

	for(S32 i = 0; i < pNode->GetNumSubNodes(); i++)
	{
		if(const RenderingData::Node* pChildNode = m_pScene->GetNodePtr(pNode->GetSubNode(i)))
			Build(pParent, pChildNode);
	}
}

void C_LevelInstance::GetDynamicMaterialData(
	RenderingData::Target::MaterialListData **ppMaterialListData,
	const RenderingData::Target::MaterialList **ppMaterialListTypes,
	const void*									*ppMaterialListDmaData) const
{
	*ppMaterialListData = m_pMaterialListData;
	*ppMaterialListTypes = m_pMaterialListTypes;
	*ppMaterialListDmaData = m_pMaterialListDmaData;
}

void C_LevelInstance::BeginFrame() const
{
	MEMORY_TRACKER_PUSH("graphics.scenegraph");

#ifdef USE_DYNAMIC_RINGBUFFER
	m_CachedRenderListNodeDataBatch.Grow();
	m_CachedRenderListNodeData.Grow();
	m_CachedRenderListNodeDataPass.Grow();
	m_CachedRenderListNodeDataPassOutput.Grow();
#endif

	if (m_pRootNode)
		m_pRootNode->BeginFrame();
	
	MEMORY_TRACKER_POP("graphics.scenegraph");
}	

void C_LevelInstance::Gather(const C_View& view) const
{
	if(g_StaticUseCache)
	{
		m_CachedScene.Gather(view);
	}
	else
	{
		if(m_pRootNode)
			m_pRootNode->Gather(view);
	}
}

void C_LevelInstance::Gather(const C_MultiView& view) const
{
	if(g_StaticUseCache)
	{
		m_CachedScene.Gather(view);
	}
	else
	{
		if(m_pRootNode)
			m_pRootNode->Gather(view);
	}
}

void C_LevelInstance::Accept(I_Visitor& visitor) const
{
	visitor.Visit(*this);
	if(m_pRootNode)
		m_pRootNode->Accept(visitor);

	m_CachedScene.Accept(visitor);
}

// Cached Static Scene

C_CachedStaticScene::~C_CachedStaticScene()
{
	Destroy();

	while(m_CachedNodeCache.Begin() != m_CachedNodeCache.End())
	{
		CachedNodeCache::Iterator it = m_CachedNodeCache.Begin();
		I_GatherNode* pGatherNode = it->GetValue();
		delete pGatherNode;
		m_CachedNodeCache.Erase(it);
	}
}

void C_CachedStaticScene::Build(const C_LevelInstance* pStaticScene)
{
	Destroy();

	m_pStaticScene = pStaticScene;
	m_pRootNode = new C_CachedStaticSceneRootNode(m_pSceneGraph, this);
	Build(m_pRootNode, pStaticScene->GetRootNode());

	// remove all cached renderlist nodes
	/*while(m_CachedRenderListNodeCache.Begin() != m_CachedRenderListNodeCache.End())
	{
		CachedRenderListNodeCache::Iterator it = m_CachedRenderListNodeCache.Begin();
		C_CachedRenderListNode* pCachedRenderListNode = it->GetValue();
		delete pCachedRenderListNode;
		m_CachedRenderListNodeCache.Erase(it);
	}*/
}

void C_CachedStaticScene::Destroy()
{
	if(m_pRootNode)
	{
		delete m_pRootNode;
		m_pRootNode = NULL;
	}
}

void C_CachedStaticScene::DeleteNode(I_GatherNode* pGatherNode)
{
	switch(pGatherNode->GetType())
	{
		case E_GatherNodeType_CachedRenderListNode:
		{
			C_CachedRenderListNode* pCachedRenderListNode = (C_CachedRenderListNode*)pGatherNode;
			m_CachedNodeCache.Insert(pCachedRenderListNode->GetRenderListNode(), pCachedRenderListNode);
		}
		break;
	default:
		delete pGatherNode;
	}
}

// build a cached version of the scene based on some info which isn't changing very often
// eg PVS data (normal camera), radius from camera (cubemap shadowmaps?), view frustum (static directional lights with shadow maps?), etc
void C_CachedStaticScene::Build(I_GatherNode* pParent, const I_GatherNode* pNode)
{
	if(pNode->GetType() == E_GatherNodeType_StaticRenderListNode)
	{
		const C_StaticRenderListNode* pRenderListNode = (const C_StaticRenderListNode*)pNode;
		CachedNodeCache::Iterator it = m_CachedNodeCache.Find(pRenderListNode);

		C_CachedRenderListNode* pCachedRenderListNode;

		// search for node in cache
		if(it != m_CachedNodeCache.End())
		{
			// grab node from cache and erase from tree
			pCachedRenderListNode = (C_CachedRenderListNode*)it->GetValue();
			m_CachedNodeCache.Erase(it);
		}
		else
		{
			pCachedRenderListNode = new C_CachedRenderListNode(m_pSceneGraph, m_pStaticScene, pRenderListNode);
			pCachedRenderListNode->Build();
		}

		pCachedRenderListNode->Cache();

		pParent->AddChild(pCachedRenderListNode);
	}

	for( S32 i = 0; i < pNode->GetNumChildren(); ++i)
		Build(pParent, pNode->GetChild(i));

}

void C_CachedStaticScene::Gather(const C_View& view) const
{
	if(m_pRootNode)
		m_pRootNode->Gather(view);
}

void C_CachedStaticScene::Gather(const C_MultiView& view) const
{
	if(m_pRootNode)
		m_pRootNode->Gather(view);
}

void C_CachedStaticScene::Accept(I_Visitor& visitor) const
{
	visitor.Visit(*this);
	m_pRootNode->Accept(visitor);
}

// static functions

void C_LevelInstance::BeginGather1(const I_View& view)
{
	// if we don't have a batcher for gathering render list nodes then create one
	C_CachedRenderListNodeGatherBatcher* pBatcher = (C_CachedRenderListNodeGatherBatcher*)view.GetBatcher();
	pBatcher->BeginGather1();
}

void C_LevelInstance::BeginGather2(const I_View& view)
{
	C_CachedRenderListNodeGatherBatcher* pBatcher = (C_CachedRenderListNodeGatherBatcher*)view.GetBatcher();
	pBatcher->BeginGather2();
}



} // namespace SceneGraph


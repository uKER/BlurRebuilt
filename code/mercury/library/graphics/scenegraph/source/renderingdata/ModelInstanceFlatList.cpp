#include "stdafx.h"

// scenegraph includes
#include "SceneGraph/RenderingData/ModelInstanceFlatList.h"
#include "SceneGraph/RenderingData/ModelInstanceFlatListGather.h"
#include "SceneGraph/RenderingData/ModelInstanceNode.h"
#include "SceneGraph/View.h"
#include "SceneGraph/MultiView.h"
#include "SceneGraph/SceneGraphImpl.h"
#include "SceneGraph/Util.h"
#include "SceneGraph/Visitor.h"
#include "SceneGraph/Util/RingBuffer.h"

#include <system/perf.h>

namespace SceneGraph
{
Btl::DynamicPool<C_ModelInstanceBasicCullNode>				g_CullNodes;
Btl::DynamicPool<ModelInstanceFlatListGatherData>			g_GatherData;
Btl::DynamicPool<ModelInstanceFlatListGatherData::Output>	g_GatherDataOutput;

Debug::Bool	g_GatherFlatListMultiView("Graphics.SceneGraph.Gather.FlatListMultiView", true);

// C_ModelInstanceBasicCullNode

C_ModelInstanceFlatList::C_ModelInstanceFlatList(const C_SceneGraph* pSceneGraph) : I_ListNode(-1, pSceneGraph)
{
}

C_ModelInstanceFlatList::~C_ModelInstanceFlatList()
{
}

void C_ModelInstanceFlatList::BeginFrame()
{
	//g_GatherData.Grow();
	//g_GatherDataOutput.Grow();
}

void C_ModelInstanceFlatList::Insert(C_ModelInstance* pModelInstance)
{
	MEMORY_TRACKER_PUSH("graphics.scenegraph");
	assert((U32)pModelInstance != 0xfefefefe);

	if(pModelInstance && pModelInstance->GetRenderListNode())
	{
		if(!pModelInstance->GetCullNode())
		{
			{
				C_ModelInstanceBasicCullNode* pCullNode = g_CullNodes.AllocateNoConstructor();
				new (pCullNode) C_ModelInstanceBasicCullNode(pModelInstance);
				pModelInstance->SetCullNode(pCullNode);
				pModelInstance->Build();

				// store position and pass indices on cull node
				pCullNode->m_PositionIndex = m_CullNodes.Size();
				m_CullNodes.PushBack(pCullNode);

				pCullNode->SetNearFarDistance(0.f, 500.0);
			}
		}
		else
		{
			// Already has a cull node, store the pointer in the list, but don't set the index on the cull node.
			for(U32 iIndex=0; iIndex<static_cast<U32>(m_CullNodes.Size()); ++iIndex)
				if(m_CullNodes[iIndex] == (C_ModelInstanceBasicCullNode*)pModelInstance->GetCullNode())
					return;
			m_CullNodes.PushBack((C_ModelInstanceBasicCullNode*)pModelInstance->GetCullNode());
		}
	}
	MEMORY_TRACKER_POP("graphics.scenegraph");
}

void C_ModelInstanceFlatList::Remove(C_ModelInstance* pModelInstance, const bool bFreeCullNode)
{
	if(pModelInstance && pModelInstance->GetCullNode())
	{
		if(bFreeCullNode)
		{
			C_ModelInstanceBasicCullNode* pBasicCullNode = (C_ModelInstanceBasicCullNode*)pModelInstance->GetCullNode();
			S32 positionIndex = pBasicCullNode->m_PositionIndex;
			m_CullNodes.EraseByIndexNonOrdered(positionIndex);

			if(positionIndex < m_CullNodes.Size())
				m_CullNodes[positionIndex]->m_PositionIndex = positionIndex;

			BIZ_ASSERT_MSG_SPEED(((INTPTR)pBasicCullNode & 0xF) == 0, "pBasicCullNode alignmnent issue: %x", pBasicCullNode);

			g_CullNodes.Free(pBasicCullNode);
			pModelInstance->SetCullNode(NULL);
		}
		else
		{
			// TODO: [Stove] This sucks, can we maintain multiple indices on the cull nodes?
			C_ModelInstanceBasicCullNode* pBasicCullNode = (C_ModelInstanceBasicCullNode*)pModelInstance->GetCullNode();
			
			U32 iIndex = Btl::FindIndex(m_CullNodes, pBasicCullNode);
			while(iIndex != m_CullNodes.Size())
			{
				m_CullNodes.EraseByIndexNonOrdered(iIndex);
				iIndex = Btl::FindIndex(m_CullNodes, pBasicCullNode);
				//LOGWARN(g_Log, "Removed modelinstance %d from index %d.", (U32)pModelInstance, iIndex);
			}
		}
	}
}

void C_ModelInstanceFlatList::Gather(const C_View& view) const
{
	if(view.RenderNodeEnabled(Flag<E_RenderNodeType_ModelInstance>()) && m_CullNodes.Size() > 0)
	{
		if(m_pSceneGraph->JobQueueActive())
		{
			PERF_EVENT("SceneGraph.ModelInstanceFlatList.GatherSetup");

			S32 numNodes = 0;
			while(numNodes < m_CullNodes.Size())
			{
				if(ModelInstanceFlatListGatherData* pData = g_GatherData.AllocatePrefetch())
				{
					if(ModelInstanceFlatListGatherData::Output* pOutput = g_GatherDataOutput.AllocatePrefetch())
					{
						pData->outputs.Clear();
						pData->viewData.Clear();
			
						pData->input.ppCullNodes = &m_CullNodes[numNodes];
						assert((U32)m_CullNodes[numNodes]->GetModelInstance() != 0xfefefefe);
						S32 numNodesRemaining = m_CullNodes.Size() - numNodes;
						pData->input.nodesCount = numNodesRemaining > MAX_FLATLIST_RESULTS ? MAX_FLATLIST_RESULTS : numNodesRemaining;
						pOutput->pView = &view;

						pData->outputs.PushBack(pOutput);
						pData->viewData.PushBack(&view.GetViewData());

						Scheduler::AddTask(ModelInstanceFlatListGatherModule, pData, NULL, view.GetJobQueueGroup(), view.GetSceneGraph()->GetJobQueuePolicyGather());			
						view.AddDeferredGatherNode(this, (const void*)pData);
					}
				}
				numNodes += MAX_FLATLIST_RESULTS;
			}
		}
		else
		{
			PERF_EVENT("SceneGraph.ModelInstanceFlatList.Gather");

			for(S32 i = 0; i < m_CullNodes.Size(); ++i)
			{
				m_CullNodes[i]->Gather(view);
			}
		}
	}
}

void C_ModelInstanceFlatList::Gather(const C_MultiView& multiView) const
{
	if(g_GatherFlatListMultiView)
	{
		if(multiView.RenderNodeEnabled(Flag<E_RenderNodeType_ModelInstance>()) && m_CullNodes.Size() > 0)
		{
			if(m_pSceneGraph->JobQueueActive())
			{
				PERF_EVENT("SceneGraph.ModelInstanceFlatList.GatherSetup");

				S32 numNodes = 0;
				while(numNodes < m_CullNodes.Size())
				{
					S32 numNodesRemaining = m_CullNodes.Size() - numNodes;
	
					if(ModelInstanceFlatListGatherData* pData = g_GatherData.AllocatePrefetch())
					{
						pData->outputs.Clear();
						pData->viewData.Clear();

						pData->input.ppCullNodes = &m_CullNodes[numNodes];
						assert((U32)m_CullNodes[numNodes]->GetModelInstance() != 0xfefefefe);
						pData->input.nodesCount = numNodesRemaining > MAX_FLATLIST_RESULTS ? MAX_FLATLIST_RESULTS : numNodesRemaining;
						
						for(S32 v = 0; v < multiView.Size(); ++v)
						{
							const SceneGraph::C_View* pView = multiView.GetView(v);

							if(pView->RenderNodeEnabled(Flag<E_RenderNodeType_ModelInstance>()))
							{
								if(ModelInstanceFlatListGatherData::Output* pOutput = g_GatherDataOutput.AllocatePrefetch())
								{
									pOutput->pView = pView;
									pData->outputs.PushBack(pOutput);
									pData->viewData.PushBack(&pView->GetViewData());
								}
							}
						}

						Scheduler::AddTask(ModelInstanceFlatListGatherModule, pData, NULL, multiView.GetJobQueueGroup(), m_pSceneGraph->GetJobQueuePolicyGather());			
						multiView.AddDeferredGatherNode(this, (const void*)pData);
					}

					numNodes += MAX_FLATLIST_RESULTS;
				}
			}
			else
			{
				PERF_EVENT("SceneGraph.ModelInstanceFlatList.Gather");

				for(S32 v = 0; v < multiView.Size(); ++v)
				{
					const SceneGraph::C_View& view = *multiView.GetView(v);
					
					for(S32 i = 0; i < m_CullNodes.Size(); ++i)
						m_CullNodes[i]->Gather(view);
				}
			}
		}
	}
	else
	{
		for(S32 v = 0; v < multiView.Size(); ++v)
		{
			Gather(*multiView.GetView(v));
		}
	}
}

void C_ModelInstanceFlatList::OnGatherDeferred(const void* p) const
{
	PERF_EVENT("SceneGraph.ModelInstanceFlatList.GatherDeferred");

	ModelInstanceFlatListGatherData* pData = (ModelInstanceFlatListGatherData*)p;

	for(S32 o = 0; o < pData->outputs.Size(); ++o)
	{
		ModelInstanceFlatListGatherData::Output* pOutput = pData->outputs[o];

		if(pOutput->numResults > 0)
		{
			const C_View* pView = pOutput->pView;
			S32 i;
			S32 numResults = (S32)pOutput->numResults;
			S32 numResultsMinusTwo = numResults - 2;
			for(i = 0; i < numResultsMinusTwo; ++i)
			{
				const C_ModelInstance* pModelInstance = pOutput->results[i].pModelInstance;
				pModelInstance->m_Frame = m_pSceneGraph->GetFrame();
				// prefetch next 2
				DCBT(0, &pOutput->results[i+1]);
				DCBT(128, &pOutput->results[i+1]);

				VolatileNodeInfo volatileNodeInfo = pOutput->results[i].volatileInfo;
				U32 predication = 0;

				if(pOutput->results[i].passIndex != 0xFFFF)
					pView->AddVisibleNode(pModelInstance, pOutput->results[i].passIndex, predication, volatileNodeInfo.data, volatileNodeInfo.screenSize);
				else
					pModelInstance->Gather(*pView, predication, volatileNodeInfo.data);
				pModelInstance->SetVisible(pView->GetTypeFlag());
			}

			for(; i < numResults; ++i)
			{
				const C_ModelInstance* pModelInstance = pOutput->results[i].pModelInstance;
				pModelInstance->m_Frame = m_pSceneGraph->GetFrame();

				VolatileNodeInfo volatileNodeInfo = pOutput->results[i].volatileInfo;

				U32 predication = 0;

				if(pOutput->results[i].passIndex != 0xFFFF)
					pView->AddVisibleNode(pModelInstance, pOutput->results[i].passIndex, predication, volatileNodeInfo.data);
				else
					pModelInstance->Gather(*pView, predication, volatileNodeInfo.data);
				pModelInstance->SetVisible(pView->GetTypeFlag());
			}
		}
		g_GatherDataOutput.Free(pOutput);
	}
	g_GatherData.Free(pData);
}

void C_ModelInstanceFlatList::Accept(I_Visitor& visitor) const
{
	for(S32 i = 0; i < m_CullNodes.Size(); ++i)
	{
		visitor.Visit(*m_CullNodes[i]->GetModelInstance());
	}

	visitor.Visit(*this);
}

} // namespace SceneGraph


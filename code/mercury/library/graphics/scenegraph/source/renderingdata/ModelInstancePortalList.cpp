#include <stdafx.h>
#include <SceneGraph/RenderingData/ModelInstancePortalList.h>
#include <SceneGraph/RenderingData/ModelInstanceNode.h>
#include <SceneGraph/Renderingdata/PortalCullNode.h>
#include <SceneGraph/SceneGraph.h>
#include <SceneGraph/SceneGraphImpl.h>
#include <SceneGraph/View.h>
#include <Btl/Containers/algorithm.h>
#include <Portal/SystemContainmentQuery.h>
#include <Portal/System.h>

namespace SceneGraph
{
	extern Debug::Bool		g_StaticUsePortals;
	extern Debug::Logger&	g_Log;
};

using namespace SceneGraph;

Debug::Bool		C_ModelInstancePortalList::m_bJobQueueActive("Graphics.SceneGraph.PortalListUseJobQueue", true);
Debug::Bool		C_ModelInstancePortalList::m_bDisablePortalList("Graphics.SceneGraph.DisablePortalList", true);
bool			C_ModelInstancePortalList::m_bDebugJobsInFlight = false;

// -----------------------------------------------------------------

/// Default c'tor.
C_ModelInstancePortalList::C_ModelInstancePortalList(const C_SceneGraph* pSceneGraph) :
	I_ListNode(-1, pSceneGraph),
	m_bLastFrameJobsHandled(false)
{
	m_JobQueueGroup				= Scheduler::CreateGroup();
	return;
}

// -----------------------------------------------------------------

/// D'tor.
C_ModelInstancePortalList::~C_ModelInstancePortalList()
{
	if(!m_bLastFrameJobsHandled)
	{
		// Flush the jobs if they haven't been handled already.
		Scheduler::Flush(m_JobQueueGroup);
		m_bDebugJobsInFlight = false;

		if(m_vLiveSpuJobData.Size() > 0)
			LOGWARN(g_Log, "BeginFrame called without matching Gather, dumping last frame's classification jobs.");

		// Discard the results and free the memory from the pool.
		while(m_vLiveSpuJobData.Size() > 0)
		{
			// Free and pop the live job data.
			PortalCullNodeClassifyData* pData = m_vLiveSpuJobData.Front();
			m_vLiveSpuJobData.EraseByIndexNonOrdered(0);
			m_ClassificationPool.Free(pData);
		}
		m_bLastFrameJobsHandled = true;

		// Sanity checks.
		BIZ_WARNING_MSG(m_vLiveSpuJobData.Size() == 0, "Sanity check failed -- Didn't handle all Spu jobs.");
		BIZ_WARNING_MSG(m_ClassificationPool.Allocated() == 0, "Sanity check failed -- Pool should be clear.");
	}

	Btl::ForEach(m_vRoomFlatLists, m_DeleteFunctor);
	m_vRoomFlatLists.Clear();
	Scheduler::DestroyGroup(m_JobQueueGroup);
	return;
}

// -----------------------------------------------------------------

/// Accept a visitor node.
void C_ModelInstancePortalList::Accept(SceneGraph::I_Visitor &rkVisitor) const
{
	// Propogate the visitor to each room's flat list.
	for(U32 iCurrentFlatList=0; iCurrentFlatList<static_cast<U32>(m_vRoomFlatLists.Size()); ++iCurrentFlatList)
		m_vRoomFlatLists[iCurrentFlatList]->Accept(rkVisitor);

	return;
}

// -----------------------------------------------------------------

/// Gather the portal list.
void C_ModelInstancePortalList::Gather(const SceneGraph::C_View &rkView) const
{
	// Only bother to do this if we have some room flat lists.
	if(!m_vRoomFlatLists.Size())
		return;	

	// Setup the gathering...
	PERF_EVENT("ModelInstancePortalList.GatherSetup");
	if(m_bDisablePortalList.Value())
	{
		// Make sure all the portal-room classification tasks have run before gathering the 
		// portal-room flat lists, move classified cull nodes into the correct room flat lists.
		Scheduler::Flush(m_JobQueueGroup);
		m_bDebugJobsInFlight = false;	// Jobs have all run -- no longer in flight.
		HandleClassifiedCullNodes();

		if(g_StaticUsePortals.Value() &&
		   rkView.GetViewData().cullFlags & E_CullMethod_Portal)	// Gather *all* rooms if the view doesn't require portal culling.
		{
			if(m_vRoomFlatLists.Size())
			{
				// Gather flat lists only for visible rooms.
				for(U32 iCurrentRoom=0; iCurrentRoom<static_cast<U32>(rkView.GetVisiblePortalRoomSet().Size()); ++iCurrentRoom)
				{
					if(rkView.GetVisiblePortalRoomSet()[iCurrentRoom])
						m_vRoomFlatLists[iCurrentRoom]->Gather(rkView);
				}
			}
		}
		else
		{
			// Gather flat lists for ALL rooms.
			for(U32 iCurrentRoom=0; iCurrentRoom<static_cast<U32>(m_vRoomFlatLists.Size()); ++iCurrentRoom)
				m_vRoomFlatLists[iCurrentRoom]->Gather(rkView);	
		}
	}
	return;
}

// -----------------------------------------------------------------

/// Return true if the job queue is active.
const bool C_ModelInstancePortalList::JobQueueActive()
{
	return m_bJobQueueActive.Value();
}

// -----------------------------------------------------------------

/// Kick off all the portal-room classification jobs for the pending queue.
void C_ModelInstancePortalList::KickClassifyJobs(const C_SceneGraph* pSceneGraph) const
{
	// Only bother to do this if we have some room flat lists.
	if(!m_vRoomFlatLists.Size())
		return;	

	// Check the results from last frame's jobs were handled.
	if(!m_bLastFrameJobsHandled)
	{
		// Flush the jobs if they haven't been handled already.
		Scheduler::Flush(m_JobQueueGroup);
		m_bDebugJobsInFlight = false;

		if(m_vLiveSpuJobData.Size() > 0)
			LOGWARN(g_Log, "BeginFrame called without matching Gather, dumping last frame's classification jobs.");

		// Discard the results and free the memory from the pool.
		while(m_vLiveSpuJobData.Size() > 0)
		{
			// Free and pop the live job data.
			PortalCullNodeClassifyData* pData = m_vLiveSpuJobData.Front();
			m_vLiveSpuJobData.EraseByIndexNonOrdered(0);
			m_ClassificationPool.Free(pData);
		}
		m_bLastFrameJobsHandled = true;

		// Sanity checks.
		BIZ_WARNING_MSG(m_vLiveSpuJobData.Size() == 0, "Sanity check failed -- Didn't handle all Spu jobs.");
		BIZ_WARNING_MSG(m_ClassificationPool.Allocated() == 0, "Sanity check failed -- Pool should be clear.");
	}
	
	// Kick the jobs to classify scheduled ModelInstances.
	if(C_ModelInstancePortalList::JobQueueActive() && 
	   m_vClassificationQueue.Size()			   && 
	   (1)/*TODO: [Stove] Need an extra check for small sized queues to avoid task-setup where benefit is outweighed.*/)
	{
		PERF_EVENT("ModelInstancePortalList.ClassifySetup");

		// Jobs are now in flight.
		m_bDebugJobsInFlight = true;

		while(m_vClassificationQueue.Size() > 0)
		{
			// Create our data for SPU job input.
			PortalCullNodeClassifyData* pData = m_ClassificationPool.Allocate();
			m_vLiveSpuJobData.PushBack(pData);

			// Fill up our input with spatial probes.
			U32 iNumProbesAddedToBatch = 0;
			for(/**/; iNumProbesAddedToBatch<PortalCullNodeClassifyData::MaxBatchSize && m_vClassificationQueue.Size(); /**/)
			{
				C_ModelInstance* pQueueFront = m_vClassificationQueue.Front();
				BIZ_WARNING_MSG(pQueueFront, "Queue contains a NULL pointer!");

				if(pQueueFront->GetCullNode() && 
				   pQueueFront->GetCullNode()->GetType() == I_ModelInstanceCullNode::MODELINSTANCECULLNODE_TYPE_PORTAL)
				{
					if(static_cast<const C_PortalCullNode*>(pQueueFront->GetCullNode())->GetSpatialProbeCount())
					{
						for(U32 iCurrentProbeIndex=0, iProbeCount=static_cast<const C_PortalCullNode*>(pQueueFront->GetCullNode())->GetSpatialProbeCount(); iCurrentProbeIndex<iProbeCount; ++iCurrentProbeIndex, ++iNumProbesAddedToBatch)
						{
							pData->m_Input.m_vSpatialProbes[iNumProbesAddedToBatch]		= static_cast<const C_PortalCullNode*>(pQueueFront->GetCullNode())->GetSpatialProbeAtIndex(iCurrentProbeIndex);
							pData->m_Input.m_vLastRoom[iNumProbesAddedToBatch]			= static_cast<const C_PortalCullNode*>(pQueueFront->GetCullNode())->GetLastRoomIndexAtIndex(iCurrentProbeIndex);
							pData->m_Input.m_vLastSystem[iNumProbesAddedToBatch]		= static_cast<const C_PortalCullNode*>(pQueueFront->GetCullNode())->GetLastPortalIndexAtIndex(iCurrentProbeIndex);
							pData->m_Output.m_vPortalIndices[iNumProbesAddedToBatch]	= 0xd15e;
							pData->m_Output.m_vRoomIndices[iNumProbesAddedToBatch]		= 0xa5ed;
							pData->m_Output.m_vModelInstances[iNumProbesAddedToBatch]	= pQueueFront;	// Write this for the mapping in the output handler.
							BIZ_WARNING_MSG(pQueueFront, "Queue contains a NULL pointer.");
						}
					}
					else
					{
						//LOGWARN(g_Log, "No spatial probes found on portal cull node for %s, model instance will always be drawn when in frustum!", pQueueFront->m_pszFriendlyName);
						m_vRoomFlatLists[0]->Insert(pQueueFront);	// No spatial probes, put the instance node in the world room (always drawn).
					}
				}
				else
				{
					LOGWARN(g_Log, "Mismatched or absent cull node for %s, model instance will always be drawn when in frustum!", pQueueFront->m_pszFriendlyName);
					m_vRoomFlatLists[0]->Insert(pQueueFront);		// No portal cull node, put in the world room (always drawn).
				}

				m_vClassificationQueue.EraseByIndexNonOrdered(0);
			}

			// Set effective addresses of the portal system objects, and Dma targets.
			pData->m_Input.m_iSystemCount	= pSceneGraph->GetPortalSystemCount();
			for(U32 iCurrentSystem=0; iCurrentSystem<pData->m_Input.m_iSystemCount; ++iCurrentSystem)
				pData->m_Input.m_vTreeEAs[iCurrentSystem] = &pSceneGraph->GetPortalSystemList().At(iCurrentSystem)->GetTracer().GetBSPTree();	
			pData->m_Input.m_pOutPortalVec	= &pData->m_Output.m_vPortalIndices[0];
			pData->m_Input.m_pOutRoomVec	= &pData->m_Output.m_vRoomIndices[0];
			pData->m_Input.m_iProbeCount	= iNumProbesAddedToBatch;
			
			if(!iNumProbesAddedToBatch)
				continue;	// Don't add the job if there were no spatial probes in it.
			else
			{
				// Add task to the scheduler.
				PortalCullNodeClassifyInput input;
				input.pData = pData;
				Scheduler::AddTask(PortalCullNodeClassifyModule,
								   (void*)&pData->m_Input, (void*)&pData->m_Output,
								   m_JobQueueGroup,										// All jobs in the our own job group so we can spin on just these later.
								   pSceneGraph->GetJobQueuePolicyGather(),				// Run jobs on all cores. 
								   &input, 
								   sizeof(PortalCullNodeClassifyInput));
				m_bLastFrameJobsHandled = false;
			}
		}
	}
	else
	{
		PERF_EVENT("ModelInstancePortalList.Classify");

		// Process all model instances that have been scheduled for classification.
		while(m_vClassificationQueue.Size() > 0)
		{
			C_ModelInstance* pQueueFront = m_vClassificationQueue.Front();
			BIZ_WARNING_MSG(pQueueFront, "Queue contains NULL pointer.");

			if(pQueueFront->GetCullNode() && 
			   pQueueFront->GetCullNode()->GetType() == I_ModelInstanceCullNode::MODELINSTANCECULLNODE_TYPE_PORTAL)
			{
				if(static_cast<const C_PortalCullNode*>(pQueueFront->GetCullNode())->GetSpatialProbeCount())
				{
					// Classify all the spatial probes on the cull node.
					for(U32 iCurrentProbeIndex=0; iCurrentProbeIndex<static_cast<const C_PortalCullNode*>(pQueueFront->GetCullNode())->GetSpatialProbeCount(); ++iCurrentProbeIndex)
					{
						Maths::HVec3 worldPosition				= static_cast<const C_PortalCullNode*>(pQueueFront->GetCullNode())->GetSpatialProbeAtIndex(iCurrentProbeIndex);
						U32			 iLastRoom					= static_cast<const C_PortalCullNode*>(pQueueFront->GetCullNode())->GetLastRoomIndexAtIndex(iCurrentProbeIndex);
						// TODO: [Stove] Changes here to support multiple portal system objects.
						const Portal::C_BSPContainer* pTree		= &pSceneGraph->GetPortalSystemList().At(0)->GetTracer().GetBSPTree();
						BIZ_WARNING_MSG(pTree, "Portal system pointer is NULL.");
						Portal::C_SystemContainmentQuery classifier(pTree);
						U32			iNewRoom = classifier.Classify(worldPosition, iLastRoom);
						m_vRoomFlatLists[iNewRoom]->Insert(pQueueFront);	// TODO: [Stove] Could draw the object twice for objects with multiple spatial probes on cull node.
					}
				}
				else
				{
					//LOGWARN(g_Log, "No spatial probes found on portal cull node for %s, model instance will always be drawn when in frustum!", pQueueFront->m_pszFriendlyName);
					m_vRoomFlatLists[0]->Insert(pQueueFront);		// No spatial probes, put model instance in the world-room.
				}
			}
			else
			{
				LOGWARN(g_Log, "Mismatched or absent cull node for %s, model instance will always be drawn when in frustum!", pQueueFront->m_pszFriendlyName);
				m_vRoomFlatLists[0]->Insert(pQueueFront);			// No or wrong cull node on model instance, put it in world-room.
			}

			// Pop the model instance from the front of the classify schedule queue.
			m_vClassificationQueue.EraseByIndexNonOrdered(0);
		}

		// No need to handle them again, as we ran them immediately on the main thread.
		m_bLastFrameJobsHandled = true;
	}
	return;
}

// -----------------------------------------------------------------

/// Handle the classified portal cull nodes.
void C_ModelInstancePortalList::HandleClassifiedCullNodes() const
{
	// TODO: [Stove] Need some slight modifications in here to support multiple portal system objects.
	
	// Process all our SPU job results.
	while(m_vLiveSpuJobData.Size() > 0)
	{
		PortalCullNodeClassifyData* pData = m_vLiveSpuJobData.Front();
		for(U32 iCurrentProbe=0, iProbeCount = pData->m_Input.m_iProbeCount; iCurrentProbe<iProbeCount; ++iCurrentProbe)
		{
			C_ModelInstance* pModelInstance = pData->m_Output.m_vModelInstances[iCurrentProbe];
			const U16 iPortalIndex			= pData->m_Output.m_vPortalIndices[iCurrentProbe];
			const U16 iRoomIndex			= pData->m_Output.m_vRoomIndices[iCurrentProbe];
			const U16 iOldRoomIndex			= pData->m_Input.m_vLastRoom[iCurrentProbe];
			const U16 iOldPortalIndex		= pData->m_Input.m_vLastSystem[iCurrentProbe];
			if(iRoomIndex	== iOldRoomIndex /*&& 
			   iPortalIndex == iOldPortalIndex*/)
				continue;	// Nothing has changed leave the model instance where it is.

			BIZ_WARNING_MSG((U32)pModelInstance, "Sanity check failed -- Null model instance.");								// Sanity checks.
			BIZ_WARNING_MSG(pModelInstance->GetCullNode(), "Sanity check failed -- Cull node missing!");						// Sanity checks.

			// TODO: [Stove] Check all indicies for the same pointer before calling remove.
			if(iOldRoomIndex != static_cast<U16>(-1))
				m_vRoomFlatLists[iOldRoomIndex]->Remove(pModelInstance, false);	// Don't free the cull node.
			
			// TODO: [Stove] We could potentially draw an object multiple times if it has multiple spatial probes!!!!
			BIZ_WARNING_MSG(iRoomIndex < static_cast<U32>(m_vRoomFlatLists.Size()), "Invalid room index!");
			m_vRoomFlatLists[iRoomIndex]->Insert(pModelInstance);
			static_cast<C_PortalCullNode*>(pModelInstance->GetCullNode())->SetLastRoomIndex(iPortalIndex, iRoomIndex);
		}

		// Free and pop the live job data.
		m_vLiveSpuJobData.EraseByIndexNonOrdered(0);
		m_ClassificationPool.Free(pData);
	}
	
	m_bLastFrameJobsHandled = true;

	// Some more sanity checks:
	BIZ_WARNING_MSG(m_vLiveSpuJobData.Size() == 0, "Sanity check failed -- Didn't handle all Spu jobs.");
	BIZ_WARNING_MSG(m_ClassificationPool.Allocated() == 0, "Sanity check failed -- Pool should be clear.");
	BIZ_WARNING_MSG(m_vClassificationQueue.Size() == 0, "Sanity check failed -- Shouldn't be any items in the classifcation queue at this point.");
	return;
}

// -----------------------------------------------------------------

/// Insert a model instance into the portal list.
void C_ModelInstancePortalList::Insert(SceneGraph::C_ModelInstance *pModelInstance)
{
	if(pModelInstance && pModelInstance->GetRenderListNode())
	{
		if(!pModelInstance->GetCullNode())
		{
			C_PortalCullNode* pCullNode = new C_PortalCullNode(pModelInstance);
			pModelInstance->SetCullNode(static_cast<I_ModelInstanceCullNode*>(pCullNode));
			pModelInstance->Build();

			m_vCullNodes.PushBack(pCullNode);
			pCullNode->SetPositionIndex(m_vCullNodes.Size() - 1);
		}
		else
		{
			if(pModelInstance->GetCullNode()->GetType() == I_ModelInstanceCullNode::MODELINSTANCECULLNODE_TYPE_PORTAL)
				LOGWARN(g_Log, "Already has a portal cull node.");
			else
				LOGWARN(g_Log, "Already has a basic cull node.");
		}
	}

	return;
}

// -----------------------------------------------------------------

/// Begin frame.
void C_ModelInstancePortalList::OnBeginFrame() const
{
	// Only bother to do this if we have some room flat lists.
	if(!m_vRoomFlatLists.Size())
		return;	

	KickClassifyJobs(GetSceneGraph());	// Kick these bad boys off early as possible.
	return;
}

// -----------------------------------------------------------------

/// Gather for the view.
void C_ModelInstancePortalList::OnGather(const C_View& rkView) const
{
	Gather(rkView); 
	return;
}

// -----------------------------------------------------------------

/// Defered gather.
void C_ModelInstancePortalList::OnGatherDeferred(const void* pData) const
{
	// Nothing to do.
	return;
}

// -----------------------------------------------------------------

/// Handle movement of a constituent model instance node.
void C_ModelInstancePortalList::OnMove(C_ModelInstance* pModelInstance)
{
	// Only bother to do this if we have some room flat lists.
	if(!m_vRoomFlatLists.Size())
		return;	
	
	// Only add classify model instances that contain cull nodes,
	// those without can go directly into the world room for gathering.
	if(pModelInstance && pModelInstance->GetCullNode())
	{
		// TODO: [Stove] IMPORTANT: This is slow as balls, make it fast.
		// If the model instance has already been scheduled for classification,
		// then we don't need to do it again.
		bool bFound = false;
		for(U32 iCurrentIndex=0; iCurrentIndex<static_cast<U32>(m_vClassificationQueue.Size()); ++iCurrentIndex)
		{
			if(m_vClassificationQueue[iCurrentIndex] == pModelInstance)
			{
				bFound = true;
				break;
			}
		}

		if(!bFound)
			m_vClassificationQueue.PushBack(pModelInstance);
	}
	else
	{
		LOGWARN(g_Log, "Mismatched or absent cull node for %s, model instance will always be drawn when in frustum!", pModelInstance->m_pszFriendlyName);
		m_vRoomFlatLists[0]->Insert(pModelInstance);
	}
	return;
}

// -----------------------------------------------------------------

/// Remove a model instance from the portal list.
void C_ModelInstancePortalList::Remove(SceneGraph::C_ModelInstance* pModelInstance, const bool bFreeCullNode)
{
	if(pModelInstance->GetCullNode())
	{
		// Non-freeing remove from any flat lists the model instance is in.
		for(U32 iCurrentRoomFlatList=0; iCurrentRoomFlatList<static_cast<U32>(m_vRoomFlatLists.Size()); ++iCurrentRoomFlatList)
			m_vRoomFlatLists[iCurrentRoomFlatList]->Remove(pModelInstance, false);

		// Remove the cull node and free it.
		const C_PortalCullNode* pCullNode = (C_PortalCullNode*)pModelInstance->GetCullNode();
		const S32 iIndex = pCullNode->GetPositionIndex();
		m_vCullNodes.EraseByIndexNonOrdered(iIndex);
		if(iIndex < m_vCullNodes.Size())
			m_vCullNodes[iIndex]->SetPositionIndex(iIndex);
	
		delete pModelInstance->GetCullNode();
		pModelInstance->SetCullNode(NULL);
	}

	// Rip it out of the classification queue regardless of if it has a cull node or not.
	for(U32 iCurrentClassificationNode=0; iCurrentClassificationNode<static_cast<U32>(m_vClassificationQueue.Size()); ++iCurrentClassificationNode)
	{
		if(m_vClassificationQueue[iCurrentClassificationNode] == pModelInstance)
		{
			m_vClassificationQueue.EraseByIndexNonOrdered(iCurrentClassificationNode);
			iCurrentClassificationNode--;
		}
	}
	return;
}

// -----------------------------------------------------------------

/// Reserve a model instance flat list for each portal room.
void C_ModelInstancePortalList::ReserveModelInstanceFlatLists(U32 iCount)
{
	BIZ_WARNING_MSG(iCount, "Trying to reserve 0 flat lists!");
	for(U32 iCurrentFlatList=0; iCurrentFlatList<iCount; ++iCurrentFlatList)
		m_vRoomFlatLists.PushBack(new C_ModelInstanceFlatList(m_pSceneGraph));

	return;
}


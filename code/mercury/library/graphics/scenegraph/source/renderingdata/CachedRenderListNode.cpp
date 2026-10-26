#include "stdafx.h"

#include "SceneGraph/RenderingData/CachedRenderListNode.h"
#include "SceneGraph/RenderingData/StaticRenderListNode.h"
#include "SceneGraph/RenderingData/StaticScene.h"

#include "SceneGraph/SceneGraphImpl.h"
#include "SceneGraph/View.h"
#include "SceneGraph/MultiView.h"
#include "SceneGraph/Util.h"
#include "SceneGraph/Visitor.h"

#include "RenderingData/RenderingData.h"

// gather
#include "SceneGraph/RenderingData/CachedRenderListNodeGather.h"

// list sorter
#include "System/Debug/I_DebugRender.h"


namespace SceneGraph
{

extern Debug::Bool g_StaticUsePvs;
extern Debug::Bool g_StaticUsePortals;
extern Debug::Bool g_UsePredication;
extern Debug::Float g_DrawDistanceScale;
extern Debug::Logger &g_Log;

Debug::Int g_RenderListMultiThreadedThreshold("Graphics.SceneGraph.RenderingData.RenderList.MultiThreadedThreshold", 0, Debug::Range(0, 512) );
Debug::Int g_RenderListsPerTask("Graphics.SceneGraph.RenderingData.RenderList.RenderListsPerTask", 16, Debug::Range(1, 16) );

template<class Nodes>
void CullBounds(const C_View& view, const Nodes& nodes, const C_Bounds* pBounds, const U16* pIndices, const U16* pFlags, const U8* pPvs, const U8* pPortal, const S32 begin, const S32 end, const U32 renderPassType, const bool useDrawDistanceScale)
{
	Maths::HF32 farClip = view.GetViewData().farClip * view.GetViewData().forwardMultiplier;
	Maths::HF32 drawDistanceScale = useDrawDistanceScale ? Maths::HF32(view.GetViewData().instanceDrawDistanceScale) : Maths::Constants<Maths::HF32>::One();
	U32 typeFlag = view.GetViewData().typeFlag;

	for(S32 i = begin; i < end; ++i)
	{
		DCBT(1024, &pBounds[i]);
		const C_Bounds& bb = pBounds[i];

		if(typeFlag == (typeFlag & pFlags[i]))
		{
			if(!pPortal || GetBitValue(pPortal, pIndices[i]))
			{
				if(!pPvs || GetBitValue(pPvs, pIndices[i]))
				{
					// load centre, size radius, and near and far clips
					Maths::HVec4 centreW(Maths::Load<Maths::HVec4>(&bb.centre));
					Maths::HVec4 sizeRadius(Maths::Load<Maths::HVec4>(&bb.size));
					Maths::HVec4 clip(Maths::Load<Maths::HVec4>(&bb.clip));
					clip *= drawDistanceScale;

					// set the max draw distance to be the minimum of draw distance and farclip
					clip.Y(Maths::Min(clip.Y(), farClip));

					// get size
					Maths::HVec3 size(sizeRadius);

					if(view.GetViewFrustum().CullAxisBox(centreW, Maths::HVec4(size), clip) || !(view.GetViewData().cullFlags & E_CullMethod_Frustum))
					{
						U32 predication = 0;
						view.AddVisibleNode(&nodes[pIndices[i]], renderPassType, predication);
					}
				}
			}
		}
	}
}

template<class Nodes>
void CullBoundsDistance(const C_View& view, const Nodes& nodes, const C_Bounds* pBounds, const U16* pIndices, const U16* pFlags, const U8* pPvs, const U8* pPortal, const S32 begin, const S32 end, const U32 renderPassType, const bool useDrawDistanceScale)
{
	Maths::HF32 farClip = view.GetViewData().farClip * view.GetViewData().forwardMultiplier;
	const Display::C_Matrix& viewMatrix = view.GetViewSettings3D().GetViewMatrix();
	Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&view.GetViewData().forwardZ));
	Maths::HF32 depthScale(view.GetViewData().depthScale);
	Maths::HF32 drawDistanceScale = useDrawDistanceScale ? Maths::HF32(view.GetViewData().instanceDrawDistanceScale) : Maths::Constants<Maths::HF32>::One();
	U32 typeFlag = view.GetViewData().typeFlag;

	for(S32 i = begin; i < end; ++i)
	{
		DCBT(1024, &pBounds[i]);
		const C_Bounds& bb = pBounds[i];

		if(typeFlag == (typeFlag & pFlags[i]))
		{
			if(!pPortal || GetBitValue(pPortal, pIndices[i]))
			{
				if(!pPvs || GetBitValue(pPvs, pIndices[i]))
				{
					// load centre, size radius, and near and far clips
					Maths::HVec4 centreW(Maths::Load<Maths::HVec4>(&bb.centre));
					Maths::HVec4 sizeRadius(Maths::Load<Maths::HVec4>(&bb.size));
					Maths::HVec4 clip(Maths::Load<Maths::HVec4>(&bb.clip));
					
					clip *= drawDistanceScale;
					// set the max draw distance to be the minimum of draw distance and farclip
					clip.Y(Maths::Min(clip.Y(), farClip));
					
					// get size
					Maths::HVec3 size(sizeRadius);

					if(view.GetViewFrustum().CullAxisBox(centreW, Maths::HVec4(size), clip) || !(view.GetViewData().cullFlags & E_CullMethod_Frustum))
					{
						Maths::HF32 depth = (centreW | forward);
						Maths::HF32 depthScaled = depth * depthScale;

						// subtract radius from depth for 
						depth -= sizeRadius.W();

						VolatileNodeInfo volatileNodeInfo;
						volatileNodeInfo.distance = (U32)depthScaled;

						U32 predication = 0;

						volatileNodeInfo.distanceSubtechnique = (view.GetViewData().distanceSubtechniqueEnable && depth > view.GetViewData().distanceSubtechniqueDistance) ? 1 : 0;

						view.AddVisibleNode(&nodes[pIndices[i]], renderPassType, predication, volatileNodeInfo.data);
					}
				}
			}
		}
	}
}

// C_CachedRenderListNode

C_CachedRenderListNode::C_CachedRenderListNode(const C_SceneGraph* pSceneGraph, const C_LevelInstance* pStaticScene, const C_StaticRenderListNode* pRenderListNode) :
	C_BoundsCull(),
	C_PassCull(),
	m_pStaticScene(pStaticScene),
	m_pRenderListNode(pRenderListNode),
	I_GatherNode(E_GatherNodeType_CachedRenderListNode, pSceneGraph)
{
}

C_CachedRenderListNode::~C_CachedRenderListNode()
{
}

void BuildCachedBounds(Btl::Vector<C_Bounds>& cachedBounds, const RenderingData::Bounds* inputBounds, const Btl::Vector<U16>& indices, Maths::HVec3& boundsMin, Maths::HVec3& boundsMax)
{
	S32 i;

	for(i = 0; i < indices.Size() - 4; ++i)
	{
		DCBT(0, &inputBounds[indices[i+4]]);

		C_Bounds& bounds = cachedBounds[i];
		
		// work out min and max bounds as we go
		const RenderingData::Bounds& renderingDataBounds = inputBounds[indices[i]];
		Maths::HVec3 minimum(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&renderingDataBounds.aabbMin));
		Maths::HVec3 maximum(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&renderingDataBounds.aabbMax));

		boundsMin = Maths::Min(boundsMin, minimum);
		boundsMax = Maths::Max(boundsMax, maximum);

		bounds.SetAxisBox(renderingDataBounds);
	}

	for( ; i < indices.Size(); ++i)
	{
		C_Bounds& bounds = cachedBounds[i];
		
		// work out min and max bounds as we go
		const RenderingData::Bounds& renderingDataBounds = inputBounds[indices[i]];
		Maths::HVec3 minimum(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&renderingDataBounds.aabbMin));
		Maths::HVec3 maximum(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&renderingDataBounds.aabbMax));
		boundsMin = Maths::Min(boundsMin, minimum);
		boundsMax = Maths::Max(boundsMax, maximum);

		bounds.SetAxisBox(renderingDataBounds);
	}

}

// build the cached renderlist node
void C_CachedRenderListNode::Build()
{
	PERF_EVENT("SceneGraph.CachedRenderListNode.Build");
	S32 numPrimitives =	m_pRenderListNode->GetRenderListNode()->GetNumPrimitives();
	S32 numInstances = m_pRenderListNode->GetRenderListNode()->GetNumInstances();
	S32 numInstanceBatches = m_pRenderListNode->GetRenderListNode()->GetNumInstanceBatches();

	// cache primitives
	// todo: use a bitvector of which primitives are visible
	{
		PERF_EVENT("SceneGraph.PrimitiveIndicesFromBitVector");
		S32 currentPass = 0;
		S32 begin = -1;
		S32 end = -1;
		S32 cachedDataIndex = -1;

		// indices need to be added in ascending order, gaps are ok
		for(S32 i = 0; i < numPrimitives; i += 1)
		{
			if(i >= end)
			{
				do
				{
					const RenderingData::RenderListNode::RenderPassInfo& renderPassInfo = m_pRenderListNode->GetRenderListNode()->GetRenderPassInfo((RenderingData::RenderPass)currentPass);
					begin = renderPassInfo.primitiveOffset;
					end = renderPassInfo.nPrimitives + renderPassInfo.primitiveOffset;
					++currentPass;
					assert(currentPass <= RenderingData::RENDER_PASS_MAX);
				}
				while(i >= end);

				cachedDataIndex = m_PrimitiveCachedData.Size();
				m_PrimitiveCachedData.PushBack();
				{
					CachedData& cachedData = m_PrimitiveCachedData.Back();
					cachedData.renderPassType = m_pSceneGraph->GetRenderingDataRenderPassMapping((RenderingData::RenderPass)(currentPass-1));
					cachedData.renderPassFlag = Flag(cachedData.renderPassType);
					cachedData.indices.Reserve(end - begin);
					EnableRenderPass(cachedData.renderPassFlag);
				}				
			}

			m_PrimitiveCachedData[cachedDataIndex].indices.PushBack(i);
			m_PrimitiveCachedData[cachedDataIndex].flags.PushBack(~(m_pRenderListNode->GetRenderListNode()->GetPrimitive(i).flags >> 16));
		}
	}

	// cache instances
	{
		PERF_EVENT("SceneGraph.InstanceIndicesFromBitVector");
		S32 currentPass = 0;
		S32 begin = -1;
		S32 end = -1;
		S32 cachedDataIndex = -1;

		// indices need to be added in ascending order, gaps are ok
		for(S32 i = 0; i < numInstances; i += 1)
		{
			if(i >= end)
			{
				do
				{
					const RenderingData::RenderListNode::RenderPassInfo& renderPassInfo = m_pRenderListNode->GetRenderListNode()->GetRenderPassInfo((RenderingData::RenderPass)currentPass);
					begin = renderPassInfo.instanceOffset;
					end = renderPassInfo.nInstances + renderPassInfo.instanceOffset;
					++currentPass;
					assert(currentPass <= RenderingData::RENDER_PASS_MAX);
				}
				while(i >= end);

				cachedDataIndex = m_InstanceCachedData.Size();
				m_InstanceCachedData.PushBack();
			
				{
					CachedData& cachedData = m_InstanceCachedData.Back();
					
					cachedData.renderPassType = m_pSceneGraph->GetRenderingDataRenderPassMapping((RenderingData::RenderPass)(currentPass-1));
					cachedData.renderPassFlag = Flag(cachedData.renderPassType);
					cachedData.indices.Reserve(end - begin);
					cachedData.flags.Reserve(end - begin);
					EnableRenderPass(cachedData.renderPassFlag);
				}
			}
			m_InstanceCachedData[cachedDataIndex].indices.PushBack(i);
			m_InstanceCachedData[cachedDataIndex].flags.PushBack(~(m_pRenderListNode->GetRenderListNode()->GetInstance(i).flags >> 16));
		}
	}
		
	/*for(S32 i = 0; i < numInstanceBatches; ++i)
	{
		m_InstanceBatchIndices.PushBack(i);
	}*/
	
	Maths::HVec3 boundsMin(10000.0f);
	Maths::HVec3 boundsMax(-10000.0f);

	{
		PERF_EVENT("SceneGraph.BoundsMemoryAllocation");
		m_NodeCount = 0;
		m_MultiThreadedGather = true;
		for(S32 i = 0; i < m_PrimitiveCachedData.Size(); ++i)
		{
			m_NodeCount += m_PrimitiveCachedData[i].indices.Size();
			m_PrimitiveCachedData[i].bounds.Resize(m_PrimitiveCachedData[i].indices.Size());
			if(m_PrimitiveCachedData[i].indices.Size() > NUM_RESULTS)
			{
				LOGWARN(g_Log, "PERFORMANCE ISSUE: Found renderlist node with %d primitives (max is %d). This node will be gathered on the main thread.\n", m_PrimitiveCachedData[i].indices.Size(), NUM_RESULTS);
				m_MultiThreadedGather = false;
			}
		}
		for(S32 i = 0; i < m_InstanceCachedData.Size(); ++i)
		{
			m_NodeCount += m_InstanceCachedData[i].indices.Size();
			m_InstanceCachedData[i].bounds.Resize(m_InstanceCachedData[i].indices.Size());
			if(m_InstanceCachedData[i].indices.Size() > NUM_RESULTS)
			{
				LOGWARN(g_Log, "PERFORMANCE ISSUE: Found renderlist node with %d instances (max is %d). This node will be gathered on the main thread.\n", m_InstanceCachedData[i].indices.Size(), NUM_RESULTS);
				m_MultiThreadedGather = false;
			}
		}
	}

	{
		PERF_EVENT("SceneGraph.CachedPrimitiveBounds");
		for(S32 i = 0; i < m_PrimitiveCachedData.Size(); ++i)
		{
			BuildCachedBounds(m_PrimitiveCachedData[i].bounds, m_pRenderListNode->GetRenderListNode()->GetPrimitiveBounds(), m_PrimitiveCachedData[i].indices, boundsMin, boundsMax);
		}
	}
	{
		PERF_EVENT("SceneGraph.CachedInstanceBounds");
		for(S32 i = 0; i < m_InstanceCachedData.Size(); ++i)
		{
			BuildCachedBounds(m_InstanceCachedData[i].bounds, m_pRenderListNode->GetRenderListNode()->GetInstanceBounds(), m_InstanceCachedData[i].indices, boundsMin, boundsMax);
		}
	}

	const RenderingData::Bounds& bounds = m_pRenderListNode->GetRenderListNode()->GetBounds();

	SetAxisBox(boundsMin, boundsMax);
	SetNearFarDistance(bounds.distanceMin, bounds.distanceMax);
}

void C_CachedRenderListNode::Cache()
{
	m_pRenderingDataNode = m_pRenderListNode->GetRenderListNode();
}

void C_CachedRenderListNode::Gather(const C_View& view) const
{
	//if(!view.RenderListEnabled())
	//return;

	// fake a multiview for now, optimise this later
	C_MultiView multiView(const_cast<C_SceneGraph*>(m_pSceneGraph));
	multiView.AddView(&view);

	C_LevelInstance::BeginGather1(multiView);
	Gather(multiView);
	C_LevelInstance::BeginGather2(multiView);

	C_CachedRenderListNodeGatherBatcher* pMultiviewBatcher = (C_CachedRenderListNodeGatherBatcher*)multiView.GetBatcher();
	C_CachedRenderListNodeGatherBatcher* pViewBatcher = (C_CachedRenderListNodeGatherBatcher*)view.GetBatcher();
	
	for(S32 i = 0; i < pMultiviewBatcher->m_CachedRenderListNodeBatches.Size(); ++i)
		pViewBatcher->m_CachedRenderListNodeBatches.PushBack(pMultiviewBatcher->m_CachedRenderListNodeBatches[i]);
}

void C_CachedRenderListNode::Gather(const C_MultiView& multiView) const
{
	// first cull the entire renderlist against each view in the multiview
	ViewPtrs visibleViews;

	// prefetch the pvs bits or its crazy L2 misses
	DCBT(0, &m_pRenderingDataNode->GetPvsBits());
	
	S32 pvsCell = multiView.GetPvsCell();

	for(S32 i = 0; i < multiView.Size(); ++i)
	{
		bool pvsVisible = (pvsCell != -1) ? (m_pRenderingDataNode->GetPvsBits())[pvsCell] : true;
		pvsVisible |= !g_StaticUsePvs;

		const C_View* pView = multiView.GetView(i);

		if(!pView->RenderListEnabled())
			continue;

		if(pvsVisible && pView->RenderPassEnabled(C_PassCull::RenderPasses()))
		{
			if(C_BoundsCull::CullAxisBoxDistance(*pView))
				visibleViews.PushBack(pView);
		}
	}
	
	// if this renderlist is visible in 1 or more views
	if(visibleViews.Size() > 0)
	{
		// TODO: [Stove] Calculate the portal bitfields for current render list for each visible view -- for now just use the first view in the list.
		// TODO: [CharlieB] make this thread safe - move bit vector creation to spu job
		if(visibleViews[0]->CullMethodEnabled(E_CullMethod_Portal))
			m_pRenderingDataNode->CalculateVisibilityFromRoomBitVector(visibleViews[0]->GetVisiblePortalRoomSet());

		if(	m_pSceneGraph->JobQueueActive() &&
			m_NodeCount > g_RenderListMultiThreadedThreshold &&
			m_MultiThreadedGather)
		{
			// get some memory for this job
			if(CachedRenderListNodeData* pData = C_LevelInstance::m_CachedRenderListNodeData.Allocate())
			{
				pData->viewData.Clear();
				pData->pCachedRenderListNode = this;

				// add the view data for all the visible views
				for(S32 i = 0; i < visibleViews.Size(); ++i)
					pData->viewData.PushBack(&visibleViews[i]->GetViewData());
		
				// cull by primitive render node type
				pData->primitiveData.Clear();
				if(multiView.RenderNodeEnabled(Flag<E_RenderNodeType_StaticPrimitive>()) && m_PrimitiveCachedData.Size() != 0)
				{
					// for now all multiviews must be using the same pvs and portal data. this could change to be per view maybe
					pData->pPrimitivePvs = ((pvsCell > -1 && g_StaticUsePvs) ? m_pRenderingDataNode->GetPrimitivePvs(pvsCell) : NULL);
					pData->primitivePvsCount = m_pRenderingDataNode->GetNumPrimitives();
					pData->pPrimitivePortal = (visibleViews[0]->GetVisiblePortalRoomSet().Size() > 0 && g_StaticUsePortals) ? m_pRenderingDataNode->GetPortalPrimitiveVisibilityBitVector().Data() : NULL;
					pData->primitivePortalCount = m_pRenderingDataNode->GetPortalPrimitiveVisibilityBitVector().Size();

					C_LevelInstance::m_CachedRenderListNodeDataPass.Prefetch();

					for(S32 i = 0; i < m_PrimitiveCachedData.Size(); ++i)
					{
						// render pass cull
						if(multiView.RenderPassEnabled(m_PrimitiveCachedData[i].renderPassFlag))
						{
							if(CachedRenderListNodeData::PassData* pPassData = C_LevelInstance::m_CachedRenderListNodeDataPass.Allocate())
							{
								pPassData->outputs.Clear();
								pData->primitiveData.PushBack(pPassData);
								
								const C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(m_PrimitiveCachedData[i].renderPassType);
								
								// input to SPU
								pPassData->input.pBounds = &m_PrimitiveCachedData[i].bounds[0];
								pPassData->input.pIndices = &m_PrimitiveCachedData[i].indices[0];
								pPassData->input.pFlags = &m_PrimitiveCachedData[i].flags[0];
								pPassData->input.boundsCount = m_PrimitiveCachedData[i].bounds.Size();
								pPassData->input.renderPassType = m_PrimitiveCachedData[i].renderPassType;
								pPassData->input.volatileInfoFlags = pRenderPass->GetVolatileInfoFlags() | multiView.GetVolatileInfoFlags();
								
								C_LevelInstance::m_CachedRenderListNodeDataPassOutput.Prefetch();

								// add some output for each view
								for(S32 v = 0; v < visibleViews.Size(); ++v)
								{
									// check view specific flags
									if(visibleViews[v]->RenderNodeEnabled(Flag<E_RenderNodeType_StaticPrimitive>()) &&
										visibleViews[v]->RenderPassEnabled(m_PrimitiveCachedData[i].renderPassFlag))
									{
										if(CachedRenderListNodeData::PassData::Output* pOutput = C_LevelInstance::m_CachedRenderListNodeDataPassOutput.AllocatePrefetch())
										{
											pOutput->numResults = 0;
											pOutput->pView = visibleViews[v];
											pOutput->viewIndex = v;
											pPassData->outputs.PushBack(pOutput);
										}
									}
								}
							}
						}
					}
				}
				// cull by instance render node type
				pData->instanceData.Clear();
				if(multiView.RenderNodeEnabled(Flag<E_RenderNodeType_StaticInstance>()) && m_InstanceCachedData.Size() != 0)
				{
					pData->pInstancePvs = ((pvsCell > -1 && g_StaticUsePvs) ? m_pRenderingDataNode->GetInstancePvs(pvsCell) : NULL);
					pData->instancePvsCount = m_pRenderingDataNode->GetNumInstances();
					pData->pInstancePortal = (visibleViews[0]->GetVisiblePortalRoomSet().Size() > 0 && g_StaticUsePortals) ? m_pRenderingDataNode->GetPortalInstanceVisibilityBitVector().Data() : NULL;
					pData->instancePortalCount = m_pRenderingDataNode->GetPortalInstanceVisibilityBitVector().Size();

					C_LevelInstance::m_CachedRenderListNodeDataPass.Prefetch();

					for(S32 i = 0; i < m_InstanceCachedData.Size(); ++i)
					{
						// render pass cull
						if(multiView.RenderPassEnabled(m_InstanceCachedData[i].renderPassFlag))
						{
							if(CachedRenderListNodeData::PassData* pPassData = C_LevelInstance::m_CachedRenderListNodeDataPass.Allocate())
							{
								pPassData->outputs.Clear();
								pData->instanceData.PushBack(pPassData);
								
								const C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(m_InstanceCachedData[i].renderPassType);
								
								// input to SPU
								pPassData->input.pBounds = &m_InstanceCachedData[i].bounds[0];
								pPassData->input.pIndices = &m_InstanceCachedData[i].indices[0];
								pPassData->input.pFlags = &m_InstanceCachedData[i].flags[0];
								pPassData->input.boundsCount = m_InstanceCachedData[i].bounds.Size();
								pPassData->input.renderPassType = m_InstanceCachedData[i].renderPassType;
								pPassData->input.volatileInfoFlags = pRenderPass->GetVolatileInfoFlags() | multiView.GetVolatileInfoFlags();
								
								C_LevelInstance::m_CachedRenderListNodeDataPassOutput.Prefetch();

								// add some output
								for(S32 v = 0; v < visibleViews.Size(); ++v)
								{
									// check view specific flags
									if(visibleViews[v]->RenderNodeEnabled(Flag<E_RenderNodeType_StaticInstance>()) &&
										visibleViews[v]->RenderPassEnabled(m_InstanceCachedData[i].renderPassFlag))
									{
										if(CachedRenderListNodeData::PassData::Output* pOutput = C_LevelInstance::m_CachedRenderListNodeDataPassOutput.AllocatePrefetch())
										{
											pOutput->numResults = 0;
											pOutput->pView = visibleViews[v];
											pOutput->viewIndex = v;
											pPassData->outputs.PushBack(pOutput);
										}
									}
								}
							}
						}
					}
				}

				C_CachedRenderListNodeGatherBatcher* pBatcher = (C_CachedRenderListNodeGatherBatcher*)multiView.GetBatcher();
				pBatcher->AddRenderListNode(pData);
			}
		}
		else
		{
			// we are gathering this render list on a single core/spu
			for(S32 v = 0; v < visibleViews.Size(); ++v)
			{
				// gather up primitive nodes
				if(visibleViews[v]->RenderNodeEnabled(Flag<E_RenderNodeType_StaticPrimitive>()))
				{
					for(S32 i = 0; i < m_PrimitiveCachedData.Size(); ++i)
					{
						const CachedData& cachedData = m_PrimitiveCachedData[i];

						// pass cull individual primitive pass
						if(visibleViews[v]->RenderPassEnabled(cachedData.renderPassFlag))
						{
							const C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(cachedData.renderPassType);
							const U8* pPvs = ((pvsCell > -1 && g_StaticUsePvs) ? m_pRenderingDataNode->GetPrimitivePvs(pvsCell) : NULL);
							const U8* pPortal = (visibleViews[v]->GetVisiblePortalRoomSet().Size() > 0 && g_StaticUsePortals) ? m_pRenderingDataNode->GetPortalPrimitiveVisibilityBitVector().Data() : NULL;
							U32 volatileInfoFlags = pRenderPass->GetVolatileInfoFlags() | visibleViews[v]->GetVolatileInfoFlags();

							if(volatileInfoFlags & E_VolatileNodeInfo_Distance)
								CullBoundsDistance(*visibleViews[v], m_pRenderListNode->GetPrimitiveNodes(), &cachedData.bounds[0], &cachedData.indices[0], &cachedData.flags[0], pPvs, pPortal, 0, cachedData.indices.Size(), cachedData.renderPassType, true);
							else
								CullBounds(*visibleViews[v], m_pRenderListNode->GetPrimitiveNodes(), &cachedData.bounds[0], &cachedData.indices[0], &cachedData.flags[0], pPvs, pPortal, 0, cachedData.indices.Size(), cachedData.renderPassType, true);
						}	
					}
				}

				// gather up instance nodes
				if(visibleViews[v]->RenderNodeEnabled(Flag<E_RenderNodeType_StaticInstance>()))
				{
					for(S32 i = 0; i < m_InstanceCachedData.Size(); ++i)
					{
						const CachedData& cachedData = m_InstanceCachedData[i];

						if(visibleViews[v]->RenderPassEnabled(cachedData.renderPassFlag))
						{
							const C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(cachedData.renderPassType);
							const U8* pPvs = ((pvsCell > -1 && g_StaticUsePvs) ? m_pRenderingDataNode->GetInstancePvs(pvsCell) : NULL);
							const U8* pPortal = (visibleViews[v]->GetVisiblePortalRoomSet().Size() > 0 && g_StaticUsePortals) ? m_pRenderingDataNode->GetPortalInstanceVisibilityBitVector().Data() : NULL;
							U32 volatileInfoFlags = pRenderPass->GetVolatileInfoFlags() | visibleViews[v]->GetVolatileInfoFlags();

							if(volatileInfoFlags & E_VolatileNodeInfo_Distance)
								CullBoundsDistance(*visibleViews[v], m_pRenderListNode->GetInstanceNodes(), &cachedData.bounds[0], &cachedData.indices[0], &cachedData.flags[0], pPvs, pPortal, 0, cachedData.indices.Size(), cachedData.renderPassType, true);
							else
								CullBounds(*visibleViews[v], m_pRenderListNode->GetInstanceNodes(), &cachedData.bounds[0], &cachedData.indices[0], &cachedData.flags[0], pPvs, pPortal, 0, cachedData.indices.Size(), cachedData.renderPassType, true);
						}
					}
				}
			}
		}
	}
}

void C_CachedRenderListNode::GatherDeferred(CachedRenderListNodeData* pData)
{
	const C_StaticRenderListNode* pRenderListNode = pData->pCachedRenderListNode->GetRenderListNode();

	// primitives
	for(S32 pass = 0; pass < pData->primitiveData.Size(); ++pass)
	{
		CachedRenderListNodeData::PassData* pPassData = pData->primitiveData[pass];
		
		for(S32 o = 0; o < pPassData->outputs.Size(); ++o)
		{
			CachedRenderListNodeData::PassData::Output* pOutput = pPassData->outputs[o];
			const C_View* pView = pOutput->pView;

			// get primitives and resize array
			C_VisibleNodes& primitives = *pView->GetVisiblePrimitives(pPassData->input.renderPassType);
			S32 size = primitives.Size();
			primitives.Resize(size + pOutput->numResults);

			// fetch start of next set of data
			S32 o1 = o + 1;
			if(o1 < pPassData->outputs.Size())
				DCBT(0, pPassData->outputs[o1]);
			
			for(U32 i = 0; i < pOutput->numResults; ++i, ++size)
			{
				DCBT(128, &pOutput->results[i]);
				
				primitives[size].m_pNode = &pRenderListNode->GetPrimitiveNodes()[pOutput->results[i].index];
				primitives[size].m_VolatileNodeInfo = pOutput->results[i].volatileInfo;
			}
			C_LevelInstance::m_CachedRenderListNodeDataPassOutput.Free(pOutput);
		}
		C_LevelInstance::m_CachedRenderListNodeDataPass.Free(pPassData);
	}

	// instances
	for(S32 i = 0; i < pData->instanceData.Size(); ++i)
	{
		CachedRenderListNodeData::PassData* pPassData = pData->instanceData[i];
		
		for(S32 o = 0; o < pPassData->outputs.Size(); ++o)
		{
			CachedRenderListNodeData::PassData::Output* pOutput = pPassData->outputs[o];
			const C_View* pView = pOutput->pView;

			// get instances and resize array
			C_VisibleNodes& instances = *pView->GetVisibleInstances(pPassData->input.renderPassType);
			S32 size = instances.Size();
			instances.Resize(size + pOutput->numResults);

			// fetch start of next set of data
			S32 o1 = o + 1;
			if(o1 < pPassData->outputs.Size())
				DCBT(0, pPassData->outputs[o1]);

			for(U32 i = 0; i < pOutput->numResults; ++i, ++size)
			{
				DCBT(128, &pOutput->results[i]);

				instances[size].m_pNode = &pRenderListNode->GetInstanceNodes()[pOutput->results[i].index];
				instances[size].m_VolatileNodeInfo = pOutput->results[i].volatileInfo;
			}
			C_LevelInstance::m_CachedRenderListNodeDataPassOutput.Free(pOutput);
		}
		C_LevelInstance::m_CachedRenderListNodeDataPass.Free(pPassData);
	}
	C_LevelInstance::m_CachedRenderListNodeData.Free(pData);
}

void C_CachedRenderListNode::DebugRender() const
{
	
}

void C_CachedRenderListNode::Accept(I_Visitor& visitor) const
{
	visitor.Visit(*this);
}

// CachedRenderListNodeGatherBatcher

void C_CachedRenderListNodeGatherBatcher::BeginGather1()
{
	if(CachedRenderListNodeDataBatch* pBatch = C_LevelInstance::m_CachedRenderListNodeDataBatch.Allocate())
	{
		pBatch->cachedRenderListNodeData.Clear();
		m_CachedRenderListNodeBatches.PushBack(pBatch);
	}
}

void C_CachedRenderListNodeGatherBatcher::BeginGather2()
{
	if(m_CachedRenderListNodeBatches.Size() != 0)
	{
		CachedRenderListNodeDataBatch* pLastBatch = m_CachedRenderListNodeBatches[m_CachedRenderListNodeBatches.Size() - 1];
		if(pLastBatch->cachedRenderListNodeData.Size() > 0 && pLastBatch->cachedRenderListNodeData.Size() < g_RenderListsPerTask)
			Scheduler::AddTask(CachedRenderListNodeGatherModule, pLastBatch, NULL, m_pView->GetJobQueueGroup(), m_pView->GetSceneGraph()->GetJobQueuePolicyGather());
	}
}

void C_CachedRenderListNodeGatherBatcher::EndGather()
{
	PERF_EVENT("SceneGraph.GatherDeferred");
	for(S32 i = 0; i < m_CachedRenderListNodeBatches.Size(); ++i)
	{
		CachedRenderListNodeDataBatch* pBatch = m_CachedRenderListNodeBatches[i];

		for(S32 j = 0; j < pBatch->cachedRenderListNodeData.Size(); ++j)
			C_CachedRenderListNode::GatherDeferred(pBatch->cachedRenderListNodeData[j]);

		C_LevelInstance::m_CachedRenderListNodeDataBatch.Free(pBatch);
	}
	m_CachedRenderListNodeBatches.Clear();
}

void C_CachedRenderListNodeGatherBatcher::AddRenderListNode(CachedRenderListNodeData* pData)
{
	if(m_CachedRenderListNodeBatches.Size() != 0)
	{
		CachedRenderListNodeDataBatch* pBatch = m_CachedRenderListNodeBatches[m_CachedRenderListNodeBatches.Size() - 1];
		if(pBatch->cachedRenderListNodeData.Size() != pBatch->cachedRenderListNodeData.Capacity())
		{
			pBatch->cachedRenderListNodeData.PushBack(pData);

			if(pBatch->cachedRenderListNodeData.Size() == g_RenderListsPerTask)
			{
				Scheduler::AddTask(CachedRenderListNodeGatherModule, pBatch, NULL, m_pView->GetJobQueueGroup(), m_pView->GetSceneGraph()->GetJobQueuePolicyGather());

				// add a new
				if(CachedRenderListNodeDataBatch* pNewBatch = C_LevelInstance::m_CachedRenderListNodeDataBatch.Allocate())
				{
					pNewBatch->cachedRenderListNodeData.Clear();
					m_CachedRenderListNodeBatches.PushBack(pNewBatch);
				}
			}
		}
	}
}

} // namespace SceneGraph



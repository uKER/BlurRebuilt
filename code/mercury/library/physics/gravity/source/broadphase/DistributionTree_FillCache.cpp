#include "StdAfx.h"
#include "gravity/broadphase/DistributionTree.h"
#include "gravity/narrowphase/CollisionFilter.inl"
#include "gravity/narrowphase/AxisBox.inl"
#include "system/perf.h"

namespace Gravity
{
	void FillLineCache(C_ObjectWrapper* pWrapper, Collision::C_LineCollisionCache& pCache, 
		Collision::C_CRAxisBox& box, Collision::C_CollisionFilter* pFilter, 
		const Maths::C_Normal PCREF normal, const Maths::HF32 PCREF maxCosAngle, SharedPhysics::C_SafeStack<true>& stack, 
		S32 size, Collision::C_CollisionManager& manager, SharedPhysics::C_SmallSPUCache& spuCache)
	{
		if(pCache.Full())
			return;
		
		while(pWrapper)
		{
			Spu::MemBaseMarker innerMarker;
			C_ObjectWrapper* pLoadedWrapper = (C_ObjectWrapper*)Spu::FetchAllocUnaligned(pWrapper, sizeof(C_ObjectWrapper));
			C_Object* pObj=(C_Object*)Spu::FetchAlloc(pLoadedWrapper->m_pObject, sizeof(C_Object));
			Collision::C_CRAxisBox& objBox=pObj->m_TransformedBox;
			Collision::C_CollisionFilter* pOtherFilter = (Collision::C_CollisionFilter*)Spu::FetchAlloc(&pObj->m_pShape->m_Filter, sizeof(Collision::C_CollisionFilter));

			if(pFilter->CanCollide(pOtherFilter) && objBox.Intersects(box))
			{
				stack.PushOffset(size);
				manager.FillCache(pObj->m_pShape, pObj->m_pGraphNode, &pCache, box, pFilter, normal, maxCosAngle, stack, size, spuCache);
				stack.PushOffset(0);
				if(pCache.Full())
					return;
			}

			pWrapper = pLoadedWrapper->m_pNext;
		}
	}
	void C_DistributionTree::FillCache(Collision::C_CRAxisBox& box, Collision::C_LineCollisionCache& pCache, SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionFilter* pFilter,
		const Maths::C_Normal PCREF normal, const Maths::HF32 PCREF maxCosAngle, Collision::C_CollisionManager& manager, SharedPhysics::C_SmallSPUCache& spuCache)
	{
		//For now, only fill cache with static and kinematic objects, although we could possibly put in
		//dynamics at some point, e.g. driving off a dynamic surface...

		PERF_EVENT("gravity.dynamic_fillcache");
		if(!m_pRootNode || pCache.Full())
			return;
		
		C_DistributionNode* node(NULL);//=m_pRootNode;

		S32 size = 1;
		C_DistributionNode* pStackTop(m_pRootNode);
		stack.Push(m_pRootNode, size );

		while(size--)
		{
			Spu::MemBaseMarker marker;
			C_DistributionNode* node = pStackTop;
			C_DistributionNode* loadedNode = (C_DistributionNode*)Spu::FetchAlloc(node, sizeof(C_DistributionNode));
			//if(size > 0)
			pStackTop = stack.Pop<C_DistributionNode>(Maths::Max(0, size-1));
			
			if(loadedNode->m_bbox.Intersects(box))
			{
				//If down here, we are at a leaf node, so we need to do the ray tracing in here
				FillLineCache(loadedNode->m_pStaticObjects, pCache, box, pFilter, normal, maxCosAngle, stack, size, manager, spuCache);
				FillLineCache(loadedNode->m_pKinematicObjects, pCache, box, pFilter, normal, maxCosAngle, stack, size, manager, spuCache);

				if(loadedNode->m_pLeftNode)
				{
					stack.Push( loadedNode->m_pRightNode, size++ );
					stack.Push( loadedNode->m_pLeftNode, size++ );
					pStackTop = loadedNode->m_pLeftNode;
				}
			}
		}
	}
}


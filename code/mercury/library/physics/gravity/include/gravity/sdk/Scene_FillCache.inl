#ifndef SCENE_FILL_CACHE_H
#define SCENE_FILL_CACHE_H

#include "gravity/sdk/scene.h"
#include "gravity/narrowphase/Shapes.h"
#include "gravity/sharedphysics/SPU_Memory.h"

namespace Gravity
{

void C_Scene::FillCache(Collision::C_CRAxisBox& box, Collision::C_LineCollisionCache& pCache, SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionFilter* pFilter,
		const Maths::C_Normal PCREF normal, const Maths::HF32 PCREF maxCosAngle)
	{
		Spu::MemBaseMarker marker;
//#if CFG_DEBUG
//		SharedPhysics::C_SmallSPUCache spuCache(1);
//#else
//		SharedPhysics::C_SmallSPUCache spuCache(4);
//#endif

		SharedPhysics::C_SmallSPUCache spuCache(2);
		for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements && !pCache.Full(); ++a)
		{
			Spu::MemBaseMarker innerMarker;
			BodyPtr* pBodyPtr = (BodyPtr*)Spu::FetchAllocUnaligned(&m_MeshCollisionBodies.m_pElems[a], sizeof(BodyPtr));
			C_Body* pBody = (C_Body*)Spu::FetchAlloc(pBodyPtr->m_pElement, sizeof(C_Body));
			Collision::C_TriangleMeshShape* pShape = (Collision::C_TriangleMeshShape*)Spu::FetchAlloc(pBody->m_pShape, sizeof(Collision::C_TriangleMeshShape));
			if(pShape->m_Filter.CanCollide(pFilter))
			{
				pShape->m_AABBTree.FillLineCollisionCache(box, pCache, stack, pFilter, pBody->m_pShape, &pBodyPtr->m_pElement->m_GraphNode,
					normal, maxCosAngle, spuCache);
			}
		}

		//Kier - removed this for Blur release - they don't have platforms to drive on, so no point 
		//going through the broad phase to find them!
#if 0
		if(m_pTree && !pCache.Full())
		{
			Spu::MemBaseMarker innerMarker;
			Gravity::C_DistributionTree* pTree = (C_DistributionTree*)Spu::FetchAlloc(m_pTree, sizeof(C_DistributionTree));
			pTree->FillCache(box, pCache, stack, pFilter, normal, maxCosAngle, *m_pCollisionManager, spuCache);
		}
#endif
		
		if(m_pGroundPlane)
		{
			Spu::MemBaseMarker innerMarker;
			Collision::C_Shape* pGroundShape = (Collision::C_Shape*)Spu::Get((INTPTR*)&m_pGroundPlane->m_pShape);	// [Blur2PC][x64] pointer-wide
			Collision::C_PlaneShape* pPlaneShape = (Collision::C_PlaneShape*)Spu::FetchAlloc(pGroundShape, sizeof(Collision::C_PlaneShape));
			if(pPlaneShape->m_Filter.CanCollide(pFilter) && (pPlaneShape->m_Plane.m_n | normal) <= maxCosAngle)
			{
				pCache.AddPlane(pGroundShape);
			}
		}
	}

}

#endif
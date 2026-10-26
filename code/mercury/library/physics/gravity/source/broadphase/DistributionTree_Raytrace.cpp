#include "StdAfx.h"
#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/BroadPhase/Object.inl"
#include "gravity/broadphase/distributiontree.h"
#include "gravity/narrowphase/PrimitiveTest.h"
#include "gravity/narrowphase/AxisBox.inl"

#include <system/perf.h>

namespace Gravity
{

	__forceinline bool TraceListWater(C_ObjectWrapper* pWrapper, Collision::C_Ray& ray, Maths::HF32& tmin, SharedPhysics::C_SafeStack<true>& stack, U32 size,
		Collision::C_CollisionManager& manager, SharedPhysics::C_SmallSPUCache& spuCache,
		const Collision::C_AxisBoxMinMax& nodeBox)
	{
		bool bHit = false;
		while(pWrapper)
		{
			Spu::MemBaseMarker innerMarker;
			C_ObjectWrapper* pLoadedWrapper = (C_ObjectWrapper*)Spu::FetchAllocUnaligned(pWrapper, sizeof(C_ObjectWrapper));
			C_Object* pObj=(C_Object*)Spu::FetchAlloc(pLoadedWrapper->m_pObject, sizeof(C_Object));
			Collision::C_CRAxisBox& objBox=pObj->m_TransformedBox;
			if(Collision::IntersectRayAABBInCell(ray.m_start, ray.m_result.m_contact, objBox.centre, 
				objBox.extents, ray.m_Radius, nodeBox, tmin))
			{
				//contact=ray.m_start + (contact - ray.m_start)*tmin;
				Collision::C_CollisionObject obj(pObj->CollisionObject());
				U32 oldOffset = stack.m_OffsetValue;
				stack.PushOffset(size);
				bHit = manager.RayTraceWater(&obj, ray, stack, spuCache) | bHit;
				stack.PushOffset(oldOffset);
			}
			
			pWrapper=pLoadedWrapper->m_pNext;
		}
		return bHit;
	}

	__forceinline bool TraceList(C_ObjectWrapper* pWrapper, Collision::C_Ray& ray, Maths::HF32& tmin, SharedPhysics::C_SafeStack<true>& stack, U32 size,
		Collision::C_CollisionManager& manager, SharedPhysics::C_SmallSPUCache& spuCache,
		const Collision::C_AxisBoxMinMax& nodeBox)
	{
		bool bHit = false;
		while(pWrapper)
		{
			Spu::MemBaseMarker innerMarker;
			C_ObjectWrapper* pLoadedWrapper = (C_ObjectWrapper*)Spu::FetchAllocUnaligned(pWrapper, sizeof(C_ObjectWrapper));
			C_Object* pObj=(C_Object*)Spu::FetchAlloc(pLoadedWrapper->m_pObject, sizeof(C_Object));
			Collision::C_CRAxisBox& objBox=pObj->m_TransformedBox;
			if(Collision::IntersectRayAABBInCell(ray.m_start, ray.m_result.m_contact, objBox.centre, 
				objBox.extents, ray.m_Radius, nodeBox))
			{
				//contact=ray.m_start + (contact - ray.m_start)*tmin;
				Collision::C_CollisionObject obj(pObj->CollisionObject());
				U32 oldOffset = stack.m_OffsetValue;
				stack.PushOffset(size);
				bHit = manager.RayTrace(&obj, ray, stack, spuCache) | bHit;
				stack.PushOffset(oldOffset);
			}
			
			pWrapper=pLoadedWrapper->m_pNext;
		}
		return bHit;
	}

	__forceinline bool TraceList(C_ObjectWrapper* pWrapper, Collision::C_TransparentRay& ray, Maths::HF32& tmin, SharedPhysics::C_SafeStack<true>& stack, U32 size,
		Collision::C_CollisionManager& manager, Collision::C_CollisionFilter* pFilter, SharedPhysics::C_SmallSPUCache& spuCache,
		const Collision::C_AxisBoxMinMax& nodeBox)
	{
		bool bHit = false;
		while(pWrapper)
		{
			Spu::MemBaseMarker innerMarker;
			C_ObjectWrapper* pLoadedWrapper = (C_ObjectWrapper*)Spu::FetchAllocUnaligned(pWrapper, sizeof(C_ObjectWrapper));
			C_Object* pObj=(C_Object*)Spu::FetchAlloc(pLoadedWrapper->m_pObject, sizeof(C_Object));
			Collision::C_CRAxisBox& objBox=pObj->m_TransformedBox;
			if(Collision::IntersectRayAABBInCell(ray.m_start, ray.m_end, objBox.centre, 
				objBox.extents, ray.m_Radius, nodeBox))
			{
				//contact=ray.m_start + (contact - ray.m_start)*tmin;
				Collision::C_CollisionObject obj(pObj->CollisionObject());
				U32 oldOffset = stack.m_OffsetValue;
				stack.PushOffset(size);
				bHit = manager.RayTrace(&obj, ray, pFilter, stack, spuCache) | bHit;
				stack.PushOffset(oldOffset);
			}
			
			pWrapper=pLoadedWrapper->m_pNext;
		}
		return bHit;
	}



	__forceinline bool FatRayTraceList(C_ObjectWrapper* pWrapper, Collision::C_Ray& ray, Maths::HF32& tmin, SharedPhysics::C_SafeStack<true>& stack, U32 size,
		Collision::C_CollisionManager& manager, SharedPhysics::C_SmallSPUCache& spuCache,
		const Collision::C_AxisBoxMinMax& nodeBox)
	{
		bool bHit = false;
		while(pWrapper)
		{
			Spu::MemBaseMarker innerMarker;
			C_ObjectWrapper* pLoadedWrapper = (C_ObjectWrapper*)Spu::FetchAllocUnaligned(pWrapper, sizeof(C_ObjectWrapper));
			C_Object* pObj=(C_Object*)Spu::FetchAlloc(pLoadedWrapper->m_pObject, sizeof(C_Object));
			Collision::C_CRAxisBox& objBox=pObj->m_TransformedBox;
			if(Collision::IntersectRayAABBInCell(ray.m_start, ray.m_result.m_contact, objBox.centre, 
				objBox.extents, ray.m_Radius, nodeBox))
			{
				//contact=ray.m_start + (contact - ray.m_start)*tmin;
				Collision::C_CollisionObject obj(pObj->CollisionObject());
				U32 oldOffset = stack.m_OffsetValue;
				stack.PushOffset(size);
				bHit = manager.FatRayTrace(&obj, ray, stack, spuCache) | bHit;
				stack.PushOffset(oldOffset);
			}
			
			pWrapper=pLoadedWrapper->m_pNext;
		}
		return bHit;
	}

	__forceinline bool FatRayTraceList(C_ObjectWrapper* pWrapper, Collision::C_TransparentRay& ray, Maths::HF32& tmin, SharedPhysics::C_SafeStack<true>& stack, U32 size,
		Collision::C_CollisionManager& manager, Collision::C_CollisionFilter* pFilter, SharedPhysics::C_SmallSPUCache& spuCache,
		const Collision::C_AxisBoxMinMax& nodeBox)
	{
		bool bHit = false;
		while(pWrapper)
		{
			Spu::MemBaseMarker innerMarker;
			C_ObjectWrapper* pLoadedWrapper = (C_ObjectWrapper*)Spu::FetchAllocUnaligned(pWrapper, sizeof(C_ObjectWrapper));
			C_Object* pObj=(C_Object*)Spu::FetchAlloc(pLoadedWrapper->m_pObject, sizeof(C_Object));
			Collision::C_CRAxisBox& objBox=pObj->m_TransformedBox;
			if(Collision::IntersectRayAABBInCell(ray.m_start, ray.m_end, objBox.centre, 
				objBox.extents, ray.m_Radius, nodeBox))
			{
				//contact=ray.m_start + (contact - ray.m_start)*tmin;
				Collision::C_CollisionObject obj(pObj->CollisionObject());
				U32 oldOffset = stack.m_OffsetValue;
				stack.PushOffset(size);
				bHit = manager.FatRayTrace(&obj, ray, pFilter, stack, spuCache) | bHit;
				stack.PushOffset(oldOffset);
			}
			
			pWrapper=pLoadedWrapper->m_pNext;
		}
		return bHit;
	}

	
	bool C_DistributionTree::RayTracing(Collision::C_Ray &ray, Collision::C_CollisionManager& manager, SharedPhysics::C_SafeStack<true> & stack, SharedPhysics::C_SmallSPUCache& spuCache)
	{
		using namespace Collision;
		PERF_EVENT("gravity.dynamic_raytrace");
		if(!m_pRootNode)
			return false;

		bool bHit = false;
		C_DistributionNode* node(NULL);//=m_pRootNode;

		S32 size = 1;
		C_DistributionNode* pStackTop(m_pRootNode);
		stack.Push(m_pRootNode, size );

		Maths::HF32 tmin(Maths::UNDEFINED);
		const Maths::HVec3 inflation(ray.m_Radius);
		const Maths::HVec3 rayStart(ray.m_start);

		while(size--)
		{
			Spu::MemBaseMarker marker;
			C_DistributionNode* node = pStackTop;
			C_DistributionNode* loadedNode = (C_DistributionNode*)Spu::FetchAlloc(node, sizeof(C_DistributionNode));
			//if(size > 0)
			pStackTop = stack.Pop<C_DistributionNode>(Maths::Max(0, size-1));
			C_AxisBoxMinMax box=loadedNode->m_bbox;
			if(Collision::IntersectRayAABBMinMax(rayStart, ray.m_result.m_contact, box.m_min-inflation, box.m_max+inflation))
			{
				//If down here, we are at a leaf node, so we need to do the ray tracing in here
				//Check whether the ray intersect with any static objects in this leaf node. 
				bHit = TraceList(loadedNode->m_pStaticObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pKinematicObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pActiveObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pSleepingObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pActiveClothObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pSleepingClothObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pActiveFluidObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pSleepingFluidObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pFluidMeshObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pRaytraceOnlyObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pActiveDebrisObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pSleepingDebrisObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;

				if(loadedNode->m_pLeftNode)
				{
					stack.Push( loadedNode->m_pRightNode,  size);
					//stack.Push( loadedNode->m_pLeftNode, size++ );
					size += 2;
					pStackTop = loadedNode->m_pLeftNode;
				}
			}
		}
		return bHit;
	}

	
	bool C_DistributionTree::RayTraceWater(Collision::C_Ray &ray, Collision::C_CollisionManager& manager, SharedPhysics::C_SafeStack<true> & stack, SharedPhysics::C_SmallSPUCache& spuCache)
	{
		using namespace Collision;
		PERF_EVENT("gravity.dynamic_raytrace");
		if(!m_pRootNode)
			return false;
		bool bHit = false;
		C_DistributionNode* node(NULL);//=m_pRootNode;

		S32 size = 1;
		C_DistributionNode* pStackTop(m_pRootNode);
		stack.Push(m_pRootNode, size );

		Maths::HF32 tmin(Maths::UNDEFINED);
		const Maths::HVec3 rayStart(ray.m_start);

		while(size--)
		{
			Spu::MemBaseMarker marker;
			C_DistributionNode* node = pStackTop;
			C_DistributionNode* loadedNode = (C_DistributionNode*)Spu::FetchAlloc(node, sizeof(C_DistributionNode));
			//if(size > 0)
			pStackTop = stack.Pop<C_DistributionNode>(Maths::Max(0, size-1));
			C_AxisBoxMinMax box=loadedNode->m_bbox;
			if(Collision::IntersectRayAABBMinMax(rayStart, ray.m_result.m_contact, box.m_min, box.m_max, tmin))
			{
				//If down here, we are at a leaf node, so we need to do the ray tracing in here
				//Check whether the ray intersect with any static objects in this leaf node. 
				bHit = TraceListWater(loadedNode->m_pActiveFluidObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = TraceListWater(loadedNode->m_pSleepingFluidObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = TraceListWater(loadedNode->m_pFluidMeshObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;

				if(loadedNode->m_pLeftNode)
				{
					stack.Push( loadedNode->m_pRightNode, size );
					//stack.Push( loadedNode->m_pLeftNode, size++ );
					size += 2;
					pStackTop = loadedNode->m_pLeftNode;
				}
			}
		}
		return bHit;
	}


	bool C_DistributionTree::RayTracing(Collision::C_TransparentRay &ray, Collision::C_CollisionFilter* pFilter, Collision::C_CollisionManager& manager, SharedPhysics::C_SafeStack<true> & stack, SharedPhysics::C_SmallSPUCache& spuCache)
	{
		Spu::MemBaseMarker outerMarker;
		using namespace Collision;
		PERF_EVENT("gravity.dynamic_raytrace");
		if(!m_pRootNode)
			return false;
		bool bHit = false;
		C_DistributionNode* node(NULL);//=m_pRootNode;

		S32 size = 1;
		C_DistributionNode* pStackTop(m_pRootNode);
		stack.Push(m_pRootNode, size );

		Maths::HF32 tmin(Maths::UNDEFINED);
		const Maths::HVec3 rayStart(ray.m_start);
		const Maths::HVec3 rayEnd(ray.m_end);

		while(size--)
		{
			Spu::MemBaseMarker marker;
			C_DistributionNode* node = pStackTop;
			C_DistributionNode* loadedNode = (C_DistributionNode*)Spu::FetchAlloc(node, sizeof(C_DistributionNode));
			//if(size > 0)
			pStackTop = stack.Pop<C_DistributionNode>(Maths::Max(0, size-1));
			C_AxisBoxMinMax box=loadedNode->m_bbox;
			if(Collision::IntersectRayAABBMinMax(rayStart, rayEnd, box.m_min, box.m_max, tmin))
			{
				//If down here, we are at a leaf node, so we need to do the ray tracing in here
				bHit = TraceList(loadedNode->m_pStaticObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pKinematicObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pActiveObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pSleepingObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pActiveClothObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pSleepingClothObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pActiveFluidObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pSleepingFluidObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pFluidMeshObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pRaytraceOnlyObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pActiveDebrisObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = TraceList(loadedNode->m_pSleepingDebrisObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				

				if(loadedNode->m_pLeftNode)
				{
					stack.Push( loadedNode->m_pRightNode, size );
					//stack.Push( loadedNode->m_pLeftNode, size++ );
					size += 2;
					pStackTop = loadedNode->m_pLeftNode;
				}
			}
		}
		return bHit;
	}


	bool C_DistributionTree::FatRayTracing(Collision::C_Ray &ray, Collision::C_CollisionManager& manager, SharedPhysics::C_SafeStack<true> & stack, SharedPhysics::C_SmallSPUCache& spuCache)
	{
		using namespace Collision;
		PERF_EVENT("gravity.dynamic_fatraytrace");
		if(!m_pRootNode)
			return false;

		bool bHit = false;
		C_DistributionNode* node(NULL);//=m_pRootNode;

		S32 size = 1;
		C_DistributionNode* pStackTop(m_pRootNode);
		stack.Push(m_pRootNode, size );

		Maths::HF32 tmin(Maths::UNDEFINED);
		const Maths::HVec3 inflation(ray.m_Radius);
		const Maths::HVec3 rayStart(ray.m_start);

		while(size--)
		{
			Spu::MemBaseMarker marker;
			C_DistributionNode* node = pStackTop;
			C_DistributionNode* loadedNode = (C_DistributionNode*)Spu::FetchAlloc(node, sizeof(C_DistributionNode));
			//if(size > 0)
			pStackTop = stack.Pop<C_DistributionNode>(Maths::Max(0, size-1));
			C_AxisBoxMinMax box=loadedNode->m_bbox;
			if(Collision::IntersectRayAABBMinMax(rayStart, ray.m_result.m_contact, box.m_min-inflation, box.m_max+inflation))
			{
				//If down here, we are at a leaf node, so we need to do the ray tracing in here
				//Check whether the ray intersect with any static objects in this leaf node. 
				bHit = FatRayTraceList(loadedNode->m_pStaticObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pKinematicObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pActiveObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pSleepingObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pActiveClothObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pSleepingClothObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pActiveFluidObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pSleepingFluidObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pFluidMeshObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pRaytraceOnlyObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pActiveDebrisObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pSleepingDebrisObjects, ray, tmin, stack, size, manager, spuCache, box) | bHit;

				if(loadedNode->m_pLeftNode)
				{
					stack.Push( loadedNode->m_pRightNode,  size);
					//stack.Push( loadedNode->m_pLeftNode, size++ );
					size += 2;
					pStackTop = loadedNode->m_pLeftNode;
				}
			}
		}
		return bHit;
	}


	bool C_DistributionTree::FatRayTracing(Collision::C_TransparentRay &ray, Collision::C_CollisionFilter* pFilter, Collision::C_CollisionManager& manager, SharedPhysics::C_SafeStack<true> & stack, SharedPhysics::C_SmallSPUCache& spuCache)
	{
		Spu::MemBaseMarker outerMarker;
		using namespace Collision;
		PERF_EVENT("gravity.dynamic_raytrace");
		if(!m_pRootNode)
			return false;
		bool bHit = false;
		C_DistributionNode* node(NULL);//=m_pRootNode;

		S32 size = 1;
		C_DistributionNode* pStackTop(m_pRootNode);
		stack.Push(m_pRootNode, size );

		Maths::HF32 tmin(Maths::UNDEFINED);
		const Maths::HVec3 rayStart(ray.m_start);
		const Maths::HVec3 rayEnd(ray.m_end);

		while(size--)
		{
			Spu::MemBaseMarker marker;
			C_DistributionNode* node = pStackTop;
			C_DistributionNode* loadedNode = (C_DistributionNode*)Spu::FetchAlloc(node, sizeof(C_DistributionNode));
			//if(size > 0)
			pStackTop = stack.Pop<C_DistributionNode>(Maths::Max(0, size-1));
			C_AxisBoxMinMax box=loadedNode->m_bbox;
			if(Collision::IntersectRayAABBMinMax(rayStart, rayEnd, box.m_min, box.m_max, tmin))
			{
				//If down here, we are at a leaf node, so we need to do the ray tracing in here
				bHit = FatRayTraceList(loadedNode->m_pStaticObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pKinematicObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pActiveObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pSleepingObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pActiveClothObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pSleepingClothObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pActiveFluidObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pSleepingFluidObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pFluidMeshObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pRaytraceOnlyObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pActiveDebrisObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				bHit = FatRayTraceList(loadedNode->m_pSleepingDebrisObjects, ray, tmin, stack, size, manager, pFilter, spuCache, box) | bHit;
				

				if(loadedNode->m_pLeftNode)
				{
					stack.Push( loadedNode->m_pRightNode, size );
					//stack.Push( loadedNode->m_pLeftNode, size++ );
					size += 2;
					pStackTop = loadedNode->m_pLeftNode;
				}
			}
		}
		return bHit;
	}


}


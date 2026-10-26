#include "StdAfx.h"

#include "gravity/narrowphase/ResultSize.h"
#include "gravity/narrowphase/InstanceMeshCollision.h"
#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/OBBTriangle.h"
#include "gravity/narrowphase/primitive.inl"
#include "gravity/narrowphase/TriangleTriangle.inl"
#include "gravity/narrowphase/ConvexTriangle.h"
#include "gravity/narrowphase/SphereTriangle.h"
#include "gravity/narrowphase/CylinderTriangle.h"
#include "gravity/narrowphase/CapsuleTriangle.h"
#include "gravity/narrowphase/ConeTriangle.h"
#include "gravity/narrowphase/OptimisedAABBTree.inl"
#include "gravity/narrowphase/CharacterControllerTriangle.inl"
#include "gravity/narrowphase/CollisionResult.inl"
#include "gravity/narrowphase/CollisionManager.inl"
#include "gravity/narrowphase/CollisionFilter.inl"
#include "gravity/narrowphase/AxisBox.inl"


namespace Collision
{

	void InstanceMeshBox(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB,  C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack)
	{

		C_InstancedTriangleMeshShape* pTree=(C_InstancedTriangleMeshShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_InstancedTriangleMeshShape));
		
		C_BoxShape* pBox=(C_BoxShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_BoxShape));

		C_BoxProxy proxy(pBox->m_Box, pTree->m_AABBTree.m_PosRot);
#ifdef PLATFORM_SPU
		if(pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_BoxProxy>(pObjB, proxy, results, stack, CollideOBBTriangleMesh<_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle>))
		{
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
#else
		if(pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_BoxProxy, CollideOBBTriangleMesh<_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle> >(pObjB, proxy, results, stack))
		{
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
#endif
	}

	void InstanceMeshCapsule(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB,  C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack)
	{
		C_InstancedTriangleMeshShape* pTree=(C_InstancedTriangleMeshShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_InstancedTriangleMeshShape));

		C_CapsuleShape* pCapsule=(C_CapsuleShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_CapsuleShape));

		C_CapsuleProxy proxy(pCapsule->m_Capsule, pTree->m_AABBTree.m_PosRot);
#ifdef PLATFORM_SPU
		if(pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_CapsuleProxy>(pObjB, proxy, results, stack, CollideCapsuleTriangleMesh<_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle>))
		{
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
#else
		if(pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_CapsuleProxy, CollideCapsuleTriangleMesh<_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle> >(pObjB, proxy, results, stack))
		{
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
#endif
	}

	void InstanceMeshSphere(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB,  C_CollisionResult<_MaxPatches, _MaxContacts>& results,SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack)
	{
		C_InstancedTriangleMeshShape* pTree=(C_InstancedTriangleMeshShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_InstancedTriangleMeshShape));
		
		C_SphereShape* pSphere=(C_SphereShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_SphereShape));

		C_SphereProxy proxy(pSphere->m_Sphere, pTree->m_AABBTree.m_PosRot);

#ifdef PLATFORM_SPU
		if(pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_SphereProxy>(pObjB, proxy, results, stack, CollideSphereTriangleMesh<_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle>))
		{
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
#else
		if(pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_SphereProxy, CollideSphereTriangleMesh<_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle> >(pObjB, proxy, results, stack))
		{
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
#endif
	}

	void InstanceMeshCylinder(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB,  C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack)
	{
		C_InstancedTriangleMeshShape* pTree = (C_InstancedTriangleMeshShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_InstancedTriangleMeshShape));

		C_CylinderShape* pCylinder=(C_CylinderShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_CylinderShape));

		C_CylinderProxy proxy(pCylinder->m_Cylinder, pTree->m_AABBTree.m_PosRot);
#ifdef PLATFORM_SPU
		if(pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_CylinderProxy>(pObjB, proxy, results, stack, CollideCylinderTriangleMesh<_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle>))
		{
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
#else
		if(pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_CylinderProxy, CollideCylinderTriangleMesh<_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle> >(pObjB, proxy, results, stack))
		{
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
#endif
	}

	void InstanceMeshCone(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB,  C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack)
	{
		C_InstancedTriangleMeshShape* pTree = (C_InstancedTriangleMeshShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_InstancedTriangleMeshShape));

		C_ConeShape* pCone=(C_ConeShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_ConeShape));

		C_ConeProxy proxy(pCone->m_Cone, pTree->m_AABBTree.m_PosRot);
#ifdef PLATFORM_SPU
		if(pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_ConeProxy>(pObjB, proxy, results, stack, CollideConeTriangleMesh<_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle>))
		{
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
#else
		if(pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_ConeProxy, CollideConeTriangleMesh<_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle> >(pObjB, proxy, results, stack))
		{
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
#endif
	}

	void InstanceMeshConvexHull(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB,  C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack)
	{
		C_InstancedTriangleMeshShape* pTree=(C_InstancedTriangleMeshShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_InstancedTriangleMeshShape));

		C_ConvexHullShape* pConvexHull=(C_ConvexHullShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_ConvexHullShape));

		C_ConvexHullProxy proxy(pConvexHull->m_Hull, pTree->m_AABBTree.m_PosRot);
#ifdef PLATFORM_SPU
		if(pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_ConvexHullProxy>(pObjB, proxy, results, stack, CollideConvexHullTriangleMesh<_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle>))
		{
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
#else
		if(pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_ConvexHullProxy, CollideConvexHullTriangleMesh<_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle> >(pObjB, proxy, results, stack))
		{
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
#endif

	}


	void InstanceMeshCharacterController(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB,  C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack)
	{
		C_InstancedTriangleMeshShape* pTree=(C_InstancedTriangleMeshShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_InstancedTriangleMeshShape));

		C_CharacterControllerShape* pCharacterController=(C_CharacterControllerShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_CharacterControllerShape));

		C_CharacterControllerProxy proxy(pCharacterController->m_Controller, pTree->m_AABBTree.m_PosRot);
		
		bool collided=false;

#ifdef PLATFORM_SPU
		collided=pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_CharacterControllerProxy>(pObjB, proxy, results, stack,  CollideControllerTriangleMesh<_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle>);
#else
		collided=pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_CharacterControllerProxy,  CollideControllerTriangleMesh<_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle> >(pObjB, proxy, results, stack);
#endif

		if(collided)
		{
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
		//if(collided)
		//{
		//	Collision::C_RayTracingResult& traceResult = pCharacterController->m_Controller.m_Ray.m_result;
		//	if(traceResult.m_pShape == pObjA->m_pShape)
		//	{
		//		pCharacterController->m_Controller.m_bContactWrittenToStream = true;
		//		//Set shape/node...
		//		//traceResult.m_pShape = a->m_pShape;
		//		traceResult.m_pNode = pObjA->m_pNodePtr;

		//		//Loop through collision result and add in contact for pill!
		//		Collision::C_ContactPatch<_MaxContacts>* pPatch = NULL;
		//		U32 incr = 0;

		//		Maths::C_Normal norm(-traceResult.m_TriangleNormal);

		//		Maths::HF32 pen(((pCharacterController->m_Controller.m_RayLength) - traceResult.m_distance));
		//		//if(traceResult.m_distance > pCharacterShape->m_Controller.m_RayLength)
		//		if(pen < pCharacterController->m_Controller.m_DeadZone)
		//		{
		//			norm = traceResult.m_TriangleNormal;
		//			pen = pCharacterController->m_Controller.m_MinPenetration+pen;
		//		}
		//		
		//		if(results.m_NumContactPatches < _MaxPatches)
		//		{
		//			pPatch = &results.m_ContactPatches[results.m_NumContactPatches];
		//			pPatch->m_NumContactPoints = 0;
		//			pPatch->m_Normal = norm;
		//			incr = 1;
		//		}
		//		pen = Maths::Max(Maths::Constants<Maths::HF32>::Zero(), pen);

		//		for(U32 i =0; i < results.m_NumContactPatches; ++i)
		//		{
		//			Collision::C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[i]);
		//			if((patch.m_Normal | norm) >= SharedPhysics::Constants::CloseNormal())
		//			{
		//				pPatch = &patch;
		//				incr = 0;
		//				break;
		//			}
		//		}
		//		if(pPatch == NULL)
		//		{
		//			pPatch = &results.m_ContactPatches[_MaxPatches - 1];
		//			pPatch->m_NumContactPoints = 0;
		//			pPatch->m_Normal = norm;
		//			incr = 0;
		//		}

		//		results.m_NumContactPatches += incr;
		//		pPatch->m_NumContactPoints = Maths::Min(pPatch->m_NumContactPoints +1, _MaxContacts);
		//		Collision::C_ContactPoint& point(pPatch->m_ContactPoints[pPatch->m_NumContactPoints - 1]);
		//		point.m_ContactPoint = traceResult.m_contact;
		//		point.m_PenetrationDepth = pen;
		//		point.m_PhysicsMaterialIndices = pObjB->m_pShape->m_PhysicsMaterialIndex << 16 | pCharacterController->m_Controller.m_FeetMaterial;
		//		point.m_EffectMaterialIndices = pObjB->m_pShape->m_EffectMaterialIndex << 16 | pCharacterController->m_Controller.m_EffectMaterial;
		//		point.m_HashValue = traceResult.m_TriangleIndex + manager.m_HashValueRandomisation++;
		//	}
		//	manager.StoreContacts<_MaxPatches, _MaxContacts, 3, 4>(pObjA, pObjB, stream, results);
		//}
	}
};

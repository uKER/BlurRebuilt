#include "StdAfx.h"

#include <core/maths/hvec/VectorInt.h>
#include <core/maths/hvec/VectorInt.inl>
//#include <core/maths/hvec/VectorOp.inl>
//#include <core/maths/hvec/storage.inl>
#include "gravity/narrowphase/GJK.h"
#include "gravity/narrowphase/GJK.inl"
#include "gravity/narrowphase/OptimisedAABBTree.inl"
#include "gravity/narrowphase/CollisionResult.inl"
#include "gravity/sharedphysics/ContactStream.inl"
#include "gravity/sharedphysics/SortedArray.inl"
//#include "gravity/narrowphase/PrimitiveTest.inl"
#include "gravity/narrowphase/CollisionManager.inl"
#include "gravity/narrowphase/ConvexHull.h"
#include "gravity/narrowphase/TriangleTriangle.inl"
//#include "gravity/narrowphase/ConvexTriangle.h"
//#include "gravity/narrowphase/ConvexTriangle.inl"
//#include "gravity/narrowphase/CompositeShapeCollision.inl"

#include "gravity/narrowphase/CylinderTriangle.h"
#include "gravity/narrowphase/CharacterControllerPrimitive.h"
#include "gravity/narrowphase/CharacterControllerTriangle.inl"
#include "gravity/narrowphase/Primitive.inl"
#include "gravity/narrowphase/CompositeShapeTriangle.h"
#include "gravity/narrowphase/CollisionFilter.inl"
#include "gravity/narrowphase/AxisBox.inl"
#include "system/perf.h"
#include "gravity/sharedphysics/SPU_Memory.h"


#ifndef PLATFORM_SPU
Debug::Logger &g_PhysCMLog( Debug::GetLogger("gravity.CollideMesh.Valide") );
#endif
//Comment to force re-build of this file


namespace Collision
{

//#pragma optimize("", off)

	void C_CollisionManager::CollideMesh( C_CollisionObject* a, C_CollisionObject* b, SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>& results, 
									 EPA& m_Epa)
{
	Spu::MemBaseMarker marker; 
	PERF_EVENT("gravity.meshCollision");
	//C_Shape* bShape = (C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(C_Shape));
	//E_ShapeType b_Shape=bShape->GetShape();
	//b->m_pSpuShape = bShape;


	
	U32 b_Shape=Spu::Get((U32*)&b->m_pShape->m_ShapeType);

	results.m_pObjectA = b;
	results.m_pObjectB = a;
	results.m_NumContactPatches = 0;

	results.m_pEpa = &m_Epa;

	const Maths::HF32 tolerance(Maths::Constants<Maths::HF32>::Epsilon()); 

	//NB... The collision mesh (a) will always be static!!!

	//S32 index;
	//SharedPhysics::C_Collider* pExistingCollider(m_ContactDatabase.FindPrevCollider(b->m_pNodePtr, a->m_pNodePtr, index));

	/*if(bContactRecycling && pExistingCollider && pExistingCollider->StillValid(tolerance))
	{
		C_Collider* pCollider = m_ContactDatabase.GetFreeCollider();
		pCollider->SetWitnessPoints(pExistingCollider->m_WitnessPointA, pExistingCollider->m_WitnessPointB);
		pCollider->SetNodes(b->m_pNodePtr, a->m_pNodePtr);
		SharedPhysics::C_ConstContactStreamPtr ptr(stream);
		EncodeContacts(pExistingCollider, stream);
		pCollider->m_StreamPtr = ptr;
		m_ContactDatabase.AddContact(pCollider);
		pCollider->m_pConstraintEdge = pExistingCollider->m_pConstraintEdge;
		pExistingCollider->m_pConstraintEdge = NULL;
		pExistingCollider->m_State = C_Collider::E_Recycled;
		return pCollider;
	}*/

	C_TriangleMeshShape* triangleMesh=(C_TriangleMeshShape*)Spu::FetchAlloc(a->m_pShape, sizeof(C_TriangleMeshShape));
	OptimisedAABBTree& tree=triangleMesh->m_AABBTree;
#ifndef PLATFORM_PC
	BIZ_ASSERT((((U32)tree.m_pNodes) & (SharedPhysics::SmallCachePageSize - 1)) == 0);
#endif
	SharedPhysics::C_SmallSPUCache spuCache;

	
	if(b_Shape == E_Continuous_Shape)
	{
		b->m_pShape = (Collision::C_Shape*)Spu::Get((INTPTR*)&((Collision::C_ContinuousShape*)b->m_pShape)->m_pDiscreteShape);	// [Blur2PC][x64] pointer-wide
		b_Shape = Spu::Get((U32*)&b->m_pShape->m_ShapeType);
	}

	switch(b_Shape)
	{
	case E_Box_Shape://0x8
		{
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_BoxShape));
#ifndef PLATFORM_SPU
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts, CollideOBBTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle> >(b, results, stack, spuCache))
#else
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts >(b, results, stack, CollideOBBTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle>, spuCache))
#endif
			{
				StoreContacts<Static_MaxPatches, Static_MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
			}
		
			return;
		}
	case E_ConvexHull_Shape://0x10
		{
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_ConvexHullShape));
		//PERF_EVENT("gravity.collision.convexhull");
#ifndef PLATFORM_SPU
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts, CollideConvexHullTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle> >(b, results, stack, spuCache))
#else
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts >(b, results, stack, CollideConvexHullTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle>, spuCache))
#endif
			{
				StoreContacts<Static_MaxPatches, Static_MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
			}
			return;
		}
	case E_Sphere_Shape://0x20
		{
			
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_SphereShape));
#ifndef PLATFORM_SPU
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts, CollideSphereTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle> >(b, results, stack, spuCache))
#else
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts >(b, results, stack, CollideSphereTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle>, spuCache))
#endif
			{
				StoreContacts<Static_MaxPatches, Static_MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
			}
			return;
		}
	case E_Capsule_Shape://0x40
		{
		
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_CapsuleShape));
#ifndef PLATFORM_SPU
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts, CollideCapsuleTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle> >(b, results, stack, spuCache))
#else
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts >(b, results, stack, CollideCapsuleTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle>, spuCache))
#endif
			{
				StoreContacts<Static_MaxPatches, Static_MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
			}
			return;
		}
	case E_Cylinder_Shape://0x80
		{
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_CylinderShape));
#ifndef PLATFORM_SPU
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts, CollideCylinderTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle> >(b, results, stack, spuCache))
#else
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts>(b, results, stack, CollideCylinderTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle>, spuCache))
#endif
			{
				StoreContacts<Static_MaxPatches, Static_MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
			}
			return;
		}
	case E_Composite_Shape://0x100
		{
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_CompositeShape));
		//Fetch entire composite shape hierarchy
#ifndef PLATFORM_SPU
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts, CollideCompositeShapeTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle> >(b, results, stack, spuCache))
#else
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts >(b, results, stack, CollideCompositeShapeTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle>, spuCache))
#endif
			{
				//StoreContacts<Static_MaxPatches, Static_MaxContacts, 3, 6>(b, a, stream, results);
				EncodeContacts(results, stream, results.m_pCurrentCollider);
			}
			return;
		}
	case E_CharacterController_Shape://0x200
		{
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_CharacterControllerShape));
			Collision::C_CharacterControllerShape* pCharacterShape = (Collision::C_CharacterControllerShape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_CharacterControllerShape));
		//pCharacterShape->m_Controller.m_Ray.m_result.ClearContacts();
#ifndef PLATFORM_SPU
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts, CollideControllerTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle> >(b, results, stack, spuCache))
#else
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts>(b, results, stack, CollideControllerTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle>, spuCache))
#endif
			{
				StoreContacts<Static_MaxPatches, Static_MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
			}
			return;
		}
	case E_Cloth_Shape://0x2000
		{
			return;
		}
	case E_Cone_Shape://0x10000
		{
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_ConeShape));		
#ifndef PLATFORM_SPU
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts, CollideConeTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle> >(b, results, stack, spuCache))
#else
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts >(b, results, stack, CollideConeTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle>, spuCache))
#endif
			{
				StoreContacts<Static_MaxPatches, Static_MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
			}
			return;
		}
	}

#ifndef PLATFORM_SPU
	//assert(0 && "Undefined collision pair!");
	//LOGWARN(g_PhysCMLog, " Undefined collision pair!");
	/*Collision::C_CRAxisBox box = b->m_TransformedBox;
	Maths::HMat33 rot;
	rot.SetIdentity();
	DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dBox(-box.extents,box.extents, Maths::HMat43(rot, box.centre), Maths::RGBA(0, 0, 255));*/

#else
	LOGWARN(0, " Undefined collision pair!");
#endif
	return;
}


void C_CollisionManager::CollideMeshSpuStream( C_CollisionObject* a, C_CollisionObject* b, SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>& results, 
									 EPA& m_Epa)
{
	Spu::MemBaseMarker marker; 
	PERF_EVENT("gravity.meshCollision");
	//C_Shape* bShape = (C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(C_Shape));
	//E_ShapeType b_Shape=bShape->GetShape();
	//b->m_pSpuShape = bShape;


	
	U32 b_Shape=Spu::Get((U32*)&b->m_pShape->m_ShapeType);

	results.m_pObjectA = b;
	results.m_pObjectB = a;
	results.m_NumContactPatches = 0;

	results.m_pEpa = &m_Epa;

	const Maths::HF32 tolerance(Maths::Constants<Maths::HF32>::Epsilon()); 

	//NB... The collision mesh (a) will always be static!!!

	//S32 index;
	//SharedPhysics::C_Collider* pExistingCollider(m_ContactDatabase.FindPrevCollider(b->m_pNodePtr, a->m_pNodePtr, index));

	/*if(bContactRecycling && pExistingCollider && pExistingCollider->StillValid(tolerance))
	{
		C_Collider* pCollider = m_ContactDatabase.GetFreeCollider();
		pCollider->SetWitnessPoints(pExistingCollider->m_WitnessPointA, pExistingCollider->m_WitnessPointB);
		pCollider->SetNodes(b->m_pNodePtr, a->m_pNodePtr);
		SharedPhysics::C_ConstContactStreamPtr ptr(stream);
		EncodeContacts(pExistingCollider, stream);
		pCollider->m_StreamPtr = ptr;
		m_ContactDatabase.AddContact(pCollider);
		pCollider->m_pConstraintEdge = pExistingCollider->m_pConstraintEdge;
		pExistingCollider->m_pConstraintEdge = NULL;
		pExistingCollider->m_State = C_Collider::E_Recycled;
		return pCollider;
	}*/

	C_TriangleMeshShape* triangleMesh=(C_TriangleMeshShape*)Spu::FetchAlloc(a->m_pShape, sizeof(C_TriangleMeshShape));
	BIZ_ASSERT(triangleMesh->m_ShapeType == Collision::E_TriangleMesh_Shape);
	OptimisedAABBTree& tree=triangleMesh->m_AABBTree;
#ifndef PLATFORM_PC
	BIZ_ASSERT((((U32)tree.m_pNodes) & (SharedPhysics::SmallCachePageSize - 1)) == 0);
#endif
	SharedPhysics::C_SmallSPUCache spuCache;

	
	/*if(b_Shape == E_Continuous_Shape)
	{
		b->m_pShape = (Collision::C_Shape*)Spu::Get((U32*)&((Collision::C_ContinuousShape*)b->m_pShape)->m_pDiscreteShape);
		b_Shape = Spu::Get((U32*)&b->m_pShape->m_ShapeType);
	}*/

	switch(b_Shape)
	{
	case E_Box_Shape://0x8
		{
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_BoxShape));
#ifndef PLATFORM_SPU
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts, CollideOBBTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle> >(b, results, stack, spuCache))
#else
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts >(b, results, stack, CollideOBBTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle>, spuCache))
#endif
			{
				StoreContactsSpu<Static_MaxPatches, Static_MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
			}
		
			return;
		}
	/*case E_ConvexHull_Shape://0x10
		{
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_ConvexHullShape));
		//PERF_EVENT("gravity.collision.convexhull");
#ifndef PLATFORM_SPU
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts, CollideConvexHullTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle> >(b, results, stack, spuCache))
#else
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts >(b, results, stack, CollideConvexHullTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle>, spuCache))
#endif
			{
				StoreContactsSpu<Static_MaxPatches, Static_MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
			}
			return;
		}
	case E_Sphere_Shape://0x20
		{
			
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_SphereShape));
#ifndef PLATFORM_SPU
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts, CollideSphereTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle> >(b, results, stack, spuCache))
#else
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts >(b, results, stack, CollideSphereTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle>, spuCache))
#endif
			{
				StoreContactsSpu<Static_MaxPatches, Static_MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
			}
			return;
		}
	case E_Capsule_Shape://0x40
		{
		
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_CapsuleShape));
#ifndef PLATFORM_SPU
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts, CollideCapsuleTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle> >(b, results, stack, spuCache))
#else
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts >(b, results, stack, CollideCapsuleTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle>, spuCache))
#endif
			{
				StoreContactsSpu<Static_MaxPatches, Static_MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
			}
			return;
		}
	case E_Cylinder_Shape://0x80
		{
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_CylinderShape));
#ifndef PLATFORM_SPU
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts, CollideCylinderTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle> >(b, results, stack, spuCache))
#else
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts>(b, results, stack, CollideCylinderTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle>, spuCache))
#endif
			{
				StoreContactsSpu<Static_MaxPatches, Static_MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
			}
			return;
		}
	case E_Composite_Shape://0x100
		{
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_CompositeShape));
		//Fetch entire composite shape hierarchy
#ifndef PLATFORM_SPU
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts, CollideCompositeShapeTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle> >(b, results, stack, spuCache))
#else
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts >(b, results, stack, CollideCompositeShapeTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle>, spuCache))
#endif
			{
				//StoreContacts<Static_MaxPatches, Static_MaxContacts, 3, 6>(b, a, stream, results);
				EncodeContactsSpu(results, stream, results.m_pCurrentCollider);
			}
			return;
		}
	case E_CharacterController_Shape://0x200
		{
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_CharacterControllerShape));
			Collision::C_CharacterControllerShape* pCharacterShape = (Collision::C_CharacterControllerShape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_CharacterControllerShape));
		//pCharacterShape->m_Controller.m_Ray.m_result.ClearContacts();
#ifndef PLATFORM_SPU
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts, CollideControllerTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle> >(b, results, stack, spuCache))
#else
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts>(b, results, stack, CollideControllerTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle>, spuCache))
#endif
			{
				StoreContactsSpu<Static_MaxPatches, Static_MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
			}
			return;
		}
	case E_Cloth_Shape://0x2000
		{
			return;
		}
	case E_Cone_Shape://0x10000
		{
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_ConeShape));		
#ifndef PLATFORM_SPU
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts, CollideConeTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle> >(b, results, stack, spuCache))
#else
			if(tree.Collide<Static_MaxPatches, Static_MaxContacts >(b, results, stack, CollideConeTriangleMesh<Static_MaxPatches, Static_MaxContacts, Collision::AABBTreeTriangle>, spuCache))
#endif
			{
				StoreContactsSpu<Static_MaxPatches, Static_MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
			}
			return;
		}*/
	}

	return;
}


#if! defined(PLATFORM_SPU) || defined (SPU_CONTINUOUS_COLLISION)
Maths::HF32 C_CollisionManager::ContinuousCollideMesh( C_CollisionObject* a, C_CollisionObject* b,  const Maths::HVec3 PCREF r, Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, SharedPhysics::C_SafeStack<true>& stack,
													  U32& physicsMaterial, U32& effectMaterial)
{
	Spu::MemBaseMarker marker; 
	PERF_EVENT("gravity.meshContinuousCollision");
	
	C_TriangleMeshShape* triangleMesh=(C_TriangleMeshShape*)Spu::FetchAlloc(a->m_pShape, sizeof(C_TriangleMeshShape));
	OptimisedAABBTree& tree=triangleMesh->m_AABBTree;

	Collision::C_ContinuousShape* pContinuousShape = (Collision::C_ContinuousShape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_ContinuousShape));

	U32 b_Shape = Spu::Get((U32*)&pContinuousShape->m_pContinuousPrimitive->m_ShapeType);
	Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
	Maths::HF32 lambda(Maths::Constants<Maths::HF32>::Zero());
	Maths::HF32 maxLambda(one);

	SharedPhysics::C_SmallSPUCache spuCache;

	switch(b_Shape)
	{
	case E_Box_Shape://0x8
		{
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_BoxShape));

			if(tree.Collide(b, stack, maxLambda, lambda, r,  nor, closestA, closestB, physicsMaterial, effectMaterial, ContinuousOBBTriangleMesh<Collision::AABBTreeTriangle>, spuCache))
			{
				return lambda;
			}
			return one;
		}
	case E_ConvexHull_Shape://0x10
		{
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_ConvexHullShape));

			if(tree.Collide(b, stack, maxLambda, lambda, r,  nor, closestA, closestB, physicsMaterial, effectMaterial, ContinuousConvexHullTriangleMesh<Collision::AABBTreeTriangle>, spuCache))
			{
				return lambda;
			}

			return one;
		}
	case E_Sphere_Shape://0x20
		{
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_SphereShape));

			if(tree.Collide(b, stack, maxLambda, lambda, r, nor, closestA, closestB, physicsMaterial, effectMaterial, ContinuousSphereTriangleMesh<Collision::AABBTreeTriangle>, spuCache))
			{
				return lambda;
			}
			return one;
		}
	case E_Capsule_Shape://0x40
		{
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_CapsuleShape));

			if(tree.Collide(b, stack, maxLambda, lambda, r,  nor, closestA, closestB, physicsMaterial, effectMaterial, ContinuousCapsuleTriangleMesh<Collision::AABBTreeTriangle>, spuCache))
			{
				return lambda;
			}
			return one;
		}
	case E_Cylinder_Shape://0x80
		{
			b->m_pSpuShape = (Collision::C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(Collision::C_CylinderShape));

			if(tree.Collide(b, stack, maxLambda, lambda, r,  nor, closestA, closestB, physicsMaterial, effectMaterial, ContinuousCylinderTriangleMesh<Collision::AABBTreeTriangle>, spuCache))
			{
				return lambda;
			}
			
			return one;
		}
	case E_Cone_Shape://0x200
		{
			return one;
		}
	default:
		return one;
	}
}
#endif

#ifndef PLATFORM_SPU
bool C_CollisionManager::ContinuousCheckOBBOBB(Collision::C_OBB& obbA, Collision::C_OBB& obbB,
											   const Maths::HVec3 PCREF r, Maths::HF32& lambda,Maths::HVec3& normal, Maths::HVec3& contactPoint)
{
	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const Maths::HF32 PCREF maxLambda(Maths::Constants<Maths::HF32>::One());
	Maths::HVec3 closestB(Maths::UNDEFINED);
	bool bResult = Collision::GJKRayCast<Collision::C_OBB, Collision::C_OBB>(obbA, obbB, r, maxLambda, lambda, normal, contactPoint, closestB);
	/*normal = Maths::Normalise(closestB - contactPoint);*/
	return bResult && lambda > zero;
}
#endif

//#pragma optimize("", on)


}


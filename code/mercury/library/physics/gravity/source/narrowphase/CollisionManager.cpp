#include "stdafx.h"
#include "gravity/narrowphase/CollisionManager.h"
#include "gravity/sharedphysics/contactstreamiterator.inl"
#include "gravity/sdk/Body.h"
#include <system/perf.h>
#include "gravity/narrowphase/shapes.h"
namespace Collision
{


Debug::Bool g_DetailedRejection("Gravity.DetailedRejection", false);

//pCollider already in SPU memory!!!
//void C_CollisionManager::UpdateCollider(SharedPhysics::C_Collider* pCollider)
//{
//	Spu::MemBaseMarker marker;
//	S32 index;
//	SharedPhysics::C_GraphNode* pNodeA((SharedPhysics::C_GraphNode*)Spu::FetchAlloc(pCollider->m_pNodeA, sizeof(SharedPhysics::C_GraphNode)));
//	SharedPhysics::C_GraphNode* pNodeB((SharedPhysics::C_GraphNode*)Spu::FetchAlloc(pCollider->m_pNodeB, sizeof(SharedPhysics::C_GraphNode)));
//
//	SharedPhysics::C_Collider* pExistingCollider = m_pContactDatabase->FindPrevCollider(pNodeA, pNodeB, index);
//	if(pExistingCollider)
//	{
//		SharedPhysics::C_Collider* pLocalExistingCollider((SharedPhysics::C_Collider*)Spu::FetchAlloc(pExistingCollider, sizeof(SharedPhysics::C_Collider)));
//		pExistingCollider->m_State = SharedPhysics::C_Collider::E_Removed;
//		pCollider->m_pConstraintEdge = pExistingCollider->m_pConstraintEdge;
//		pExistingCollider->m_pConstraintEdge = NULL;
//#ifdef PLATFORM_SPU
//		Spu::Store(pExistingCollider, pLocalExistingCollider, sizeof(SharedPhysics::C_Collider));
//#endif
//	}
//}

//
//void C_CollisionManager::UpdateCollider(SharedPhysics::C_Collider* pCollider)
//{
//	/*SharedPhysics::C_GraphNode* pNodeA(pCollider->m_pNodeA);
//	SharedPhysics::C_GraphNode* pNodeB(pCollider->m_pNodeB);*/
//	SharedPhysics::C_ConstraintBody* pBodyA = (SharedPhysics::C_ConstraintBody*)Spu::FetchAllocAsync(pCollider->m_pNodeA, sizeof(SharedPhysics::C_ConstraintBody), 10);
//	SharedPhysics::C_ConstraintBody* pBodyB = (SharedPhysics::C_ConstraintBody*)Spu::FetchAllocAsync(pCollider->m_pNodeB, sizeof(SharedPhysics::C_ConstraintBody), 10);
//	Spu::Wait(10);
//	SharedPhysics::C_ConstraintEdge* pExistingConstraint = pBodyA->m_GraphNode.FindPrevConstraint(&pBodyB->m_GraphNode);
//	
//	//SharedPhysics::C_Collider* pExistingCollider = m_pContactDatabase->FindPrevCollider(pNodeA, pNodeB, index);
//	//if(pExistingCollider)
//	if(pExistingConstraint)
//	{
//		SharedPhysics::C_ConstraintEdge* pSpuExistingConstraint = (SharedPhysics::C_ConstraintEdge*)Spu::FetchAlloc(pExistingConstraint, sizeof(SharedPhysics::C_ConstraintEdge));
//		
//		if(pSpuExistingConstraint->m_pCollider)
//		{
//			//pExistingCollider->m_State = SharedPhysics::C_Collider::E_Removed;
//			//pCollider->m_pConstraintEdge = pExistingCollider->m_pConstraintEdge;
//			//pExistingCollider->m_pConstraintEdge = NULL;
//			
//			pCollider->m_pConstraintEdge = pExistingConstraint;
//
//			//pExistingConstraint->m_pCollider->m_State = SharedPhysics::C_Collider::E_Removed;
//			//pExistingConstraint->m_pCollider->m_pConstraintEdge = NULL;
//
//			//Quick sets for SPU
//			Spu::Set(&pSpuExistingConstraint->m_pCollider->m_State, (U32)SharedPhysics::C_Collider::E_Removed);
//			Spu::Set((U32*)&pSpuExistingConstraint->m_pCollider->m_pConstraintEdge, NULL);
//
//#ifdef USE_PERSISTENT_MANIFOLD
//			Collision::C_PersistentContactManifold* pExistingManifold = (SharedPhysics::Collision::C_PersistentContactManifold*)Spu::FetchAlloc(&pSpuExistingConstraint->m_pCollider->m_PersistentManifold, sizeof(Collision::C_PersistentContactManifold));
//			pCollider->m_PersistentManifold = *pExistingManifold;
//			if(pCollider->m_pNodeA != pExistingConstraint->m_pCollider->m_pNodeA)
//			{
//				pCollider->m_PersistentManifold.FlipNormals();
//			}
//			/*pCollider->m_PersistentManifold.RefreshManifoldContacts(pNodeA->m_pBody->GetShape()->m_PosRot,
//					pNodeB->m_pBody->GetShape()->m_PosRot);*/
//
//		
//			const Maths::HMat43 matA(pBodyA->m_RigidBody.m_OrientationMatrix, pBodyA->m_RigidBody.m_Position);
//			const Maths::HMat43 matB(pBodyB->m_RigidBody.m_OrientationMatrix, pBodyB->m_RigidBody.m_Position);
//
//			pCollider->m_PersistentManifold.RefreshManifoldContacts(matA, matB);
//				 
//#endif
//		}
//	}
//}


void C_CollisionManager::PatchColliders(Gravity::C_BodyList& wakingList)
{
	PERF_EVENT("Gravity.CollisionManager.PatchColliders");
	const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
	const U32 numColliders  = m_pContactDatabase->m_pCurrentBuffer->GetNumberCollidingPairs();
	m_NumCollidingPairs = numColliders;
	m_NumMeshPairs = m_pContactDatabase->m_pCurrentBuffer->GetNumberMeshPairs();
	m_MeshIterator = m_pContactDatabase->m_pCurrentBuffer->GetMeshIterator();

	//m_pContactDatabase->m_pCurrentBuffer->Resize(numColliders);

	for(U32 a = 0; a < numColliders; ++a)
	{
		SharedPhysics::C_Collider* pCollider = m_pContactDatabase->m_pCurrentBuffer->GetCollider(a);
		{
			SharedPhysics::C_GraphNode* pNodeA(pCollider->m_pNodeA);
			SharedPhysics::C_GraphNode* pNodeB(pCollider->m_pNodeB);
			
			if(pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
			{
				if(!pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered))
				{
					wakingList.AddBody(pNodeB->m_pBody);
					pNodeB->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered);
					pNodeB->m_TotalKineticEnergy = zero;
					pNodeB->m_pRigidBody->m_KineticEnergy = zero;
				}
			}
			else if (pNodeA->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
			{
				if(!pNodeA->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered))
				{
					wakingList.AddBody(pNodeA->m_pBody);
					pNodeA->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered);
					pNodeA->m_TotalKineticEnergy = zero;
					pNodeA->m_pRigidBody->m_KineticEnergy = zero;
				}
			}

			BIZ_ASSERT((!pNodeA->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static)) || 
				pNodeA->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered));

			BIZ_ASSERT((!pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static)) || 
				pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered));
		}
	}

	for(U32 a = 0; a < m_NumMeshPairs; ++a)
	{
		SharedPhysics::C_Collider* pCollider = m_MeshIterator.Get();
		{

			SharedPhysics::C_GraphNode* pNodeA(pCollider->m_pNodeA);
			SharedPhysics::C_GraphNode* pNodeB(pCollider->m_pNodeB);

			if(pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
			{
				if(!pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered))
				{
					wakingList.AddBody(pNodeB->m_pBody);
					pNodeB->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered);
					pNodeB->m_TotalKineticEnergy = zero;
					pNodeB->m_pRigidBody->m_KineticEnergy = zero;
				}
			}
			else if (pNodeA->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
			{
				if(!pNodeA->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered))
				{
					wakingList.AddBody(pNodeA->m_pBody);
					pNodeA->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered);
					pNodeA->m_TotalKineticEnergy = zero;
					pNodeA->m_pRigidBody->m_KineticEnergy = zero;
				}
			}

			//KierS-  don't need this!!!

			////Find this using the constraints in the 
			//SharedPhysics::C_ConstraintEdge* pExistingConstraint = pNodeA->FindPrevConstraint(pNodeB);
			////SharedPhysics::C_Collider* pExistingCollider = m_pContactDatabase->FindPrevCollider(pNodeA, pNodeB, index);
			////if(pExistingCollider)
			//if(pExistingConstraint && pExistingConstraint->m_pCollider)
			//{
			//	//pExistingCollider->m_State = SharedPhysics::C_Collider::E_Removed;
			//	//pCollider->m_pConstraintEdge = pExistingCollider->m_pConstraintEdge;
			//	//pExistingCollider->m_pConstraintEdge = NULL;
			//	pCollider->m_pConstraintEdge = pExistingConstraint;
			//	pExistingConstraint->m_pCollider->m_State = SharedPhysics::C_Collider::E_Removed;
			//	pExistingConstraint->m_pCollider->m_pConstraintEdge = NULL;

			//}
		}
	}
}

void C_CollisionManager::PatchWakingColliders(Gravity::C_BodyList& interferedList)
{
	PERF_EVENT("Gravity.CollisionManager.PatchWakingColliders");
	const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
	//const U32 numColliders = m_pContactDatabase->m_pCurrentBuffer->NumColliders();
	const U32 numColliders  = m_pContactDatabase->m_pCurrentBuffer->GetNumberCollidingPairs();
	const U32 numMeshPairs = m_pContactDatabase->m_pCurrentBuffer->GetNumberMeshPairs();

	for(U32 a = m_NumCollidingPairs; a < numColliders; ++a)
	{
		SharedPhysics::C_Collider* pCollider = m_pContactDatabase->m_pCurrentBuffer->GetCollider(a);
		//SharedPhysics::C_Collider* pCollider = m_Iterator.Get();
		//if(!g_DetailedRejection || pCollider->m_StreamPtr.m_pCurrentReadPoint != NULL)
		{
			//m_pContactDatabase->m_pCurrentBuffer->AddToDatabase(pCollider);

			SharedPhysics::C_GraphNode* pNodeA(pCollider->m_pNodeA);
			SharedPhysics::C_GraphNode* pNodeB(pCollider->m_pNodeB);


			if(pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static) && 
				!pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered))
			{
				pNodeB->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered);
				interferedList.AddBody(pNodeB->m_pBody);
			}

	//		SharedPhysics::C_ConstraintEdge* pExistingConstraint = pNodeA->FindPrevConstraint(pNodeB);

	//		if(pExistingConstraint && pExistingConstraint->m_pCollider)
	//		{
	//			pCollider->m_pConstraintEdge = pExistingConstraint;
	//			pExistingConstraint->m_pCollider->m_State = SharedPhysics::C_Collider::E_Removed;
	//			pExistingConstraint->m_pCollider->m_pConstraintEdge = NULL;

	//#ifdef USE_PERSISTENT_MANIFOLD
	//			pCollider->m_PersistentManifold = pExistingConstraint->m_pCollider->m_PersistentManifold;
	//			/*pCollider->m_PersistentManifold.RefreshManifoldContacts(pNodeA->m_pBody->GetShape()->m_PosRot,
	//				pNodeB->m_pBody->GetShape()->m_PosRot);*/
	//#endif
	//		}


		}
	}

	//for(U32 a = m_NumMeshPairs; a < numMeshPairs; ++a)
	//{
	//	//SharedPhysics::C_Collider* pCollider = m_pContactDatabase->m_pCurrentBuffer->Collider(a);
	//	SharedPhysics::C_Collider* pCollider = m_MeshIterator.Get();
	//	//if(!g_DetailedRejection || pCollider->m_StreamPtr.m_pCurrentReadPoint != NULL)
	//	{
	//		//m_pContactDatabase->m_pCurrentBuffer->AddToDatabase(pCollider);

	//		SharedPhysics::C_GraphNode* pNodeA(pCollider->m_pNodeA);
	//		SharedPhysics::C_GraphNode* pNodeB(pCollider->m_pNodeB);

	//		/*if(pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static) && 
	//			!pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered))
	//		{
	//			pNodeB->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered);
	//			interferedList.AddBody(pNodeB->m_pBody);
	//		}*/


	////		SharedPhysics::C_ConstraintEdge* pExistingConstraint = pNodeA->FindPrevConstraint(pNodeB);

	////		if(pExistingConstraint && pExistingConstraint->m_pCollider)
	////		{
	////			pCollider->m_pConstraintEdge = pExistingConstraint;
	////			pExistingConstraint->m_pCollider->m_State = SharedPhysics::C_Collider::E_Removed;
	////			pExistingConstraint->m_pCollider->m_pConstraintEdge = NULL;

	////#ifdef USE_PERSISTENT_MANIFOLD
	////			pCollider->m_PersistentManifold = pExistingConstraint->m_pCollider->m_PersistentManifold;
	////		/*	pCollider->m_PersistentManifold.RefreshManifoldContacts(pNodeA->m_pBody->GetShape()->m_PosRot,
	////				pNodeB->m_pBody->GetShape()->m_PosRot);*/
	////#endif
	////		}


	//	}
	//}

}


void C_CollisionManager::CreateColliders(SharedPhysics::C_ConstContactStreamPtr& ptr, Solver2::C_ConstraintGraphManager& constraintManager)
{
	{
		//First bit - write the sentinel at the end of the stream to make sure we don't go past the end!
		SharedPhysics::C_ContactStreamResult* res = (SharedPhysics::C_ContactStreamResult*)ptr.m_pStream->m_pCurrentWritePoint;
		res->m_ValidData = SharedPhysics::E_NullData;
	}
	//S32 index;
	//SharedPhysics::C_ContactStreamIterator iter(&ptr, &constraintManager, Maths::Constants<Maths::HF32>::Zero());
	//while(iter.hasNextContact())
	//{
	//	SharedPhysics::C_ContactStreamResult* result = iter.nextContact();
	//	if(result->m_ValidData == SharedPhysics::E_ValidData)
	//	{
	//		//find old collision data!
	//		SharedPhysics::C_Collider* pExistingCollider(m_pContactDatabase->FindPrevCollider(result->m_pNodeA, result->m_pNodeB, index));
	//		SharedPhysics::C_Collider* pCollider = m_pContactDatabase->GetFreeCollider();
	//		pCollider->SetNodes(result->m_pNodeA, result->m_pNodeB);
	//		pCollider->m_pConstraintEdge = NULL;
	//		pCollider->m_State = SharedPhysics::C_Collider::E_Normal;
	//		if(pExistingCollider)
	//		{
	//			m_pContactDatabase->RemoveCollider(pExistingCollider);
	//			pCollider->m_pConstraintEdge = pExistingCollider->m_pConstraintEdge;
	//			pExistingCollider->m_pConstraintEdge = NULL;
	//		}
	//		pCollider->m_StreamPtr = SharedPhysics::C_ConstContactStreamPtr(ptr.m_pStream, (U8*)result);
	//		m_pContactDatabase->AddContact(pCollider);
	//	}
	//}
}

//SharedPhysics::C_Collider* C_CollisionManager::CreateColliderNoLock(SharedPhysics::C_GraphNode* a, SharedPhysics::C_GraphNode* b,
//															  SharedPhysics::C_Collider::E_ColliderType type)
//{
////	PERF_EVENT("CollisionManager.CreateCollider");
//	//bool bSwap = (a->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject)) || 
//	//	(a->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping) && b->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active));
//
//	bool bNoSwap = (a->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Waking)) 
//		|| (a->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping) && b->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject));
//
//	SharedPhysics::C_GraphNode* _a = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bNoSwap, a, b);
//	SharedPhysics::C_GraphNode* _b = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bNoSwap, b, a);
//	SharedPhysics::C_Collider* pCollider = m_pContactDatabase->GetFreeColliderNoLock();
//#ifdef USE_PERSISTENT_MANIFOLD
//	pCollider->m_PersistentManifold.Reset();
//#endif
//	pCollider->m_Type = type;
//
//	pCollider->SetNodes(_a, _b);
//	
//	pCollider->m_pConstraintEdge = NULL;
//	pCollider->m_State = SharedPhysics::C_Collider::E_Normal;
//
//	//Try to disable this - should hopefully speed things up!!!
//	//m_pContactDatabase->AddContact(pCollider);
//	return pCollider;
//}

SharedPhysics::C_Collider* C_CollisionManager::CreateCollider(SharedPhysics::C_GraphNode* a, SharedPhysics::C_GraphNode* b,
															  SharedPhysics::C_Collider::E_ColliderType type)
{
//	PERF_EVENT("CollisionManager.CreateCollider");
	//bool bSwap = (a->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject)) || 
	//	(a->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping) && b->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active));

	bool bNoSwap = (a->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Waking)) 
		|| (a->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping) && b->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject));

	SharedPhysics::C_GraphNode* _a = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bNoSwap, a, b);
	SharedPhysics::C_GraphNode* _b = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bNoSwap, b, a);
	SharedPhysics::C_PersistentContactCollider* pCollider = m_pContactDatabase->GetFreeCollider();
#ifdef USE_PERSISTENT_MANIFOLD
	pCollider->m_PersistentManifold.Reset();
#endif

	pCollider->m_Type = type;

	pCollider->SetNodes(_a, _b);
	
	pCollider->m_pConstraintEdge = NULL;
	pCollider->m_State = SharedPhysics::C_Collider::E_Normal;

	//Try to disable this - should hopefully speed things up!!!
	//m_pContactDatabase->AddContact(pCollider);
	return pCollider;
}

SharedPhysics::C_Collider* C_CollisionManager::CreateColliderMesh(SharedPhysics::C_GraphNode* a, SharedPhysics::C_GraphNode* b,
															  SharedPhysics::C_Collider::E_ColliderType type)
{
	/*bool bNoSwap = (a->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Waking)) 
		|| (a->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping) && b->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject));*/


	/*SharedPhysics::C_GraphNode* _a = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bNoSwap, a, b);
	SharedPhysics::C_GraphNode* _b = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bNoSwap, b, a);*/
	SharedPhysics::C_Collider* pCollider = m_pContactDatabase->GetFreeMeshCollider();
//#ifdef USE_PERSISTENT_MANIFOLD
//	pCollider->m_PersistentManifold.Reset();
//#endif

	//BIZ_ASSERT(b->m_pBody->GetShape()->GetShape() == Collision::E_TriangleMesh_Shape);

	pCollider->m_Type = type;

	pCollider->SetNodes(a, b);
	
	pCollider->m_pConstraintEdge = NULL;
	pCollider->m_State = SharedPhysics::C_Collider::E_Normal;

	return pCollider;
}

void C_CollisionManager::DestructShape(Collision::C_Shape* pShape)
{
	switch(pShape->m_ShapeType)
	{
	case E_Box_Shape://0x8
		{
			Collision::C_BoxShape* pBoxShape = (Collision::C_BoxShape*)pShape;
			pBoxShape->~C_BoxShape();
			break;
		}
	case E_ConvexHull_Shape://0x10
		{
			Collision::C_ConvexHullShape* pBoxShape = (Collision::C_ConvexHullShape*)pShape;
			pBoxShape->~C_ConvexHullShape();
			break;
		}
	case E_Sphere_Shape://0x20
		{
			Collision::C_SphereShape* pBoxShape = (Collision::C_SphereShape*)pShape;
			pBoxShape->~C_SphereShape();
			break;
		}
	case E_Capsule_Shape://0x40
		{
			Collision::C_CapsuleShape* pBoxShape = (Collision::C_CapsuleShape*)pShape;
			pBoxShape->~C_CapsuleShape();
			break;
		}
	case E_Cylinder_Shape://0x80
		{
			Collision::C_CylinderShape* pBoxShape = (Collision::C_CylinderShape*)pShape;
			pBoxShape->~C_CylinderShape();
			break;
		}
	case E_Composite_Shape://0x100
		{
			Collision::C_CompositeShape* pBoxShape = (Collision::C_CompositeShape*)pShape;
			pBoxShape->~C_CompositeShape();
			break;
		}
	case E_Cone_Shape://0x10000
		{
			Collision::C_ConeShape* pBoxShape = (Collision::C_ConeShape*)pShape;
			pBoxShape->~C_ConeShape();
			break;
		}
	case E_InstancedTriangleMesh_Shape://0x10000
		{
			Collision::C_InstancedTriangleMeshShape* pBoxShape = (Collision::C_InstancedTriangleMeshShape*)pShape;
			pBoxShape->~C_InstancedTriangleMeshShape();
			break;
		}
	}
}


//bool C_CollisionManager::CollideMesh(const C_ShapeWrapper* const  __restrict a, const C_ShapeWrapper*  const __restrict b, SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionResult<MaxPatches, MaxContacts>& results )
//{
//	E_ShapeType b_Shape=b->m_pShape->GetShape();
//
//
//	C_TriangleMeshShape* triangleMesh=(C_TriangleMeshShape*)a->m_pShape;
//	OptimisedAABBTree& tree=triangleMesh->m_AABBTree;
//	//Object a is a triangle mesh!
//	if(b_Shape==E_Box_Shape)
//	{
//		return tree.Collide<MaxPatches,MaxContacts, CollideOBBTriangleMesh<MaxPatches, MaxContacts, Collision::AABBTreeTriangle>>(b, results, stack);
//	}
//
//	if(b_Shape==E_Capsule_Shape)
//	{
//		return tree.Collide<MaxPatches,MaxContacts, CollideCapsuleTriangleMesh<MaxPatches, MaxContacts, Collision::AABBTreeTriangle>>(b, results, stack);
//	}
//
//	if(b_Shape==E_Cylinder_Shape)
//	{
//		//tree.Collide<4,16, AABBTreeTriangle, GJKCPair>(b, *pCollider, results);
//		//ReduceContactsAndPackToStream<MaxPatches, MaxContacts, 1, 1>(results, stream);
//		return NULL;
//	}
//
//	if(b_Shape==E_Sphere_Shape)
//	{
//		return tree.Collide<MaxPatches,MaxContacts, CollideSphereTriangleMesh<MaxPatches, MaxContacts, Collision::AABBTreeTriangle>>(b, results, stack);
//	}
//	assert(0 && "Undefined collision pair!");
//	return false;
//
//}
};

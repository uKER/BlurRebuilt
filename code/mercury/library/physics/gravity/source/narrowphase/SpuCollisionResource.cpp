#include "Stdafx.h"

#include "gravity/narrowphase/SpuCollisionResource.h"
#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/SphereSphere.inl"
#include "gravity/narrowphase/CapsuleSphere.inl"
#include "gravity/narrowphase/CapsuleCapsule.inl"
#include "gravity/narrowphase/OBBOBB.inl"
#include "gravity/narrowphase/OBBSphere.inl"
#include "gravity/narrowphase/CompositeShapeCollision.inl"
#include "gravity/narrowphase/CollisionManager.h"
#include "gravity/narrowphase/CollisionManager.inl"
#include "gravity/narrowphase/CollisionResult.inl"
#include "gravity/narrowphase/collisionfilter.inl"
#include "gravity/narrowphase/GJKCollision.h"


namespace Collision
{

	void CollideBoxBoxResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream, EPA& m_Epa)
	{
		C_BoxShape* aBox = (C_BoxShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_BoxShape));
		C_BoxShape* bBox = (C_BoxShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_BoxShape));


		U32 physicsMaterial(aBox->PhysicsMaterialIndex() << 16 | bBox->PhysicsMaterialIndex());
		U32 effectMaterial(aBox->EffectMaterialIndex() << 16 | bBox->EffectMaterialIndex());
		C_ContactPatch<_MaxContacts> patch;

		if(CollideBoxBox(aBox->m_Box, bBox->m_Box, patch, physicsMaterial, effectMaterial, m_Epa))
		{
			results.AddPatch(patch);
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}

	}

	void CollideBoxSphereResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream, bool reverse)
	{
		C_BoxShape* pShapeA=(C_BoxShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_BoxShape));
		C_SphereShape* pShapeB=(C_SphereShape*) Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_SphereShape));

		U32 physicsMaterial = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		U32 effectMaterial = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();

		C_ContactPatch<_MaxContacts> patch;

		if(CollideOBBSphere(pShapeA->m_Box, pShapeB->m_Sphere, patch, physicsMaterial, effectMaterial))
		{
			if(reverse)
			{
				//manager.ReverseContacts(results);
				patch.m_Normal = -patch.m_Normal;
			}
			results.AddPatch(patch);
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		
		}
		
	}



	void CollideCompositeShapeResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack, EPA& m_Epa, bool reverse)
	{
		/*C_CompositeShape* pShapeA = (C_CompositeShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_CompositeShape));
		C_Shape* pShapeB=(C_Shape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_Shape));*/

		//OK. Test the OBB box of one with the bounding box of the other - local space!!!

		//{
		//	Collision::C_Shape* pShapeA = (Collision::C_Shape*)Spu::FetchAllocAsync(pObjA->m_pShape, sizeof(Collision::C_Shape), 3);
		//	Collision::C_Shape* pShapeB = (Collision::C_Shape*)Spu::FetchAllocAsync(pObjB->m_pShape, sizeof(Collision::C_Shape), 3);
		//	Spu::Wait(3);

		//	//Create OBBs from these

		//	//TODO finish off this test!!!
		//	Collision::C_OBB obbA, obbB;
		//	obbA.c = pShape->
		//	Collision::GJKCPair(

		//}
		//DebugBreak();
		Spu::MemBaseMarker marker;
#ifndef PLATFORM_SPU
		Collision::C_ContactPatch<_MaxContacts> patch;
#else
		Collision::C_ContactPatch<_MaxContacts>* _patch = (Collision::C_ContactPatch<_MaxContacts>*)Spu::Alloc(sizeof(Collision::C_ContactPatch<_MaxContacts>));
		Collision::C_ContactPatch<_MaxContacts>& patch = *_patch;
#endif
		U32 numContactPatches = results.m_NumContactPatches;

		//Object A's shape is a composite shape. Object B's shape is unknown - we need to find out more.

		Collision::C_Shape* pCompositeShape = (Collision::C_Shape*)Spu::FetchAllocAsync(pObjA->m_pShape, sizeof(C_CompositeShape), 8);
		U32 size = Spu::Get((U32*)&pObjB->m_pShape->m_Size);
		Collision::C_Shape* pUnknownShape = (Collision::C_Shape*)Spu::FetchAllocAsync(pObjB->m_pShape, size, 8);

		Spu::Wait(8);

		if(CollideCompositeShape(manager, pCompositeShape, pUnknownShape, results, stack, m_Epa, reverse, patch))
		{
			//DebugBreak();
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
		
	}


	void CollidedCapsuleCapsuleResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream)
	{
		C_CapsuleShape* pShapeA=(C_CapsuleShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_CapsuleShape));
		C_CapsuleShape* pShapeB=(C_CapsuleShape*) Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_CapsuleShape));

		U32 physicsMaterial =  pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		U32 effectMaterial = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();
		C_ContactPatch<_MaxContacts> patch;

		if(CollideCapsuleCapsule(pShapeA->m_Capsule, pShapeB->m_Capsule, patch, physicsMaterial, effectMaterial))
		{
			results.AddPatch(patch);
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
		
	}

	void CollidedSphereCapsuleResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream, bool reverse)
	{

		C_SphereShape* pShapeA=(C_SphereShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_SphereShape));
		C_CapsuleShape* pShapeB=(C_CapsuleShape*) Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_CapsuleShape));
		

		U32 physicsMaterial =  pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		U32 effectMaterial = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();

		C_ContactPatch<_MaxContacts> patch;
		if(CollideCapsuleSphere( pShapeB->m_Capsule, pShapeA->m_Sphere, patch, physicsMaterial, effectMaterial))
		{
			if(reverse)
			{
				//manager.ReverseContacts(results);
				patch.m_Normal = -patch.m_Normal;
			}
			results.AddPatch(patch);
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
			
		}
	}


	void CollidedSphereSphereResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream)
	{
		C_SphereShape* pShapeA=(C_SphereShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_SphereShape));
		C_SphereShape* pShapeB=(C_SphereShape*) Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_SphereShape));

		U32 physicsMaterial =  pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		U32 effectMaterial = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();
		C_ContactPatch<_MaxContacts> patch;
		/*C_SphereShape* aSphere=(C_SphereShape*)a->m_pShape;
		C_SphereShape* bSphere=(C_SphereShape*)b->m_pShape;*/
		if(CollideSphereSphere(pShapeA->m_Sphere, pShapeB->m_Sphere, patch, physicsMaterial, effectMaterial))
		{
			results.AddPatch(patch);
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB , stream, results);
		}
	}



	void CollidedCharacterControllerCapsuleResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream, bool reverse)
	{

		C_CharacterControllerShape* pCharacter=(C_CharacterControllerShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_CharacterControllerShape));
		C_CapsuleShape* pCapsule=(C_CapsuleShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_CapsuleShape));

		U32 physicsMaterial = pCharacter->PhysicsMaterialIndex() << 16 | pCapsule->PhysicsMaterialIndex();
		U32 effectMaterial = pCharacter->EffectMaterialIndex() << 16 | pCapsule->EffectMaterialIndex();
		C_ContactPatch<_MaxContacts> patch;
		
		if(CollideCapsuleCapsule(pCharacter->m_Controller.m_EnclosingCapsule, pCapsule->m_Capsule, patch, physicsMaterial, effectMaterial))
		{
			if(reverse)
			{
				//manager.ReverseContacts(results);
				patch.m_Normal = -patch.m_Normal;
			}
			results.AddPatch(patch);
			manager.StoreContactsCharacter<_MaxPatches, _MaxContacts, 1, 1>(pObjA, pObjB, stream, results);
			
		}
	}


	void CollidedCharacterControllersResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream)
	{

		C_CharacterControllerShape* pCharacter=(C_CharacterControllerShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_CharacterControllerShape));
		C_CharacterControllerShape* pCharacterB=(C_CharacterControllerShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_CharacterControllerShape));

		U32 physicsMaterial = pCharacter->PhysicsMaterialIndex() << 16 | pCharacterB->PhysicsMaterialIndex();
		U32 effectMaterial = pCharacter->EffectMaterialIndex() << 16 | pCharacterB->EffectMaterialIndex();
		C_ContactPatch<_MaxContacts> patch;
		
		if(CollideCapsuleCapsule(pCharacter->m_Controller.m_EnclosingCapsule, pCharacterB->m_Controller.m_EnclosingCapsule, patch, physicsMaterial, effectMaterial ))
		{
			results.AddPatch(patch);
			manager.StoreContactsCharacter<_MaxPatches, _MaxContacts, 1, 1>(pObjA, pObjB, stream, results);
		}
	}


	
	void CollidedSphereCharacterControllerResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream, bool reverse)
	{
		C_SphereShape* pShapeA=(C_SphereShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_SphereShape));
		C_CharacterControllerShape* pShapeB=(C_CharacterControllerShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_CharacterControllerShape));
		
		U32 physicsMaterial =  pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		U32 effectMaterial = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();

		C_ContactPatch<_MaxContacts> patch;

		if(CollideCapsuleSphere(pShapeB->m_Controller.m_EnclosingCapsule, pShapeA->m_Sphere, patch, physicsMaterial, effectMaterial))
		{
			if(reverse)
			{
				//manager.ReverseContacts(results);
				patch.m_Normal = -patch.m_Normal;
			}
			results.AddPatch(patch);
			manager.StoreContactsCharacter<_MaxPatches, _MaxContacts, 2, 2>(pObjA, pObjB, stream, results);
		}
	}
}

  #include "StdAfx.h"
#include "gravity/narrowphase/GJKCollision.h"
#include "gravity/narrowphase/GJK.inl"
#include "gravity/narrowphase/primitive.inl"
#include "gravity/narrowphase/CollisionResult.inl"
#include "gravity/narrowphase/CollisionManager.h"
#include "gravity/narrowphase/CollisionManager.inl"
#include "gravity/narrowphase/CollisionFilter.inl"


namespace Collision
{
	bool GJKBoxBox(C_OBB& boxA, C_OBB& boxB, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa,
		Maths::HVec3& closestA, Maths::HVec3& closestB)
	{
		return GJKCPairMargin<C_OBB, C_OBB>(boxA, boxB, 
			contact, normal.m_V, penetration, epa);
	}

	bool GJKBoxCapsule(C_OBB& boxA, C_Capsule& capsule, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa)
	{
		//return GJKCPair<C_OBB, C_Capsule>(boxA, capsule, contact, normal.m_V, penetration, epa);

		return GJKCPairMargin<C_OBB, C_Capsule>(boxA, capsule,
			contact, normal.m_V, penetration, epa);
	}
	bool GJKBoxCylinder(C_OBB& boxA,C_Cylinder& cylinder, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa)
	{
		return GJKCPairMargin<C_OBB, C_Cylinder>(boxA, cylinder, 
			contact, normal.m_V, penetration, epa);
	}


	bool GJKBoxConvex(C_OBB& boxA,C_ConvexHull& convex, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa)
	{
		C_SpuConvexHull spuConvex(convex);
		//spuConvex.Initialise(convex);
		return GJKCPairMargin<C_OBB, C_SpuConvexHull>(boxA, spuConvex, 
			contact, normal.m_V, penetration, epa);
	/*	return GJKCPairMargin<C_OBB, C_ConvexHull>(boxA, convex, 
			contact, normal.m_V, penetration, epa);*/
	}


	bool GJKBoxCone(C_OBB& boxA, C_Cone& cone, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa)
	{
		return GJKCPairMargin<C_OBB, C_Cone>(boxA, cone, 
			contact, normal.m_V, penetration, epa);
	}


	void GJKBoxCapsule(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse)
	{
		C_BoxShape* pShapeA = (C_BoxShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_BoxShape));
		C_CapsuleShape* pShapeB = (C_CapsuleShape* ) Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_CapsuleShape));
		

		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();
		Maths::HF32 penetrationDepth(Maths::UNDEFINED);

		if(GJKCPairMargin<C_OBB, C_Capsule>(pShapeA->m_Box, pShapeB->m_Capsule, 
			point.m_ContactPoint, patch.m_Normal.m_V, penetrationDepth, m_Epa))
		{
			Maths::Store(penetrationDepth, &point.m_PenetrationDepth);
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
			
		}
	}

	
	void GJKBoxCylinder(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse)
	{
		C_BoxShape* pShapeA=(C_BoxShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_BoxShape));
		C_CylinderShape* pShapeB= (C_CylinderShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_CylinderShape));
		
		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();
		Maths::HF32 penetrationDepth(Maths::UNDEFINED);

		if(GJKCPairMargin<C_OBB, C_Cylinder>(pShapeA->m_Box, pShapeB->m_Cylinder, 
			point.m_ContactPoint, patch.m_Normal.m_V, penetrationDepth, m_Epa))
		{
			Maths::Store(penetrationDepth, &point.m_PenetrationDepth);
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
	}

	void GJKBoxConvex(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse)
	{
		C_BoxShape* pShapeA = (C_BoxShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_BoxShape));
		C_ConvexHullShape* pShapeB= (C_ConvexHullShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_ConvexHullShape));

		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();
		Maths::HF32 penetrationDepth(Maths::UNDEFINED);

		
		C_SpuConvexHull spuConvex(pShapeB->m_Hull);
		//convex.Initialise(pShapeB->m_Hull);
		if(GJKCPairMargin<C_OBB, C_SpuConvexHull>(pShapeA->m_Box, spuConvex, 
			point.m_ContactPoint, patch.m_Normal.m_V, penetrationDepth, m_Epa))
		{
			Maths::Store(penetrationDepth, &point.m_PenetrationDepth);
			//DebugBreak();
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
			
		}
	}

	void GJKBoxCone(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse)
	{
		C_BoxShape* pShapeA = (C_BoxShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_BoxShape));
		C_ConeShape* pShapeB= (C_ConeShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_ConeShape));

		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();
		Maths::HF32 penetrationDepth(Maths::UNDEFINED);

		if(GJKCPairMargin<C_OBB, C_Cone>(pShapeA->m_Box, pShapeB->m_Cone, 
			point.m_ContactPoint, patch.m_Normal.m_V, penetrationDepth, m_Epa))
		{
			Maths::Store(penetrationDepth, &point.m_PenetrationDepth);
			//DebugBreak();
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
			
		}
	}

	void GJKCharacterControllerBox(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse)
	{
		C_CharacterControllerShape* pShapeA	=(C_CharacterControllerShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_CharacterControllerShape));
		C_BoxShape* pShapeB					=(C_BoxShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_BoxShape));
		
		
		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();


		C_Capsule& capsule = pShapeA->m_Controller.GetCapsule(pShapeB);
		Maths::HF32 penetrationDepth(Maths::UNDEFINED);

		/*if(GJKCPairMargin<C_Capsule, C_OBB>(pShapeA->m_Controller.m_EnclosingCapsule, pShapeB->m_Box, 
			point.m_ContactPoint, patch.m_Normal.m_V, point.m_PenetrationDepth, m_Epa))*/
		if(GJKCPairMargin<C_Capsule, C_OBB>(capsule, pShapeB->m_Box, point.m_ContactPoint, patch.m_Normal.m_V, penetrationDepth, m_Epa))
		{
			Maths::Store(penetrationDepth, &point.m_PenetrationDepth);
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point.m_ContactPoint, 0.25f, Maths::RGBA(0, 0, 255, 255));
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(point.m_ContactPoint, patch.m_Normal.m_V, point.m_PenetrationDepth, 0.025f, Maths::RGBA(0, 255, 255, 0));
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContactsCharacter<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjB, pObjA, stream, results);
			
		}
	}


};
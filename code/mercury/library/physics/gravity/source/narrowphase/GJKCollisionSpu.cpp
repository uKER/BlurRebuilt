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
												//Sphere vs....
	
	void GJKSphereCylinder(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse)
	{
		C_SphereShape* pShapeA=(C_SphereShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_SphereShape));
		C_CylinderShape* pShapeB=(C_CylinderShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_CylinderShape));

		/*C_SphereShape* pShapeA((C_SphereShape*)a->m_pShape);
		C_CylinderShape* pShapeB((C_CylinderShape*)b->m_pShape);*/

		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();
		Maths::HF32 penDepth(Maths::UNDEFINED);

	
		if(GJKCPairMargin<C_Sphere, C_Cylinder>(pShapeA->m_Sphere, pShapeB->m_Cylinder, 
			point.m_ContactPoint, patch.m_Normal.m_V, penDepth, m_Epa))
		{
			Maths::Store(penDepth, &point.m_PenetrationDepth);
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
			
		}
	}

	void GJKSphereConvex(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse)
	{
		Spu::MemBaseMarker marker;
		C_SphereShape* pShapeA = (C_SphereShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_SphereShape));
		C_ConvexHullShape* pShapeB = (C_ConvexHullShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_ConvexHullShape));

		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();

		C_SpuConvexHull spuConvex(pShapeB->m_Hull);
		//spuConvex.Initialise(pShapeB->m_Hull);
		Maths::HF32 penDepth(Maths::UNDEFINED);

		if(GJKCPairMargin<C_Sphere, C_SpuConvexHull>(pShapeA->m_Sphere, spuConvex, 
			point.m_ContactPoint, patch.m_Normal.m_V,penDepth, m_Epa))
		/*if(GJKCPairMargin<C_Sphere, C_ConvexHull>(pShapeA->m_Sphere, pShapeB->m_Hull, 
			point.m_ContactPoint, patch.m_Normal.m_V, point.m_PenetrationDepth, m_Epa))*/
		{
			Maths::Store(penDepth, &point.m_PenetrationDepth);
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
			
		}
	}

	void GJKSphereCone(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse)
	{
		C_SphereShape* pShapeA = (C_SphereShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_SphereShape));
		C_ConeShape* pShapeB = (C_ConeShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_ConeShape));

		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();
		Maths::HF32 penDepth(Maths::UNDEFINED);

		if(GJKCPairMargin<C_Sphere, C_Cone>(pShapeA->m_Sphere, pShapeB->m_Cone, 
			point.m_ContactPoint, patch.m_Normal.m_V, penDepth, m_Epa))
		{
			Maths::Store(penDepth, &point.m_PenetrationDepth);
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
			
		}
	
	}

	
						//Capsule vs...

	void GJKCapsuleConvex(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse)
	{

		C_CapsuleShape* pShapeA=(C_CapsuleShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_CapsuleShape));
		C_ConvexHullShape* pShapeB = (C_ConvexHullShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_ConvexHullShape));

		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16| pShapeB->EffectMaterialIndex();

		//C_SpuConvexHull spuConvexHull(pShapeB->m_Hull);
		//spuConvexHull.Initialise(pShapeB->m_Hull);
		//if(GJKCPairMargin<C_Capsule, C_SpuConvexHull>(pShapeA->m_Capsule, spuConvexHull, 
		//	point.m_ContactPoint, patch.m_Normal.m_V, point.m_PenetrationDepth, m_Epa))
		Maths::HF32 penDepth(Maths::UNDEFINED);
		if(GJKCPairMargin<C_Capsule, C_ConvexHull>(pShapeA->m_Capsule, pShapeB->m_Hull, 
			point.m_ContactPoint, patch.m_Normal.m_V, penDepth, m_Epa))
		{
			Maths::Store(penDepth, &point.m_PenetrationDepth);
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
			
		}

	}

	void GJKCapsuleCone(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse)
	{

		C_CapsuleShape* pShapeA=(C_CapsuleShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_CapsuleShape));
		C_ConeShape* pShapeB = (C_ConeShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_ConeShape));

		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16| pShapeB->EffectMaterialIndex();
		Maths::HF32 penDepth(Maths::UNDEFINED);

		if(GJKCPairMargin<C_Capsule, C_Cone>(pShapeA->m_Capsule, pShapeB->m_Cone, 
			point.m_ContactPoint, patch.m_Normal.m_V, penDepth, m_Epa))
		{
			Maths::Store(penDepth, &point.m_PenetrationDepth);
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
			
		}

	}

	void GJKCapsuleCylinder(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse)
	{

		C_CapsuleShape* pShapeA=(C_CapsuleShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_CapsuleShape));
		C_CylinderShape* pShapeB=(C_CylinderShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_CylinderShape));

		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();
		Maths::HF32 penDepth(Maths::UNDEFINED);

		if(GJKCPairMargin<C_Capsule, C_Cylinder>(pShapeA->m_Capsule, pShapeB->m_Cylinder, 
			point.m_ContactPoint, patch.m_Normal.m_V, penDepth, m_Epa))
		{
			Maths::Store(penDepth, &point.m_PenetrationDepth);
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
			
		}
	}

	void GJKCapsuleCapsule(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa)
	{

		C_CapsuleShape* pShapeA=(C_CapsuleShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_CapsuleShape));
		C_CapsuleShape* pShapeB=(C_CapsuleShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_CapsuleShape));

		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();
		Maths::HF32 penDepth(Maths::UNDEFINED);

		if(GJKCPairMargin<C_Capsule, C_Capsule>(pShapeA->m_Capsule, pShapeB->m_Capsule, 
			point.m_ContactPoint, patch.m_Normal.m_V, penDepth, m_Epa))
		{
			Maths::Store(penDepth, &point.m_PenetrationDepth);
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
	}

	void GJKCapsuleCharacter(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse)
	{

		C_CapsuleShape* pShapeA=(C_CapsuleShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_CapsuleShape));
		C_CharacterControllerShape* pShapeB=(C_CharacterControllerShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_CharacterControllerShape));

		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();

		C_Capsule& capsule = pShapeB->m_Controller.GetCapsule(pShapeA);
		Maths::HF32 penDepth(Maths::UNDEFINED);
		/*if(GJKCPairMargin<C_Capsule, C_Capsule>(pShapeA->m_Capsule, pShapeB->m_Controller.m_EnclosingCapsule, 
			point.m_ContactPoint, patch.m_Normal.m_V, point.m_PenetrationDepth, m_Epa))*/
		if(GJKCPairMargin<C_Capsule, C_Capsule>(pShapeA->m_Capsule, capsule, point.m_ContactPoint, patch.m_Normal.m_V, penDepth, m_Epa))
		{
			Maths::Store(penDepth, &point.m_PenetrationDepth);
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
	}

	
						//Cylinder vs...
	
	void GJKCylinderCylinder(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reserve)
	{

		C_CylinderShape* pShapeA=(C_CylinderShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_CylinderShape));
		C_CylinderShape* pShapeB=(C_CylinderShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_CylinderShape));

		/*C_CylinderShape* pShapeB((C_CylinderShape*)b->m_pShape);
		C_CylinderShape* pShapeA((C_CylinderShape*)a->m_pShape);*/

		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();
		Maths::HF32 penDepth(Maths::UNDEFINED);

		if(GJKCPairMargin<C_Cylinder, C_Cylinder>(pShapeA->m_Cylinder, pShapeB->m_Cylinder, 
			point.m_ContactPoint, patch.m_Normal.m_V, penDepth, m_Epa))
		{
			Maths::Store(penDepth, &point.m_PenetrationDepth);
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}

	}
						//Convex vs...
	
	void GJKConvexConvex(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream, EPA& m_Epa)
	{
		Spu::MemBaseMarker marker;
		C_ConvexHullShape* pShapeA = (C_ConvexHullShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_ConvexHullShape));
		C_ConvexHullShape* pShapeB = (C_ConvexHullShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_ConvexHullShape));

		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();
		Maths::HF32 penDepth(Maths::UNDEFINED);

		C_SpuConvexHull spuConvexA(pShapeA->m_Hull);
		//spuConvexA.Initialise(pShapeA->m_Hull);
		C_SpuConvexHull spuConvexB(pShapeB->m_Hull);
		//spuConvexB.Initialise(pShapeB->m_Hull);
		if(GJKCPairMargin<C_SpuConvexHull, C_SpuConvexHull>(spuConvexA, spuConvexB, point.m_ContactPoint, patch.m_Normal.m_V, penDepth, m_Epa))
		//if(GJKCPairMargin<C_ConvexHull, C_ConvexHull>(pShapeA->m_Hull, pShapeB->m_Hull, point.m_ContactPoint, patch.m_Normal.m_V, penDepth, m_Epa))
		{
			Maths::Store(penDepth, &point.m_PenetrationDepth);
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
			
	}

	void GJKConvexCone(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream, EPA& m_Epa,
		bool reverse)
	{
		Spu::MemBaseMarker marker;
		C_ConvexHullShape* pShapeA = (C_ConvexHullShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_ConvexHullShape));
		C_ConeShape* pShapeB = (C_ConeShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_ConeShape));

		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();

		C_SpuConvexHull spuConvex(pShapeA->m_Hull);
		Maths::HF32 penDepth(Maths::UNDEFINED);
		//spuConvex.Initialise(pShapeA->m_Hull);
		//if(GJKCPairMargin<C_ConvexHull, C_Cone>(pShapeA->m_Hull, pShapeB->m_Cone, point.m_ContactPoint, patch.m_Normal.m_V, point.m_PenetrationDepth, m_Epa))
		if(GJKCPairMargin<C_SpuConvexHull, C_Cone>(spuConvex, pShapeB->m_Cone, point.m_ContactPoint, patch.m_Normal.m_V, penDepth, m_Epa))
		{
			Maths::Store(penDepth, &point.m_PenetrationDepth);
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
			
	}

	void GJKCylinderConvex(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream, EPA& m_Epa,
		bool reverse)
	{
		Spu::MemBaseMarker marker;
		C_CylinderShape* pShapeA = (C_CylinderShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_CylinderShape));
		C_ConvexHullShape* pShapeB = (C_ConvexHullShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_ConvexHullShape));

		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();
		Maths::HF32 penDepth(Maths::UNDEFINED);

		C_SpuConvexHull spuConvexHull(pShapeB->m_Hull);
		//spuConvexHull.Initialise(pShapeB->m_Hull);
		if(GJKCPairMargin<C_Cylinder, C_SpuConvexHull>(pShapeA->m_Cylinder, spuConvexHull, point.m_ContactPoint, patch.m_Normal.m_V, penDepth, m_Epa))
		//if(GJKCPairMargin<C_Cylinder, C_ConvexHull>(pShapeA->m_Cylinder, pShapeB->m_Hull, point.m_ContactPoint, patch.m_Normal.m_V, point.m_PenetrationDepth, m_Epa))
		{
			Maths::Store(penDepth, &point.m_PenetrationDepth);
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
			
	}

	void GJKCylinderCone(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream, EPA& m_Epa,
		bool reverse)
	{
		Spu::MemBaseMarker marker;
		C_CylinderShape* pShapeA = (C_CylinderShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_CylinderShape));
		C_ConeShape* pShapeB = (C_ConeShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_ConeShape));

		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();
		Maths::HF32 penDepth(Maths::UNDEFINED);

		if(GJKCPairMargin<C_Cylinder, C_Cone>(pShapeA->m_Cylinder, pShapeB->m_Cone, point.m_ContactPoint, patch.m_Normal.m_V, penDepth, m_Epa))
		{
			Maths::Store(penDepth, &point.m_PenetrationDepth);
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}
			
	}

								//Cone vs Cone
	void GJKConeCone(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse)
	{
		C_ConeShape* pShapeA = (C_ConeShape*)Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_ConeShape));
		C_ConeShape* pShapeB = (C_ConeShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_ConeShape));

		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();
		Maths::HF32 penDepth(Maths::UNDEFINED);

		if(GJKCPairMargin<C_Cone, C_Cone>(pShapeA->m_Cone, pShapeB->m_Cone, point.m_ContactPoint, patch.m_Normal.m_V, penDepth, m_Epa))
		{
			Maths::Store(penDepth, &point.m_PenetrationDepth);
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
		}

	}


								//CharacterController vs......


	void GJKCharacterControllerCylinder(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse)
	{

		C_CharacterControllerShape* pShapeA=(C_CharacterControllerShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_CharacterControllerShape));
		C_CylinderShape* pShapeB=(C_CylinderShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_CylinderShape));

		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 | pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();

		C_Capsule& capsule = pShapeA->m_Controller.GetCapsule(pShapeB);
		Maths::HF32 penDepth(Maths::UNDEFINED);

		/*if(GJKCPairMargin<C_Capsule, C_Cylinder>(pShapeA->m_Controller.m_EnclosingCapsule, pShapeB->m_Cylinder, 
			point.m_ContactPoint, patch.m_Normal.m_V, point.m_PenetrationDepth, m_Epa))*/
		if(GJKCPairMargin<C_Capsule, C_Cylinder>(capsule, pShapeB->m_Cylinder, point.m_ContactPoint, patch.m_Normal.m_V, penDepth, m_Epa))
		{
			Maths::Store(penDepth, &point.m_PenetrationDepth);
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
			
		}

	}

	void GJKCharacterControllerConvex(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse)
	{
		Spu::MemBaseMarker marker;
		C_CharacterControllerShape* pShapeA=(C_CharacterControllerShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_CharacterControllerShape));
		C_ConvexHullShape* pShapeB=(C_ConvexHullShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_ConvexHullShape));
		
		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 |pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();

		C_Capsule& capsule = pShapeA->m_Controller.GetCapsule(pShapeB);
		/*if(GJKCPairMargin<C_Capsule, C_ConvexHull>(pShapeA->m_Controller.m_EnclosingCapsule,pShapeB->m_Hull, 
			point.m_ContactPoint, patch.m_Normal.m_V, point.m_PenetrationDepth, m_Epa))*/
		C_SpuConvexHull spuConvex(pShapeB->m_Hull);
		Maths::HF32 penDepth(Maths::UNDEFINED);
		if(GJKCPairMargin<C_Capsule, C_SpuConvexHull>(capsule, spuConvex, point.m_ContactPoint, patch.m_Normal.m_V, penDepth, m_Epa))
		//if(GJKCPairMargin<C_Capsule, C_ConvexHull>(capsule, pShapeB->m_Hull, point.m_ContactPoint, patch.m_Normal.m_V, point.m_PenetrationDepth, m_Epa))
		{
			Maths::Store(penDepth, &point.m_PenetrationDepth);
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContactsCharacter<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
			
		}

	}

	void GJKCharacterControllerCone(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse)
	{
		C_CharacterControllerShape* pShapeA=(C_CharacterControllerShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_CharacterControllerShape));
		C_ConeShape* pShapeB=(C_ConeShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_ConeShape));
		
		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 |pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();

		C_Capsule& capsule = pShapeA->m_Controller.GetCapsule(pShapeB);
		Maths::HF32 penDepth(Maths::UNDEFINED);

		/*if(GJKCPairMargin<C_Capsule, C_Cone>(pShapeA->m_Controller.m_EnclosingCapsule,pShapeB->m_Cone, 
			point.m_ContactPoint, patch.m_Normal.m_V, point.m_PenetrationDepth, m_Epa))*/
		if(GJKCPairMargin<C_Capsule, C_Cone>(capsule,pShapeB->m_Cone, point.m_ContactPoint, patch.m_Normal.m_V, penDepth, m_Epa))
		{
			Maths::Store(penDepth, &point.m_PenetrationDepth);
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContactsCharacter<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
			
		}

	}

	void GJKCharacterControllerCharacterController(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse)
	{
		C_CharacterControllerShape* pShapeA=(C_CharacterControllerShape*) Spu::FetchAlloc(pObjA->m_pShape, sizeof(C_CharacterControllerShape));
		C_CharacterControllerShape* pShapeB=(C_CharacterControllerShape*) Spu::FetchAlloc(pObjB->m_pShape, sizeof(C_CharacterControllerShape));
		
		results.m_NumContactPatches = 1;
		C_ContactPatch<_MaxContacts>& patch(results.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		point.m_HashValue = 1;
		point.m_PhysicsMaterialIndices = pShapeA->PhysicsMaterialIndex() << 16 |pShapeB->PhysicsMaterialIndex();
		point.m_EffectMaterialIndices = pShapeA->EffectMaterialIndex() << 16 | pShapeB->EffectMaterialIndex();
		Maths::HF32 penDepth(Maths::UNDEFINED);

	
		if(GJKCPairMargin<C_Capsule, C_Capsule>(pShapeA->m_Controller.m_EnclosingCapsule,pShapeB->m_Controller.m_EnclosingCapsule, 
			point.m_ContactPoint, patch.m_Normal.m_V, penDepth, m_Epa))
		{
			Maths::Store(penDepth, &point.m_PenetrationDepth);
			if(reverse)
			{
				manager.ReverseContacts(results);
			}
			manager.StoreContactsCharacter<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(pObjA, pObjB, stream, results);
			
		}

	}
}

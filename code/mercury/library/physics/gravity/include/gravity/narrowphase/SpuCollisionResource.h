#ifndef SPUCOLLISIONRESOURCE_H
#define SPUCOLLISIONRESOURCE_H

#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/CollisionManager.h"
#include "gravity/narrowphase/GJK.h"


namespace Collision
{
    class C_CollisionManager;

	//void ReverseContacts(C_CollisionResult<MaxPatches, MaxContacts>& results);

	void CollideBoxBoxResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, SharedPhysics::C_ContactStream& stream, EPA& m_Epa);

	void CollidedCapsuleCapsuleResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, SharedPhysics::C_ContactStream& stream);

	void CollidedSphereSphereResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, SharedPhysics::C_ContactStream& stream);
	
	
	void CollideBoxSphereResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, SharedPhysics::C_ContactStream& stream, bool reverse);

	//void CollideSphereBoxResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<MaxPatches, MaxContacts>& res, SharedPhysics::C_ContactStream& stream);

	
	void CollideCompositeShapeResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack, EPA& m_Epa, bool reverse);

	//void CollideShapeCompositeResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<MaxPatches, MaxContacts>& res, SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack, EPA& m_Epa, const Maths::HVec3 PCREF relativeVelocity);

	
	void CollidedSphereCapsuleResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, SharedPhysics::C_ContactStream& stream, bool reverse);

	//void CollidedSphereCapsuleResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<MaxPatches, MaxContacts>& res, SharedPhysics::C_ContactStream& stream);

	
	//void CollidedCapsuleCharacterControllerResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<MaxPatches, MaxContacts>& res, SharedPhysics::C_ContactStream& stream, bool reverse);
	
	void CollidedCharacterControllerCapsuleResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, SharedPhysics::C_ContactStream& stream, bool reverse);

	
	void CollidedCharacterControllersResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, SharedPhysics::C_ContactStream& stream);
	
	
	//void CollidedSphereCharacterControllerResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<MaxPatches, MaxContacts>& res, SharedPhysics::C_ContactStream& stream, bool);

	void CollidedSphereCharacterControllerResource(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, SharedPhysics::C_ContactStream& stream, bool reverse);
}

#endif

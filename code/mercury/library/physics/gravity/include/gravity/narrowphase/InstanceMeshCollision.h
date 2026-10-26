#ifndef INSTANCEMESHCOLLISION_H
#define INSTANCEMESHCOLLISION_H

#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/CollisionManager.h"

namespace Collision
{
	class C_CollisionManager; 
	void InstanceMeshBox(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB,  C_CollisionResult<_MaxPatches, _MaxContacts>& res,
		SharedPhysics::C_ContactStream& stream,SharedPhysics::C_SafeStack<true>& stack);

	void InstanceMeshCapsule(C_CollisionManager& manager,C_CollisionObject* pObjA, C_CollisionObject* pObjB,  C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack);

	void InstanceMeshSphere(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB,  C_CollisionResult<_MaxPatches, _MaxContacts>& res,
		SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack);
	
	void InstanceMeshCylinder(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB,  C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack);
	
	void InstanceMeshConvexHull(C_CollisionManager& manager,C_CollisionObject* pObjA, C_CollisionObject* pObjB,  C_CollisionResult<_MaxPatches, _MaxContacts>& res,
		SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack);

	void InstanceMeshCharacterController(C_CollisionManager& manager,C_CollisionObject* pObjA, C_CollisionObject* pObjB,  C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack);

	void InstanceMeshCone(C_CollisionManager& manager,C_CollisionObject* pObjA, C_CollisionObject* pObjB,  C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack);
}

#endif


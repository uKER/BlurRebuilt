#ifndef GJK_COLLISION_H
#define GJK_COLLISION_H

#include "gravity/narrowphase/Primitive.h"
//#include "gravity/narrowphase/gjk.h"

#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/CollisionManager.h"
#include "gravity/narrowphase/GJK.h"

namespace Collision
{
	class C_CollisionManager;
						//Box vs...

	//For composite object collision detection
	bool GJKBoxBox(C_OBB& boxA, C_OBB& boxB, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);

	bool GJKBoxCapsule(C_OBB& boxA, C_Capsule& capsule, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);

	bool GJKBoxCylinder(C_OBB& boxA, C_Cylinder& cylinder, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);

	bool GJKBoxConvex(C_OBB& boxA, C_ConvexHull& convex, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);

	bool GJKBoxCone(C_OBB& boxA, C_Cone& cone, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);

						//Sphere vs...

	bool GJKSphereCylinder(C_Sphere& sphere, C_Cylinder& cylinder, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);

	bool GJKSphereConvex(C_Sphere& sphere, C_ConvexHull& hull, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);

	bool GJKSphereCone(C_Sphere& sphere, C_Cone& cone, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);

						//Capsule vs...

	bool GJKCapsuleCapsule( C_Capsule& capsule, C_Capsule& capsule2, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);

	bool GJKCapsuleCylinder(C_Capsule& capsule, C_Cylinder& cylinder, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);

	bool GJKCapsuleConvex(C_Capsule& capsule, C_ConvexHull& hull, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);

	bool GJKCapsuleCone(C_Capsule& capsule, C_Cone& cone, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);

						//Cylinder vs...

	bool GJKCylinderCylinder(C_Cylinder& cylinderA, C_Cylinder& cylinderB, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);

	bool GJKCylinderConvex(C_Cylinder& cylinder, C_ConvexHull& hull, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);

	bool GJKCylinderCone(C_Cylinder& cylinder, C_Cone& cone, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);


						//Convex vs...

	bool GJKConvexConvex(C_ConvexHull& hull, C_ConvexHull& hullB, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);

	/*bool GJKConvexConvex(const C_SpuConvexHull& hull, const C_SpuConvexHull& hullB, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);*/

	bool GJKConvexCone(C_ConvexHull& hull, C_Cone& cone, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);


						//Cone vs...
	bool GJKConeCone(C_Cone& cone1, C_Cone& cone2, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa);


	/*
							For all objects collision detections
	*/
						//Box vs...
	void GJKBoxCapsule(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);
	
	void GJKBoxCylinder(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);
	
	void GJKBoxConvex(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);

	void GJKBoxCone(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res,
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);



						//Sphere vs...
	
	void GJKSphereCylinder(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);

	void GJKSphereConvex(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);

	void GJKSphereCone(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);


						//Capsule vs...

	void GJKCapsuleCapsule(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa);

	void GJKCapsuleCharacter(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverese);
	
	void GJKCapsuleCylinder(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);

	void GJKCapsuleConvex(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);

	void GJKCapsuleCone(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);

	
						//Cylinder vs...
	
	void GJKCylinderCylinder(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);

	void GJKCylinderConvex(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);

	void GJKCylinderCone(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);
	

						//Convex vs...
	
	void GJKConvexConvex(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa);

	void GJKConvexCone(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);

					   //Cone vs Cone
	void GJKConeCone(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);


						//CharacterController vs....
	/*
		The implementation is in GJK_Box.cpp
	*/
	void GJKCharacterControllerBox(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);
	
	/*
		The implementation is in GJKCollisionSpu.cpp
	*/
	void GJKCharacterControllerCylinder(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
	SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);

	void GJKCharacterControllerConvex(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);

	void GJKCharacterControllerCone(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);

	void GJKCharacterControllerCharacterController(C_CollisionManager& manager, C_CollisionObject* pObjA, C_CollisionObject* pObjB, C_CollisionResult<_MaxPatches, _MaxContacts>& res, 
		SharedPhysics::C_ContactStream& stream, EPA& m_Epa, bool reverse);

}
#endif
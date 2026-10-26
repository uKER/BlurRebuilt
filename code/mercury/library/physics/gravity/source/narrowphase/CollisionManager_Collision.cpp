#include "StdAfx.h"

#include "gravity/narrowphase/CollisionManager.h"
#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/OBBOBB.h"
#include "gravity/narrowphase/OBBSphere.inl"
#include "gravity/narrowphase/OBBRay.h"
#include "gravity/narrowphase/CapsuleCapsule.inl"
#include "gravity/narrowphase/CapsuleSphere.inl"
#include "gravity/narrowphase/CapsuleRay.h"
#include "gravity/narrowphase/SphereRay.inl"
#include "gravity/narrowphase/SphereSphere.inl"
#include "gravity/narrowphase/CylinderRay.h"
#include "gravity/narrowphase/ConvexHullRay.h"
#include "gravity/narrowphase/CompositeRay.h"
#include "gravity/narrowphase/primitive.h"
#include "gravity/narrowphase/primitive.inl"
#include "gravity/narrowphase/GJK.h"
#include "gravity/narrowphase/GJK.inl"
#include "gravity/narrowphase/RayTriangle.inl"
#include "gravity/narrowphase/OptimisedAABBTree.inl"
#include "gravity/narrowphase/CollisionResult.inl"
#include "gravity/narrowphase/CollisionManager.inl"
#include "gravity/narrowphase/ConvexHull.h"
#include "gravity/narrowphase/TriangleTriangle.inl"
#include "gravity/narrowphase/ConvexTriangle.h"
#include "gravity/narrowphase/CompositeShapeCollision.inl"
#include "gravity/narrowphase/CylinderTriangle.h"
#include "gravity/narrowphase/CharacterControllerPrimitive.h"
#include "gravity/narrowphase/CharacterControllerTriangle.inl"
#include "gravity/narrowphase/CompositeShapeTriangle.h"
#include "gravity/narrowphase/GJKCollision.h"
#include "gravity/narrowphase/InstanceMeshCollision.h"
#include "gravity/narrowphase/SpuCollisionResource.h"
#include "gravity/narrowphase/CollisionFilter.inl"
#include "gravity/narrowphase/AxisBox.inl"

#include "gravity/sharedphysics/ContactStream.inl"
#include "gravity/sharedphysics/MemoryAllocator.inl"
#include "gravity/sharedphysics/Select.h"

#include "system/perf.h"

#include <core/maths/hvec/VectorInt.h>
#include <core/maths/hvec/VectorInt.inl>


namespace Collision
{


void C_CollisionManager::Collide(Collision::C_CollisionObject * a, 
													   Collision::C_CollisionObject * b, SharedPhysics::C_ContactStream& stream, 
													   SharedPhysics::C_SafeStack<true>& stack,
													   Collision::C_CollisionResult<_MaxPatches, _MaxContacts>& results,
													   EPA& m_Epa)
{
	
	Spu::MemBaseMarker outerMarker;
	PERF_EVENT("gravity.dynamiccollision");
	results.m_NumContactPatches = 0;
	//Collision::C_CollisionObject* a=(Collision::C_CollisionObject*) Spu::FetchAlloc(aObject, sizeof(Collision::C_CollisionObject));
	//Collision::C_CollisionObject* b=(Collision::C_CollisionObject*) Spu::FetchAlloc(bObject, sizeof(Collision::C_CollisionObject));
	//SharedPhysics::C_GraphNode* pNode=(SharedPhysics::C_GraphNode*) Spu::FetchAlloc(a->m_pNodePtr, sizeof(SharedPhysics::C_GraphNode));
	//SharedPhysics::C_ContactDatabase * pDatabase = (SharedPhysics::C_ContactDatabase*)Spu::FetchAlloc(m_pContactDatabase, sizeof(SharedPhysics::C_ContactDatabase));

	//SharedPhysics::C_Collider* pOldCollider = m_pContactDatabase->FindPrevCollider(results->m_pNodeA, result->m_pNodeB, index);


	//DebugBreak();
	//assert(pNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active));
	U32 a_Shape=(E_ShapeType)Spu::Get((U32*)&a->m_pShape->m_ShapeType);
	U32 b_Shape=(E_ShapeType)Spu::Get((U32*)&b->m_pShape->m_ShapeType);

	if(b_Shape == E_Continuous_Shape)
	{
		b->m_pShape = (Collision::C_Shape*)Spu::Get((INTPTR*)&((Collision::C_ContinuousShape*)b->m_pShape)->m_pDiscreteShape);	// [Blur2PC][x64] pointer-wide
		b_Shape = Spu::Get((U32*)&b->m_pShape->m_ShapeType);
	}

	if(a_Shape == E_Continuous_Shape)
	{
		a->m_pShape = (Collision::C_Shape*)Spu::Get((INTPTR*)&((Collision::C_ContinuousShape*)a->m_pShape)->m_pDiscreteShape);	// [Blur2PC][x64] pointer-wide
		a_Shape = Spu::Get((U32*)&a->m_pShape->m_ShapeType);
	}

	//static C_CollisionResult<_MaxPatches, _MaxContacts> results;
	results.m_pObjectA = const_cast<C_CollisionObject*>(a);
	results.m_pObjectB = const_cast<C_CollisionObject*>(b);


	results.m_pEpa = &m_Epa;

	if(a_Shape == E_Composite_Shape)
	{

		CollideCompositeShapeResource((*this), a, b, results, stream, stack, m_Epa, false);
		return;
	}

	if(b_Shape == E_Composite_Shape)
	{
 		CollideCompositeShapeResource((*this), b, a, results, stream, stack, m_Epa, true);
		return;
	}

	/*if(a_Shape == E_InstancedTriangleMesh_Shape || b_Shape == E_InstancedTriangleMesh_Shape)
		return;*/

//	S32 index; 

	//Object a is box
	U32 shapeType(a_Shape|b_Shape);
	switch(shapeType)
	{
	case (E_Box_Shape | E_Box_Shape)://0x8: //E_Box_Shape & E_BoxShape
		{
			CollideBoxBoxResource((*this), a, b, results, stream, m_Epa);
		}
		return;
	case (E_ConvexHull_Shape | E_ConvexHull_Shape)://0x10:
		{
			//Need to do GJK/EPA with persistent contact manifold...
			GJKConvexConvex((*this), a, b, results, stream, m_Epa);
		}
		return;
	case (E_ConvexHull_Shape | E_Box_Shape)://0x18:
		{
			bool con((a_Shape == E_ConvexHull_Shape) & (b_Shape == E_Box_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			//Need to do GJK/EPA with persistent contact manifold...
			GJKBoxConvex((*this), pObjA, pObjB, results, stream, m_Epa, con);

			
			/*if((a_Shape == E_ConvexHull_Shape) & (b_Shape == E_Box_Shape))
				GJKBoxConvex((*this), b, a, results, stream, m_Epa, true);
			else
				GJKBoxConvex((*this), a, b, results, stream, m_Epa, false);*/
		}
		return;
	case (E_Sphere_Shape | E_Sphere_Shape)://0x20:
		{
			CollidedSphereSphereResource((*this), a, b, results, stream);
		}
		return;
	case (E_Sphere_Shape | E_Box_Shape)://0x28: //E_BoxShape & E_Sphere_Shape
		{
			bool con((a_Shape == E_Sphere_Shape) & (b_Shape == E_Box_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			CollideBoxSphereResource((*this), pObjA, pObjB, results, stream, con);


		/*	if((a_Shape==E_Box_Shape) & (b_Shape==E_Sphere_Shape))
				CollideBoxSphereResource((*this), a, b, results, stream, false);
			else
				CollideBoxSphereResource((*this), b, a, results, stream, true);*/
		}
		return;
	case ( E_Sphere_Shape | E_ConvexHull_Shape)://0x30:
		{

			bool con((a_Shape == E_ConvexHull_Shape) & (b_Shape == E_Sphere_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			GJKSphereConvex((*this), pObjA, pObjB, results, stream, m_Epa, con);


			////Need to do GJK/EPA with persistent contact manifold...
			//if((a_Shape == E_Sphere_Shape) & (b_Shape == E_ConvexHull_Shape))
			//	GJKSphereConvex((*this), a, b, results, stream, m_Epa, false);
			//else
			//	GJKSphereConvex((*this), b, a, results, stream, m_Epa, true);
		}
		return;

	case (E_Capsule_Shape | E_Capsule_Shape)://0x40:
		{
			CollidedCapsuleCapsuleResource((*this), a, b, results, stream);
		}
		return;
	case (E_Capsule_Shape | E_Box_Shape): //0x48: //E_Box_Shape & E_Capsule_Shape
		{

			bool con((a_Shape == E_Capsule_Shape) & (b_Shape == E_Box_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			GJKBoxCapsule((*this), pObjA, pObjB, results, stream, m_Epa, con);


		/*	if((a_Shape==E_Box_Shape) & (b_Shape==E_Capsule_Shape))
				GJKBoxCapsule((*this), a, b, results, stream, m_Epa, false);
			else
				GJKBoxCapsule((*this), b, a, results, stream, m_Epa, true);*/
		}
		return;
	case ( E_Capsule_Shape | E_ConvexHull_Shape)://0x50:
		{
			bool con((a_Shape == E_ConvexHull_Shape) & (b_Shape == E_Capsule_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			GJKCapsuleConvex((*this), pObjA, pObjB, results, stream, m_Epa, con);

			/*if((a_Shape == E_Capsule_Shape) & (b_Shape == E_ConvexHull_Shape))
				GJKCapsuleConvex((*this), a, b, results, stream, m_Epa, false);
			else
				GJKCapsuleConvex((*this), b, a, results, stream, m_Epa, true);*/
		}
		return;
	case (E_Capsule_Shape | E_Sphere_Shape): //0x60:
		{
			bool con((a_Shape == E_Capsule_Shape) & (b_Shape == E_Sphere_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			CollidedSphereCapsuleResource((*this), pObjA, pObjB, results, stream, con);

			/*if((a_Shape==E_Sphere_Shape) & (b_Shape==E_Capsule_Shape))
				CollidedSphereCapsuleResource((*this), a, b, results, stream, false);
			else
				CollidedSphereCapsuleResource((*this), b, a, results, stream, true);*/
		}
		return;
	
	case (E_Cylinder_Shape | E_Cylinder_Shape)://0x80:
		{
			GJKCylinderCylinder((*this), a, b, results, stream, m_Epa, false);
		}
		return;
	case (E_Cylinder_Shape | E_Box_Shape): //0x88:
		{
			bool con((a_Shape == E_Cylinder_Shape) & (b_Shape == E_Box_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			GJKBoxCylinder((*this), pObjA, pObjB, results, stream, m_Epa, con);


			/*if((a_Shape == E_Cylinder_Shape) & (b_Shape == E_Box_Shape))
				GJKBoxCylinder((*this), b, a, results, stream, m_Epa, true);
			else
				GJKBoxCylinder((*this), a, b, results, stream, m_Epa, false);*/
		}
		return;
	case ( E_Cylinder_Shape | E_ConvexHull_Shape ): //0x90:
		{
			bool con((a_Shape == E_ConvexHull_Shape) & (b_Shape == E_Cylinder_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			GJKCylinderConvex((*this), pObjA, pObjB, results, stream, m_Epa, con);

			/*if((a_Shape == E_Cylinder_Shape) & (b_Shape == E_ConvexHull_Shape))
				GJKCylinderConvex((*this), a, b, results, stream, m_Epa, false);
			else
				GJKCylinderConvex((*this), b, a, results, stream, m_Epa, true);*/
		}
		return;
	case (E_Cylinder_Shape | E_Sphere_Shape): //0xA0:
		{
			bool con((a_Shape == E_Cylinder_Shape) & (b_Shape == E_Sphere_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			GJKSphereCylinder((*this), pObjA, pObjB, results, stream, m_Epa, con);


			/*if((a_Shape == E_Sphere_Shape) & (b_Shape == E_Cylinder_Shape))
				GJKSphereCylinder((*this), a, b, results, stream, m_Epa, false);
			else
				GJKSphereCylinder((*this), b, a, results, stream, m_Epa, true);*/
		}
		return;
	case (E_Cylinder_Shape | E_Capsule_Shape): //0xC0:
		{
			bool con((a_Shape == E_Cylinder_Shape) & (b_Shape == E_Capsule_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			GJKCapsuleCylinder((*this), pObjA, pObjB, results, stream, m_Epa, con);

			/*if((a_Shape == E_Cylinder_Shape) & (b_Shape == E_Capsule_Shape))
				GJKCapsuleCylinder((*this), b, a, results, stream, m_Epa, true);
			else
				GJKCapsuleCylinder((*this), a, b, results, stream, m_Epa, false);*/
		}
		return;
	
	case ( E_CharacterController_Shape | E_Box_Shape )://0x208:
		{
			bool con((a_Shape == E_Box_Shape) & (b_Shape == E_CharacterController_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			GJKCharacterControllerBox((*this), pObjA, pObjB, results, stream, m_Epa, con);


			/*if((a_Shape == E_CharacterController_Shape) && (b_Shape == E_Box_Shape))
				GJKCharacterControllerBox((*this), a, b, results, stream, m_Epa, false);
			else
				GJKCharacterControllerBox((*this), b, a, results, stream, m_Epa, true);*/
		}
		return;
	case ( E_CharacterController_Shape | E_ConvexHull_Shape ): //0x210:
		{
			bool con((a_Shape == E_ConvexHull_Shape) & (b_Shape == E_CharacterController_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			GJKCharacterControllerConvex((*this), pObjA, pObjB, results, stream,m_Epa, con);

			/*if((a_Shape == E_CharacterController_Shape) & (b_Shape == E_ConvexHull_Shape))
				GJKCharacterControllerConvex((*this), a, b, results, stream, m_Epa, false);
			else
				GJKCharacterControllerConvex((*this), b, a, results, stream,m_Epa, true);*/
		}
		return;
	case (E_CharacterController_Shape | E_Sphere_Shape): //0x220:
		{
			bool con((a_Shape == E_CharacterController_Shape) & (b_Shape == E_Sphere_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			CollidedSphereCharacterControllerResource((*this), pObjA, pObjB, results, stream, con);


			/*if((a_Shape==E_Sphere_Shape) & (b_Shape==E_CharacterController_Shape))
				CollidedSphereCharacterControllerResource((*this), a, b, results, stream, false);
			else
				CollidedSphereCharacterControllerResource((*this), b, a, results, stream, true);*/
		}
		return;
	case ( E_CharacterController_Shape | E_Capsule_Shape ): //0x240:
		{
			bool con((a_Shape == E_Capsule_Shape) & (b_Shape == E_CharacterController_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			CollidedCharacterControllerCapsuleResource((*this), pObjA, pObjB, results, stream, con);

			/*if((a_Shape == E_Capsule_Shape) & (b_Shape == E_CharacterController_Shape))
				CollidedCharacterControllerCapsuleResource((*this),b, a, results, stream, true);
			else
				CollidedCharacterControllerCapsuleResource((*this), a, b, results, stream, false);*/
		}
		return;
	case (E_CharacterController_Shape | E_Cylinder_Shape): //0x280:
		{
			bool con((a_Shape == E_Cylinder_Shape) & (b_Shape == E_CharacterController_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			GJKCharacterControllerCylinder((*this), pObjA, pObjB, results, stream, m_Epa, con);


			/*if((a_Shape == E_CharacterController_Shape) && (b_Shape == E_Cylinder_Shape))
				GJKCharacterControllerCylinder((*this), a, b, results, stream, m_Epa, false);
			else
				GJKCharacterControllerCylinder((*this), b, a, results, stream, m_Epa, true);*/
		}
		return;
	case (E_CharacterController_Shape | E_CharacterController_Shape)://0x200:
		{
			CollidedCharacterControllersResource((*this), a, b, results, stream);
		}
		return;
	case (E_InstancedTriangleMesh_Shape | E_InstancedTriangleMesh_Shape): //0x800
		{
			BIZ_ASSERT("Should not go here");
		}
		return;
	case ( E_InstancedTriangleMesh_Shape | E_Box_Shape )://0x808:
		{
			//if((a_Shape == E_Box_Shape) & (b_Shape == E_InstancedTriangleMesh_Shape))
			InstanceMeshBox((*this),b, a, results, stream, stack);
		}
		return;
	case ( E_InstancedTriangleMesh_Shape | E_ConvexHull_Shape ): //0x810:
		{
			//if((a_Shape == E_ConvexHull_Shape) & (b_Shape == E_InstancedTriangleMesh_Shape))
			InstanceMeshConvexHull((*this),b, a, results, stream, stack);
		}
		return;
	case ( E_InstancedTriangleMesh_Shape | E_Sphere_Shape ): //0x820:
		{
			//if((a_Shape == E_Sphere_Shape) & (b_Shape == E_InstancedTriangleMesh_Shape))
			InstanceMeshSphere((*this),b, a, results, stream, stack);
		}
		return;
	case ( E_InstancedTriangleMesh_Shape | E_Capsule_Shape )://0x840:
		{
			//if((a_Shape == E_Capsule_Shape) & (b_Shape == E_InstancedTriangleMesh_Shape))
			InstanceMeshCapsule((*this),b, a, results, stream, stack);
		}
		return;
	case ( E_InstancedTriangleMesh_Shape | E_Cylinder_Shape ): //0x880:
		{
			//if((a_Shape == E_Cylinder_Shape) & (b_Shape == E_InstancedTriangleMesh_Shape))
			InstanceMeshCylinder((*this),b, a, results, stream, stack);
		}
		return;
	case ( E_InstancedTriangleMesh_Shape | E_CharacterController_Shape ): //0xA00:
		{
			//if((a_Shape == E_CharacterController_Shape) & (b_Shape == E_InstancedTriangleMesh_Shape))
			InstanceMeshCharacterController((*this),b, a, results, stream, stack);
		}
		return;
	case (E_Cloth_Shape | E_Cloth_Shape):
		{
		}
		return;
	case (E_Cone_Shape | E_Cone_Shape)://0x10000:
		{
			GJKConeCone((*this), a, b, results, stream, m_Epa, false);
		}
		return;
	case (E_Cone_Shape | E_Box_Shape): //0x10008: //E_Box_Shape & E_Cone_Shape
		{
			bool con((a_Shape == E_Cone_Shape) & (b_Shape == E_Box_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			GJKBoxCone((*this), pObjA, pObjB, results, stream, m_Epa, con);

			/*if((a_Shape==E_Box_Shape) & (b_Shape == E_Cone_Shape))
				GJKBoxCone((*this), a, b, results, stream, m_Epa, false);
			else
				GJKBoxCone((*this), b, a, results, stream, m_Epa, true);*/
			
		}
		return;
	case (E_Cone_Shape | E_ConvexHull_Shape): //0x10010:
		{
			bool con((a_Shape == E_Cone_Shape) & (b_Shape == E_ConvexHull_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			GJKConvexCone((*this), pObjA, pObjB, results, stream, m_Epa, con);

			/*if((a_Shape == E_ConvexHull_Shape) & (b_Shape == E_Cone_Shape))
				GJKConvexCone((*this), a, b, results, stream, m_Epa, false);
			else
				GJKConvexCone((*this), b, a, results, stream, m_Epa, true);*/
		}
		return;
	case (E_Cone_Shape | E_Sphere_Shape): //0x10020:
		{

			bool con((a_Shape == E_Cone_Shape) & (b_Shape == E_Sphere_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			GJKSphereCone((*this), pObjA, pObjB, results, stream, m_Epa, con);

			/*if((a_Shape == E_Sphere_Shape) & (b_Shape == E_Cone_Shape))
				GJKSphereCone((*this), a, b, results, stream, m_Epa, false);
			else
				GJKSphereCone((*this), b, a, results, stream, m_Epa, true);*/
		}
		return;
	case (E_Cone_Shape |  E_Capsule_Shape): //0x10040:
		{

			bool con((a_Shape == E_Cone_Shape) & (b_Shape == E_Capsule_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			GJKCapsuleCone((*this), pObjA, pObjB, results, stream, m_Epa, con);

		/*	if((a_Shape == E_Capsule_Shape) & (b_Shape == E_Cone_Shape))
				GJKCapsuleCone((*this), a, b, results, stream, m_Epa, false);
			else
				GJKCapsuleCone((*this), b, a, results, stream, m_Epa, true);*/
		}
		return;
	case ( E_Cone_Shape | E_Cylinder_Shape): //0x10080:
		{
			bool con((a_Shape == E_Cone_Shape) & (b_Shape == E_Cylinder_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			GJKCylinderCone((*this), pObjA, pObjB, results, stream, m_Epa, con);


			/*if((a_Shape == E_Cylinder_Shape) & (b_Shape == E_Cone_Shape))
				GJKCylinderCone((*this), a, b, results, stream, m_Epa, false);
			else
				GJKCylinderCone((*this), b, a, results, stream, m_Epa, true);*/
		}
		return;
	case (E_Cone_Shape | E_CharacterController_Shape): //0x10200:
		{
			bool con((a_Shape == E_Cone_Shape) & (b_Shape == E_CharacterController_Shape));
			Collision::C_CollisionObject * pObjA = SharedPhysics::Fast_Select(con, b, a);
			Collision::C_CollisionObject * pObjB = SharedPhysics::Fast_Select(con, a, b);

			GJKCharacterControllerCone((*this), pObjA, pObjB, results, stream, m_Epa, con);


		/*	if((a_Shape == E_Cone_Shape) & (b_Shape == E_CharacterController_Shape))
				GJKCharacterControllerCone((*this), b, a, results, stream, m_Epa, true);
			else
				GJKCharacterControllerCone((*this), a, b, results, stream, m_Epa, false);*/

		}
		return;
	case (E_Cone_Shape | E_InstancedTriangleMesh_Shape): //0x10800
		{
			InstanceMeshCone((*this),b, a, results, stream, stack);
		}
		return;
	}

	return;
}



/*
	Both pointer a and b are PPU pointer
*/
bool C_CollisionManager::Collide(const C_Shape* const _a, const C_Shape*  const _b,  Collision::C_CollisionResult<_MaxPatches, _MaxContacts>& results,
								 SharedPhysics::C_SafeStack<true>& stack, EPA& m_Epa, bool bFlip, Collision::C_ContactPatch<_MaxContacts>& patch)
{
//#ifndef PLATFORM_SPU
	
	Spu::MemBaseMarker marker;
	//const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());

	C_Shape* a = SharedPhysics::Fast_Select<C_Shape>(bFlip, _b, _a);
	C_Shape* b = SharedPhysics::Fast_Select<C_Shape>(bFlip, _a, _b);
	
	U32 a_Shape=a->m_ShapeType;
	U32 b_Shape=b->m_ShapeType;


	//Can't happen - already been checked and avoided above
	/*if(b_Shape == E_Continuous_Shape)
	{
		b = (Collision::C_Shape*)Spu::Get((U32*)&((Collision::C_ContinuousShape*)b)->m_pDiscreteShape);
		b_Shape = Spu::Get((U32*)&b->m_ShapeType);
	}*/

	//if(a_Shape == E_Continuous_Shape)
	//{
	//	a = (Collision::C_Shape*)Spu::Get((U32*)&((Collision::C_ContinuousShape*)a)->m_pDiscreteShape);
	//	a_Shape = Spu::Get((U32*)&a->m_ShapeType);
	//}


	U32 a_PhysicsMaterialIndex=a->m_PhysicsMaterialIndex;
	U32 b_PhysicsMaterialIndex=b->m_PhysicsMaterialIndex;

	U32 a_EffectMaterialIndex=a->m_EffectMaterialIndex;
	U32 b_EffectMaterialIndex=b->m_EffectMaterialIndex;

	results.m_pEpa = &m_Epa;
	
	const U32 physicsMaterial(a_PhysicsMaterialIndex << 16 | b_PhysicsMaterialIndex);
	const U32 effectMaterial(a_EffectMaterialIndex << 16 | b_EffectMaterialIndex);
	
	
	//Temporary hack for Nightwing performance!
	/*if(a_Shape == E_InstancedTriangleMesh_Shape || b_Shape == E_InstancedTriangleMesh_Shape)
		return false;*/
	
	
	
	//Composite - composite - in case we have recursive composite descriptions!
	if(a_Shape == E_Composite_Shape)
	{

		if(CollideCompositeShape(*this, a, b, results, stack, m_Epa, false, patch))
		{
			return true;
		}
		return false;
	}

	if(b_Shape == E_Composite_Shape)
	{
		//Do composite - composite collision detection!
		U32 numPatches = results.m_NumContactPatches;
		if(CollideCompositeShape((*this), b, a, results, stack, m_Epa, true, patch))
		{
			//ReverseContacts(results);
			return true;
		}
		return false;
	}

	U32 shapeType(a_Shape|b_Shape);
	switch(shapeType)
	{
	case (E_Box_Shape | E_Box_Shape)://0x8
		{
			C_BoxShape* aBox = (C_BoxShape*)a;
			C_BoxShape* bBox = (C_BoxShape*)b;
			if(CollideBoxBox(aBox->m_Box, bBox->m_Box, patch, physicsMaterial, effectMaterial, m_Epa))
			{
				results.AddPatch(patch);
				return true;
			}
			return false;
		}
	case (E_ConvexHull_Shape | E_ConvexHull_Shape)://0x10
		{
			C_ConvexHullShape* pConv1 = (C_ConvexHullShape*)a;
			C_ConvexHullShape* pConv2 = (C_ConvexHullShape*)b;

			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			if(GJKConvexConvex(pConv1->m_Hull, pConv2->m_Hull, 
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				results.AddPatch(patch);
				return true;
			}
			return false;
		}
	case (E_ConvexHull_Shape | E_Box_Shape  )://0x18
		{
			bool con((a_Shape == E_Box_Shape) & (b_Shape == E_ConvexHull_Shape));
			C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);
			
			C_BoxShape* pBox = (C_BoxShape*)aShape;
			C_ConvexHullShape* pConv = (C_ConvexHullShape*)bShape;
			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			if(GJKBoxConvex(pBox->m_Box, pConv->m_Hull, 
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;
			
		}
	case (E_Sphere_Shape | E_Sphere_Shape)://0x20
		{
			C_SphereShape* aSphere=(C_SphereShape*)a;
			C_SphereShape* bSphere=(C_SphereShape*)b;
			if(CollideSphereSphere(aSphere->m_Sphere, bSphere->m_Sphere, patch, physicsMaterial, effectMaterial))
			{
				results.AddPatch(patch);
				return true;
			}
			return false;

		}
	case (E_Sphere_Shape | E_Box_Shape )://0x28
		{
			bool con((a_Shape==E_Box_Shape) & (b_Shape==E_Sphere_Shape));
			Collision::C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			Collision::C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);
	
			C_BoxShape* pBox=(C_BoxShape*)aShape;
			C_SphereShape* pSphere=(C_SphereShape*)bShape;
			if(CollideOBBSphere(pBox->m_Box, pSphere->m_Sphere, patch, physicsMaterial, effectMaterial))
			{
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;
		}
	case(E_Sphere_Shape | E_ConvexHull_Shape)://0x30
		{
			bool con((a_Shape == E_Sphere_Shape) & (b_Shape == E_ConvexHull_Shape));
			C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);
			
			C_SphereShape* pSphere = (C_SphereShape*)aShape;
			C_ConvexHullShape* pConv = (C_ConvexHullShape*)bShape;
			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			if(GJKSphereConvex(pSphere->m_Sphere, pConv->m_Hull, 
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;
			
		}
	case (E_Capsule_Shape|E_Capsule_Shape)://0x40
		{
			
			C_CapsuleShape* aCapsule=(C_CapsuleShape*)a;
			C_CapsuleShape* bCapsule=(C_CapsuleShape*)b;

			if(CollideCapsuleCapsule(aCapsule->m_Capsule, bCapsule->m_Capsule, patch, physicsMaterial, effectMaterial))
			{
				results.AddPatch(patch);
				return true;
			}
			return false;
		}
	case (E_Capsule_Shape | E_Box_Shape )://0x48
		{

			bool con((a_Shape==E_Box_Shape) & (b_Shape==E_Capsule_Shape));
			Collision::C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			Collision::C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);

			C_BoxShape* pBox=(C_BoxShape*)aShape;
			C_CapsuleShape* pCapsule=(C_CapsuleShape*)bShape;
			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			if(GJKBoxCapsule(pBox->m_Box, pCapsule->m_Capsule, 
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;
		}
	case (E_Capsule_Shape | E_ConvexHull_Shape)://0x50
		{
			bool con((a_Shape == E_Capsule_Shape) & (b_Shape == E_ConvexHull_Shape));
			C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);
			
			C_CapsuleShape* pCap = (C_CapsuleShape*)aShape;
			C_ConvexHullShape* pConv = (C_ConvexHullShape*)bShape;
			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			if( GJKCapsuleConvex(pCap->m_Capsule, pConv->m_Hull, 
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;
		
		}
	case (E_Capsule_Shape | E_Sphere_Shape)://0x60
		{
			bool con((a_Shape==E_Capsule_Shape) & (b_Shape==E_Sphere_Shape));
		
			Collision::C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			Collision::C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);

			C_CapsuleShape* pCapsule=(C_CapsuleShape*)aShape;
			C_SphereShape* pSphere=(C_SphereShape*)bShape;

			if(CollideCapsuleSphere(pCapsule->m_Capsule, pSphere->m_Sphere, patch, physicsMaterial, effectMaterial))
			{
				//ReverseContacts(results);
				//patch.m_Normal = -patch.m_Normal;
				patch.m_Normal = Maths::Select(con, -patch.m_Normal, patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;
		}
	case (E_Cylinder_Shape | E_Cylinder_Shape)://0x80
		{
			bool con((a_Shape == E_Cylinder_Shape) & (b_Shape == E_Cylinder_Shape));
			C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);
			
			C_CylinderShape* pCyl1 = (C_CylinderShape*)aShape;
			C_CylinderShape* pCyl2 = (C_CylinderShape*)bShape;
			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			if(GJKCylinderCylinder(pCyl1->m_Cylinder, pCyl2->m_Cylinder, 
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				results.AddPatch(patch);
				return true;
			}
			return false;
		}
	case ( E_Cylinder_Shape | E_Box_Shape )://0x88
		{
			bool con((a_Shape == E_Box_Shape) & (b_Shape == E_Cylinder_Shape));
			C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);
			
			C_BoxShape* pBox = (C_BoxShape*)aShape;
			C_CylinderShape* pCyl = (C_CylinderShape*)bShape;
			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			if( GJKBoxCylinder(pBox->m_Box, pCyl->m_Cylinder, 
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;	
		}
	case (E_Cylinder_Shape | E_ConvexHull_Shape)://0x90
		{

			bool con((a_Shape == E_Cylinder_Shape) & (b_Shape == E_ConvexHull_Shape));
			C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);
			
			C_CylinderShape* pCyl = (C_CylinderShape*)aShape;
			C_ConvexHullShape* pConv = (C_ConvexHullShape*)bShape;
			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			if(GJKCylinderConvex(pCyl->m_Cylinder, pConv->m_Hull, 
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;
		}
	case (E_Cylinder_Shape | E_Sphere_Shape )://0xA0
		{
			bool con((a_Shape == E_Sphere_Shape) & (b_Shape == E_Cylinder_Shape));

			C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);

			C_SphereShape* pSphere = (C_SphereShape*)aShape;
			C_CylinderShape* pCyl = (C_CylinderShape*)bShape;
			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			if(GJKSphereCylinder(pSphere->m_Sphere, pCyl->m_Cylinder, 
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;	
		}
	case (E_Cylinder_Shape | E_Capsule_Shape )://0xC0
		{
			bool con((a_Shape == E_Capsule_Shape) & (b_Shape == E_Cylinder_Shape));
			C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);
			
			C_CapsuleShape* pCap = (C_CapsuleShape*)aShape;
			C_CylinderShape* pCyl = (C_CylinderShape*)bShape;
			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			if(GJKCapsuleCylinder(pCap->m_Capsule, pCyl->m_Cylinder, 
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;
		}
		//Can't happen - character controllers won't be part of composite shapes
	//case (E_CharacterController_Shape | E_CharacterController_Shape)://0x200
	//	{
	//		C_CharacterControllerShape* pCharacter1=(C_CharacterControllerShape*)Spu::FetchAlloc(a, sizeof(C_CharacterControllerShape));
	//		C_CharacterControllerShape* pCharacter2=(C_CharacterControllerShape*)Spu::FetchAlloc(b, sizeof(C_CharacterControllerShape));

	//		if(CollideCapsuleCapsule(pCharacter1->m_Controller.m_EnclosingCapsule, pCharacter2->m_Controller.m_EnclosingCapsule, patch, physicsMaterial, effectMaterial))
	//		{
	//			results.AddPatch(patch);
	//			return true;
	//		}
	//		return false;
	//	}
	case ( E_CharacterController_Shape | E_Box_Shape )://0x208
		{
			//Character controller can't be part of composite shape, so must be the b-shape
			//BIZ_ASSERT(b_Shape == E_CharacterController_Shape);
			bool con((a_Shape == E_Box_Shape) & (b_Shape == E_CharacterController_Shape));
			C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);

			C_BoxShape* pBox=(C_BoxShape*)aShape;
			C_CharacterControllerShape* pCharacter=(C_CharacterControllerShape*)bShape;
			
			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			//CollideOBBRay(pBox, pCharacter->m_Controller.m_Ray);

			if(GJKBoxCapsule(pBox->m_Box, pCharacter->m_Controller.m_EnclosingCapsule, 
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;

		}
	case (E_CharacterController_Shape | E_ConvexHull_Shape)://0x210
		{
			//BIZ_ASSERT(b_Shape == E_CharacterController_Shape);
			bool con((a_Shape == E_CharacterController_Shape) & (b_Shape == E_ConvexHull_Shape));
			C_Shape* aShape  = SharedPhysics::Fast_Select(con, a, b);
			C_Shape* bShape  = SharedPhysics::Fast_Select(con, b, a);

			C_CharacterControllerShape* pCharacter=(C_CharacterControllerShape*)aShape;
			C_ConvexHullShape* pConv=(C_ConvexHullShape*)bShape;
			
			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			if(GJKCapsuleConvex(pCharacter->m_Controller.m_EnclosingCapsule, pConv->m_Hull, 
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;
		}
	case ( E_CharacterController_Shape | E_Sphere_Shape )://0x220
		{
			//BIZ_ASSERT(b_Shape == E_CharacterController_Shape);
			bool con((a_Shape==E_Sphere_Shape) & (b_Shape==E_CharacterController_Shape));
			C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);

			C_SphereShape* pSphere=(C_SphereShape*)aShape;
			C_CharacterControllerShape* pCharacter=(C_CharacterControllerShape*)bShape;

			if(CollideCapsuleSphere( pCharacter->m_Controller.m_EnclosingCapsule, pSphere->m_Sphere, patch, physicsMaterial, effectMaterial))
			{
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;

		}
	case ( E_CharacterController_Shape | E_Capsule_Shape )://0x240
		{
			//BIZ_ASSERT(b_Shape == E_CharacterController_Shape);
			bool con((a_Shape == E_Capsule_Shape) & (b_Shape == E_CharacterController_Shape));
			C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);

			C_CapsuleShape* pCapsule=(C_CapsuleShape*)aShape;
			C_CharacterControllerShape* pCharacter=(C_CharacterControllerShape*)bShape;
			

			if(CollideCapsuleCapsule(pCapsule->m_Capsule, pCharacter->m_Controller.m_EnclosingCapsule, patch, physicsMaterial, effectMaterial))
			{
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;
		}
		case (E_CharacterController_Shape | E_Cylinder_Shape)://0x280
		{

			//BIZ_ASSERT(b_Shape == E_CharacterController_Shape);
			bool con((a_Shape == E_CharacterController_Shape) & (b_Shape == E_Cylinder_Shape));
			C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);

			C_CharacterControllerShape* pCharacter=(C_CharacterControllerShape*)aShape;
			C_CylinderShape* pCyl=(C_CylinderShape*)bShape;
			
			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			if(GJKCapsuleCylinder(pCharacter->m_Controller.m_EnclosingCapsule, pCyl->m_Cylinder, 
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;
		}
	case (E_InstancedTriangleMesh_Shape | E_InstancedTriangleMesh_Shape)://0x800
		{
			BIZ_ASSERT("Should not go here");
			return false;
		}
	case (E_InstancedTriangleMesh_Shape | E_Box_Shape)://0x808
		{
			//NB - we only ever !!!EVER!!! need to do the collision this way around, because we will only
			//be colliding against instanced level mesh or kinematic mesh!
			if((a_Shape == E_Box_Shape) & (b_Shape == E_InstancedTriangleMesh_Shape))
			{
				C_InstancedTriangleMeshShape* pTree=(C_InstancedTriangleMeshShape*)b;
				C_BoxShape* pBox=(C_BoxShape*)a;

				C_BoxProxy proxy(pBox->m_Box, pTree->m_PosRot);
				C_CollisionObject obj(pBox, a->m_pPpuPtr);
#ifndef PLATFORM_SPU
				return ( pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_BoxProxy, CollideOBBTriangleMesh<_MaxPatches, 
					_MaxContacts, Collision::AABBTreeTriangle> >(&obj, proxy, results, stack));
#else
				return ( pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_BoxProxy >(&obj, proxy, results, stack, CollideOBBTriangleMesh<_MaxPatches, 
					_MaxContacts, Collision::AABBTreeTriangle>));
			
#endif
			}
			return false;
		}
	case (E_InstancedTriangleMesh_Shape | E_ConvexHull_Shape)://0x810
		{
			//NB - we only ever !!!EVER!!! need to do the collision this way around, because we will only
			//be colliding against instanced level mesh or kinematic mesh!
			if((a_Shape == E_ConvexHull_Shape) & (b_Shape == E_InstancedTriangleMesh_Shape ))
			{
				C_InstancedTriangleMeshShape* pTree=(C_InstancedTriangleMeshShape*)b;
				C_ConvexHullShape* pConv=(C_ConvexHullShape*)a;
				C_ConvexHullProxy proxy(pConv->m_Hull, pTree->m_PosRot);
				C_CollisionObject obj(pConv, a->m_pPpuPtr);
#ifndef PLATFORM_SPU
				return( pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_ConvexHullProxy, CollideConvexHullTriangleMesh<
					_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle> >(&obj, proxy, results, stack));
#else
				return( pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_ConvexHullProxy>(&obj, proxy, results, stack, CollideConvexHullTriangleMesh<
					_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle> ));
#endif
			}
			return false;
		}
	case (E_InstancedTriangleMesh_Shape | E_Sphere_Shape )://0x820
		{
			//NB - we only ever !!!EVER!!! need to do the collision this way around, because we will only
			//be colliding against instanced level mesh or kinematic mesh!
			if((a_Shape == E_Sphere_Shape) & (b_Shape == E_InstancedTriangleMesh_Shape))
			{
				C_InstancedTriangleMeshShape* pTree=(C_InstancedTriangleMeshShape*)b;
				C_SphereShape* pSphere=(C_SphereShape*)a;
				C_SphereProxy proxy(pSphere->m_Sphere, pTree->m_PosRot);
				C_CollisionObject obj(pSphere, a->m_pPpuPtr);

#ifndef PLATFORM_SPU
				return ( pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_SphereProxy, CollideSphereTriangleMesh<
					_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle> >(&obj, proxy, results, stack));
#else
				return ( pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_SphereProxy >(&obj, proxy, results, stack, CollideSphereTriangleMesh<
					_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle>));
#endif
			}
			return false;
		}
	case (E_InstancedTriangleMesh_Shape | E_Capsule_Shape)://0x840
		{
			//NB - we only ever !!!EVER!!! need to do the collision this way around, because we will only
			//be colliding against instanced level mesh or kinematic mesh!
			if((a_Shape == E_Capsule_Shape) & (b_Shape == E_InstancedTriangleMesh_Shape))
			{
				C_InstancedTriangleMeshShape* pTree=(C_InstancedTriangleMeshShape*)b;
				C_CapsuleShape* pCap=(C_CapsuleShape*)a;

				C_CollisionObject obj(pCap, a->m_pPpuPtr);
				C_CapsuleProxy proxy(pCap->m_Capsule, pTree->m_PosRot);
#ifndef PLATFORM_SPU
				return ( pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_CapsuleProxy, 
					CollideCapsuleTriangleMesh<_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle> >(&obj, proxy, results, stack));
#else
				return ( pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_CapsuleProxy >(&obj, proxy, results, stack,
					CollideCapsuleTriangleMesh<_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle>));
#endif
			}
			return false;
		}
	case (E_InstancedTriangleMesh_Shape | E_Cylinder_Shape)://0x880
		{
			//NB - we only ever !!!EVER!!! need to do the collision this way around, because we will only
			//be colliding against instanced level mesh or kinematic mesh!
			if((a_Shape == E_Cylinder_Shape) & (b_Shape == E_InstancedTriangleMesh_Shape))
			{
				C_InstancedTriangleMeshShape* pTree=(C_InstancedTriangleMeshShape*)b;
				C_CylinderShape* pCyl=(C_CylinderShape*)a;
				C_CylinderProxy proxy(pCyl->m_Cylinder, pTree->m_PosRot);
				C_CollisionObject obj(pCyl, a->m_pPpuPtr);
#ifndef PLATFORM_SPU
				return ( pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_CylinderProxy, CollideCylinderTriangleMesh<
					_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle> >(&obj, proxy, results, stack));
#else
				return ( pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_CylinderProxy>(&obj, proxy, results, stack , 
					CollideCylinderTriangleMesh<_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle> ));
#endif
			}
			return false;
		}
	case (E_Cone_Shape | E_Cone_Shape)://100000
		{
			
			C_ConeShape* pCone1 = (C_ConeShape*)a;
			C_ConeShape* pCone2 = (C_ConeShape*)b;
			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			if(GJKConeCone(pCone1->m_Cone, pCone2->m_Cone, point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				results.AddPatch(patch);
				return true;
			}
			return false;	
		}
	case ( E_Cone_Shape | E_Box_Shape)://0x10008
		{
			bool con((a_Shape == E_Box_Shape)&(b_Shape == E_Cone_Shape));
			Collision::C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			Collision::C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);

			C_BoxShape* pBox=(C_BoxShape*)aShape;
			C_ConeShape* pCone=(C_ConeShape*)bShape;
			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			if(GJKBoxCone(pBox->m_Box, pCone->m_Cone, 
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;
		}
	case (E_Cone_Shape | E_ConvexHull_Shape)://0x10010
		{

			bool con((a_Shape == E_ConvexHull_Shape) & (b_Shape == E_Cone_Shape));
			C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);
			
			C_ConvexHullShape* pConv = (C_ConvexHullShape*)aShape;
			C_ConeShape* pCone = (C_ConeShape*)bShape;

			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			if(GJKConvexCone(pConv->m_Hull, pCone->m_Cone,  
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;
		}
	case ( E_Cone_Shape | E_Sphere_Shape)://0x10020
		{
			bool con((a_Shape==E_Sphere_Shape) & (b_Shape==E_Cone_Shape));
			C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);
			
			C_SphereShape* pSphere=(C_SphereShape*)aShape;
			C_ConeShape*   pCone  =(C_ConeShape*)bShape;

			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			if(GJKSphereCone(pSphere->m_Sphere, pCone->m_Cone, 
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;	
		}
	case (E_Cone_Shape | E_Capsule_Shape )://0x10040
		{
			bool con((a_Shape==E_Capsule_Shape) & (b_Shape==E_Cone_Shape));
			C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);
			
			C_CapsuleShape* pCapsule=(C_CapsuleShape*)aShape;
			C_ConeShape* pCone=(C_ConeShape*)bShape;

			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			if( GJKCapsuleCone(pCapsule->m_Capsule, pCone->m_Cone, 
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;
		}
	case ( E_Cone_Shape | E_Cylinder_Shape)://0x10080
		{
			bool con((a_Shape == E_Cylinder_Shape) & (b_Shape == E_Cone_Shape));
			C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);
			
			C_CylinderShape* pCyl = (C_CylinderShape*)aShape;
			C_ConeShape* pCone = (C_ConeShape*)bShape;
			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			if(GJKCylinderCone(pCyl->m_Cylinder, pCone->m_Cone, 
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;		
		}
	case (E_Cone_Shape | E_CharacterController_Shape )://0x10200
		{
			//BIZ_ASSERT(b_Shape == E_CharacterController_Shape);
			bool con((a_Shape == E_CharacterController_Shape) & (b_Shape == E_Cone_Shape));
			C_Shape* aShape = SharedPhysics::Fast_Select(con, a, b);
			C_Shape* bShape = SharedPhysics::Fast_Select(con, b, a);
			C_CharacterControllerShape* pCharacter=(C_CharacterControllerShape*)aShape;
			C_ConeShape* pCone=(C_ConeShape*)bShape;
			
			patch.m_NumContactPoints = 1;
			C_ContactPoint& point(patch.m_ContactPoints[0]);
			point.m_HashValue = 1;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			Maths::HF32 penDepth(Maths::UNDEFINED);

			//CollideOBBRay(pBox, pCharacter->m_Controller.m_Ray);

			if(GJKCapsuleCone(pCharacter->m_Controller.m_EnclosingCapsule, pCone->m_Cone, 
				point.m_ContactPoint, patch.m_Normal, penDepth, m_Epa))
			{
				Maths::Store(penDepth, &point.m_PenetrationDepth);
				patch.m_Normal = Maths::Select(con, patch.m_Normal, -patch.m_Normal);
				results.AddPatch(patch);
				return true;
			}
			return false;
		}
	case (E_Cone_Shape | E_InstancedTriangleMesh_Shape)://0x10800
		{
			//NB - we only ever !!!EVER!!! need to do the collision this way around, because we will only
			//be colliding against instanced level mesh or kinematic mesh!
			if((a_Shape == E_Cone_Shape) & (b_Shape == E_InstancedTriangleMesh_Shape))
			{
				C_InstancedTriangleMeshShape* pTree=(C_InstancedTriangleMeshShape*)b;
				C_ConeShape* pCone=(C_ConeShape*)a;
				C_ConeProxy proxy(pCone->m_Cone, pTree->m_PosRot);
				C_CollisionObject obj(pCone, a->m_pPpuPtr);
#ifndef PLATFORM_SPU
				return ( pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_ConeProxy, CollideConeTriangleMesh<
					_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle> >(&obj, proxy, results, stack));
#else
				return ( pTree->m_AABBTree.Collide<_MaxPatches, _MaxContacts, C_ConeProxy>(&obj, proxy, results, stack , 
					CollideConeTriangleMesh<_MaxPatches, _MaxContacts, Collision::AABBTreeTriangle> ));

#endif
			}
			return false;
		}
	}

	return false;
}

}


#include "StdAfx.h"
#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/collisionmanager.h"
#include "gravity/cloths/ropecollision.h"
#include "gravity/narrowphase/GJK.h"
#include "gravity/narrowphase/shapes.h"
#include "gravity/narrowphase/OBBRay.inl"
#include "gravity/sharedphysics/ContactStream.inl"
#include "gravity/narrowphase/SphereSphere.h"
#include "gravity/narrowphase/OBBSphere.h"
#include "gravity/narrowphase/CapsuleSphere.h"


namespace Collision
{
	bool C_CollisionManager::CollideRopeDynamic(C_CollisionObject* obj, SharedPhysics::C_GraphNode* pRopeNode,
		C_RopeShape* pRope, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_ContactStream& ropeContactStream)
	{
		C_ClothContactBuffer<ClothContactSize> contacts;
		//This will eventually be a switch table of all different collision types!
		if(obj->m_pShape->GetShape() == Collision::E_Sphere_Shape)
		{
			//if(pCloth->m_Cloth.Collide(obj, pClothNode, stack, contacts, SphereClothTriangle))
			if(pRope.m_Rope.CollideParticles(obj, pRopeNode, stack, contacts, SphereRopeParticles))
			{
				//Encode contacts...
				EncodeContacts<RopeContactSize>(contacts, ropeContactStream);
				return true;
			}
			//return pCloth->m_Cloth.CollideAndResolve(obj, stack, SphereClothTriangle);
		}
		else if (obj->m_pShape->GetShape() == Collision::E_Box_Shape)
		{
			//if(pCloth->m_Cloth.Collide(obj, pClothNode, stack, contacts, BoxClothTriangle))
			if(pRope.m_Rope.CollideParticles(obj, pRopeNode, stack, contacts, BoxRopeParticles))
			{
				EncodeContacts<RopeContactSize>(contacts, ropeContactStream);
				return true;
			}
		}
		else if (obj->m_pShape->GetShape() == Collision::E_Capsule_Shape)
		{
			if(pRope.m_Rope.CollideParticles(obj, pRopeNode, stack, contacts, CapsuleRopeParticles))
			{
				EncodeContacts<RopeContactSize>(contacts, ropeContactStream);
				return true;
			}
		}
		else if(obj->m_pShape->GetShape() == Collision::E_Cylinder_Shape)
		{
			if(pRope.m_Rope.CollideParticles(obj, pRopeNode, stack, contacts, CylinderRopeParticles))
			{
				EncodeContacts<RopeContactSize>(contacts, ropeContactStream);
				return true;
			}
		}
		else if(obj->m_pShape->GetShape() == Collision::E_Cone_Shape)
		{
			if(pRope.m_Rope.CollideParticles(obj, pRopeNode, stack, contacts, ConeRopeParticles))
			{
				EncodeContacts<RopeContactSize>(contacts, ropeContactStream);
				return true;
			}
		}
		else if(obj->m_pShape->GetShape() == Collision::E_ConvexHull_Shape)
		{
			if(pRope.m_Rope.CollideParticles(obj, pRopeNode, stack, contacts, ConvexHullRopeParticles))
			{
				EncodeContacts<RopeContactSize>(contacts, ropeContactStream);
				return true;
			}
		}
		else if(obj->m_pShape->GetShape() == Collision::E_Composite_Shape)
		{
			if(pRope.m_Rope.CollideParticles(obj, pRopeNode, stack, contacts, CompositeRopeParticles))
			{
				EncodeContacts<RopeContactSize>(contacts, ropeContactStream);
				return true;
			}
		}
		return false;
	}
}
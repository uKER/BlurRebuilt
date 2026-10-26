#include "StdAfx.h"
#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/collisionmanager.h"
#include "gravity/narrowphase/GJK.h"
#include "gravity/narrowphase/shapes.h"
#include "gravity/narrowphase/OBBRay.inl"
#include "gravity/sharedphysics/ContactStream.inl"
#include "gravity/narrowphase/SphereSphere.h"
#include "gravity/narrowphase/OBBSphere.h"
#include "gravity/narrowphase/CapsuleSphere.h"
#include "gravity/cloths/ropecollision.h"



namespace Collision
{
	bool C_CollisionManager::CollideRopeDynamic(C_CollisionObject* obj, SharedPhysics::C_GraphNode* pRopeNode,
		C_RopeShape* pRope, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_ContactStream& ropeContactStream)
	{
		C_ParticleContactBuffer<ClothContactSize> contacts;
		//This will eventually be a switch table of all different collision types!
		const U32 shapeType = obj->m_pShape->GetShape();
		switch(shapeType)
		{
		case Collision::E_Box_Shape:
			{
				if(pRope->m_RopeTree.CollideParticles(obj, pRopeNode, stack, contacts, BoxRopeParticles))
				{
					EncodeContacts<RopeContactSize>(contacts, ropeContactStream);
					return true;
				}
				return false;
			}
		case Collision::E_ConvexHull_Shape:
			{
				if(pRope->m_RopeTree.CollideParticles(obj, pRopeNode, stack, contacts, ConvexHullRopeParticles))
				{
					EncodeContacts<RopeContactSize>(contacts, ropeContactStream);
					return true;
				}
				return false;
			}
		case Collision::E_Sphere_Shape:
			{
				if(pRope->m_RopeTree.CollideParticles(obj, pRopeNode, stack, contacts, SphereRopeParticles))
				{
					//Encode contacts...
					EncodeContacts<RopeContactSize>(contacts, ropeContactStream);
					return true;
				}
				return false;
			}
		case Collision::E_Capsule_Shape:
			{
				if(pRope->m_RopeTree.CollideParticles(obj, pRopeNode, stack, contacts, CapsuleRopeParticles))
				{
					EncodeContacts<RopeContactSize>(contacts, ropeContactStream);
					return true;
				}
				return false;
			}
		case Collision::E_Cylinder_Shape:
			{
				if(pRope->m_RopeTree.CollideParticles(obj, pRopeNode, stack, contacts, CylinderRopeParticles))
				{
					EncodeContacts<RopeContactSize>(contacts, ropeContactStream);
					return true;
				}
				return false;
			}
		case Collision::E_Composite_Shape:
			{
				if(pRope->m_RopeTree.CollideParticles(obj, pRopeNode, stack, contacts, CompositeRopeParticles))
				{
					EncodeContacts<RopeContactSize>(contacts, ropeContactStream);
					return true;
				}
			}
		case Collision::E_Cone_Shape:
			{
				if(pRope->m_RopeTree.CollideParticles(obj, pRopeNode, stack, contacts, ConeRopeParticles))
				{
					EncodeContacts<RopeContactSize>(contacts, ropeContactStream);
					return true;
				}
			}
		default:
			{
				BIZ_ASSERT("Not a valid shape");
			}
		};
		return false;
	}
}
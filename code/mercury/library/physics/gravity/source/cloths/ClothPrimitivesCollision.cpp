#include "StdAfx.h"
#include "gravity/Cloths/SoftBody.h"
#include "gravity/narrowphase/Primitive.h"
#include "gravity/narrowphase/primitiveTest.h"
#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/SphereSphere.h"
#include "gravity/narrowphase/CapsuleSphere.h"
#include "gravity/narrowphase/OBBSphere.h"
#include "gravity/narrowphase/CylinderSphere.h"
#include "gravity/narrowphase/ConeSphere.h"
#include "gravity/narrowphase/ConvexSphere.h"
#include "gravity/narrowphase/GJK.h"
#include "gravity/narrowphase/GJK.inl"
#include "gravity/narrowphase/GJKCollision.h"

namespace Collision
{
	bool SphereSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<SoftBodyContactSize>& contacts)
	{
		using namespace Maths;

		const HF32 zero(Constants<HF32>::Zero());
		Collision::C_SphereShape* pSphere = (Collision::C_SphereShape*)pShape;
		HVec3 closest(UNDEFINED);
		Collision::C_Sphere& sphereA = pSphere->m_Sphere;
		
		Collision::C_Sphere sphereB;
		sphereB.Radius(tree.m_Thickness);
		bool result = false;

		U32 endIndex = startIndex + numVerts;
		for(U32 i=startIndex; i<endIndex; ++i)
		{
			Collision::C_GravityParticle& particle = tree.m_pParticles[i];
			
			sphereB.Centre(particle.m_CurPosition);

			if(contacts.m_NumContacts == SoftBodyContactSize)
				break;
			C_ParticleContact& contact = contacts.m_Contacts[contacts.m_NumContacts];
			if(Collision::CollideSphereSphere(sphereA, sphereB, contact.m_Contact, contact.m_Penetration, contact.m_EscapeVector))
			{
				/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact.m_Contact, 0.05f, Maths::RGBA(0, 255, 0));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(contact.m_Contact, contact.m_EscapeVector, contact.m_Penetration, 0.02f, Maths::RGBA(255, 0, 255));*/
				//contact.m_TriangleNormal = particle.m_AverageNormal;
				//contact.m_EscapeVector = -particle.m_AverageNormal;
				const Maths::HF32 side((sphereA.Centre() - sphereB.Centre()) | particle.m_AverageNormal);
				contact.m_EscapeVector =Maths::Select(side.GT(zero), -particle.m_AverageNormal, particle.m_AverageNormal);
				//contact.m_EscapeVector = Maths::C_Normal(sphereB.Centre() - contact.m_Contact);
				//contact.m_TriangleNormal = contact.m_EscapeVector;
				contact.m_pNodePtr = pNode;
				contact.m_pParticlePtr = pSoftBodyNode;
				contact.m_Index = i;
				contacts.m_NumContacts++;
				contact.m_Type = C_ParticleContact::E_VertexConstraint;
				result = true;
			}
		}
		
		return result;
	}

	bool CapsuleSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<SoftBodyContactSize>& contacts)
	{
		using namespace Maths;

		const HF32 zero(Constants<HF32>::Zero());
		Collision::C_CapsuleShape* pCapsule = (Collision::C_CapsuleShape*)pShape;
		Collision::C_Capsule& capsule = pCapsule->m_Capsule;
		
		Collision::C_Sphere sphere;
		sphere.Radius(tree.m_Thickness);
		bool result = false;

		U32 endIndex = startIndex + numVerts;
		for(U32 i=startIndex; i<endIndex; ++i)
		{
			Collision::C_GravityParticle& particle = tree.m_pParticles[i];
			
			sphere.Centre(particle.m_CurPosition);

			if(contacts.m_NumContacts == SoftBodyContactSize)
				break;
			C_ParticleContact& contact = contacts.m_Contacts[contacts.m_NumContacts];
			if(Collision::CollideCapsuleSphere(capsule, sphere, contact.m_Contact, contact.m_Penetration, contact.m_EscapeVector))
			{
				//DbUtil::DefaultDebugRenderSys()->Immediate()->Draw3dStar(contact.m_Contact, 0.25f, RGBA(0, 0, 255));
				//DbUtil::DefaultDebugRenderSys()->Immediate()->Draw3dLine(contact.m_Contact, contact.m_Contact + contact.m_EscapeVector*contact.m_Penetration, RGBA(0, 0, 255));
				//contact.m_EscapeVector = -particle.m_AverageNormal;
				const Maths::HF32 side((capsule.Centre() - sphere.Centre()) | particle.m_AverageNormal);
				contact.m_EscapeVector =Maths::Select(side.GT(zero), -particle.m_AverageNormal, particle.m_AverageNormal);
				//contact.m_EscapeVector = Maths::C_Normal(sphere.Centre() - contact.m_Contact);
				contact.m_TriangleNormal = particle.m_AverageNormal;
				//contact.m_TriangleNormal = contact.m_EscapeVector;
				contact.m_pNodePtr = pNode;
				contact.m_pParticlePtr = pSoftBodyNode;
				contact.m_Index = i;
				contacts.m_NumContacts++;
				contact.m_Type = C_ParticleContact::E_VertexConstraint;
				result = true;
			}
		}
		
		return result;
	}

	

	bool BoxSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<SoftBodyContactSize>& contacts)
	{
		using namespace Maths;
		//Collide box-triangle!
		const HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const HF32 one(Maths::Constants<Maths::HF32>::One());
		const HF32 half(Constants<HF32>::Half());
		Collision::C_BoxShape* pBox = (Collision::C_BoxShape*)pShape;
		Collision::C_OBB& obb = pBox->m_Box;

		Collision::C_Sphere sphereB;
		sphereB.Radius(tree.m_Thickness);

		bool result = false;

		const Maths::HMat33 localBox(obb.Orientation().InverseRotation());
		U32 endIndex = startIndex + numVerts;
		for(U32 i=startIndex; i<endIndex; ++i)
		{
			Collision::C_GravityParticle& particle = tree.m_pParticles[i];
			
			sphereB.Centre(particle.m_CurPosition);

			if(contacts.m_NumContacts == SoftBodyContactSize)
				break;

			C_ParticleContact& contact = contacts.m_Contacts[contacts.m_NumContacts];
			if(Collision::CollideOBBSphere(obb, sphereB, contact.m_Contact, contact.m_Penetration, contact.m_EscapeVector))
			{
				/*const Maths::HVec3 localContact((sphereB.Centre() - obb.Centre()) * localBox);

				const Maths::Scalar_Bool bInside(Maths::Abs(localContact).All_LE(obb.Extent()));*/
				const Maths::HF32 side((obb.Centre() - sphereB.Centre()) | particle.m_AverageNormal);
				contact.m_EscapeVector =Maths::Select(side.GT(zero), -particle.m_AverageNormal, particle.m_AverageNormal);
				//contact.m_EscapeVector = Maths::C_Normal(sphereB.Centre() - contact.m_Contact);
				contact.m_TriangleNormal = particle.m_AverageNormal;
				contact.m_pNodePtr = pNode;
				contact.m_pParticlePtr = pSoftBodyNode;
				contact.m_Index = i;
				contacts.m_NumContacts++;
				contact.m_Type = C_ParticleContact::E_VertexConstraint;
				result = true;
			}
		}

		return result;
	}

	void GetNormalAndPenetration(const Collision::C_OBB& obb, const Maths::HVec3 PCREF contact, Maths::C_Normal& normal, Maths::HF32& penetration)
	{
		//One of 6 axes to choose from +ve/-ve x-, y-, and z-axes

		//Choose an axis!
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HVec3 pen(obb.Extent() - Maths::Abs(contact));
		//Select which one is the smaller abs value!
		const Maths::HVec3 absPen(Maths::Abs(pen));
		const Maths::Scalar_Bool bX((absPen.X().LT(absPen.Y())) & (absPen.X().LT(absPen.Z())));
		const Maths::Scalar_Bool bY(absPen.Y().LT(absPen.Z()));

		const Maths::C_Normal norm(Maths::C_Normal(Maths::Select(bX, Maths::Constants<Maths::HVec3>::UnitAxisPositiveX(),
			Maths::Select(bY, Maths::Constants<Maths::HVec3>::UnitAxisPositiveY(), Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ())), Maths::C_Normal::UNIT_LENGTH_VECTOR));
		const Maths::HF32 pp(norm | pen);
		const Maths::HF32 ss(norm | contact);
		normal = Maths::Select(ss.GE(zero), norm, -norm) * obb.Orientation();
		penetration = pp;
	}


	bool CollideBoxSphere(const Maths::HVec3 PCREF point, const Maths::HF32 PCREF radius, const Collision::C_OBB& obb, Maths::C_Normal& normal,
		Maths::HF32& penetration)
	{
		using namespace Maths;
		const HF32 zero(Constants<HF32>::Zero());
		const HVec3 e(obb.Extent());
		const HVec3 closest(Max(-e, Min(e, point)));
		const HVec3 dif(closest - point);
		const HF32 dis(dif | dif);
		if(dis.GT(radius * radius))
			return false;
		const C_Normal dir(-point);
		const HVec3 offset(dir * radius);
		GetNormalAndPenetration(obb, point + offset, normal, penetration);
		return penetration >= zero;
	}


	bool CylinderSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode, SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<SoftBodyContactSize>& contacts)
	{
		using namespace Maths;
		//Collide box-triangle!
		const HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const HF32 one(Maths::Constants<Maths::HF32>::One());
		const HF32 half(Constants<HF32>::Half());
		Collision::C_CylinderShape* pCylinder = (Collision::C_CylinderShape*)pShape;
		Collision::C_Cylinder& cylinder = pCylinder->m_Cylinder;

		Collision::EPA epa;

		Collision::C_Sphere sphereB;
		sphereB.Radius(tree.m_Thickness);

		bool result = false;

		U32 endIndex = startIndex + numVerts;
		for(U32 i=startIndex; i<endIndex; ++i)
		{
			Collision::C_GravityParticle& particle = tree.m_pParticles[i];
			
			sphereB.Centre(particle.m_CurPosition);

			if(contacts.m_NumContacts == SoftBodyContactSize)
				break;

			C_ParticleContact& contact = contacts.m_Contacts[contacts.m_NumContacts];
			if(Collision::GJKSphereCylinder(sphereB, cylinder, contact.m_Contact, contact.m_EscapeVector, contact.m_Penetration, epa))
			{
				//contact.m_EscapeVector = -particle.m_AverageNormal;
				const Maths::HF32 side((cylinder.Centre() - sphereB.Centre()) | particle.m_AverageNormal);
				contact.m_EscapeVector =Maths::Select(side.GT(zero), -particle.m_AverageNormal, particle.m_AverageNormal);
				contact.m_TriangleNormal = particle.m_AverageNormal;
				contact.m_pNodePtr = pNode;
				contact.m_pParticlePtr = pSoftBodyNode;
				contact.m_Index = i;
				contacts.m_NumContacts++;
				contact.m_Type = C_ParticleContact::E_VertexConstraint;
				result = true;
			}
		}

		return result;
	}

	bool ConeSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode, SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<SoftBodyContactSize>& contacts)
	{
		using namespace Maths;
		//Collide box-triangle!
		const HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const HF32 one(Maths::Constants<Maths::HF32>::One());
		const HF32 half(Constants<HF32>::Half());
		Collision::C_ConeShape* pCone = (Collision::C_ConeShape*)pShape;
		Collision::C_Cone& cone = pCone->m_Cone;

		Collision::C_Sphere sphere;
		sphere.Radius(tree.m_Thickness);

		bool result = false;

		U32 endIndex = startIndex + numVerts;
		for(U32 i=startIndex; i<endIndex; ++i)
		{
			Collision::C_GravityParticle& particle = tree.m_pParticles[i];
			
			sphere.Centre(particle.m_CurPosition);

			if(contacts.m_NumContacts == SoftBodyContactSize)
				break;

			C_ParticleContact& contact = contacts.m_Contacts[contacts.m_NumContacts];
			if(Collision::CollideConeSphere(cone, sphere, contact.m_Contact, contact.m_Penetration, contact.m_EscapeVector))
			{
				//contact.m_EscapeVector = - particle.m_AverageNormal;
				const Maths::HF32 side((cone.Centre() - sphere.Centre()) | particle.m_AverageNormal);
				contact.m_EscapeVector =Maths::Select(side.GT(zero), -particle.m_AverageNormal, particle.m_AverageNormal);
				contact.m_TriangleNormal = particle.m_AverageNormal;
				contact.m_pNodePtr = pNode;
				contact.m_pParticlePtr = pSoftBodyNode;
				contact.m_Index = i;
				contacts.m_NumContacts++;
				contact.m_Type = C_ParticleContact::E_VertexConstraint;
				result = true;
			}
		}

		return result;
	}

	bool ConvexHullSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode, SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<SoftBodyContactSize>& contacts)
	{
		using namespace Maths;
		//Collide box-triangle!
		const HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const HF32 one(Maths::Constants<Maths::HF32>::One());
		const HF32 half(Constants<HF32>::Half());
		Collision::C_ConvexHullShape* pConvexHull = (Collision::C_ConvexHullShape*)pShape;
		Collision::C_ConvexHull& hull = pConvexHull->m_Hull;

		Collision::C_Sphere sphere;
		sphere.Radius(tree.m_Thickness);
		Collision::EPA epa;

		bool result = false;

		U32 endIndex = startIndex + numVerts;
		for(U32 i=startIndex; i<endIndex; ++i)
		{
			Collision::C_GravityParticle& particle = tree.m_pParticles[i];
			
			sphere.Centre(particle.m_CurPosition);

			if(contacts.m_NumContacts == SoftBodyContactSize)
				break;

			C_ParticleContact& contact = contacts.m_Contacts[contacts.m_NumContacts];
			HVec3 normal(UNDEFINED);
		
			if(Collision::GJKCPair<C_Sphere, C_ConvexHull>(sphere, hull, contact.m_Contact, normal, contact.m_Penetration, epa))
			//if(Collision::CollideConvexSphere(hull, sphere, contact.m_Contact, contact.m_Penetration, contact.m_EscapeVector))
			{
				//contact.m_EscapeVector = -particle.m_AverageNormal;
				const Maths::HF32 side((hull.Centre() - sphere.Centre()) | particle.m_AverageNormal);
				contact.m_EscapeVector =Maths::Select(side.GT(zero), -particle.m_AverageNormal, particle.m_AverageNormal);
				contact.m_TriangleNormal = particle.m_AverageNormal;
				contact.m_pNodePtr = pNode;
				contact.m_pParticlePtr = pSoftBodyNode;
				contact.m_Index = i;
				contacts.m_NumContacts++;
				contact.m_Type = C_ParticleContact::E_VertexConstraint;
				result = true;
			}
		}

		return result;


	}

	bool CompositeSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode, SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
	const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<SoftBodyContactSize>& contacts)
	{
		Collision::C_CompositeShape* pComposite = (Collision::C_CompositeShape*)pShape;

		Collision::C_CompositeShapeIterator iter(pComposite);
		bool collided = false;
		//for(U32 i=0; i<pComposite->m_NumShapes; ++i)
		while(iter.HasNextShape())
		{
			iter.NextShape();
			Collision::C_Shape* pSubShape = iter.GetCurrentShape<Collision::C_Shape>();
			switch(pSubShape->m_ShapeType)
			{
				
			case Collision::E_Box_Shape:
				{
					collided |= BoxSoftBodyTriangle(pSubShape, pSoftBodyNode, pNode, tree, startIndex, numVerts, contacts);
					break;
				}
			case Collision::E_ConvexHull_Shape:
				{
					collided |= ConvexHullSoftBodyTriangle(pSubShape, pSoftBodyNode, pNode, tree, startIndex, numVerts, contacts);
					break;
				}
			case Collision::E_Sphere_Shape:
				{
					collided |= SphereSoftBodyTriangle(pSubShape, pSoftBodyNode, pNode, tree, startIndex, numVerts, contacts);
					break;
				}
			case Collision::E_Capsule_Shape:
				{
					collided |=CapsuleSoftBodyTriangle(pSubShape, pSoftBodyNode, pNode, tree, startIndex, numVerts, contacts);
					break;
				}
			case Collision::E_Cylinder_Shape:
				{
					collided |= CylinderSoftBodyTriangle(pSubShape, pSoftBodyNode, pNode, tree, startIndex, numVerts, contacts);
					break;
				}
			case Collision::E_Composite_Shape:
				{
					collided |= CompositeSoftBodyTriangle(pSubShape, pSoftBodyNode, pNode, tree, startIndex, numVerts, contacts);
					break;
				}
			case Collision::E_Cone_Shape:
				{
					collided |= ConeSoftBodyTriangle(pSubShape, pSoftBodyNode, pNode, tree, startIndex, numVerts, contacts);
					break;
				}
			default:
				return false;
			};
		}
		return collided;
	}
};
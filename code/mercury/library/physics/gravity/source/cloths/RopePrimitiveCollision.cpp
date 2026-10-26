#include "StdAfx.h"
#include "gravity/Cloths/RopeCollision.h"
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

namespace Collision
{
	
	bool CapsuleRopeParticles(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pRopeNode, 
		SharedPhysics::C_GraphNode* pNode, C_ParticleContactBuffer<RopeContactSize>& contacts)
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
			Collision::C_GravityParticle& particle = tree.m_Particles[i];
			
			sphere.Centre(particle.m_CurPosition);

			if(contacts.m_NumContacts == ClothContactSize)
				break;
			C_ClothContact& contact = contacts.m_Contacts[contacts.m_NumContacts];
			if(Collision::CollideCapsuleSphere(capsule, sphere, contact.m_Contact, contact.m_Penetration, contact.m_EscapeVector))
			{
				//DbUtil::DefaultDebugRenderSys()->Immediate()->Draw3dStar(contact.m_Contact, 0.25f, RGBA(0, 0, 255));
				//DbUtil::DefaultDebugRenderSys()->Immediate()->Draw3dLine(contact.m_Contact, contact.m_Contact + contact.m_EscapeVector*contact.m_Penetration, RGBA(0, 0, 255));
				contact.m_TriangleNormal = particle.m_AverageNormal;
				//contact.m_TriangleNormal = contact.m_EscapeVector;
				contact.m_pNodePtr = pNode;
				contact.m_pClothPtr = pClothNode;
				contact.m_Index = i;
				contacts.m_NumContacts++;
				contact.m_Type = C_ClothContact::E_VertexConstraint;
				result = true;
			}
		}
		
		return result;
	}



	bool BoxParticles(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_RopeTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<RopeContactSize>& contacts)
	{
		using namespace Maths;
		//Collide box-triangle!
		const HF32 _zero(Maths::Constants<Maths::HF32>::Zero());
		const HF32 one(Maths::Constants<Maths::HF32>::One());
		const HF32 half(Constants<HF32>::Half());
		Collision::C_BoxShape* pBox = (Collision::C_BoxShape*)pShape;
		Collision::C_OBB& obb = pBox->m_Box;

		Collision::C_Sphere sphereB;
		sphereB.Radius(tree.m_Thickness);

		bool result = false;

		U32 endIndex = startIndex + numVerts;
		for(U32 i=startIndex; i<endIndex; ++i)
		{
			Collision::C_GravityParticle& particle = tree.m_Particles[i];
			
			sphereB.Centre(particle.m_CurPosition);

			if(contacts.m_NumContacts == ClothContactSize)
				break;

			C_ClothContact& contact = contacts.m_Contacts[contacts.m_NumContacts];
			if(Collision::CollideOBBSphere(obb, sphereB, contact.m_Contact, contact.m_Penetration, contact.m_EscapeVector))
			{
				contact.m_TriangleNormal = particle.m_AverageNormal;
				contact.m_pNodePtr = pNode;
				contact.m_pClothPtr = pClothNode;
				contact.m_Index = i;
				contacts.m_NumContacts++;
				contact.m_Type = C_ClothContact::E_VertexConstraint;
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
		const Maths::HVec3 pen(obb.e - Maths::Abs(contact));
		//Select which one is the smaller abs value!
		const Maths::HVec3 absPen(Maths::Abs(pen));
		const Maths::Scalar_Bool bX((absPen.X().LT(absPen.Y())) & (absPen.X().LT(absPen.Z())));
		const Maths::Scalar_Bool bY(absPen.Y().LT(absPen.Z()));

		const Maths::C_Normal norm(Maths::C_Normal(Maths::Select(bX, Maths::Constants<Maths::HVec3>::UnitAxisPositiveX(),
			Maths::Select(bY, Maths::Constants<Maths::HVec3>::UnitAxisPositiveY(), Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ())), Maths::C_Normal::UNIT_LENGTH_VECTOR));
		const Maths::HF32 pp(norm | pen);
		const Maths::HF32 ss(norm | contact);
		normal = Maths::Select(ss.GE(zero), norm, -norm) * obb.u;
		penetration = pp;
	}

	void GetNormalAndPenetration(const Collision::C_OBB& obb, const Maths::HVec3 PCREF contact, const Maths::HVec3 PCREF a, 
		const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c,
		Maths::C_Normal& normal, Maths::HF32& penetration)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HVec3 pen(obb.e - Maths::Abs(contact));
		//Select which one is the smaller abs value!
		const Maths::HVec3 absPen(Maths::Abs(pen));
		const Maths::Scalar_Bool bX((absPen.X().LT(absPen.Y())) & (absPen.X().LT(absPen.Z())));
		const Maths::Scalar_Bool bY(absPen.Y().LT(absPen.Z()));

		const Maths::C_Normal norm(Maths::C_Normal(Maths::Select(bX, Maths::Constants<Maths::HVec3>::UnitAxisPositiveX(),
			Maths::Select(bY, Maths::Constants<Maths::HVec3>::UnitAxisPositiveY(), Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ())), Maths::C_Normal::UNIT_LENGTH_VECTOR));
		const Maths::HF32 pp(norm | pen);
		const Maths::HF32 ss(norm | contact);
		normal = Maths::Select(ss.GE(zero), norm, -norm);
		const Maths::HF32 nA(normal | a);
		const Maths::HF32 nB(normal | b);
		const Maths::HF32 nC(normal | c);
		const Maths::HF32 extents(obb.e | Maths::Abs(normal.m_V));


		const Maths::HF32 deepest(Maths::Min(nA, Maths::Min(nB, nC)));
		penetration = extents - deepest;
	}

	bool CollideBoxSphere(const Maths::HVec3 PCREF point, const Maths::HF32 PCREF radius, const Collision::C_OBB& obb, Maths::C_Normal& normal,
		Maths::HF32& penetration)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HVec3 closest(Maths::Max(-obb.e, Maths::Min(obb.e, point)));
		const Maths::HVec3 dif(closest - point);
		const Maths::HF32 dis(dif | dif);
		if(dis.GT(radius * radius))
			return false;
		const Maths::C_Normal dir(-point);
		const Maths::HVec3 offset(dir * radius);
		GetNormalAndPenetration(obb, point + offset, normal, penetration);
		return penetration >= zero;
	}

	

	bool CylinderRopeParticles(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pRopeNode, SharedPhysics::C_GraphNode* pNode, Collision::C_RopeTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<RopeContactSize>& contacts)
	{
		using namespace Maths;
		//Collide box-triangle!
		const HF32 _zero(Maths::Constants<Maths::HF32>::Zero());
		const HF32 one(Maths::Constants<Maths::HF32>::One());
		const HF32 half(Constants<HF32>::Half());
		Collision::C_CylinderShape* pCylinder = (Collision::C_CylinderShape*)pShape;
		Collision::C_Cylinder& cylinder = pCylinder->m_Cylinder;

		Collision::C_Sphere sphereB;
		sphereB.Radius(tree.m_Thickness);

		bool result = false;

		U32 endIndex = startIndex + numVerts;
		for(U32 i=startIndex; i<endIndex; ++i)
		{
			Collision::C_GravityParticle& particle = tree.m_Particles[i];
			
			sphereB.Centre(particle.m_CurPosition);

			if(contacts.m_NumContacts == ClothContactSize)
				break;

			C_ClothContact& contact = contacts.m_Contacts[contacts.m_NumContacts];
			if(Collision::CollideCylinderSphere(cylinder, sphereB, contact.m_Contact, contact.m_Penetration, contact.m_EscapeVector))
			{
				contact.m_TriangleNormal = particle.m_AverageNormal;
				contact.m_pNodePtr = pNode;
				contact.m_pClothPtr = pClothNode;
				contact.m_Index = i;
				contacts.m_NumContacts++;
				contact.m_Type = C_ClothContact::E_VertexConstraint;
				result = true;
			}
		}

		return result;
	}

	bool ConeRopeParticles(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode, SharedPhysics::C_GraphNode* pNode, Collision::C_RopeTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<RopeContactSize>& contacts)
	{
		using namespace Maths;
		//Collide box-triangle!
		const HF32 _zero(Maths::Constants<Maths::HF32>::Zero());
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
			Collision::C_GravityParticle& particle = tree.m_Particles[i];
			
			sphere.Centre(particle.m_CurPosition);

			if(contacts.m_NumContacts == ClothContactSize)
				break;

			C_ClothContact& contact = contacts.m_Contacts[contacts.m_NumContacts];
			if(Collision::CollideConeSphere(cone, sphere, contact.m_Contact, contact.m_Penetration, contact.m_EscapeVector))
			{
				//contact.m_EscapeVector = - contact.m_EscapeVector;
				contact.m_TriangleNormal = particle.m_AverageNormal;
				contact.m_pNodePtr = pNode;
				contact.m_pClothPtr = pClothNode;
				contact.m_Index = i;
				contacts.m_NumContacts++;
				contact.m_Type = C_ClothContact::E_VertexConstraint;
				result = true;
			}
		}

		return result;
	}

	bool ConvexHullRopeParticles(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pRopeNode, SharedPhysics::C_GraphNode* pNode, Collision::C_RopeTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<RopeContactSize>& contacts)
	{
		using namespace Maths;
		//Collide box-triangle!
		const HF32 _zero(Maths::Constants<Maths::HF32>::Zero());
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
			Collision::C_GravityParticle& particle = tree.m_Particles[i];
			
			sphere.Centre(particle.m_CurPosition);

			if(contacts.m_NumContacts == ClothContactSize)
				break;

			C_ClothContact& contact = contacts.m_Contacts[contacts.m_NumContacts];
			HVec3 normal(UNDEFINED);
		
			if(Collision::GJKCPair<C_Sphere, C_ConvexHull>(sphere, hull, contact.m_Contact, normal, contact.m_Penetration, epa))
			//if(Collision::CollideConvexSphere(hull, sphere, contact.m_Contact, contact.m_Penetration, contact.m_EscapeVector))
			{
				contact.m_EscapeVector = normal;
				contact.m_TriangleNormal = particle.m_AverageNormal;
				contact.m_pNodePtr = pNode;
				contact.m_pClothPtr = pClothNode;
				contact.m_Index = i;
				contacts.m_NumContacts++;
				contact.m_Type = C_ClothContact::E_VertexConstraint;
				result = true;
			}
		}

		return result;


	}

	bool CompositeRopeParticles(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pRopeNode, SharedPhysics::C_GraphNode* pNode, Collision::C_RopeTree& tree,
	const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<RopeContactSize>& contacts)
	{
		Collision::C_CompositeShape* pComposite = (Collision::C_CompositeShape*)pShape;

		for(U32 i=0; i<pComposite->m_NumShapes; ++i)
		{
			Collision::C_Shape* pSubShape = pComposite->GetShape(i);
			switch(pSubShape->m_ShapeType)
			{
				
			case Collision::E_Box_Shape:
				return BoxRopeParticles(pSubShape, pRopeNode, pNode, tree, startIndex, numVerts, contacts);
			case Collision::E_ConvexHull_Shape:
				return ConvexHullRopeParticles(pSubShape, pRopeNode, pNode, tree, startIndex, numVerts, contacts);
			case Collision::E_Sphere_Shape:
				return SphereRopeParticles(pSubShape, pRopeNode, pNode, tree, startIndex, numVerts, contacts);
			case Collision::E_Capsule_Shape:
				return CapsuleRopeParticles(pSubShape, pRopeNode, pNode, tree, startIndex, numVerts, contacts);
			case Collision::E_Cylinder_Shape:
				return CylinderRopeParticles(pSubShape, pRopeNode, pNode, tree, startIndex, numVerts, contacts);
			case Collision::E_Composite_Shape:
				return CompositeRopeParticles(pSubShape, pRopeNode, pNode, tree, startIndex, numVerts, contacts);
			case Collision::E_Cone_Shape:
				return ConeRopeParticles(pSubShape, pRopeNode, pNode, tree, startIndex, numVerts, contacts);
			default:
				return false;
			};
		}
		return false;
	}
};
#include "Stdafx.h"
#include "gravity/cloths/Cloth.h"
#include "gravity/sdk/Body.h"
#include "gravity/SharedPhysics/RigidBody.h"
#include "system/perf.h"
#include "gravity/SharedPhysics/Select.h"

namespace Collision
{

	void C_Cloth::UpdateParticlesBasedOnImpulse(const Maths::HF32 PCREF time)
	{
		using namespace Maths;

		const HF32 recipTime(time.Reciprocal());
		for(S32 a = 0; a < m_Contacts.m_numElements; ++a)
		{
			C_ParticleContact& con = m_Contacts[a];
			C_GravityParticle& parA = m_pParticles[con.m_Index];
			const HVec3 impulseA = CalculateImpulse(con, parA, recipTime);
			parA.AddExternalForce(-impulseA);

			/*const Maths::HVec3 changeVelA(-impulseA*parA.m_MassInverse );
			parA.m_CurPosition +=  changeVelA * time;*/
			con.m_pNodePtr->m_pRigidBody->ApplyImpulseWorldSpace(impulseA, con.m_Contact);

			/*const Maths::HVec3 changeVelA(-impulseA*parA.m_MassInverse );
			parA.m_CurPosition +=  changeVelA * time;  */
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(con.m_Contact, con.m_EscapeVector, con.m_Penetration, 0.02f, RGBA(0, 0, 255));
		}
	}

	void C_Cloth::UpdateRigidBodiesBasedOnSpring(const Maths::HF32 PCREF time)
	{
		using namespace Maths;

		const HF32 recipTime(time.Reciprocal());
		for(S32 a = 0; a < m_Contacts.m_numElements; ++a)
		{
			C_ParticleContact& con = m_Contacts[a];
			C_GravityParticle& parA = m_pParticles[con.m_Index];

//			const Maths::HVec3 con.m_Contact

		}
	}


	void C_Cloth::ProjectParticles(const Maths::HF32 PCREF time)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		
		 
		const Maths::HF32 recipTime(time.Reciprocal());
		for(S32 a = 0; a < m_Contacts.m_numElements; ++a)
		{
			C_ParticleContact& con = m_Contacts[a];
			C_GravityParticle& parA = m_pParticles[con.m_Index];
			//parA.m_CurPosition = Maths::Select(parA.m_Mass.EQ(zero), parA.m_CurPosition, con.m_Contact + con.m_EscapeVector * m_Thickness);
			const Maths::HVec3 prePosition(parA.m_CurPosition);
			parA.m_CurPosition = Maths::Select(parA.m_Mass.EQ(zero), parA.m_CurPosition, parA.m_CurPosition + con.m_EscapeVector * con.m_Penetration);
			/*const Maths::HVec3 impulseA((parA.m_CurPosition - prePosition) * recipTime * parA.m_Mass);
			con.m_pNodePtr->m_pRigidBody->ApplyImpulseWorldSpace(-impulseA, con.m_Contact);*/
		}
		
		for(S32 a = 0; a < m_OneWayContacts.m_numElements; ++a)
		{
			Collision::C_ParticleContact& con = m_OneWayContacts[a];
			
			C_GravityParticle& parA = m_pParticles[con.m_Index];
			parA.m_CurPosition = Maths::Select(parA.m_Mass.EQ(zero), parA.m_CurPosition, parA.m_CurPosition + con.m_EscapeVector * con.m_Penetration);
			
		}
	}

	
	void C_Cloth::TimeStep(const Maths::HF32 PCREF time, SharedPhysics::C_SafeStack<true>& stack)
	{
		TimeStep(time);
		UpdateTree(stack);
	}

	void C_Cloth::TimeStep(const Maths::HF32 PCREF time)
	{
		//const Maths::HF32 recipTime(time.Reciprocal());
		AccumulateForces();

		//Project all particles outside of rigid bodies
		ProjectParticles(time);

		Tear();

		//Satisfy constraints of the cloth's texture 
		SatisfyConstraints();

		//Based on the after-constraint's particles position calculate impulse 
		//Apply impulses back to bodies!
		UpdateParticlesBasedOnImpulse(time);

		//Add external force to the particles
		Verlet(time);

		m_Contacts.m_numElements = 0;
		m_OneWayContacts.m_numElements = 0;
	}

	

	//void C_Cloth::FlattenEdge(const C_SoftBodyEdge& edge, const Maths::HF32 PCREF time)
	//{
	//	if(edge.m_TriangleIndexB == -1)
	//		return;
	//	const Maths::HF32 recipTime(time);
	//	const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
	//	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	//	C_SoftBodyTriangle& triA(m_pTriangles[edge.m_TriangleIndexA]);
	//	C_SoftBodyTriangle& triB(m_pTriangles[edge.m_TriangleIndexB]);
	//	C_GravityParticle& parA(m_pParticles[edge.particleA]);
	//	C_GravityParticle& parB(m_pParticles[edge.particleB]);

	//	U32 spareVertA(Maths::Select(triA.m_VertexIndexA != edge.particleA &&
	//		triA.m_VertexIndexA != edge.particleB, triA.m_VertexIndexA, 
	//		Maths::Select(triA.m_VertexIndexB != edge.particleA && 
	//		triA.m_VertexIndexB != edge.particleB, triA.m_VertexIndexB, triA.m_VertexIndexC)));

	//	U32 spareVertB(Maths::Select(triB.m_VertexIndexA != edge.particleA &&
	//		triB.m_VertexIndexA != edge.particleB, triB.m_VertexIndexA, 
	//		Maths::Select(triB.m_VertexIndexB != edge.particleA && 
	//		triB.m_VertexIndexB != edge.particleB, triB.m_VertexIndexB, triB.m_VertexIndexC)));

	//	//Calculate average normal!
	//	const Maths::HVec3 avgNormal((triA.m_Normal.m_V + triB.m_Normal.m_V) * half);
	//	const Maths::HVec3 avgPoint((m_pParticles[spareVertA].m_CurPosition + m_pParticles[spareVertB].m_CurPosition) * half);
	//	const Maths::HF32 d(avgPoint | avgNormal);
	//	const Maths::HF32 relSpeedA(Maths::Max(parA.Velocity(recipTime) | avgNormal, zero));
	//	const Maths::HF32 relSpeedB(Maths::Max(parB.Velocity(recipTime) | avgNormal, zero));

	//	const Maths::HF32 recipSumAbsSpeed(Maths::Reciprocal(Maths::Abs(relSpeedA) + Maths::Abs(relSpeedB)));
	//	const Maths::HF32 recipSumMass(Maths::Reciprocal(parA.m_Mass + parB.m_Mass));
	//	
	//	const Maths::HF32 scaleA(Maths::Select(parA.m_Mass.EQ(zero), zero, ((parA.m_CurPosition | avgNormal)
	//		- d) * m_Starch * parB.m_Mass * recipSumMass * relSpeedA*recipSumAbsSpeed)/* * m_Starch*/);

	//	const Maths::HF32 scaleB(Maths::Select(parB.m_Mass.EQ(zero), zero, ((parB.m_CurPosition | avgNormal)
	//		- d) * m_Starch * parA.m_Mass * recipSumMass * relSpeedB*recipSumAbsSpeed)/* * m_Starch*/);

	//	//if(m_Particles[edge.particleA].m_Mass != zero)
	//	{
	//		parA.m_CurPosition -= avgNormal * scaleA;
	//		//parA.m_Velocity -= avgNormal * scaleA * doubleRecipTime;
	//	}
	//	//if(m_Particles[edge.particleB].m_Mass != zero)
	//	{
	//		parB.m_CurPosition -= avgNormal * scaleB;
	//		//parB.m_Velocity -= avgNormal * scaleB*doubleRecipTime;
	//	}
	//}

	//Calculates the friction and contact impulse!
	Maths::HVec3 C_Cloth::CalculateImpulse(C_ParticleContact& contact, C_GravityParticle& particle, const Maths::HF32 PCREF recipTime)
	{
		using namespace Maths;

		const HF32 one(Constants<HF32>::One());
		const HF32 zero(Constants<HF32>::Zero());
		SharedPhysics::C_RigidBody* pRigid = contact.m_pNodePtr->m_pRigidBody;
		C_Normal n(-contact.m_EscapeVector);
		//C_Normal n(contact.m_TriangleNormal);
		const HVec3 r(pRigid->CenterOfRotation() - contact.m_Contact);
		

		const HVec3 rXn(n^r);
		const HF32 temp= n | (r ^ ( rXn * pRigid->m_InverseInertiaTensor ));
		const HF32 denom(pRigid->ReciprocalMass() + particle.m_MassInverse + temp);

		//NB. Add on penetration velocity into point velocity!
		const HVec3 pointVel(pRigid->PointVelocity(contact.m_Contact) + (m_Baumgarte * recipTime * contact.m_Penetration * contact.m_EscapeVector));
	
		const HVec3 relVel(pointVel - particle.Velocity(recipTime));
		//const HVec3 relVel(pointVel);

		const HF32 numer = (relVel|n);//- (pRigid->m_AngularVelocity|rXn);
		
		////The collision impulse in the normal direction
		const HF32 J(-(one + m_CoefficientOfRestitution)*Maths::Min(numer, zero)*denom.Reciprocal());

		/*const C_Normal v(relVel^n);
		const C_Normal frictionDirection(n^v);*/
		const HVec3 v((relVel | n) * n);
		const C_Normal frictionDirection(relVel - v);
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(constraint.m_OriginalContact, frictionDirection, 0.5f, 0.05f, Maths::RGBA(0, 255, 255));
		
		return J*n - m_FrictionCoefficient *J* frictionDirection;		
	}

		//Calculates the friction and contact impulse!
	Maths::HVec3 C_Cloth::CalculateImpulse(C_ParticleContact& contact, const Maths::HF32 PCREF recipTime)
	{
		using namespace Maths;

		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());
		SharedPhysics::C_RigidBody* pRigid = contact.m_pNodePtr->m_pRigidBody;
		C_Normal n(-contact.m_EscapeVector);
		//C_Normal n(contact.m_TriangleNormal);
		const HVec3 r(pRigid->CenterOfRotation() - contact.m_Contact);
		

		const HVec3 rXn(n^r);
		const HF32 temp= n | (r ^ ( rXn * pRigid->m_InverseInertiaTensor ));
		const HF32 denom(pRigid->ReciprocalMass() + temp);
		const HVec3 pointVel = pRigid->PointVelocity(contact.m_Contact);
	
		//const HVec3 relVel(pointVel - particle.Velocity(recipTime));
		const HVec3 relVel(pointVel);

		const HF32 numer = (relVel|n);//- (pRigid->m_AngularVelocity|rXn);
		
		////The collision impulse in the normal direction
		const HF32 J(-(one + m_CoefficientOfRestitution)*numer*denom.Reciprocal());

		const C_Normal v(relVel^n);
		const C_Normal frictionDirection(n^v);
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(constraint.m_OriginalContact, frictionDirection, 0.5f, 0.05f, Maths::RGBA(0, 255, 255));
		
		return J*n - m_FrictionCoefficient *J* frictionDirection;		
	}

	void C_Cloth::AddTriangleContact(SharedPhysics::C_GraphNode* pClothNode, SharedPhysics::C_GraphNode* pNodePtr, const Maths::HVec3 PCREF contact,
		 const Maths::C_Normal PCREF escapeVector, const Maths::C_Normal PCREF triangleNormal, const Maths::HF32 PCREF penetration,
		 const Maths::HF32 PCREF stiffness, U32 triangleIndex)
	{
		Collision::C_ParticleContact constraint;
		constraint.m_Type = C_ParticleContact::E_TriangleConstraint;
		constraint.m_Contact = contact;
		constraint.m_EscapeVector = escapeVector;
		constraint.m_TriangleNormal = triangleNormal;
		constraint.m_Index = triangleIndex;
		constraint.m_Penetration = penetration;
		constraint.m_pNodePtr = pNodePtr;
		constraint.m_pParticlePtr = pClothNode;
		constraint.m_Stiffness = stiffness;
		SharedPhysics::SortedArray<C_ParticleContact>* pArr = 
			SharedPhysics::Fast_Select<SharedPhysics::SortedArray<C_ParticleContact> >(pNodePtr->TestNodeFlags(SharedPhysics::C_GraphNode::E_OneWayClothCollision | SharedPhysics::C_GraphNode::E_Type_KinematicObject | SharedPhysics::C_GraphNode::E_Type_Static), 
			&m_OneWayContacts, &m_Contacts);
		pArr->AddToEnd(&constraint);
	}

	void C_Cloth::AddVertexContact(SharedPhysics::C_GraphNode* pClothNode, SharedPhysics::C_GraphNode* pNodePtr, const Maths::HVec3 PCREF contact,
		const Maths::C_Normal PCREF escapeVector, const Maths::C_Normal PCREF triangleNormal, const Maths::HF32 PCREF penetration,
		const Maths::HF32 PCREF stiffness, U32 vertexIndex)
	{
		Collision::C_ParticleContact constraint;
		constraint.m_Type = C_ParticleContact::E_VertexConstraint;
		constraint.m_Contact = contact;
		constraint.m_EscapeVector = escapeVector;
		constraint.m_TriangleNormal = triangleNormal;
		constraint.m_Index = vertexIndex;
		constraint.m_Penetration = penetration;
		constraint.m_pNodePtr = pNodePtr;
		constraint.m_pParticlePtr = pClothNode;
		constraint.m_Stiffness = stiffness;
		SharedPhysics::SortedArray<C_ParticleContact>* pArr = 
			SharedPhysics::Fast_Select<SharedPhysics::SortedArray<C_ParticleContact> >(pNodePtr->TestNodeFlags(SharedPhysics::C_GraphNode::E_OneWayClothCollision | SharedPhysics::C_GraphNode::E_Type_KinematicObject | SharedPhysics::C_GraphNode::E_Type_Static), &m_OneWayContacts, &m_Contacts);
		pArr->AddToEnd(&constraint);
	}


	void C_Cloth::Tear()
	{
		using namespace Maths;
	
		const HF32 tearThreshold(m_TearThreshold);
		for(U32 j=0; j< m_numEdges; ++j)
		{
			C_ConstrainedParticleEdge& edge=m_pEdges[j];
			if(edge.m_Active)
			{
				//const Maths::HF32 restLength(Maths::Load<Maths::HF32>(&edge.restLength));
				const HF32 restLength(Maths::Load<Maths::HF32>(&edge.m_RestLength));
				Collision::C_GravityParticle& parA = m_pParticles[edge.m_IndexA];
				Collision::C_GravityParticle& parB = m_pParticles[edge.m_IndexB];
				
				//constraint these two particles using the strench and compress coefficient
				const HF32 recipMassSum(Reciprocal(parA.m_Mass + parB.m_Mass));

				const HVec3 delta(parB.m_CurPosition - parA.m_CurPosition);
				const HF32 distanceSq(delta|delta);
				const HF32 tear(tearThreshold*restLength);
				if(distanceSq > (tear * tear))
				{
					RemoveEdge(j);
				}
			}
		}
	}
}

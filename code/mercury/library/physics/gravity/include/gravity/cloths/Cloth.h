#ifndef GRAVITY_CLOTH_H
#define GRAVITY_CLOTH_H

#include "gravity/cloths/Particle.h"
#include "gravity/sharedphysics/SortedArray.h"
#include "gravity/cloths/SoftBody.h"




namespace SharedPhysics
{
	class C_GraphNode;
}

namespace Collision
{

	class C_CollisionObject;


	class C_Cloth : public C_SoftBodyTree
	{

	public:

		C_Cloth(const U32 numIteration = 5): C_SoftBodyTree(Collision::E_ClothSystem, numIteration), m_TearThreshold(1.0e6f),
			m_ClothMass(Maths::UNDEFINED), m_BodyLinearDampingCoefficient(Maths::Constants<Maths::HF32>::One()), 
			m_BodyAngularDampingCoefficient(Maths::Constants<Maths::HF32>::One()), 
			m_FrictionCoefficient(Maths::Constants<Maths::HF32>::Zero()), m_CoefficientOfRestitution(Maths::Constants<Maths::HF32>::Zero()),
			m_Starch(Maths::Constants<Maths::HF32>::Zero()), m_Slop(Maths::Constants<Maths::HF32>::Zero()), 
			m_StretchCoefficient(Maths::Constants<Maths::HF32>::Zero()), m_CompressCoefficient(Maths::Constants<Maths::HF32>::Zero()),
			m_Baumgarte(Maths::Constants<Maths::HF32>::Zero()),
			m_TotalMass(Maths::Constants<Maths::HF32>::Zero()), m_ReciprocalTotalMass(Maths::Constants<Maths::HF32>::Zero())
		{
		}

		virtual ~C_Cloth()
		{
		}

		C_Cloth& operator = (const C_Cloth& other)
		{
			m_ClothMass = other.m_ClothMass;
			m_BodyLinearDampingCoefficient = other.m_BodyLinearDampingCoefficient;
			m_BodyAngularDampingCoefficient = other.m_BodyAngularDampingCoefficient;
			m_FrictionCoefficient = other.m_FrictionCoefficient;
			m_CoefficientOfRestitution = other.m_CoefficientOfRestitution;
			m_Starch = other.m_Starch;
			m_Slop = other.m_Slop;
			m_Damping = other.m_Damping;
			m_StretchCoefficient = other.m_StretchCoefficient;
			m_CompressCoefficient = other.m_CompressCoefficient;
			m_TearThreshold = other.m_TearThreshold;

			m_numEdges = other.m_numEdges;
			m_numTriangles = other.m_numTriangles;
			m_numParticles = other.m_numParticles;
			m_numNodes = other.m_numNodes;


			if(m_pNodes)
				delete[] m_pNodes;

			if(m_pEdges)
				delete[] m_pEdges;

			if(m_pEdgeIndices)
				delete[] m_pEdgeIndices;

			if(m_pTriangles)
				delete[] m_pTriangles;

			if(m_pParticles)
				delete[] m_pParticles;
	
			for(U32 a = 0; a < m_numNodes; ++a)
			{
				m_pNodes[a] = other.m_pNodes[a];
			}

			for(U32 a = 0; a < m_numEdges; ++a)
			{
				m_pEdges[a] = other.m_pEdges[a];
			}

			for(U32 a = 0; a < m_numEdges; ++a)
			{
				m_pEdgeIndices[a] = other.m_pEdgeIndices[a];
			}

			for(U32 a = 0; a < m_numTriangles; ++a)
			{
				m_pTriangles[a] = other.m_pTriangles[a];
			}

			for(U32 a = 0; a < m_numParticles; ++a)
			{
				m_pParticles[a] = other.m_pParticles[a];
			}
			return *this;
		}

		C_Cloth& operator = (const C_SoftBodyTree& other)
		{
			m_pExternalConstants = other.m_pExternalConstants;
			m_MaxSpeedSq = other.m_MaxSpeedSq;
			m_numIterations = other.m_numIterations;
			m_numEdges = other.m_numEdges;
			m_numTriangles = other.m_numTriangles;
			m_numParticles = other.m_numParticles;
			m_numNodes = other.m_numNodes;
			m_Damping = other.m_Damping;


			if(m_pNodes)
				delete[] m_pNodes;

			if(m_pEdges)
				delete[] m_pEdges;

			if(m_pEdgeIndices)
				delete[] m_pEdgeIndices;

			if(m_pTriangles)
				delete[] m_pTriangles;


		
			if(m_pParticles)
				delete[] m_pParticles;

			m_pNodes = new C_SoftBodyAABBNode[m_numNodes];
			m_pEdges = new C_ConstrainedParticleEdge[m_numEdges];
			m_pEdgeIndices = new C_EdgeTriangleIndices[m_numEdges];
			m_pTriangles = new C_SoftBodyTriangle[m_numTriangles];
			m_pParticles = new C_GravityParticle[m_numParticles];

			for(U32 a = 0; a < m_numNodes; ++a)
			{
				m_pNodes[a] = other.m_pNodes[a];
			}

			for(U32 a = 0; a < m_numEdges; ++a)
			{
				m_pEdges[a] = other.m_pEdges[a];
			}

			for(U32 a = 0; a < m_numEdges; ++a)
			{
				m_pEdgeIndices[a] = other.m_pEdgeIndices[a];
			}

			for(U32 a = 0; a < m_numTriangles; ++a)
			{
				m_pTriangles[a] = other.m_pTriangles[a];
			}

			for(U32 a = 0; a < m_numParticles; ++a)
			{
				m_pParticles[a] = other.m_pParticles[a];
			}
			return *this;
		}	

		void SetMass(const Maths::HF32 PCREF mass)
		{
			//We've been given the total mass. Need to share this among all the particles...
			m_TotalMass = mass;
			m_ReciprocalTotalMass = Maths::Reciprocal(mass);
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			const Maths::HF32 numParticles((F32)m_numParticles);
			const Maths::HF32 particleMass(mass/numParticles);
			const Maths::HF32 particleRecipMass(particleMass.Reciprocal());
			for(U32 a = 0; a < m_numParticles; ++a)
			{
				if(m_pParticles[a].m_Mass > zero)
				{
					m_pParticles[a].m_Mass = particleMass;
					m_pParticles[a].m_MassInverse = particleRecipMass;
				}
			}
		}


		void SetStretchCoefficient(const Maths::HF32 PCREF stretch)
		{
			m_StretchCoefficient = stretch;
		}

		void SetCompressCoefficient(const Maths::HF32 PCREF compress)
		{
			m_CompressCoefficient = compress;
		}

		void SetTearThreshold(const Maths::HF32 PCREF tearThreshold)
		{
			m_TearThreshold = tearThreshold;
		}

		void SetReferencePoint(const Maths::HVec3 PCREF reference, const U32 index)
		{
			Collision::C_GravityParticle& par = m_pParticles[index];
			par.MakeStatic();
		}
	
		void AddTriangleContact(SharedPhysics::C_GraphNode* pClothNode, SharedPhysics::C_GraphNode* pNodePtr, const Maths::HVec3 PCREF originalContact,
		const Maths::C_Normal PCREF escapeVector, const Maths::C_Normal PCREF triangleNormal, const Maths::HF32 PCREF OriginalPenetration,
		 const Maths::HF32 PCREF stiffness, U32 triangleIndex);


		void AddVertexContact(SharedPhysics::C_GraphNode* pClothNode, SharedPhysics::C_GraphNode* pNodePtr, const Maths::HVec3 PCREF originalContact,
		 const Maths::C_Normal PCREF escapeVector, const Maths::C_Normal PCREF triangleNormal, const Maths::HF32 PCREF originalPenetration,
		 const Maths::HF32 PCREF stiffness, U32 vertexIndex);

		void SetNumIterations(const U32 numIterations)
		{
			m_numIterations = numIterations;
		}

		void UpdateRigidBodiesBasedOnSpring(const Maths::HF32 PCREF time);
		
		void ProjectParticles(const Maths::HF32 PCREF time);

		void SetFrictionCoefficient(const Maths::HF32 PCREF friction)
		{
			m_FrictionCoefficient = friction;
		}

		void SetCoefficientOfRestitution(const Maths::HF32 PCREF coefficientOfRestitution)
		{
			m_CoefficientOfRestitution = coefficientOfRestitution;
		}

		void SetStarch(const Maths::HF32 PCREF starch)
		{
			m_Starch = starch;
		}

		void UpdateParticlesBasedOnImpulse(const Maths::HF32 PCREF time);
		
		void Tear();

		void TimeStep(const Maths::HF32 PCREF time, SharedPhysics::C_SafeStack<true>& stack);

		void TimeStep(const Maths::HF32 PCREF time);

		void FlattenEdge(const C_SoftBodyEdge& edge, const Maths::HF32 PCREF doubleRecipTime);

		Maths::HVec3 CalculateImpulse(C_ParticleContact& constraint, C_GravityParticle& particle,const Maths::HF32 PCREF recipTime);

		Maths::HVec3 CalculateImpulse(C_ParticleContact& constraint, const Maths::HF32 PCREF recipTime);
	
		
		SharedPhysics::SortedArray<C_ParticleContact> m_Contacts;
		SharedPhysics::SortedArray<C_ParticleContact> m_OneWayContacts;	
		Maths::HF32 m_ClothMass;
		Maths::HF32 m_BodyLinearDampingCoefficient;
		Maths::HF32 m_BodyAngularDampingCoefficient;
		Maths::HF32 m_FrictionCoefficient;
		Maths::HF32 m_CoefficientOfRestitution;
		Maths::HF32 m_Starch;
		Maths::HF32 m_Slop;
		Maths::HF32 m_StretchCoefficient;
		Maths::HF32 m_CompressCoefficient;
		Maths::HF32 m_TearThreshold;
		Maths::HF32 m_Baumgarte;
		Maths::HF32 m_TotalMass;
		Maths::HF32 m_ReciprocalTotalMass;
	};
}
#endif // GRAVITY_CLOTH_H

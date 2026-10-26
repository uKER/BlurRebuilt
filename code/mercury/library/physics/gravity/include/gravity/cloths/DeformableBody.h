#ifndef DEFORMABLEBODY_H
#define DEFORMABLEBODY_H

#include "gravity/cloths/SoftBody.h"
#include "gravity/cloths/Particle.h"
#include "gravity/sharedPhysics/SortedArray.h"

namespace Collision
{

	class C_DeformableParticle : public C_GravityParticle
	{
	public:
		C_DeformableParticle() : C_GravityParticle(), m_InitialPosition(Maths::ZERO_INIT), m_GoalPosition(Maths::ZERO_INIT), m_Velocity(Maths::ZERO_INIT)
		{
		}

		virtual ~C_DeformableParticle()
		{
		}
		
		Maths::HVec3 m_InitialPosition;
		Maths::HVec3 m_GoalPosition;
		Maths::HVec3 m_Velocity;
	};

	class C_DeformableBody : public C_SoftBodyTree
	{
	public:
		C_DeformableBody(const U32 numIteration = 5): C_SoftBodyTree(Collision::E_DeformableSystem, numIteration), m_SumOfMassInverse(Maths::Constants<Maths::HF32>::Zero()),
			m_InitialCenterOfMass(Maths::Constants<Maths::HVec3>::Zero()), m_CurCenterOfMass(Maths::Constants<Maths::HVec3>::Zero()),
			m_StiffnessCoefficient(Maths::Constants<Maths::HF32>::Zero())
		{
		}

		virtual ~C_DeformableBody()
		{
		}

		void Initialize();

		virtual void SetNumParticles(const U32 numParticles)
		{
			m_numParticles = numParticles;
			m_pParticles = new C_DeformableParticle[m_numParticles];
		}

		void CalculateInitialCenterOfMass();

		void CalculateInitialVectorFromPointToCenterOfMass();

		void CalculateCurCenterOfMass();
	
		void CalcualteGoalPosition();

		void TimeStep(const Maths::HF32 PCREF time, SharedPhysics::C_SafeStack<true>& stack);

		void TimeStep(const Maths::HF32 PCREF time);

		C_DeformableBody& operator = (const C_SoftBodyTree& other)
		{
			//m_CompressCoefficient = other.m_CompressCoefficient;
			//m_Damping = other.m_Damping;
			m_pExternalConstants = other.m_pExternalConstants;
			m_MaxSpeedSq = other.m_MaxSpeedSq;
			m_numIterations = other.m_numIterations;
			//m_StretchCoefficient = other.m_StretchCoefficient;
			m_Thickness = other.m_Thickness;
			m_numEdges = other.m_numEdges;
			m_numTriangles = other.m_numTriangles;
			m_numParticles = other.m_numParticles;
			m_numNodes = other.m_numNodes;

			if(m_pNodes)
				delete[] m_pNodes;

			if(m_pEdges)
				delete[] m_pEdges;

			if(m_pTriangles)
				delete[] m_pTriangles;

			if(m_pParticles)
				delete[] m_pParticles;

			m_pNodes = new C_SoftBodyAABBNode[m_numNodes];
			m_pEdges = new C_ConstrainedParticleEdge[m_numEdges];
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
		

		void SetStiffnessCoefficient(const Maths::HF32 PCREF stiffness)
		{
			m_StiffnessCoefficient = stiffness;
		}

		virtual void Verlet(const Maths::HF32 PCREF time);

		SharedPhysics::SortedArray<Maths::HVec3 > m_InitialQ;
		Maths::HVec3 m_InitialCenterOfMass;
		Maths::HVec3 m_CurCenterOfMass;
		Maths::HF32 m_SumOfMassInverse;
		Maths::HF32 m_StiffnessCoefficient;
	};
}

#endif

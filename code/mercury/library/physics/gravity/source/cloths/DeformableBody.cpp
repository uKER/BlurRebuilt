#include "stdafx.h"
#include "gravity/cloths/DeformableBody.h"

namespace Collision
{

	void C_DeformableBody::Initialize()
	{
		CalculateInitialCenterOfMass();
		CalculateInitialVectorFromPointToCenterOfMass();
	}

	void C_DeformableBody::CalculateInitialCenterOfMass()
	{
		using namespace Maths;

		HVec3 sumOfPosition = Constants<HVec3>::Zero();
		
		for(U32 i=0; i<m_numParticles; ++i)
		{
			const C_DeformableParticle* particle = (C_DeformableParticle*)&m_pParticles[i];
			sumOfPosition += particle->m_InitialPosition * particle->m_Mass;
			m_SumOfMassInverse += particle->m_MassInverse;
		}

		m_InitialCenterOfMass = sumOfPosition * m_SumOfMassInverse;

	}

	void C_DeformableBody::CalculateInitialVectorFromPointToCenterOfMass()
	{
		for(U32 i=0; i<m_numParticles; ++i)
		{
			const C_DeformableParticle* particle = (C_DeformableParticle*)&m_pParticles[i];
			Maths::HVec3 q(particle->m_InitialPosition - m_InitialCenterOfMass);
			m_InitialQ.AddToEnd(&q);
		}
	}

	void C_DeformableBody::CalculateCurCenterOfMass()
	{
		using namespace Maths;

		HVec3 sumOfPosition = Constants<HVec3>::Zero();
	
		for(U32 i=0; i<m_numParticles; ++i)
		{
			const C_DeformableParticle* particle = (C_DeformableParticle*)&m_pParticles[i];
			sumOfPosition += particle->m_CurPosition * particle->m_Mass;
		}

		m_CurCenterOfMass = sumOfPosition * m_SumOfMassInverse;
	}

	void C_DeformableBody::CalcualteGoalPosition()
	{
		using namespace Maths;

		HMat33 Apq(Maths::Constants<HMat33>::Zero());
		//HMat33 Aqq(Maths::Constants<HMat33>::Zero());

		for(U32 i=0; i<m_numParticles; ++i)
		{
			const C_DeformableParticle* particle = (C_DeformableParticle*)&m_pParticles[i];
			const HVec3 q=m_InitialQ[i];
			const HVec3 p=particle->m_CurPosition - m_InitialCenterOfMass;
			const HVec3 a0 = particle->m_Mass * q.X() * p;
			const HVec3 a1 = particle->m_Mass * q.Y() * p;
			const HVec3 a2 = particle->m_Mass * q.Z() * p;
			Apq = Apq + HMat33(a0, a1, a2);
			
			/*const HVec3 b0 = m_Particles[i].m_Mass * p.X() * p;
			const HVec3 b1 = m_Particles[i].m_Mass * p.Y() * p;
			const HVec3 b2 = m_Particles[i].m_Mass * p.Z() * p;

			Aqq += HMat33(b0, b1, b2);*/
		}

		HMat33 A = Apq * Transpose(Apq);

		C_Eigen33 eigen = A.EigenDecomposition();

		//Construct the matrix V which volumn is the eigenvector of A, the eigenvector of a sysmetric matrix is alway orthogonal
		const HMat33 V(eigen.Vector0(), eigen.Vector1(), eigen.Vector2());
		const HMat33 invV(Transpose(V));
		
		//Get the square root of mat sqrt(A) = V* sqrt(D) * invV;  
		HMat33 D(Constants<HMat33>::Identity());
		
		D.AX(D.AX() * Sqrt(eigen.Value0()));
		D.AY(D.AY() * Sqrt(eigen.Value1()));
		D.AZ(D.AZ() * Sqrt(eigen.Value2()));

		const HMat33 S = invV * D * V;

		const HMat33 invS = Transpose(S);
		const HMat33 R = invS * Apq;

		for(U32 i=0; i<m_numParticles; ++i)
		{
			C_DeformableParticle* particle = (C_DeformableParticle*)&m_pParticles[i];
			particle->m_GoalPosition = m_InitialQ[i] * R + m_CurCenterOfMass;
		}
	}

	void C_DeformableBody::Verlet(const Maths::HF32 PCREF time)
	{
		const Maths::HF32 recipTime(time.Reciprocal());
		const Maths::HF32 sqTime(time * time);
		for(U32 i=0; i<m_numParticles; ++i)
		{
			C_DeformableParticle* particle = (C_DeformableParticle*)&m_pParticles[i];
			const Maths::HVec3 dif((particle->m_GoalPosition - particle->m_CurPosition) * m_StiffnessCoefficient);
			const Maths::HVec3 vel(particle->m_Velocity + dif* recipTime);
			particle->m_CurPosition = particle->m_CurPosition + vel*time + ( particle->m_ExternalForce*particle->m_MassInverse*sqTime);
		}
	}

	void C_DeformableBody::TimeStep(const Maths::HF32 PCREF time, SharedPhysics::C_SafeStack<true>& stack)
	{
		TimeStep(time);
		UpdateTree(stack);
	}

	void C_DeformableBody::TimeStep(const Maths::HF32 PCREF time)
	{
		/*CalculateCurCenterOfMass();
		CalcualteGoalPosition();
		Verlet(time);*/
	}
}
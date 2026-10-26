#include "StdAfx.h"
#include "gravity/cloths/Particle.h"
#include "gravity/sharedphysics/rigidbody.h"
#include "gravity/sdk/Body.h"
#include <system/perf.h>

namespace Collision
{

	void C_ParticleSystem::AccumulateForces()
	{
		for(U32 i=0; i<m_numParticles; ++i)
		{
			m_pParticles[i].SetExternalForce(m_pExternalConstants->m_Gravity * m_pParticles[i].m_Mass + m_pExternalConstants->m_Wind);
		}
	}

	void C_ParticleSystem::Verlet(const Maths::HF32 PCREF time)
	{
		PERF_EVENT("C_ParticleSystem.Verlet");
		using namespace Maths;
		HF32 sqTime=time*time;
		HF32 one(Constants<HF32>::One());
		HF32 two(Constants<HF32>::Two());
		const Maths::HF32 recipTime(time.Reciprocal());
		const HF32 damping(m_Damping);
		//const Maths::HF32 recipTimeSq(recipTime * recipTime);

		Maths::HF32 maxSpeedSq(Maths::Constants<Maths::HF32>::Zero());
		for(U32 i=0; i<m_numParticles; ++i)
		{
			Collision::C_GravityParticle& particle(m_pParticles[i]);
			if(particle.m_Active)
			{
				const HVec3 initialPosition=particle.m_CurPosition;
				const HVec3 prePosition=particle.m_PrePosition;
				const Maths::HVec3 dif((initialPosition - prePosition) * damping);
				const HVec3 curPosition = initialPosition + dif + ( m_pParticles[i].m_ExternalForce*m_pParticles[i].m_MassInverse*sqTime);
				particle.m_PrePosition=initialPosition;
				particle.m_CurPosition = curPosition;
				const Maths::HVec3 vel((curPosition - prePosition) * recipTime);
				const Maths::HF32 speedSq((vel | vel));
				/*const Maths::HVec3 displacement((curPosition - prePosition));
				const Maths::HF32 movement(displacement | displacement);*/
				//maxSpeedSq = Maths::Max(maxSpeedSq, movement);
				maxSpeedSq = Maths::Max(maxSpeedSq, speedSq);
			}
		}
		m_MaxSpeedSq = maxSpeedSq;
	}

	void C_ConstrainedParticleSystem::ConnectParticle(U32 particleIndex, Gravity::C_Body* pBody, const Maths::HVec3 PCREF relativePosition)
	{
		BIZ_ASSERT(m_numJoints < m_maxJoints);
		BIZ_ASSERT(particleIndex < m_numParticles);
		Collision::C_GravityParticle& particle = m_pParticles[particleIndex];
		particle.m_Mass = particle.m_MassInverse = Maths::Constants<Maths::HF32>::Zero();
		Collision::C_ParticleJoint& joint = m_pJoints[m_numJoints++];
		joint.m_ParticleIndex = particleIndex;
		joint.m_pBody = pBody;
		joint.m_RelativePosition = relativePosition;
	}

	void C_ConstrainedParticleSystem::RemoveEdge(U32 edgeIndex)
	{
		Collision::C_ConstrainedParticleEdge& edge = m_pEdges[edgeIndex];
		//m_pEdges[edgeIndex] = m_pEdges[m_NumEdges - 1];
		edge.m_Active = false;

		Collision::C_GravityParticle& parA = m_pParticles[edge.m_IndexA];
		Collision::C_GravityParticle& parB = m_pParticles[edge.m_IndexB];

		U32 endA = this->m_pParticleHeader[edge.m_IndexA + 1];
		U32 endB = this->m_pParticleHeader[edge.m_IndexB + 1];
	
		U32 numActive = 0;
		for(U32 a = m_pParticleHeader[edge.m_IndexA]; a < endA; ++a)
		{
			if(m_pParticleEdgeTable[a] == edgeIndex)
			{
				m_pParticleEdgeTable[a] = -1;
			}
			else if (m_pParticleEdgeTable[a] != -1)
			{
				++numActive;
			}
		}
		if(numActive == 0)
		{
			parA.m_Active = false;
		}

		numActive = 0;
		for(U32 a = m_pParticleHeader[edge.m_IndexB]; a < endB; ++a)
		{
			if(m_pParticleEdgeTable[a] == edgeIndex)
			{
				m_pParticleEdgeTable[a] = -1;
			}
			else if (m_pParticleEdgeTable[a] != -1)
			{
				++numActive;
			}
		}
		if(numActive == 0)
		{
			parB.m_Active = false;
		}
	}

	void C_ConstrainedParticleSystem::CalculateParticleEdgeData()
	{
		m_pParticleHeader = new U32[m_numParticles+1];
		U32 offset = 0;
		m_pParticleHeader[0] = 0;
		for(U32 a = 0; a < m_numParticles; ++a)
		{
			//Collision::C_GravityParticle& par = m_Particles[a];
			for(U32 e = 0; e < this->m_numEdges; ++e)
			{
				Collision::C_ConstrainedParticleEdge& edge = m_pEdges[e];
				if(a == edge.m_IndexA || a == edge.m_IndexB)
				{
					++offset;
				}
			}
			m_pParticleHeader[a+1] = offset;
		}
		this->m_pParticleEdgeTable = new S32[offset];
		offset = 0;
		for(U32 a = 0; a < m_numParticles; ++a)
		{
			//Collision::C_GravityParticle& par = m_Particles[a];
			for(U32 e = 0; e < m_numEdges; ++e)
			{
				Collision::C_ConstrainedParticleEdge& edge = m_pEdges[e];
				if(a == edge.m_IndexA || a == edge.m_IndexB)
				{
					m_pParticleEdgeTable[offset++] = e;
				}
			}
		}

	}

	void C_ConstrainedParticleSystem::SatisfyConstraints()
	{
#if !CFG_CONVERTER
		PERF_EVENT("Gravity.ConstrainedParticleSystem.SatisfyConstraints");
		using namespace Maths;
		
		const HF32 zero(Constants<HF32>::Zero());

		for(U32 i = 0; i < m_numJoints; ++i)
		{
			Collision::C_ParticleJoint& joint = this->m_pJoints[i];
			Collision::C_GravityParticle& particle = m_pParticles[joint.m_ParticleIndex];
			SharedPhysics::C_RigidBody& body = joint.m_pBody->GetBody();
			const Maths::HVec3 transformedPoint((joint.m_RelativePosition * body.m_OrientationMatrix) + body.m_Position);
			particle.m_CurPosition = particle.m_PrePosition = transformedPoint;
		}


		const HF32 stretchCoefficient(m_StretchCoefficient);
		const HF32 compressCoefficient(m_CompressCoefficient);

		for(U32 i=0; i< m_numIterations; ++i)
		{
			for(U32 j=0; j< this->m_numEdges; ++j)
			{
				C_ConstrainedParticleEdge& edge=m_pEdges[j];
				if(edge.m_Active)
				{
					//const Maths::HF32 restLength(Maths::Load<Maths::HF32>(&edge.restLength));
					const HF32 restLength(Maths::Load<Maths::HF32>(&edge.m_RestLength));
					Collision::C_GravityParticle& parA = m_pParticles[edge.m_IndexA];
					Collision::C_GravityParticle& parB = m_pParticles[edge.m_IndexB];
					
					//constraint these two particles using the strench and compress coefficient
					const HVec3 delta(parB.m_CurPosition - parA.m_CurPosition);
					const HF32 recipDistance(delta.ReciprocalLength());

					//Pull out reciprocals
					const Maths::HVec3 tempVector(recipDistance, parA.m_Mass + parB.m_Mass, recipDistance);
					const Maths::HVec3 recipVector(Maths::Reciprocal(tempVector));
					const HF32 distance(recipVector.X());
					const HF32 recipMassSum(recipVector.Y());

					const HF32 stretch(stretchCoefficient*restLength);
					const HF32 compress(compressCoefficient*restLength);

					const HVec3 dir(delta * recipDistance);
					Scalar_Bool con1(stretch.LT(distance));
					Scalar_Bool con2(compress.GT(distance));

					const HF32 dif(Select(con1, distance - stretch, Select(con2, distance - compress, zero)));
					const HVec3 difVec = dir * dif * recipMassSum;

					parA.m_CurPosition = parA.m_CurPosition + (parA.m_Mass * difVec);
					parB.m_CurPosition = parB.m_CurPosition - (parB.m_Mass * difVec);
				}
			}

		}
#endif
	}

//	void C_ConstrainedParticleSystem::SatisfyConstraints()
//	{
//#if !CFG_CONVERTER
//		PERF_EVENT("Gravity.CPS.SatisfyConstraints");
//		using namespace Maths;
//		
//		const HF32 zero(Constants<HF32>::Zero());
//
//		for(U32 i = 0; i < m_numJoints; ++i)
//		{
//			Collision::C_ParticleJoint& joint = this->m_pJoints[i];
//			Collision::C_GravityParticle& particle = m_pParticles[joint.m_ParticleIndex];
//			SharedPhysics::C_RigidBody& body = joint.m_pBody->GetBody();
//			const Maths::HVec3 transformedPoint((joint.m_RelativePosition * body.m_OrientationMatrix) + body.m_Position);
//			particle.m_CurPosition = particle.m_PrePosition = transformedPoint;
//		}
//
//		for(U32 i=0; i< m_numIterations; ++i)
//		{
//			for(U32 j=0; j< m_numEdges; ++j)
//			{
//				C_ConstrainedParticleEdge& edge=m_pEdges[j];
//				if(edge.m_Active)
//				{
//					//const Maths::HF32 restLength(Maths::Load<Maths::HF32>(&edge.restLength));
//					const HF32 restLength(Maths::Load<Maths::HF32>(&edge.m_RestLength));
//					Collision::C_GravityParticle& parA = m_pParticles[edge.m_IndexA];
//					Collision::C_GravityParticle& parB = m_pParticles[edge.m_IndexB];
//					//HVec3& p1=parA.m_CurPosition;
//					//HVec3& p2=parB.m_CurPosition;
//					
//					////constraint these two particles into the original length
//					/*const HVec3 delta=p2-p1;
//					const HF32 distance=Sqrt(delta|delta);
//					const F32 diff=distance - edge.restLength;
//					const HVec3 difVec=(delta/distance)*half*diff;
//				
//					p1 +=difVec;
//					p2 -=difVec;*/
//					
//					//constraint these two particles using the strench and compress coefficient
//					const HF32 recipMassSum(Reciprocal(parA.m_Mass + parB.m_Mass));
//					const HVec3 delta(parB.m_CurPosition - parA.m_CurPosition);
//					//const HF32 distance=Sqrt(delta|delta);
//					const HF32 recipDist(delta.ReciprocalLength());
//					const HF32 distance(recipDist.Reciprocal());
//
//
//					const HF32 tearThreshold(m_TearThreshold*restLength);
//
//					if(distance > tearThreshold)
//					{
//						RemoveEdge(j);
//						j--;
//						continue;
//					}
//
//					const HF32 stretch(m_StretchCoefficient*restLength);
//					const HF32 compress(m_CompressCoefficient*restLength);
//
//					const HVec3 dir(delta / distance);
//				
//					Scalar_Bool con1(stretch.LT(distance));
//					Scalar_Bool con2(compress.GT(distance));
//
//					const HF32 dif(Select(con1, distance - stretch, Select(con2, distance - compress, zero)));
//					const HVec3 difVec = dir * dif * recipMassSum/* * m_Starch*/;
//
//					parA.m_CurPosition = parA.m_CurPosition + (parA.m_Mass * difVec);
//					parB.m_CurPosition = parB.m_CurPosition - (parB.m_Mass * difVec);
//
//				}
//			}
//
//		}
//#endif
//	}

}


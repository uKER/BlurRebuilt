#include "StdAfx.h"
#include "gravity/cloths/Rope.h"
#include "system/perf.h"

namespace Collision
{
	void C_Rope::InitialiseEdgesFromParticleList()
	{
		m_numEdges = (m_numParticles > 0) ? m_numParticles - 1 : 0;
		m_pEdges = new C_ConstrainedParticleEdge[m_numEdges];
		for(U32 a = 0; a < m_numEdges; ++a)
		{
			C_ConstrainedParticleEdge& edge = m_pEdges[a];
			edge.m_IndexA = a;
			edge.m_IndexB = a + 1;
			edge.m_RestLength = m_RestLength;
		}

	}

	void C_Rope::TimeStep(const Maths::HF32 PCREF time)
	{
		PERF_EVENT("Gravity.Rope.Solve");
		AccumulateForces();
		SatisfyConstraints();
		Verlet(time);
	}

	void C_Rope::GetBoundingBox(Collision::C_CRAxisBox& box)
	{
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		Maths::HVec3 vMin(Maths::Constants<Maths::HVec3>::Max());
		Maths::HVec3 vMax(-Maths::Constants<Maths::HVec3>::Max());

		for(U32 a = 0; a < m_numParticles; ++a)
		{
			vMin = Maths::Min(vMin, m_pParticles[a].m_CurPosition);
			vMax = Maths::Max(vMax, m_pParticles[a].m_CurPosition);
		}
		
		const Maths::HVec3 centre((vMin + vMax)*half);
		const Maths::HVec3 extents(vMax - centre);
		box.centre = centre;
		box.extents = extents;
	}

}
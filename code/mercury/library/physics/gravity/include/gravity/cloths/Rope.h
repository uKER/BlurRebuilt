#ifndef GRAVITY_ROPE_H
#define GRAVITY_ROPE_H

#include "gravity/cloths/particle.h"
#include "gravity/narrowphase/AxisBox.h"


namespace Collision
{
	/*
	 * Rope implemented using a spline, for which each vertex is extruded into a 
	 * capsule for collision detection and raytracing
	 */

	class C_RopeEdge
	{
	public:
		U32 m_IndexA, m_IndexB;
		F32 m_RestLength;

		bool operator >(const C_RopeEdge& other) const
		{
			return m_IndexA > other.m_IndexA || 
				(m_IndexA == other.m_IndexA && m_IndexB > other.m_IndexB);	
		}

		bool operator <=(const C_RopeEdge& other) const
		{
			return m_IndexA < other.m_IndexA || 
				(m_IndexA == other.m_IndexA && m_IndexB <= other.m_IndexB);	
		}

		bool operator <(const C_RopeEdge& other) const
		{
			return m_IndexA < other.m_IndexA || 
				(m_IndexA == other.m_IndexA && m_IndexB < other.m_IndexB);	
		}

		bool operator == (const C_RopeEdge& other) const
		{
			return (m_IndexA == other.m_IndexA && m_IndexB == other.m_IndexB) || 
				(m_IndexA == other.m_IndexB && m_IndexB == other.m_IndexA);
		}

		template < typename T_Stream > 
		void Serialise( T_Stream& stream ) const
		{
			stream << m_IndexA << m_IndexB << m_RestLength;
		}

		template < typename T_Stream > 
		void Deserialise( T_Stream& stream )
		{
			stream >> m_IndexA >> m_IndexB >> m_RestLength;
		}

		C_RopeEdge()
		{
			m_IndexA = m_IndexB = 0;
			m_RestLength = 0.f;
		}
	};


	class C_Rope : public C_ConstrainedParticleSystem
	{
	public:
		Maths::HF32 m_Thickness;
		Maths::HF32 m_RestLength;
	
		C_Rope() : C_ConstrainedParticleSystem(Collision::E_RopeSystem), m_Thickness(Maths::UNDEFINED), m_RestLength(Maths::UNDEFINED)
		{
		}

		void InitialiseEdgesFromParticleList();

		void TimeStep(const Maths::HF32 PCREF time);

		void GetBoundingBox(Collision::C_CRAxisBox& box);
	};
}

#endif
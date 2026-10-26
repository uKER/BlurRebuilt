#ifndef WATER_QUERY_H
#define WATER_QUERY_H

#include "gravity/narrowphase/CollisionFilter.h"

namespace Gravity
{

	class C_WaterQuery
	{
		friend class C_Scene;

		Maths::HVec3 m_TestPoint;
		Maths::C_Normal m_SurfaceNormal;
		Maths::HVec3 m_SurfacePoint;
		Maths::HF32 m_Depth;
		Collision::C_CollisionFilter m_Filter;
		bool m_UnderWater;
		
	public:
		C_WaterQuery(const Maths::HVec3 PCREF testPoint, const Collision::C_CollisionFilter* pFilter) : m_TestPoint(testPoint), m_SurfaceNormal(Maths::UNDEFINED),
			m_SurfacePoint(Maths::UNDEFINED), m_Depth(Maths::UNDEFINED), m_Filter(*pFilter), m_UnderWater(false)
		{
		}
		
		bool IsUnderWater()
		{
			return m_UnderWater;
		}

		Maths::C_Normal GetSurfaceNormal()
		{
			return m_SurfaceNormal;
		}

		Maths::HVec3 GetSurfacePoint()
		{
			return m_SurfacePoint;
		}

		Maths::HF32 GetDepth()
		{
			return m_Depth;
		}

		Collision::C_CollisionFilter* GetFilter()
		{
			return &m_Filter;
		}
	};

}

#endif

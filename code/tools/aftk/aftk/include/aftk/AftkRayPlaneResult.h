#ifndef AFTK_RAY_PLANE_RESULT_H
#define AFTK_RAY_PLANE_RESULT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_RayPlaneResult
	{
	public:
		C_RayPlaneResult();

		C_RayPlaneResult(bool hit, F32 distance);

		C_RayPlaneResult(const C_RayPlaneResult& rhs);

		bool Hit() const;

		F32 GetDistance() const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		void Set(bool hit);

		void Set(bool hit, F32 distance);

		bool m_hit;
		F32 m_distance;

		friend class C_Ray;
	#endif
	};

	FORCE_INLINE C_RayPlaneResult::C_RayPlaneResult()
	{
	}

	FORCE_INLINE C_RayPlaneResult::C_RayPlaneResult(
		bool hit,
		F32 distance)
	{
		Set(hit, distance);
	}

	FORCE_INLINE C_RayPlaneResult::C_RayPlaneResult(const C_RayPlaneResult& rhs)
	{
		*this = rhs;
	}

	FORCE_INLINE bool C_RayPlaneResult::Hit() const
	{
		return m_hit;
	}

	FORCE_INLINE F32 C_RayPlaneResult::GetDistance() const
	{
		return m_distance;
	}

	FORCE_INLINE void C_RayPlaneResult::Set(bool hit)
	{
		m_hit = hit;
	}

	FORCE_INLINE void C_RayPlaneResult::Set(bool hit, F32 distance)
	{
		m_hit = hit;
		m_distance = distance;
	}
}

#endif

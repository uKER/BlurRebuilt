#ifndef AFTK_RAY_TRIANGLE_RESULT_H
#define AFTK_RAY_TRIANGLE_RESULT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_RayTriangleResult
	{
	public:
		C_RayTriangleResult();

		C_RayTriangleResult(
			bool hit,
			const Position3& intersectionPoint);

		C_RayTriangleResult(const C_RayTriangleResult& rhs);

		bool Hit() const;

		const Position3& GetIntersectionPoint() const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	//private:
		void Set(
			bool hit,
			const Position3& intersectionPoint);

		void Set(bool hit);

		bool m_hit;
		Position3 m_intersectionPoint;

		friend class C_Ray;
	#endif
	};

	FORCE_INLINE C_RayTriangleResult::C_RayTriangleResult()
	{
	}

	FORCE_INLINE C_RayTriangleResult::C_RayTriangleResult(
		bool hit,
		const Position3& intersectionPoint)
	{
		Set(hit,
			m_intersectionPoint);
	}

	FORCE_INLINE C_RayTriangleResult::C_RayTriangleResult(const C_RayTriangleResult& rhs)
	{
		*this = rhs;
	}

	FORCE_INLINE bool C_RayTriangleResult::Hit() const
	{
		return m_hit;
	}

	FORCE_INLINE const Position3& C_RayTriangleResult::GetIntersectionPoint() const
	{
		return m_intersectionPoint;
	}

	FORCE_INLINE void C_RayTriangleResult::Set(bool hit)
	{
		m_hit = hit;
	}

	FORCE_INLINE void C_RayTriangleResult::Set(
		bool hit,
		const Position3& intersectionPoint)
	{
		m_hit = hit;
		m_intersectionPoint = intersectionPoint;
	}
}

#endif

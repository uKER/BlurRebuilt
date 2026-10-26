#ifndef AFTK_RAY_H
#define AFTK_RAY_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Ray
	{
	public:
		C_Ray(
			const Position3& position,
			const Normal3& direction, 
			F32 length);

		C_Ray(const C_Ray& rhs);

		void IntersectMesh(
			C_RayMeshResult& result, 
			C_SceneNode* pNode,
			U32 lodLevel,
			bool evaluate,
			bool findClosest) const;

		void IntersectTriangle(
			C_RayTriangleResult& result, 
			const Position3& p1, 
			const Position3& p2,
			const Position3& p3) const;

		void IntersectPlane(
			C_RayPlaneResult& result,
			const Position3& p0, 
			const Position3& p1,
			const Position3& p2) const;

		const Position3& Origin() const;

		Position3& Origin();

		const Normal3& Direction() const;

		F32 Length() const;

		void SetOrigin(const Position3& origin);

		void SetDirection(const Normal3& direction);

		void SetLength(F32 length);

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		Position3 m_origin;
		Normal3 m_direction;
		F32 m_length;
	#endif
	};

	 
	FORCE_INLINE C_Ray::C_Ray(
			const Position3& position,
			const Normal3& direction, 
			F32 length)

	{
		m_origin = position;
		m_direction = direction;
		m_length = length;
	}

	FORCE_INLINE C_Ray::C_Ray(const C_Ray& rhs)
	{
		*this = rhs;
	}

	FORCE_INLINE const Position3& C_Ray::Origin() const
	{
		return m_origin;
	}

	FORCE_INLINE Position3& C_Ray::Origin()
	{
		return m_origin;
	}

	FORCE_INLINE F32 C_Ray::Length() const
	{
		return m_length;
	}

	FORCE_INLINE const Normal3& C_Ray::Direction() const
	{
		return m_direction;
	}

	FORCE_INLINE void C_Ray::SetOrigin(const Position3& origin)
	{
		m_origin = origin;
	}

	FORCE_INLINE void C_Ray::SetLength(F32 length)
	{
		m_length = length;
	}
}

#endif

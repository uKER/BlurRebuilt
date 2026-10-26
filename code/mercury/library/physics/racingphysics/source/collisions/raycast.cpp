#include "stdafx.h"
#include "racingphysics/collisions/raycast.h"

//#pragma optimize("", off)

#include "racingphysics/collisions/collisioncache.inl"

//using namespace Maths;

//////////////////////////////////////////////////////////////////////////////////

namespace RacingPhysics
{

RayCast::RayCast(const Point3R& start, const Normal3R& direction, float maxDistance)
	:	m_start(start), 
		m_end(start + direction * maxDistance),
		m_direction(direction),
		m_maxDistance(maxDistance)
		//m_oneXZ(1.0, 0.0, 1.0)
{
	Point3R max = Max(m_start, m_end);
	Point3R min = Min(m_start, m_end);
	
	m_boundingBox.SetMinMax(min, max);
}

CollisionCache::BooleanType RayCast::CollideFeature(const CollisionCache::Triangle& triangle, CollisionCache::Contact& contact) const
{
	const Point3R& v0 = triangle.vertex0;
	const Point3R& v1 = triangle.vertex1;
	const Point3R& v2 = triangle.vertex2;

	// reject by triangle normal
	ScalarType a1 = triangle.normal | (v0 - m_start);
	ScalarType a2 = triangle.normal | (v0 - m_end);
	BooleanType reject1 = (a1 * a2 > Maths::Constants<ScalarType>::Zero());

	// reject by edges
	Vector3R l0 = (m_start - v0) ^ m_direction;
	ScalarType b1 = (m_start - v1) | l0;
	ScalarType b2 = (m_start - v2) | l0;
	BooleanType reject2 = (b1 * b2 > Maths::Constants<ScalarType>::Zero()); // plane containing the line and one vertex must intersect the opposite edge

	Vector3R l1 = (m_start - v1) ^ m_direction;
	ScalarType c1 = (m_start - v0) | l1;
	ScalarType c2 = (m_start - v2) | l1;
	BooleanType reject3 = (c1 * c2 > Maths::Constants<ScalarType>::Zero()); // checking two edges is enough

	BooleanType collision = !(reject1 | reject2 | reject3);

	if (collision)
	{
		ScalarType f = triangle.normal | m_direction;
		ScalarType d = triangle.normal | (v0 - m_start);

		ScalarType distance = Maths::Select((Abs(f) > Maths::Constants<ScalarType>::Epsilon()),
											Abs(d / f),
											Maths::Constants<ScalarType>::Zero());

		if (distance < contact.intersectionDistance)
		{
			contact.collisionType = CollisionCache::COLLISION_TYPE_TRIANGLE_SURFACE;
			contact.intersectionNormal = triangle.normal;
			contact.intersectionPoint = m_start + m_direction * distance;
			contact.intersectionDistance = distance;
			contact.surfaceFlags = triangle.surfaceFlags;

			return Maths::Constants<BooleanType>::True();
		}
	}

	return Maths::Constants<BooleanType>::False();
}

CollisionCache::BooleanType RayCast::CollideFeature(const CollisionCache::VerticalSurface& verticalSurface, CollisionCache::Contact& contact) const
{
	const Point3R& v0 = verticalSurface.vertex0;
	const Point3R& v1 = verticalSurface.vertex1;

	// find intersection
	ScalarType f = verticalSurface.normal | m_direction;
	ScalarType d0 = verticalSurface.normal | (v0 - m_start);

	ScalarType d = Maths::Select((Abs(f) > Maths::Constants<ScalarType>::Epsilon()),
								d0 / f,
								Maths::Constants<ScalarType>::Zero());

	Point3R intersectionPoint = m_start + m_direction * d;

	ScalarType s = (intersectionPoint - v0) | verticalSurface.direction.Normal();

	if ((s > Maths::Constants<ScalarType>::Zero()) & (s < verticalSurface.direction.Length())
		& (d > Maths::Constants<ScalarType>::Zero()) & (d < contact.intersectionDistance))
	{
		contact.collisionType = CollisionCache::COLLISION_TYPE_VERTICAL_SURFACE;
		contact.intersectionNormal = verticalSurface.normal;
		contact.intersectionPoint = intersectionPoint;
		contact.intersectionDistance = d;
		contact.surfaceFlags = verticalSurface.surfaceFlags;

		return Maths::Constants<BooleanType>::True();
	}

	return Maths::Constants<BooleanType>::False();
}

//////////////////////////////////////////////////////////////////

CollisionCache::Contact RayCast::GetCollision(const CollisionCache& cache, U32 flags) const
{
	return cache.CollideRay<RayCast, BoundingBoxCollision, false>(*this, m_boundingBox, m_maxDistance, flags);
}

int RayCast::CountCollisions(const CollisionCache& cache, U32 flags) const
{
	return cache.CountRayCollisions<RayCast, BoundingBoxCollision>(*this, m_boundingBox, m_maxDistance, flags);
}

}

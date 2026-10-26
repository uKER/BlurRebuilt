#include "stdafx.h"
#include "racingphysics/collisions/sweptsphere.h"
#include "racingphysics/collisions/raycast.h"
#include "racingphysics/collisions/collisioncache.inl"

//#pragma optimize("", off)

//using namespace Maths;
using namespace UtMaths;

////////////////////////////////////////////////////////////////////////////////////////
// Moving sphere

namespace RacingPhysics
{

SweptSphereCollision::SweptSphereCollision(const Point3R& start, float radius, const Normal3R& direction, float maxDistance)
	:	m_start(start), 
		m_radius(radius),
		m_direction(direction),
		m_radiusSquared(radius * radius),
		m_maxDistance(maxDistance)
		//m_oneXZ(1.0, 0.0, 1.0)
{
	Point3R p1 = start;
	Point3R p2 = start + direction * maxDistance;
	Vector3R r = Vector3R(radius, radius, radius);

	Point3R max = Max(p1 + r, p2 + r);
	Point3R min = Min(p1 - r, p2 - r);
	
	m_boundingBox.SetMinMax(min, max);
}

CollisionCache::BooleanType SweptSphereCollision::CollideFeature(const CollisionCache::Triangle& triangle, CollisionCache::Contact& contact) const
{
	RayCast rayCast(m_start - triangle.normal * m_radius, m_direction, contact.intersectionDistance, CollisionCache::NullBoundingBox());

	return rayCast.CollideFeature(triangle, contact);
}

CollisionCache::ScalarType SweptSphereCollision::GetLineIntersectionDistance(const Point3R& v0, const Normal3R& lineDirection, const CollisionCache::ScalarType& maxDistance) const
{
	// solve quadratic for distance
	Vector3R v = v0 - m_start;

	ScalarType d1 = (lineDirection | m_direction);
	ScalarType d2 = (v | lineDirection);
	ScalarType d3 = (v | m_direction);
	ScalarType d4 = (v | v);
	ScalarType r2 = m_radiusSquared;

	ScalarType a = Maths::Constants<ScalarType>::One() - (d1 * d1);
	ScalarType b = Maths::Constants<ScalarType>::Two() * (-d3 + d2 * d1);
	ScalarType c = d4 - d2 * d2 - r2;

	ScalarType x = b * b - Maths::Constants<ScalarType>::Four() * a * c;

	// no roots - no intersection, or parallel lines
	if (x <= Maths::Constants<ScalarType>::Zero()) 
		return maxDistance;

	ScalarType sx = Sqrt(x);
	ScalarType distance = maxDistance;

	// check both roots for minimum
	ScalarType d = ((-b + sx) / (Maths::Constants<ScalarType>::Two() * a));
	distance = Maths::Select(((d > Maths::Constants<ScalarType>::Zero()) & (d < distance)),
						d,
						distance);

	d = ((-b - sx) / (Maths::Constants<ScalarType>::Two() * a));
	distance = Maths::Select(((d > Maths::Constants<ScalarType>::Zero()) & (d < distance)),
						d,
						distance);

	return distance;
}

CollisionCache::BooleanType SweptSphereCollision::CollideFeature(const CollisionCache::TriangleEdge& triangleEdge, CollisionCache::Contact& contact) const
{
	ScalarType distance = GetLineIntersectionDistance(triangleEdge.vertex0, triangleEdge.direction.Normal(), contact.intersectionDistance);

	// find normal and intersection point
	if (distance < contact.intersectionDistance)
	{
		Point3R centre = m_start + m_direction * distance;
		ScalarType l = ((centre - triangleEdge.vertex0) | triangleEdge.direction.Normal());

		// check whether point is on the line segment
		if (l > Maths::Constants<ScalarType>::Zero() && l < triangleEdge.direction.Length())
		{
			Point3R position = triangleEdge.vertex0 + triangleEdge.direction.Normal() * l;

			contact.intersectionPoint = position;
			contact.intersectionNormal = Normal3R((centre - position) / m_radius, Normal3R::UNIT_LENGTH_VECTOR);
			contact.collisionType = CollisionCache::COLLISION_TYPE_TRIANGLE_EDGE;
			contact.intersectionDistance = distance;
			contact.surfaceFlags = triangleEdge.surfaceFlags;

			return Maths::Constants<BooleanType>::True();
		}
	}

	return Maths::Constants<BooleanType>::False();
}

CollisionCache::BooleanType SweptSphereCollision::CollideFeature(const CollisionCache::VerticalEdge& verticalEdge, CollisionCache::Contact& contact) const
{
	ScalarType distance = GetLineIntersectionDistance(verticalEdge.vertex0, verticalEdge.direction.Normal(), contact.intersectionDistance);

	// find normal and intersection point
	if (distance < contact.intersectionDistance)
	{
		Point3R centre = m_start + m_direction * distance;
		ScalarType l = ((centre - verticalEdge.vertex0) | verticalEdge.direction.Normal());

		// check whether point is on the line segment
		//if (l > Constants<ScalarType>::Zero() && l < triangleEdge.direction.Length())
		{
			Point3R position = verticalEdge.vertex0 + verticalEdge.direction.Normal() * l;

			contact.intersectionPoint = position;
			contact.intersectionNormal = Normal3R((centre - position) / m_radius, Normal3R::UNIT_LENGTH_VECTOR);
			contact.collisionType = CollisionCache::COLLISION_TYPE_VERTICAL_EDGE;
			contact.intersectionDistance = distance;
			contact.surfaceFlags = verticalEdge.surfaceFlags;

			return Maths::Constants<BooleanType>::True();
		}
	}

	return Maths::Constants<BooleanType>::False();
}

CollisionCache::BooleanType SweptSphereCollision::CollideFeature(const CollisionCache::TriangleVertex& vertex, CollisionCache::Contact& contact) const
{
	Vector3R v = m_start - vertex.position;

	ScalarType b = Maths::Constants<ScalarType>::Two() * (m_direction | v);
	ScalarType c = (v | v) - m_radius * m_radius;

	ScalarType x = b * b - Maths::Constants<ScalarType>::Four() * c;
	if (x <= Maths::Constants<ScalarType>::Zero()) 
		return Maths::Constants<BooleanType>::False();

	ScalarType sx = Sqrt(x);
	ScalarType distance = contact.intersectionDistance;

	ScalarType d = ((-b + sx) / Maths::Constants<ScalarType>::Two());
	distance = Maths::Select(((d > Maths::Constants<ScalarType>::Zero()) & (d < distance)),
						d,
						distance);

	d = ((-b - sx) / Maths::Constants<ScalarType>::Two());
	distance = Maths::Select(((d > Maths::Constants<ScalarType>::Zero()) & (d < distance)),
						d,
						distance);

	// find normal and intersection point
	if (distance < contact.intersectionDistance)
	{
		Point3R c = m_start + m_direction * distance;
		contact.intersectionNormal = Normal3R((c - vertex.position) / m_radius, Normal3R::UNIT_LENGTH_VECTOR);
		contact.intersectionPoint = vertex.position;
		contact.collisionType = CollisionCache::COLLISION_TYPE_TRIANGLE_VERTEX;
		contact.intersectionDistance = distance;
		contact.surfaceFlags = vertex.surfaceFlags;

		return Maths::Constants<BooleanType>::True();
	}

	return Maths::Constants<BooleanType>::False();
}

CollisionCache::BooleanType SweptSphereCollision::CollideFull(const CollisionCache::Triangle& triangle, CollisionCache::Contact& contact) const
{
	BooleanType collision = Maths::Constants<BooleanType>::False();

	// could reject by triangle normal here

	CollisionCache::TriangleEdge e0(triangle.vertex0, triangle.vertex1, triangle.surfaceFlags);
	CollisionCache::TriangleEdge e1(triangle.vertex1, triangle.vertex2, triangle.surfaceFlags);
	CollisionCache::TriangleEdge e2(triangle.vertex2, triangle.vertex0, triangle.surfaceFlags);

	CollisionCache::TriangleVertex v0(triangle.vertex0, triangle.surfaceFlags);
	CollisionCache::TriangleVertex v1(triangle.vertex1, triangle.surfaceFlags);
	CollisionCache::TriangleVertex v2(triangle.vertex2, triangle.surfaceFlags);

	// surface
	collision |= CollideFeature(triangle, contact);

	// edges
	collision |= CollideFeature(e0, contact);
	collision |= CollideFeature(e1, contact);
	collision |= CollideFeature(e2, contact);

	// vertices
	collision |= CollideFeature(v0, contact);
	collision |= CollideFeature(v1, contact);
	collision |= CollideFeature(v2, contact);

	assert(!collision || (contact.intersectionDistance >= 0.f));

	return collision;
}

CollisionCache::BooleanType SweptSphereCollision::CollideFeature(const CollisionCache::VerticalSurface& verticalSurface, CollisionCache::Contact& contact) const
{
	CollisionCache::VerticalEdge v0(verticalSurface.vertex0, verticalSurface.vertex0 + Maths::Constants<Normal3R>::Y(), verticalSurface.surfaceFlags);
	CollisionCache::VerticalEdge v1(verticalSurface.vertex1, verticalSurface.vertex1 + Maths::Constants<Normal3R>::Y(), verticalSurface.surfaceFlags);

	RayCast rayCast(m_start - verticalSurface.normal * m_radius, m_direction, contact.intersectionDistance, CollisionCache::NullBoundingBox());
	BooleanType collision = rayCast.CollideFeature(verticalSurface, contact);

	collision |= CollideFeature(v0, contact);
	collision |= CollideFeature(v1, contact);

	return collision;
}

////////////////////////////////////////////////////

CollisionCache::Contact SweptSphereCollision::GetCollision(const CollisionCache& cache, U32 flags) const
{
	return cache.CollideSweptVolume<SweptSphereCollision, BoundingBoxCollision, false>(*this, m_boundingBox, m_maxDistance, flags);
}

}

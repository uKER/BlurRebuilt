#include "stdafx.h"
#include "racingphysics/collisions/spherecollision.h"
#include "racingphysics/collisions/collisioncache.inl"

//#pragma optimize("", off)

///////////////////////////////////////////////////////////////////////////

namespace RacingPhysics
{

//using namespace Maths;

SphereCollision::SphereCollision(const Point3R& centre, float radius)
		:	m_centre(centre), 
			m_radius(radius), 
			m_radiusSquared(radius * radius),		
			//m_oneXZ(1.0, 0.0, 1.0),
			m_boundingBox(m_centre - Vector3R(radius, radius, radius), m_centre + Vector3R(radius, radius, radius))
{}

// Collide with triangle surface only, edges and vertices are separate.
SphereCollision::BooleanType SphereCollision::CollideFeature(const CollisionCache::Triangle& triangle, CollisionCache::Contact& contact) const
{
	const Point3R& v0 = triangle.vertex0;
	const Point3R& v1 = triangle.vertex1;
	const Point3R& v2 = triangle.vertex2;

	// reject by triangle normal
	ScalarType d = triangle.normal | (m_centre - v0);
	ScalarType intersectionDistance = m_radius - d;
	if ((intersectionDistance < contact.intersectionDistance) 
						| (intersectionDistance > m_radius * Maths::Constants<ScalarType>::Two()))
		return Maths::Constants<BooleanType>::False();

	// reject by edges
	Vector3R l0 = (m_centre - v0) ^ triangle.normal;
	ScalarType b1 = (m_centre - v1) | l0;
	ScalarType b2 = (m_centre - v2) | l0;
	BooleanType reject2 = (b1 * b2 > Maths::Constants<ScalarType>::Zero());

	// checking two edges is enough
	Vector3R l1 = (m_centre - v1) ^ triangle.normal;
	ScalarType c1 = (m_centre - v0) | l1;
	ScalarType c2 = (m_centre - v2) | l1;
	BooleanType reject3 = (c1 * c2 > Maths::Constants<ScalarType>::Zero());

	BooleanType collision = !(reject2 | reject3);

	if (collision)
	{
		contact.collisionType = CollisionCache::COLLISION_TYPE_TRIANGLE_SURFACE;
		contact.intersectionNormal = triangle.normal;
		contact.intersectionPoint = m_centre - triangle.normal * d;
		contact.intersectionDistance = intersectionDistance;
		contact.surfaceFlags = triangle.surfaceFlags;
	}

	return collision;
}

// Collide with edge only, ignore collision with vertices.
SphereCollision::BooleanType SphereCollision::CollideFeature(const CollisionCache::TriangleEdge& triangleEdge, CollisionCache::Contact& contact) const
{
	const Point3R& v0 = triangleEdge.vertex0;
	const Point3R& v1 = triangleEdge.vertex1;

	// test distance from line
	Vector3R v = m_centre - v0;
	ScalarType d = v | triangleEdge.direction.Normal();
	ScalarType s2 = (v | v) - d * d;
	BooleanType reject1 = (s2 > m_radiusSquared);
	
	// test line segment
	d = (m_centre - v0) | triangleEdge.direction.Normal();
	BooleanType reject2 = (d < Maths::Constants<ScalarType>::Zero() | d > triangleEdge.direction.Length());

	BooleanType collision = !(reject1 | reject2);

	if (collision)
	{
		ScalarType intersectionDistance = m_radius - Maths::Sqrt(s2);

		if (intersectionDistance > contact.intersectionDistance)
		{
			contact.collisionType = CollisionCache::COLLISION_TYPE_TRIANGLE_EDGE;
			contact.intersectionNormal = Normalised(v - d * triangleEdge.direction.Normal());
			contact.intersectionDistance = intersectionDistance;
			contact.intersectionPoint = m_centre - contact.intersectionNormal * (m_radius - intersectionDistance);
			contact.surfaceFlags = triangleEdge.surfaceFlags;
		}
	}

	return collision;
}

SphereCollision::BooleanType SphereCollision::CollideFeature(const CollisionCache::TriangleVertex& vertex, CollisionCache::Contact& contact) const
{
	const Point3R& p = vertex.position;
	Vector3R v = m_centre - p;
	ScalarType d2 = Magnitude2(v);

	BooleanType collision = (d2 < m_radiusSquared);

	if (collision)
	{
		ScalarType intersectionDistance = m_radius - Maths::Sqrt(d2);

		if (intersectionDistance > contact.intersectionDistance)
		{
			contact.collisionType = CollisionCache::COLLISION_TYPE_TRIANGLE_VERTEX;
			contact.intersectionNormal = Normalised(v);
			contact.intersectionPoint = p;
			contact.intersectionDistance = intersectionDistance;
			contact.surfaceFlags = vertex.surfaceFlags;
		}
	}

	return collision;
}

SphereCollision::BooleanType SphereCollision::CollideFull(const CollisionCache::TriangleEdge& triangleEdge, CollisionCache::Contact& contact) const
{
	BooleanType collision = Maths::Constants<BooleanType>::False();
	
	// edge
	{
		const Point3R& v0 = triangleEdge.vertex0;
		const Point3R& v1 = triangleEdge.vertex1;

		// test distance from line
		Vector3R v = m_centre - v0;
		ScalarType d = v | triangleEdge.direction.Normal();
		ScalarType s2 = (v | v) - d * d;
		if (s2 > m_radiusSquared)
			return collision;

		// test line segment
		d = (m_centre - v0) | triangleEdge.direction.Normal();
		BooleanType reject2 = (d < Maths::Constants<ScalarType>::Zero() | d > triangleEdge.direction.Length());

		collision = !(reject2);

		if (collision)
		{
			ScalarType intersectionDistance = m_radius - Maths::Sqrt(s2);

			if (intersectionDistance > contact.intersectionDistance)
			{
				contact.collisionType = CollisionCache::COLLISION_TYPE_TRIANGLE_EDGE;
				contact.intersectionNormal = Normalised(v - d * triangleEdge.direction.Normal());
				contact.intersectionDistance = intersectionDistance;
				contact.intersectionPoint = m_centre - contact.intersectionNormal * (m_radius - intersectionDistance);
				contact.surfaceFlags = triangleEdge.surfaceFlags;
			}
		}
	}

	// vertices
	collision |= CollideFeature(CollisionCache::TriangleVertex(triangleEdge.vertex0, triangleEdge.surfaceFlags), contact);
	collision |= CollideFeature(CollisionCache::TriangleVertex(triangleEdge.vertex1, triangleEdge.surfaceFlags), contact);

	return collision;
}

SphereCollision::BooleanType SphereCollision::CollideFull(const CollisionCache::Triangle& triangle, CollisionCache::Contact& contact) const
{
	BooleanType collision = Maths::Constants<BooleanType>::False();

	// surface
	{
		const Point3R& v0 = triangle.vertex0;
		const Point3R& v1 = triangle.vertex1;
		const Point3R& v2 = triangle.vertex2;

		// reject by triangle normal
		ScalarType d = triangle.normal | (m_centre - v0);
		ScalarType intersectionDistance = m_radius - d;
		if ((intersectionDistance < contact.intersectionDistance) 
							| (intersectionDistance > m_radius * Maths::Constants<ScalarType>::Two()))
			return collision;

		// reject by edges
		Vector3R l0 = (m_centre - v0) ^ triangle.normal;
		ScalarType b1 = (m_centre - v1) | l0;
		ScalarType b2 = (m_centre - v2) | l0;
		BooleanType reject2 = (b1 * b2 > Maths::Constants<ScalarType>::Zero());

		// checking two edges is enough
		Vector3R l1 = (m_centre - v1) ^ triangle.normal;
		ScalarType c1 = (m_centre - v0) | l1;
		ScalarType c2 = (m_centre - v2) | l1;
		BooleanType reject3 = (c1 * c2 > Maths::Constants<ScalarType>::Zero());

		collision = !(reject2 | reject3);

		if (collision)
		{
			contact.collisionType = CollisionCache::COLLISION_TYPE_TRIANGLE_SURFACE;
			contact.intersectionNormal = triangle.normal;
			contact.intersectionPoint = m_centre - triangle.normal * d;
			contact.intersectionDistance = intersectionDistance;
			contact.surfaceFlags = triangle.surfaceFlags;
		}
	}

	// edges
	collision |= CollideFeature(CollisionCache::TriangleEdge(triangle.vertex0, triangle.vertex1, triangle.surfaceFlags), contact);
	collision |= CollideFeature(CollisionCache::TriangleEdge(triangle.vertex1, triangle.vertex2, triangle.surfaceFlags), contact);
	collision |= CollideFeature(CollisionCache::TriangleEdge(triangle.vertex2, triangle.vertex0, triangle.surfaceFlags), contact);

	// vertices
	collision |= CollideFeature(CollisionCache::TriangleVertex(triangle.vertex0, triangle.surfaceFlags), contact);
	collision |= CollideFeature(CollisionCache::TriangleVertex(triangle.vertex1, triangle.surfaceFlags), contact);
	collision |= CollideFeature(CollisionCache::TriangleVertex(triangle.vertex2, triangle.surfaceFlags), contact);

	return collision;
}

CollisionCache::BooleanType SphereCollision::CollideFeature(const CollisionCache::VerticalSurface& verticalSurface, CollisionCache::Contact& contact) const
{
	const Point3R& v0 = verticalSurface.vertex0;
	const Point3R& v1 = verticalSurface.vertex1;

	Vector3R d0 = ComponentMultiply(m_centre - v0, Maths::Constants<Vector3R>::XZ());
	Vector3R d1 = ComponentMultiply(m_centre - v1, Maths::Constants<Vector3R>::XZ());

	ScalarType d02 = d0 | d0;
	ScalarType d12 = d1 | d1;

	// reject by normal
	ScalarType dn = verticalSurface.normal | d0;
	if (dn > m_radius | dn < -m_radius)
		return Maths::Constants<BooleanType>::False();

	// test surface
	ScalarType d = (d0 | verticalSurface.direction.Normal());
	BooleanType t1 = (d > Maths::Constants<ScalarType>::Zero());
	BooleanType t2 = (d < verticalSurface.direction.Length());

	if (t1 & t2)
	{
		//ScalarType s2 = d02 - d * d;
		ScalarType intersectionDistance = m_radius - dn;//m_radius - Maths::Sqrt(s2) * Maths::Sign(dn);

		if (intersectionDistance > contact.intersectionDistance)
		{
			contact.collisionType = CollisionCache::COLLISION_TYPE_VERTICAL_SURFACE;
			contact.intersectionNormal = verticalSurface.normal;
			contact.intersectionDistance = intersectionDistance;
			contact.intersectionPoint = m_centre - verticalSurface.normal * (m_radius - intersectionDistance);
			contact.surfaceFlags = verticalSurface.surfaceFlags;
			return Maths::Constants<BooleanType>::True();
		}
	}

	// test endpoints
	if (d02 < m_radiusSquared)
	{
		ScalarType intersectionDistance = m_radius - Maths::Sqrt(d02);

		if (intersectionDistance > contact.intersectionDistance)
		{
			contact.collisionType = CollisionCache::COLLISION_TYPE_VERTICAL_EDGE;
			contact.intersectionNormal = Normalised(d0);
			contact.intersectionPoint = m_centre - d0;
			contact.intersectionDistance = intersectionDistance;
			contact.surfaceFlags = verticalSurface.surfaceFlags;
			return Maths::Constants<BooleanType>::True();
		}
	}

	if (d12 < m_radiusSquared)
	{
		ScalarType intersectionDistance = m_radius - Maths::Sqrt(d12);

		if (intersectionDistance > contact.intersectionDistance)
		{
			contact.collisionType = CollisionCache::COLLISION_TYPE_VERTICAL_EDGE;
			contact.intersectionNormal = Normalised(d1);
			contact.intersectionPoint = m_centre - d1;
			contact.intersectionDistance = intersectionDistance;
			contact.surfaceFlags = verticalSurface.surfaceFlags;
			return Maths::Constants<BooleanType>::True();
		}
	}

	return Maths::Constants<BooleanType>::False();
}

///////////////////////////////////////////////////

CollisionCache::Contact SphereCollision::GetFirstCollision(const CollisionCache& cache, U32 flags) const
{
	return cache.CollideVolume<SphereCollision, BoundingBoxCollision, true>(*this, m_boundingBox, flags);
}

CollisionCache::Contact SphereCollision::GetDeepestCollision(const CollisionCache& cache, U32 flags) const
{
	return cache.CollideVolume<SphereCollision, BoundingBoxCollision, false>(*this, m_boundingBox, flags);
}

}





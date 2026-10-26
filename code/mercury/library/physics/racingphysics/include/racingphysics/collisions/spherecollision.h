#ifndef RP_SPHERE_COLLISION_H
#define RP_SPHERE_COLLISION_H
#ifdef _MSC_VER
#pragma once
#endif

#include "collisioncache.h"
#include "boundingbox.h"

namespace RacingPhysics
{

// Class which checks for sphere collisions using the collision cache.
class SphereCollision
{
public:
	SphereCollision() {}

	// initialise
	SphereCollision(const Point3R& centre, float radius);
	
	// Use this to initialise without a bounding box.
	// Only the primitive collision functions are usable in this case.
	FORCE_INLINE SphereCollision(const Point3R& centre, float radius, CollisionCache::NullBoundingBox empty)
		:	m_centre(centre), 
			m_radius(radius), 
			m_radiusSquared(radius * radius)
	{}

	const Point3R& GetCentre() const {return m_centre;}
	CollisionCache::ScalarType GetRadius() const {return m_radius;}

	// cache collision functions
	CollisionCache::Contact GetFirstCollision(const CollisionCache& cache, U32 flags = 0) const;
	CollisionCache::Contact GetDeepestCollision(const CollisionCache& cache, U32 flags = 0) const;

	// primitive collision functions, main feature only
	CollisionCache::BooleanType CollideFeature(const CollisionCache::Triangle& triangle, CollisionCache::Contact& contact) const;
	CollisionCache::BooleanType CollideFeature(const CollisionCache::TriangleEdge& triangleEdge, CollisionCache::Contact& contact) const;
	CollisionCache::BooleanType CollideFeature(const CollisionCache::TriangleVertex& vertex, CollisionCache::Contact& contact) const;
	CollisionCache::BooleanType CollideFeature(const CollisionCache::VerticalSurface& verticalSurface, CollisionCache::Contact& contact) const;
	
	// full primitive collisions including sub-features
	CollisionCache::BooleanType CollideFull(const CollisionCache::Triangle& triangle, CollisionCache::Contact& contact) const;
	CollisionCache::BooleanType CollideFull(const CollisionCache::TriangleEdge& triangleEdge, CollisionCache::Contact& contact) const;
	CollisionCache::BooleanType CollideFull(const CollisionCache::TriangleVertex& vertex, CollisionCache::Contact& contact) const {return CollideFeature(vertex, contact);}
	CollisionCache::BooleanType CollideFull(const CollisionCache::VerticalSurface& verticalSurface, CollisionCache::Contact& contact) const {return CollideFeature(verticalSurface, contact);}

private:
	FRIEND_CLASS CollisionCache;

	Point3R m_centre;
	CollisionCache::ScalarType m_radius;
	CollisionCache::ScalarType m_radiusSquared;

	BoundingBoxCollision m_boundingBox;

	typedef CollisionCache::ScalarType ScalarType;
	typedef CollisionCache::BooleanType BooleanType;
};

}

#endif

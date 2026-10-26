#ifndef RP_RAYCAST_H
#define RP_RAYCAST_H
#ifdef _MSC_VER
#pragma once
#endif

#include "collisioncache.h"
#include "boundingbox.h"

namespace RacingPhysics
{

// Class which does raycasting using the collision cache.
class RayCast
{
public:
	RayCast() {}

	// initialise
	RayCast(const Point3R& start, const Normal3R& direction, float maxDistance);

	// collide
	CollisionCache::Contact GetCollision(const CollisionCache& cache, U32 flags = 0) const;
	int CountCollisions(const CollisionCache& cache, U32 flags = 0) const;

	float GetMaxDistance() const {return m_maxDistance;}
	const Point3R& GetStart() const {return m_start;}

	// primitive collision functions
	CollisionCache::BooleanType CollideFeature(const CollisionCache::Triangle& triangle, CollisionCache::Contact& contact) const;
	CollisionCache::BooleanType CollideFeature(const CollisionCache::VerticalSurface& verticalSurface, CollisionCache::Contact& contact) const;

	// collide full (identical for raycast)
	CollisionCache::BooleanType CollideFull(const CollisionCache::Triangle& triangle, CollisionCache::Contact& contact) const {return CollideFeature(triangle, contact);}
	CollisionCache::BooleanType CollideFull(const CollisionCache::VerticalSurface& verticalSurface, CollisionCache::Contact& contact) const {return CollideFeature(verticalSurface, contact);}

	// Use this to initialise without a bounding box.
	// Only this primitive collision functions are usable in this case.
	FORCE_INLINE RayCast(const Point3R& start, const Normal3R& direction, float maxDistance, const CollisionCache::NullBoundingBox& empty)
		:	m_start(start), 
			m_end(start + direction * maxDistance),
			m_direction(direction),
			m_maxDistance(maxDistance)
		{}

private:
	FRIEND_CLASS CollisionCache;

	Point3R m_start;
	Point3R m_end;
	Normal3R m_direction;
	CollisionCache::ScalarType m_maxDistance;

	BoundingBoxCollision m_boundingBox;

	typedef CollisionCache::ScalarType ScalarType;
	typedef CollisionCache::BooleanType BooleanType;
};

}

#endif

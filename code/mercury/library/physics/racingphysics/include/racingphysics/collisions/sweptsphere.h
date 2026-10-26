#ifndef RP_SWEPTSPHERE_H
#define RP_SWEPTSPHERE_H
#ifdef _MSC_VER
#pragma once
#endif

#include "collisioncache.h"
#include "boundingbox.h"

namespace RacingPhysics
{

class SweptSphereCollision
{
public:
	SweptSphereCollision() {}

	// initialise
	SweptSphereCollision(const Point3R& startPosition, float radius, const Normal3R& direction, float maxDistance);

	// collide
	CollisionCache::Contact GetCollision(const CollisionCache& cache, U32 flags = 0) const;

	float GetMaxDistance() const {return m_maxDistance;}
	const Point3R& GetStart() const {return m_start;}
	const Normal3R& GetDirection() const {return m_direction;}

	// primitive collision functions, main feature only
	CollisionCache::BooleanType CollideFeature(const CollisionCache::Triangle& triangle, CollisionCache::Contact& contact) const;
	CollisionCache::BooleanType CollideFeature(const CollisionCache::TriangleEdge& triangleEdge, CollisionCache::Contact& contact) const;
	CollisionCache::BooleanType CollideFeature(const CollisionCache::TriangleVertex& vertex, CollisionCache::Contact& contact) const;
	CollisionCache::BooleanType CollideFeature(const CollisionCache::VerticalSurface& verticalSurface, CollisionCache::Contact& contact) const;
	CollisionCache::BooleanType CollideFeature(const CollisionCache::VerticalEdge& verticalEdge, CollisionCache::Contact& contact) const;

	// full primitive collisions including sub-features
	CollisionCache::BooleanType CollideFull(const CollisionCache::Triangle& triangle, CollisionCache::Contact& contact) const;
	CollisionCache::BooleanType CollideFull(const CollisionCache::TriangleVertex& vertex, CollisionCache::Contact& contact) const {return CollideFeature(vertex, contact);}
	CollisionCache::BooleanType CollideFull(const CollisionCache::VerticalSurface& verticalSurface, CollisionCache::Contact& contact) const {return CollideFeature(verticalSurface, contact);}

private:
	CollisionCache::ScalarType GetLineIntersectionDistance(const Point3R& v0, const Normal3R& lineDirection, const CollisionCache::ScalarType& maxDistance) const;

	FRIEND_CLASS CollisionCache;

	Point3R m_start;
	CollisionCache::ScalarType m_radius;
	Normal3R m_direction;
	CollisionCache::ScalarType m_maxDistance;
	CollisionCache::ScalarType m_radiusSquared;
	BoundingBoxCollision m_boundingBox;

	typedef CollisionCache::ScalarType ScalarType;
	typedef CollisionCache::BooleanType BooleanType;
};

}

#endif

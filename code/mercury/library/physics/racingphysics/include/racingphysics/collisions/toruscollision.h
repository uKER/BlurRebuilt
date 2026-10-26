#ifndef RP_TORUSCOLLISION_H
#define RP_TORUSCOLLISION_H
#ifdef _MSC_VER
#pragma once
#endif

#include "collisioncache.h"
#include "boundingbox.h"

namespace RacingPhysics
{

// Class which checks for torus collisions using the collision cache.
class TorusCollision
{
public:
	TorusCollision() {}

	// initialise
	TorusCollision(const Point3R& centre, const Normal3R& normal, float hoopRadius, float tubeRadius);

	// collide
	CollisionCache::Contact GetFirstCollision(const CollisionCache& cache, U32 flags = 0) const;
	CollisionCache::Contact GetDeepestCollision(const CollisionCache& cache, U32 flags = 0) const;

	const Point3R& GetCentre() const {return m_centre;}
	const Normal3R& GetNormal() const {return m_normal;}
	float GetHoopRadius() const {return m_hoopRadius;}
	float GetTubeRadius() const {return m_tubeRadius;}

private:
	FRIEND_CLASS CollisionCache;

	// collision functions
	CollisionCache::BooleanType CollideFeature(const CollisionCache::Triangle& triangle, CollisionCache::Contact& contact) const;
	CollisionCache::BooleanType CollideFeature(const CollisionCache::TriangleEdge& triangleEdge, CollisionCache::Contact& contact) const;
	CollisionCache::BooleanType CollideFeature(const CollisionCache::TriangleVertex& vertex, CollisionCache::Contact& contact) const;
	CollisionCache::BooleanType CollideFeature(const CollisionCache::VerticalSurface& verticalSurface, CollisionCache::Contact& contact) const;

	Point3R m_centre;
	Normal3R m_normal;
	CollisionCache::ScalarType m_hoopRadius;
	CollisionCache::ScalarType m_hoopRadiusSquared;
	CollisionCache::ScalarType m_tubeRadius;
	CollisionCache::ScalarType m_tubeRadiusSquared;
	CollisionCache::ScalarType m_fullRadius;
	CollisionCache::ScalarType m_fullRadiusSquared;

	BoundingBoxCollision m_boundingBox;

	typedef CollisionCache::ScalarType ScalarType;
	typedef CollisionCache::BooleanType BooleanType;
};

}

#endif

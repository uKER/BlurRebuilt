#ifndef RP_INFINITE_PLANE_H
#define RP_INFINITE_PLANE_H
#ifdef _MSC_VER
#pragma once
#endif

#include "racingphysics/physics/geometry.h"

namespace RacingPhysics
{

class CollisionCache;

class InfinitePlane
{
public:
	InfinitePlane()
	{
		m_normal = Maths::Constants<Normal3R>::Y();
		m_position = Maths::Constants<Point3R>::Origin();
	}

	void Initialise(const Normal3R& normal, const Point3R& position)
	{
		m_normal = normal;
		m_position = position;
	}

	void DisplayDebug();

	void BuildCache(CollisionCache* cache, const Point3R& min, const Point3R& max);

	float GetHeight(const Point3R& p) const;
	
private:
	Normal3R m_normal;
	Point3R m_position;
};

class InfinitePlaneSystem
{
public:
	static InfinitePlaneSystem& Instance() {return s_instance;}

	InfinitePlane& GetPlane() {return m_plane;}

private:
	InfinitePlaneSystem() {}
	InfinitePlane m_plane;

	static InfinitePlaneSystem s_instance;
};

}

#endif

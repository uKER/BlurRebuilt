#ifndef __PHYSICS_SPHERE_COLLISION_H__
#define __PHYSICS_SPHERE_COLLISION_H__

#include "Impact\TriangleId.h"

namespace Impact
{

/// calculates collision between a sphere and a ConvexHull body
class SphereCollision : public I_PersistentData
{
public:
	/// constructor
		SphereCollision();

	/// calculate contact points between a convex hull and a trimesh body
//	int CollideContinuous(ContactVector &contacts, Body *p[2], Shape *pShape[2]);

	/// as above but using continuous collision detection
	/// falls back to the default function at low speeds
	int Collide(ContactVector &contacts, Body *p[2], Shape *pShape[2]);

private:
	int				m_iSeparatingPlane;
//	bool			m_bInitialised;

//	Maths::HVec3	m_vSpherePrev;

};

};

#endif

#ifndef RP_COLLISION_OBJECTS_H
#define RP_COLLISION_OBJECTS_H
#ifdef _MSC_VER
#pragma once
#endif

#include "collisioncache.h"
#include "racingphysics/debug/colour.h"

namespace RacingPhysics
{

enum CollisionType
{
	COLLISION_TYPE_NONE,
	COLLISION_TYPE_POLYTOPE_HIT_MESH,
	COLLISION_TYPE_SPHERE_HIT_MESH,
	COLLISION_TYPE_SPHERE_HIT_POLYTOPE, // internal collision
	COLLISION_TYPE_POLYTOPE_HIT_POLYTOPE,
};

enum COTransformState
{
	CO_TRANSFORM_STATE_UNTRANSFORMED,
	CO_TRANSFORM_STATE_CENTRE,
	CO_TRANSFORM_STATE_AABB,
	CO_TRANSFORM_STATE_STAGE1,
	CO_TRANSFORM_STATE_FULL
};

struct CarCollisionSettings
{
	float frontCornerDistance;
	float rearCornerDistance;
};

class RigidBody;
//class CMEdge;
class CollisionCache;

class COObject
{
	friend class COObjectPolytope;

public:

	enum COObjectType
	{
		COOBJECT_TYPE_UNKNOWN = -1,
		COOBJECT_TYPE_SPHERE,
	};

	COObject() : m_type(COOBJECT_TYPE_UNKNOWN) {}

	COObjectType Type() { return m_type; }

	// initialise
	void Initialise(const Axis* a);
	virtual bool InitialiseFromFile(const Axis* a, const Point3R& c, const char* filename) = 0;
	//virtual void InitialiseCuboid(const Axis* a, const Point3R& c, const Vector3R& size) = 0;
	//virtual void InitialiseCar(const Axis* a, const Point3R& c, const Vector3R& size, const CarCollisionSettings& settings) = 0;
	void SetAxis(const Axis* a);
	
	// display (for debug)
	void Display(ColourARGB c = ColourARGB::Green) const {Display(*m_axis, c);}
	virtual void Display(const Axis& a, ColourARGB c = ColourARGB::Green) const = 0;
	void DisplayAABB() const;
	virtual void Update() {}

	// info
	const Vector3R& AABB() const {TransformBoundingBox(*m_axis); return m_aabb;}
	const Point3R& TransformedCentre() const {TransformCentre(*m_axis); return m_transformedCentre;}
	float Radius() const {return m_radius;}
	float Length() const {return m_size.Z();}
	float Width() const {return m_size.X();}
	float Height() const {return m_size.Y();}
	const Point3R& Centre() const {return m_centre;}
	const Axis& GetAxis() const {return *m_axis;}

	// transform
	virtual void TransformStage1(const Axis& a) const = 0;
	virtual void TransformStage2(const Axis& a) const = 0;
	virtual void TransformAll(const Axis& a) const = 0;
	void TransformBoundingBox(const Axis& a) const;
	void TransformCentre(const Axis& a) const;
	void ClearTransformState() const {m_transformState = CO_TRANSFORM_STATE_UNTRANSFORMED;}
	bool ShouldTransform(COTransformState state) const {return (m_transformState < state);}

	// collide
	virtual CollisionCache::Contact GetIntersection(const COObject& other, bool transform = true) const = 0;
	virtual CollisionCache::Contact GetSeparation(const COObject& other, float maxDistance, bool transform = true) const = 0;
	virtual bool CollideWithObject(const COObject& other, bool transform = true) const = 0;
	
	bool CollideRadius(const COObject& other, float tolerance) const;
	bool CollideRadius(const Point3R& position, float tolerance, bool verticalOnly = false) const;
	bool CollideBoundingBoxes(const COObject& other, float tolerance = 0.f) const;
	
	virtual bool CollideWithPoint(const Point3R& position, float radius = 0.f, bool verticalOnly = false) const = 0;
	virtual float GetPointIntersection(const Point3R& position, float radius, Point3R* intersectionPoint, Normal3R* normal, bool verticalOnly = false, const Axis *pCustomAxis = 0) const = 0;

	typedef Btl::Vector<CollisionCache::Contact> ContactList;

	virtual void GetContacts(const COObject& other, ContactList& contacts, bool transform = true) const = 0;
	virtual CollisionCache::Contact GetCollision(const CollisionCache& cache, U32 flags) const = 0;
	virtual void GetContacts(const CollisionCache& cache, U32 flags, ContactList& contacts) const = 0;
	virtual CollisionCache::Contact GetSeparation(const CollisionCache& cache, U32 flags, float maxDistance) const = 0;
	virtual CollisionCache::Contact GetIntersection(const CollisionCache& cache, U32 flags) const = 0;

protected:
	void TransformBoundingBox_Sub(const Axis& a) const;

	const Axis* m_axis;
	Point3R m_centre;
	Vector3R m_size;
	float m_radius;
	COObjectType m_type;

	mutable Point3R m_transformedCentre;
	mutable Vector3R m_aabb;
	mutable BoundingBoxCollision m_boundingBox;

	mutable COTransformState m_transformState;
};

__forceinline void COObject::TransformBoundingBox(const Axis& a) const
{
	if (m_transformState >= CO_TRANSFORM_STATE_AABB)
		return;

	TransformBoundingBox_Sub(a);
}

}

#endif


#ifndef RP_COLLISION_SPHERES_H
#define RP_COLLISION_SPHERES_H
#ifdef _MSC_VER
#pragma once
#endif

#include "collisionobjects.h"
#include "collisioncache.h"

namespace RacingPhysics
{

class CMEdge;
class COObjectSpheres;

//////////////////////////////////////////////////////

// [Jan] I have moved COSphere out of COObjectSpheres so it can be forward declared in ConePhysics.h
class COSphere
{
public:
	COSphere() {}
	COSphere(const Point3R& centre, FloatVR radius) 
	{
		Set(&m_v, MakeVector(centre), radius);
	}
	
	FloatVR Radius() const {return m_v.W();}
	const Point3R& Centre() const {return (Point3R&) m_v;}

	void SetRadius(FloatVR r) {m_v.W(r);}
	void SetCentre(const Point3R& p) {Set(&m_v, MakeVector(p), m_v.W());}

private:
	Vector4R m_v;
};

class COObjectSpheres : public COObject
{
public:

	struct Node
	{
		COSphere sphere;
		Btl::Vector<COSphere> subNodes;
	};

	/////////////////////////////////////////////////////////

	COObjectSpheres() { m_type = COOBJECT_TYPE_SPHERE; }

	// initialise
	virtual bool InitialiseFromFile(const Axis* a, const Point3R& c, const char* filename);	
	virtual void InitialiseCar(const Axis* a, const Point3R& c, const Vector3R& size, const Point3R* wheels, float wheelRadius);	
	virtual void InitialiseBike(const Axis* a, const Point3R& c, const Vector3R& size,
								const Point3R& p1, const Point3R& p2, float wheelRadius);

	void Shutdown();
	
	// display (for debug)
	virtual void Display(const Axis& a, ColourARGB c = ColourARGB::Green) const;

	// transform
	virtual void TransformStage1(const Axis& a) const;
	virtual void TransformStage2(const Axis& a) const;
	virtual void TransformAll(const Axis& a) const;
	
	// collide
	virtual CollisionCache::Contact GetIntersection(const COObject& other, bool transform = true) const;
	virtual CollisionCache::Contact GetSeparation(const COObject& other, float maxDistance, bool transform = true) const;
	virtual bool CollideWithObject(const COObject& other, bool transform = true) const;
		
	virtual bool CollideWithPoint(const Point3R& position, float radius = 0.f, bool verticalOnly = false) const;
	virtual float GetPointIntersection(const Point3R& position, float radius, Point3R* intersectionPoint, Normal3R* normal, bool verticalOnly = false, const Axis *pCustomAxis = 0) const;
	
	virtual void GetContacts(const COObject& other, ContactList& contacts, bool transform = true) const;

	virtual CollisionCache::Contact GetCollision(const CollisionCache& cache, U32 flags) const;
	virtual void GetContacts(const CollisionCache& cache, U32 flags, ContactList& contacts) const;
	virtual CollisionCache::Contact GetSeparation(const CollisionCache& cache, U32 flags, float maxDistance) const;
	virtual CollisionCache::Contact GetIntersection(const CollisionCache& cache, U32 flags) const;

	// collision functions
	template <typename PrimitiveType> CollisionCache::BooleanType CollideFeature(const PrimitiveType& p, CollisionCache::Contact& contact) const;

	Btl::Vector<Node> &GetOriginalData();
	Btl::Vector<Node> &GetUnmodifiedData();

	void ResetSpheres();

private:
	void Finish();
	void FinishNode(int node);

	template <typename PrimitiveType>
	CollisionCache::BooleanType GetContacts(const PrimitiveType& p, CollisionCache::Contact& contact) const;

	template <typename PrimitiveType>
	CollisionCache::BooleanType GetSeparation(const PrimitiveType& p, CollisionCache::Contact& contact) const;

	template <typename PrimitiveType>
	CollisionCache::BooleanType GetCollision(const PrimitiveType& p, CollisionCache::Contact& contact) const;

	Btl::Vector<Node> m_originalData;
	int m_notGroundStartIndex;

	mutable Btl::Vector<Node> m_transformedData;	
	mutable Btl::Vector<Node> m_unmodifiedData;
	mutable Btl::Vector<Btl::Vector<int> > m_sphereContactIndices;

	mutable ContactList* m_pContactList;
	mutable bool m_separation;
	mutable float m_maxDistance;
};

}

#endif // COLLISION_SPHERES_H

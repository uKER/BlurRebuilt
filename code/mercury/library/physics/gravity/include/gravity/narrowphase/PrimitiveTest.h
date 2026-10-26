#ifndef _NEW_PRIMITIVETEST_H
#define _NEW_PRIMITIVETEST_H

#include "gravity/narrowphase/Primitive.h"
#include "gravity/narrowphase/CollisionResult.h"

namespace Collision
{
//template <int MaxContacts>
//class C_ContactPatch;

//calculate the closest point between point q and segment ab
NOALIAS inline void ClosestPtPointSegment(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, Maths::HVec3& closest);
//calculate the closest point between point q and segment ab
NOALIAS inline void ClosestPtPointSegment(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, Maths::HVec3& closest, Maths::HF32& t);

//calculate the closest point between point q and line ab
NOALIAS inline void ClosestPtPointLine(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, Maths::HVec3& closest);

//Michelle Closest Point Segment Segment
NOALIAS  inline void ClosestPtSegmentSegment(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, Maths::HVec3& c1, Maths::HVec3& c2);
//Return the square distance between two segments
NOALIAS  inline void DistanceSegmentSegment(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, Maths::HF32& sqDist);
//Return whethere two segments intersect 
//NOALIAS  inline bool IntersectSegmentSegment(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b);

//NOALIAS inline bool IntersectSegmentSegment(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, Maths::HVec3& res);
//Return whether a segment and plane intersect, if it is intersect, return the intersect point
NOALIAS  inline bool IntersectSegmentPlane(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const C_Plane& p, Maths::HVec3& res);
NOALIAS  inline int IntersectSegmentPlane1(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const C_Plane& p, Maths::HVec3& res);
NOALIAS  inline U32 IntersectSegmentBox(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF extents, Maths::HVec3& minContacts, Maths::HVec3& maxContacts);
NOALIAS  inline bool IntersectSegmentBox(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF extents, Maths::HVec3& minContacts, Maths::HVec3& maxContacts, Maths::HF32& _tmin, Maths::HF32& _tmax );
Maths::Scalar_Bool IntersectSegmentBox(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF extents, Maths::HVec3& minContacts, Maths::HVec3& maxContacts, Maths::HVec3& normal);
NOALIAS  Maths::Scalar_Bool IntersectRayAABBMinMax(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF min, const Maths::HVec3 PCREF max, Maths::HF32& tmin);
NOALIAS  Maths::Scalar_Bool IntersectRayAABBMinMax(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF min, const Maths::HVec3 PCREF max);
NOALIAS  inline bool IntersectRayAABB(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF extents, Maths::HF32& tmin);
NOALIAS inline bool IntersectRayAABBInCell(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF extents, 
										   const Maths::HF32 PCREF inflation, const Collision::C_AxisBoxMinMax& containingBox, Maths::HF32& tmin);
NOALIAS  inline U32 IntersectRayAABB(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF d, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF extents, Maths::HVec3& minContacts, Maths::HVec3& maxContacts);
NOALIAS  inline U32 IntersectRayPlane(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const C_Plane& p, Maths::HVec3& res);

NOALIAS  inline bool IntersectCapsuleSegment(const C_Capsule& c, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b);
NOALIAS  inline bool IntersectCapsuleCapsule(const C_Capsule& a, const C_Capsule& b);
NOALIAS  inline bool IntersectCapsuleSphere(const C_Capsule& capsule, const C_Sphere& sphere);

void ClosestPtPointCylinder(const Maths::HVec3 PCREF p, const Collision::C_Cylinder& cylinder, Maths::HVec3& closest);
//Chris Ericson Closest Point Segment Segment
void CEClosestPtSegmentSegment(const Maths::HVec3 PCREF p1, const Maths::HVec3 PCREF q1, const Maths::HVec3 PCREF p2, const Maths::HVec3 PCREF q2, Maths::HVec3& c1, Maths::HVec3& c2);
void ClosestPtPointTriangle2(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Maths::HVec3& closest);
void ClosestPtPointTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Maths::HF32& u, Maths::HF32& v, Maths::HF32& w);
void ClosestPtPointPlane(const C_Plane PCREF plane, const Maths::HVec3 PCREF p, Maths::HVec3& closest);
void ClosestPtPointTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Maths::HVec3& closest); 
Maths::HVec3 ClosestPtPointTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c);
Maths::HVec3 ClosestPtPointTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::C_Normal PCREF normal);

void ClosestPtSegmentTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Maths::HVec3& c1, Maths::HVec3& c2, Maths::HF32& sqDist);
void ClosestPtSegmentTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::C_Normal PCREF normal, Maths::HVec3& c1, Maths::HVec3& c2, Maths::HF32& sqDist);
//Maths::HF32 ClosestPtSegmentTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Maths::HVec3& c1, Maths::HVec3& c2);

void ClosestPtSegmentTriangleSegment(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Maths::HVec3& closest);
void ClosestPtTriangleTriangle(const Maths::HVec3 PCREF a1, const Maths::HVec3 PCREF b1, const Maths::HVec3 PCREF c1, const Maths::HVec3 PCREF a2, const Maths::HVec3 PCREF b2, const Maths::HVec3 PCREF c2, Maths::HVec3& p1, Maths::HVec3& p2);
void ClosestPtPointTetrahedron(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF d, Maths::HVec3& closest);

bool PointInTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c);
bool PointInsideTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c);
bool PointOutsideOfPlane(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF  b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF d);

//bool TomasIntersectTriangleTriangle(const Maths::HVec3 PCREF t10, const Maths::HVec3 PCREF t11, const Maths::HVec3 PCREF t12, const Maths::HVec3 PCREF t20, const Maths::HVec3 PCREF t21, const Maths::HVec3 PCREF t22);

template <int MaxContacts>
bool CollideTriangleTriangle(const Maths::HVec3 PCREF t10, const Maths::HVec3 PCREF t11, const Maths::HVec3 PCREF t12, const Maths::HVec3 PCREF t20, const Maths::HVec3 PCREF t21, const Maths::HVec3 PCREF t22, 
								  const Maths::C_Normal PCREF normA, const Maths::C_Normal PCREF normB, Collision::C_ContactPatch<MaxContacts>& res,
								  const U32 hashValue, const U32 materialIndex);

template <int MaxContacts>
bool OrenCollideTriangleTriangle(const Maths::HVec3 PCREF t10, const Maths::HVec3 PCREF t11, const Maths::HVec3 PCREF t12, const Maths::HVec3 PCREF t20, const Maths::HVec3 PCREF t21, const Maths::HVec3 PCREF t22, 
								  const Maths::C_Normal PCREF normA, const Maths::C_Normal PCREF normB, Collision::C_ContactPatch<MaxContacts>& res,
								  const U32 hashValue, const U32 materialIndex);

/*
	The first triangle will be the mesh triangle
*/
//template <int MaxContacts>
//bool TomasCollideTriangleTriangle(const Maths::HVec3 PCREF t10, const Maths::HVec3 PCREF t11, const Maths::HVec3 PCREF t12, const Maths::HVec3 PCREF t20, const Maths::HVec3 PCREF t21, const Maths::HVec3 PCREF t22, Collision::C_ContactPatch<MaxContacts>& patch, 
//								  const U32 hashValue, const U32 materialIndex);

inline bool IntersectTriangleRay(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, Maths::HF32& v, Maths::HF32& w);
inline bool IntersectTriangleRay(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, Maths::HVec3& res);
bool IntersectTriangleSegment(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q);
bool IntersectTriangleSegment(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c,  const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, Maths::HVec3& res);
Maths::Scalar_Bool IntersectTriangleSegmentSB(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c,  const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, Maths::HVec3& res);
Maths::Scalar_Bool IntersectTriangleSegmentSB(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c,  const Maths::C_Normal PCREF normal, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, Maths::HVec3& res);
Maths::Scalar_Bool IntersectTriangleSegmentSB(const C_RayCastTriangle& triangle, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::C_Normal PCREF direction, const Maths::HF32 PCREF radius, Maths::HVec3& res, Maths::HF32& distance, Maths::C_Normal& resultNormal);
Maths::Scalar_Bool DetailedSweptSphereTriangle(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::C_Normal PCREF triNormal, const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, const Maths::C_Normal PCREF direction, const Maths::HF32 PCREF radius, Maths::HVec3& contact, Maths::C_Normal& normal, Maths::HF32& distance);
//bool IntersectTriangleSegment2(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF normal, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q,  Maths::HVec3& res, Maths::HF32& dist);
bool IntersectTriangleBoxSegment(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b,  const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::C_Normal PCREF normal, Maths::HVec3& res, F32& pDepth);

bool IntersectAABBSegment(const C_AABB& a, const Maths::HVec3 PCREF p0, const Maths::HVec3 PCREF p1);
bool IntersectAABBs(const C_AABB& a, const C_AABB& b);
bool IntersectAABBs(const Maths::HVec3 PCREF aCenter, const Maths::HVec3 PCREF aExtents, const Maths::HVec3 PCREF bCenter, const Maths::HVec3 PCREF bExtents );
bool IntersectAABBPlane(const C_AABB& a, const Maths::HVec3 PCREF n, const Maths::HF32 PCREF d);
bool IntersectAABBTriangle(const C_AABB& aabb, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c);
bool CollideAABBCapsule(const C_AABB& aabb, const C_Capsule& capsule);

bool IntersectAABBMinMax(const C_AABBMinMax& a, const C_AABBMinMax& max);

void ClosestPtPointAABB(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF _min, const Maths::HVec3 PCREF _max, Maths::HVec3& closest);

void ClosestPtPointOBB(const Maths::HVec3 PCREF p, const C_OBB& obb, Maths::HVec3& closest);
bool IntersectOBBs(const C_OBB& a, const C_OBB& b);
bool IntersectOBBs2(const C_OBB& a, const C_OBB& b);
bool IntersectSpherePlane(const C_Sphere& s, const Maths::HVec3 PCREF n, const Maths::HF32 PCREF d);
bool IntersectOBBSegment(const C_OBB& obb, const Maths::HVec3 PCREF p0, const Maths::HVec3 PCREF p1);
bool IntersectOBBPlane(const C_OBB& obb, const Maths::HVec3 PCREF n, const Maths::HF32 PCREF d);
bool IntersectOBBTriangle(const C_OBB& obb, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c);
bool CollideOBBCapsule(const C_OBB& obb, const C_Capsule& capsule);

bool IntersectCapsuleTriangle(const C_Capsule& cap, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c);

bool CollideCapsuleTriangle(const C_Capsule& cap, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Maths::HVec3& contact, Maths::C_Normal& normal, Maths::HF32& pDepth);
//bool CollideCapsuleSphere(const C_Capsule& cap, const C_Sphere& sphere, Maths::HVec3& contact, Maths::C_Normal& normal, Maths::HF32& pDepth);
bool CollideCapsuleCapsule(const C_Capsule& a, const C_Capsule& b, Maths::HVec3& contact, Maths::C_Normal& normal, Maths::HF32& pDepth);

bool CollideCylinderSegment(const C_Cylinder& c, const Maths::HVec3& a, const Maths::HVec3& b);


bool CollideConvexOBBTriangle(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, 
		const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF centre, const Maths::HVec3 PCREF extents,
		const Maths::HMat33 PCREF rot);

//Yes/No Test for the Trigger sphere and the implementation is in PrimitiveIntersectTest.cpp
bool CollideCapsuleSphere(const C_Capsule& capsule, const C_Sphere& sphere);
bool CollideOBBSphere(const C_OBB& obb, const C_Sphere& sphere);
bool CollideSphereSphere(const C_Sphere& sphereA, const C_Sphere& sphereB);
bool CollideCylinderSphere(const C_Cylinder& cylinder, const C_Sphere& sphere);
bool CollideConeSphere(const C_Cone& cone, const C_Sphere& sphere);


bool TomasCollideTriangleTriangle(const Maths::HVec3 PCREF t10, const Maths::HVec3 PCREF t11, const Maths::HVec3 PCREF t12, const Maths::HVec3 PCREF t20, const Maths::HVec3 PCREF t21, const Maths::HVec3 PCREF t22, 
								  const Maths::C_Normal PCREF normA, const Maths::C_Normal PCREF normB, Maths::HVec3& contactA, Maths::HVec3& contactB, Maths::HF32& penA, Maths::HF32& penB);
U32 CollideCapsuleTriangle(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, 
							  const Maths::C_Normal PCREF normal, Collision::C_Capsule& capsule,
							  Maths::C_Normal& resultNormal, Maths::HVec3* contacts,
							  Maths::HF32* pens);




//bool CollideInfinitePlaneComposite(const C_FinitePlane& plane, const C_Composite& composite, const Maths::HVec3& contact, const Maths::HF32 peneration);
}; 

#include "gravity/narrowphase/PrimitiveTest.inl"

#endif

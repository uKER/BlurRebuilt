#ifndef SPHERETRIANGLE_H
#define SPHERETRIANGLE_H

#include "gravity/narrowphase/Primitive.h"
#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/Shape.h"


namespace Collision
{

template <int MaxContacts>
bool CollideSphereTriangle(C_Sphere& sphere, const Maths::HVec3& a, const Maths::HVec3& b, const Maths::HVec3& c, C_ContactPatch<MaxContacts>& patch, const U32 physicsMaterial, const U32 effectMaterial);

template <int MaxContacts>
bool CollideSpherePlane(const C_Sphere& sphere, C_Plane& plane, C_ContactPatch<MaxContacts>& patch, const U32 physicsMaterial, const U32 effectMaterial);

template <int MaxPatches, int MaxContacts>
bool CollideSphereTriangle(C_Sphere& sphere, const Maths::HVec3& a, const Maths::HVec3& b, const Maths::HVec3& c, C_CollisionResult<MaxPatches, MaxContacts>& results);


//template <int MaxPatches, int MaxContacts, class Triangle>
//bool CollideSphereTriangleMesh(C_Shape* sphere, const MathsStorage::Float3* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& results, const U32 physicsMaterial, const U32 effectMaterial, const U32 collisionFlags);
//
//template <class Triangle>
//bool ContinuousSphereTriangleMesh(C_Shape* sphere, const MathsStorage::Float3* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, const Maths::HVec3 PCREF r, Maths::HF32& maxLambda, Maths::HF32& lambda, Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB);

template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideSphereTriangleMesh(C_Shape* sphere, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, 
							   const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& results, const U32 physicsMaterial, const U32 effectMaterial, const U32 collisionFlags, 
							   const Maths::HVec3& low, const Maths::HVec3& high);

template <class Triangle>
bool ContinuousSphereTriangleMesh(C_Shape* sphere, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, 
								  const U32 triangleStartIndex, const Maths::HVec3 PCREF r, Maths::HF32& maxLambda, Maths::HF32& lambda, Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, 
								  const Maths::HVec3& low, const Maths::HVec3& high);
};

#include "gravity/narrowphase/SphereTriangle.inl"

#endif



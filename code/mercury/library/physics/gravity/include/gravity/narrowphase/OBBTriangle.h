#ifndef OBB_TRIANGLE_H
#define OBB_TRIANGLE_H

#include "gravity/narrowphase/shapes.h"
#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/Primitive.h"
#include "gravity/narrowphase/CollisionProxy.h"

namespace Collision
{

//template <int MaxPatches, int MaxContacts>
//bool CollideOBBTriangle(const Collision::C_OBB& obb, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, C_CollisionResult<MaxPatches, MaxContacts>& results);

//template <int MaxContacts>
//bool CollideOBBTriangle(const Collision::C_OBB& obb, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, 
//						const Maths::C_Normal PCREF normal, C_ContactPatch<MaxContacts>& patch, const Collision::AABBTreeTriangle& triangle, const U32 triangleIndex, const U32 physicsMaterial, const U32 effectMaterial);

//template <int MaxPatches, int MaxContacts, class Triangle>
//bool CollideOBBTriangleMesh(C_Shape*  obb, const MathsStorage::Float3* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& results, const U32 physicsMaterial, const U32 effectMaterial, const U32 collisionFlags);
//
//template <int MaxPatches, int MaxContacts, class Triangle>
//bool CollideOBBTriangleMesh(C_BoxProxy& proxy, const MathsStorage::Float3* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& results, const U32 physicsMaterial, const U32 effectMaterial, const U32 collisionFlags);
//
template <class Triangle>
bool ContinuousOBBTriangleMesh(C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, 
							   const U32 triangleStartIndex, const Maths::HVec3 PCREF r, Maths::HF32& maxLambda, Maths::HF32& lambda, Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, 
							   const Maths::HVec3& low, const Maths::HVec3& high);


template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideOBBTriangleMesh(C_Shape*  obb, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, const Triangle* const __restrict tris, const U32 numTris,
							const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& results, const U32 physicsMaterial, const U32 effectMaterial, const U32 collisionFlags, 
							const Maths::HVec3& low, const Maths::HVec3& high);

template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideOBBTriangleMesh(C_BoxProxy& proxy, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, const Triangle* const __restrict tris, const U32 numTris,
							const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& results, const U32 physicsMaterial, const U32 effectMaterial, const U32 collisionFlags, 
							const Maths::HVec3& low, const Maths::HVec3& high);

//template <class Triangle>
//bool ContinuousOBBTriangleMesh(C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, const Maths::HVec3 PCREF r, Maths::HF32& maxLambda, Maths::HF32& lambda, Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB);
};

#include "gravity/narrowphase/OBBTriangle.inl"
#endif

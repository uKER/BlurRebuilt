#ifndef CONVEX_TRIANGLE_H
#define CONVEX_TRIANGLE_H

#include "gravity/narrowphase/Shape.h"

namespace Collision
{

									//----------ConvexHull Triangle Mesh
template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideConvexHullTriangleMesh( C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
		const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
		const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high);

template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideConvexHullTriangleMesh(C_ConvexHullProxy& proxy, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
		const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
		const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high);

template <int MaxContacts>
bool CollideConvexHullTriangle(C_ConvexHull& convex, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Collision::C_ContactPatch<MaxContacts>* pPatch, const U32 physicsMaterial, const U32 effectMaterial);


template <class Triangle>
bool ContinuousConvexHullTriangleMesh(C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, 
									  const Maths::HVec3 PCREF r, Maths::HF32& maxLambda, Maths::HF32& lambda, Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, const Maths::HVec3& low, const Maths::HVec3& high);


//								//--------------Cone Triangle Mesh
//template <int MaxPatches, int MaxContacts, class Triangle>
//bool CollideConeTriangleMesh( C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
//		const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
//		const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high);
//
//template <int MaxPatches, int MaxContacts, class Triangle>
//bool CollideConvexConeTriangleMesh(C_ConeProxy& proxy, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
//		const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, 
//		const U32 effectMaterial, const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high);
//
//								//--------------Capsule Triangle Mesh
//template <int MaxPatches, int MaxContacts, class Triangle>
//bool CollideConvexCapsuleTriangleMesh(C_CapsuleProxy& proxy, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
//		const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
//		const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high);

								//--------------Cylinder Triangle Mesh
//template <int MaxPatches, int MaxContacts, class Triangle>
//bool CollideConvexCylinderTriangleMesh(C_Shape* shape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
//		const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
//		const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high);
//
//template <int MaxPatches, int MaxContacts, class Triangle>
//bool CollideConvexCylinderTriangleMesh(C_CylinderProxy& proxy, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
//		const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
//		const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high);

};

#include "gravity/narrowphase/ConvexTriangle.inl"
#endif

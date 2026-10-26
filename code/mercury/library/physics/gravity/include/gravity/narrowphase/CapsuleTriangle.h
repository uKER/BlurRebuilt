#ifndef CAPSULETRIANGLE_H
#define CAPSULETRIANGLE_H


#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/Primitive.h"

namespace Collision
{
template <int MaxContacts>
bool CollideCapsuleTriangle(const C_Capsule& capsule, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, C_ContactPatch<MaxContacts>& patch);

template <int MaxContacts>
bool CollideCapsuleTriangle(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, 
							  const Maths::C_Normal PCREF norm, Collision::C_Capsule& capsule, C_ContactPatch<MaxContacts>& patch,
							  const U32 physicsMaterial, const U32 effectMaterial, const U32 triangleStartIndex, const U32 triangleIndex);

template <int MaxContacts>
bool CollideCapsuleTriangle(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, 
							  const Maths::C_Normal PCREF norm, Collision::C_CapsuleProxy& proxy, C_ContactPatch<MaxContacts>& patch,
							  const U32 physicsMaterial, const U32 effectMaterial, const U32 triangleStartIndex, const U32 triangleIndex);

template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideCapsuleTriangleMesh( C_Shape*  capsule, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, 
								const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& results, const U32 materialIndex, const Maths::HVec3& low, const Maths::HVec3& high);


template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideCapsuleTriangleMesh(C_CapsuleProxy& proxy, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
		const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
		const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high);


template <class Triangle>
bool ContinuousCapsuleTriangleMesh( C_Shape*  capsule, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, 
								   const U32 triangleStartIndex, const Maths::HVec3 PCREF r, Maths::HF32& maxLambda, Maths::HF32& lambda, Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, const Maths::HVec3& low, const Maths::HVec3& high);

};

#include "gravity/narrowphase/CapsuleTriangle.inl"

#endif



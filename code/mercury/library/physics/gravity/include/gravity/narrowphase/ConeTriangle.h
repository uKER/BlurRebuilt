#ifndef CONETRIANGLE_H
#define CONETRIANGLE_H

#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/Primitive.h"

namespace Collision
{
//template <int MaxPatches, int MaxContacts>
//bool CollideCapsuleTriangle(const C_Capsule& capsule, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, C_CollisionResult<MaxPatches, MaxContacts>& results);

template <int MaxContacts>
bool CollideConeTriangle(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, 
							  const Maths::C_Normal PCREF norm, Collision::C_Capsule& capsule, C_ContactPatch<MaxContacts>& patch,
							  const U32 physicsMaterial, const U32 effectMaterial, const U32 triangleStartIndex, const U32 triangleIndex);

template <int MaxContacts>
bool CollideConeTriangle(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, 
							  const Maths::C_Normal PCREF norm, Collision::C_CapsuleProxy& proxy, C_ContactPatch<MaxContacts>& patch,
							  const U32 physicsMaterial, const U32 effectMaterial, const U32 triangleStartIndex, const U32 triangleIndex);

//template <int MaxPatches, int MaxContacts, class Triangle>
//bool CollideConeTriangleMesh( C_Shape*  capsule, const MathsStorage::Float3* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, 
//							 const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& results,
//							 const U32 physicsMaterial, const U32 effectMaterial);


template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideConeTriangleMesh( C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
		const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
		const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high);

template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideConeTriangleMesh(C_ConeProxy& proxy, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
		const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, 
		const U32 effectMaterial, const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high);

};


#include "gravity/narrowphase/ConeTriangle.inl"

#endif

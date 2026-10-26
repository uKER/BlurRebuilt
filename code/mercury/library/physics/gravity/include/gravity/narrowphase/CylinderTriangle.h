#ifndef CYLINDERTRIANGLE_H
#define CYLINDERTRIANGLE_H


#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/Primitive.h"

namespace Collision
{
template <int MaxContacts>
bool CollideCylinderTriangle(const Collision::C_Cylinder& cylinder, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, C_ContactPatch<MaxContacts>& results, const U32 hashValue, const U32 physicsMaterial , const U32 effectMaterial);

template <int MaxContacts>
bool CollideCylinderTriangle2(const Collision::C_Cylinder& cylinder, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, C_ContactPatch<MaxContacts>& results);

//template <int MaxPatches, int MaxContacts, class Triangle>
//bool CollideCylinderTriangleMesh(const C_Shape* const __restrict cylinder, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::Float3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, 
//								 const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& results, const U32 materialIndex, const Maths::HVec3& low, const Maths::HVec3& high);
template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideCylinderTriangleMesh(C_Shape* shape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
		const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
		const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high);

template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideCylinderTriangleMesh(C_CylinderProxy& proxy, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
		const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
		const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high);

template <class Triangle>
bool ContinuousCylinderTriangleMesh(C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, 
									const Maths::HVec3 PCREF r, Maths::HF32& maxLambda, Maths::HF32& lambda, Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, const Maths::HVec3& low, const Maths::HVec3& high);
};

#include "gravity/narrowphase/CylinderTriangle.inl"

#endif
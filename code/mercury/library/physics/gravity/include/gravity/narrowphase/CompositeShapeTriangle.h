#ifndef COMPOSITESHAPETRIANGLE_H
#define COMPOSITESHAPETRIANGLE_H

namespace Collision
{

template <int MaxContacts>
bool CollideCompositeShapeTriangle(const C_Shape* pShape, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Collision::C_ContactPatch<MaxContacts>& patch, const U32 physicsMaterial, const U32 effectMaterial,
								   const U32 collisionFlags);

template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideCompositeShapeTriangleMesh(const C_Shape* const __restrict pShape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& results, const U32 physicsMaterail, const U32 effectMaterial,
									   const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high);
};

#include "gravity/narrowphase/CompositeShapeTriangle.inl"

#endif


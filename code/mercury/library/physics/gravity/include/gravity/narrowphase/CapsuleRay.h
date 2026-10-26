#ifndef CAPSULERAY_H
#define CAPSULERAY_H

#include "gravity/narrowphase/Ray.h"

namespace Collision
{
NOALIAS FORCE_INLINE bool CollideCapsuleRay(const C_Shape* const __restrict capsule, C_Ray& ray);
NOALIAS FORCE_INLINE bool CollideCapsuleRay(const C_Capsule* const __restrict capsule, const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, Maths::HVec3& contact);

NOALIAS FORCE_INLINE bool CollideCapsuleRay(const C_Shape* const __restrict capsule, C_TransparentRay& ray,
											SharedPhysics::C_GraphNode* pGraphNode);

NOALIAS FORCE_INLINE bool CollideCapsuleRay(const Maths::HVec3 PCREF capsuleA, const Maths::HVec3 PCREF capsuleB, const Maths::HF32 PCREF radius, const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, Maths::HVec3& hitPoint, Maths::C_Normal& normal);

};

#include "gravity/narrowphase/CapsuleRay.inl"

#endif


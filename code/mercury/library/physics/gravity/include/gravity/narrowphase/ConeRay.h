#ifndef CONE_H
#define CONE_H

#include "gravity/narrowphase/Ray.h"

namespace Collision
{
NOALIAS FORCE_INLINE bool CollideConeRay(const C_Shape* const __restrict cone, C_Ray& ray);
NOALIAS FORCE_INLINE bool CollideConeRay(const C_Shape* const __restrict cone, C_TransparentRay& ray, SharedPhysics::C_GraphNode* pGraphNode);
NOALIAS FORCE_INLINE bool CollideConeRay(const C_Cone* const __restrict cone, const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, Maths::HVec3& contact);


};

#include "gravity/narrowphase/ConeRay.inl"

#endif

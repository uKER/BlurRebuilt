#ifndef OBBRAY_H
#define OBBRAY_H

#include "gravity/narrowphase/Ray.h"


namespace Collision
{
NOALIAS inline bool CollideOBBRay(const C_Shape* const obb, C_Ray& ray);
NOALIAS inline bool CollideOBBFatRay(const C_Shape* const obb, C_Ray& ray);
NOALIAS inline bool CollideAABBSweptSphere(const Maths::HVec3 PCREF max, const Maths::HVec3 PCREF min, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q,
							const Maths::HF32 PCREF radius);

NOALIAS inline bool CollideOBBRay(const C_Shape* const obb, C_TransparentRay& ray);

};

#include "gravity/narrowphase/OBBRay.inl"

#endif

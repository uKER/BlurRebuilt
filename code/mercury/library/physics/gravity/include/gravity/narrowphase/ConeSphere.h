#ifndef CONESPHERE_H
#define	CONESPHERE_H

#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/Primitive.h"

namespace Collision
{
//Called by the particle system collide with the dynamic objects
bool CollideConeSphere(const C_Cone& cone, const C_Sphere& sphere, Maths::HVec3& contact, Maths::HF32& pDepth, Maths::C_Normal& normal);

template <int MaxContacts>
bool CollideConeSphere(const C_Cone& cone, const C_Sphere& sphere, C_ContactPatch<MaxContacts>& patch, const U32 materialIndices);
};

#include "gravity/narrowphase/ConeSphere.inl"


#endif
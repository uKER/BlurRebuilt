#ifndef OBBSPHERE_H
#define OBBSPHERE_H

#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/Primitive.h"
/*
 * This file performs collision detection between two OBBs.
 */

namespace Collision
{

//Called by the particle system collide with the dynamic objects
bool CollideOBBSphere(const C_OBB& capsule, const C_Sphere& sphere, Maths::HVec3& contact, Maths::HF32& pDepth, Maths::C_Normal& normal);

template <int MaxContacts>
bool CollideOBBSphere(const C_OBB& obb, const C_Sphere& sphere, C_ContactPatch<MaxContacts>& patch, const U32 materialIndices);



};

#include "gravity/narrowphase/OBBSphere.inl"

#endif

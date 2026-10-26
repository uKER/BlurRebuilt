#ifndef SPHERESPHERE_H
#define SPHERESPHERE_H

#include "gravity/narrowphase/Primitive.h"
#include "gravity/narrowphase/CollisionResult.h"


namespace Collision
{

//Called by the particle system collide with the dynamic objects
bool CollideSphereSphere(const C_Sphere& capsule, const C_Sphere& sphere, Maths::HVec3& contact, Maths::HF32& pDepth, Maths::C_Normal& normal);

template <int MaxContacts>
bool CollideSphereSphere(const C_Sphere& sphereA, const C_Sphere& sphereB, C_ContactPatch<MaxContacts>& results, const U32 materialIndex);


};

#include "gravity/narrowphase/SphereSphere.inl"

#endif
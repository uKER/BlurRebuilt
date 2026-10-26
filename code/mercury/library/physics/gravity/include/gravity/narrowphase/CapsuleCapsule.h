#ifndef CAPSULECAPSULE_H
#define CAPSULECAPSULE_H

#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/Primitive.h"

namespace Collision
{
template <int MaxContacts>
bool CollideCapsuleCapsule(const C_Capsule& capA, const C_Capsule& capB, C_ContactPatch<MaxContacts>& results, const U32 materialIndex);
};

#include "gravity/narrowphase/CapsuleCapsule.inl"

#endif

#ifndef BOXCAPSULE_H
#define BOXCAPSULE_H

#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/Primitive.h"

namespace Collision
{
template <int MaxPatches, int MaxContacts>
bool CollideOBBCapsule(const C_OBB& obb, const C_Capsule& capsule, C_CollisionResult<MaxPatches, MaxContacts>& results);
};

#include "gravity/narrowphase/OBBCapsule.inl"

#endif

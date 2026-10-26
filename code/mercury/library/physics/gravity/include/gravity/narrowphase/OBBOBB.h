#ifndef OBBOBB_H
#define OBBOBB_H

#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/Primitive.h"
/*
 * This file performs collision detection between two OBBs.
 */

namespace Collision
{

template <int MaxContacts>
bool CollideBoxBox(C_OBB& obbA, C_OBB& obbB, C_ContactPatch<MaxContacts>& patch, const U32 materialIndices,
				   Collision::EPA& epa);

};

#include "gravity/narrowphase/OBBOBB.inl"

#endif

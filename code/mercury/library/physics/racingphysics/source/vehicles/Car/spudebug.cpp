#include "stdafx.h"
#include "racingphysics/vehicles/car/car.h"
#include "gravity/sdk/shapedescriptors.h"
#include "gravity/sdk/shapedescriptors.inl"
#include "gravity/sdk/bodyhelper.h"
#include "gravity/sdk/bodyhelper.inl"
#include "gravity/sharedphysics/rigidbody.h"
#include "gravity/sharedphysics/rigidbody.inl"
#include "gravity/narrowphase/optimisedaabbtree.inl"

/////////////////////////////////////////////////////////////////////////

#ifdef DEBUG_RACING_PHYSICS
#pragma optimize ("", off)
#endif

namespace RacingPhysics
{

extern Debug::Bool g_applyGravity;
extern Debug::Bool g_updatePhysics;

}

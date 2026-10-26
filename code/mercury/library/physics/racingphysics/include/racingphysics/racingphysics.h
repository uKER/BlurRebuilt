#ifndef RACING_PHYSICS_H
#define RACING_PHYSICS_H
#ifdef _MSC_VER
#pragma once
#endif

#include "core/maths.h"
#include "racingphysics/vehicles/motorbike/motorbike.h"
#include "racingphysics/vehicles/car/car.h"
#include "racingphysics/collisions/infiniteplane.h"
#include "racingphysics/physics/geometry.h"
#include "racingphysics/physics/axis.h"
#include "racingphysics/physics/quaternion.h"
#include "racingphysics/physics/physicstimer.h"

namespace RacingPhysics
{
	void CreateXt(XtDb* database=NULL);
	void DestroyXt();
	void UpdateXt();
}

#endif


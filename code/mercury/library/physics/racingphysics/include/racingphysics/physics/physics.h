//////////////////////////////////////////////////////////////////////
// Physics
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef RP_PHYSICS_H
#define RP_PHYSICS_H

#include "geometry.h"
#include "axis.h"

namespace RacingPhysics
{

const float MAX_VELOCITY = 200.f;
const float MAX_VELOCITY_Y = 50.f;
const float COLLISION_EPSILON = 0.0005f;

template <class T>
__forceinline T RK4(const T& a, const T& b, const T& c, const T& d)
{
	return FloatVR(1.0f / 6.0f) * (a + FloatVR(2.0f) * (b + c) + d);
}

template <typename T>
__forceinline T ExponentialSmoothing(const T& current, const T& previous, float timeStep, float decayTime)
{
	float f = exp(-timeStep / decayTime);
	return previous * f + current * (1.f - f);
}

namespace PhysicsConstants
{
	extern const float DEGREES;
	extern const float MPH;
	extern const float GRAVITY;
	extern const float HORSEPOWER;
	extern const float RPM;
}

// Check that values have been initialised (clear to invalid values at the start of the update).
FORCE_INLINE const float& Checked(const float& f) {RP_ASSERT(Maths::Finite(f)); return f;}
FORCE_INLINE const bool& Checked(const bool& b) {RP_ASSERT((char&) b == 0 || (char&) b == 1); return b;}
FORCE_INLINE const Normal3R& Checked(const Normal3R& n) {RP_ASSERT(Maths::Finite(n)); return n;}
FORCE_INLINE const Point3R& Checked(const Point3R& p) {RP_ASSERT(Maths::Finite(p)); return p;}
FORCE_INLINE const Vector3R& Checked(const Vector3R& v) {RP_ASSERT(Maths::Finite(v)); return v;}

}

#endif // PHYSICS_H

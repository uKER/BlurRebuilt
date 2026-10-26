#include "stdafx.h"
#include "racingphysics/physics/axis.h"
#include "racingphysics/physics/quaternion.h"

#ifdef DEBUG_RACING_PHYSICS
#pragma optimize ("", off)
#endif

namespace RacingPhysics
{

void Axis::Blend(const Axis& a1, const Axis& a2, float f)
{
	Quaternion q1(a1);
	Quaternion q2(a2);
	Quaternion q;
	q.BlendFast(q1, q2, f);

	SetRotation(q);
	position = Lerp(a2.Position(), a1.Position(), f);
}

#if !defined(CFG_SPEED)

bool RotationMatrix::DebugTest() const
{
	Vector3R vx = Matrix3().AX();
	Vector3R vy = Matrix3().AY();
	Vector3R vz = Matrix3().AZ();

	FloatVR x = Magnitude(vx);
	FloatVR y = Magnitude(vy);
	FloatVR z = Magnitude(vz);

	FloatVR f099(0.99f);
	FloatVR f101(1.01f);
	FloatVR f001(0.01f);

	BoolVR test_x = (x > f099) & (x < f101);
	BoolVR test_y = (y > f099) & (y < f101);
	BoolVR test_z = (z > f099) & (z < f101);

	FloatVR xy = vx | vy;
	FloatVR xz = vx | vz;
	FloatVR yz = vy | vz;

	BoolVR test_xy = Maths::Abs(xy) < f001;
	BoolVR test_xz = Maths::Abs(xz) < f001;
	BoolVR test_yz = Maths::Abs(yz) < f001;

	BoolVR result = test_x & test_y & test_z & test_xy & test_xz & test_yz;

	return result == Constants<BoolVR>::True();
}


bool Axis::DebugTest() const
{
	FloatVR fmax(100000.f);

	BoolVR test_x = Maths::Abs(position.X()) < fmax;
	BoolVR test_y = Maths::Abs(position.Y()) < fmax;
	BoolVR test_z = Maths::Abs(position.Z()) < fmax;

	BoolVR result = test_x & test_y & test_z;

	return RotationMatrix::DebugTest() && result;
}

#endif

void Axis::GetRollPitchYaw_Trig(float& tanRoll, float& sinPitch, float& tanYaw) const
{
	sinPitch = -Matrix3().AZ().Y();

	if (Maths::Abs(sinPitch) < 1.f - Maths::Constants<float>::Epsilon())
	{
		tanYaw = Matrix3().AZ().X() / Matrix3().AZ().Z();
		tanRoll = Matrix3().AX().Y() / Matrix3().AY().Y();
	}
	else
	{
		tanYaw = Matrix3().AY().X() / Matrix3().AY().Z();
		tanRoll = 0.f;
	}
}

}

namespace Maths
{

void Normalise(HMat33* m)
{
	HVec3 ax = m->AX();
	HVec3 ay = m->AY();
	HVec3 az = m->AZ();
	
	HF32 one = Constants<HF32>::One();
	HF32 zero = Constants<HF32>::Zero();
	HF32 half = Constants<HF32>::Half();

	HF32 xx = Constants<HF32>::One() - ax.LengthSquared();
	HF32 yy = Constants<HF32>::One() - ay.LengthSquared();
	HF32 xy = ax | ay;

	ax *= (one + xx * half);
	ay *= (one + yy * half);

	HVec3 ayn = -(HVec3(xy) - ax * ay) / ax;
	//const HVec3 invRoot3(0.577350269, 0.577350269, 0.577350269);
	//ay = Select(Abs(ax) < invRoot3, ay, ayn);

	if (Maths::Abs(ax.X()) > half)
		ay.X(ayn.X());
	else if (Maths::Abs(ax.Z()) > half)
		ay.Z(ayn.Z());
	else
		ay.Y(ayn.Y());	

	az = ax ^ ay;

	m->AX(ax);
	m->AY(ay);
	m->AZ(az);
}

}

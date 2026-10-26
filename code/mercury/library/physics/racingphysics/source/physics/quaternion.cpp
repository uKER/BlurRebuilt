#include "stdafx.h"
#include "racingphysics/physics/quaternion.h"
#include "racingphysics/physics/rotationmatrix.h"

namespace RacingPhysics
{

void Quaternion::Rotate(const Vector3R& v)
{
	int n = (int) (Magnitude(v) / 0.01f);
	if (n < 2) 
	{
		*this -= (*this * v) * 0.5f;
		Normalise();
	}
	else 
	{
		Vector3R v1 = v * (1.f / float(n));

		for (int i = 0; i < n; i++)
		{
			*this -= (*this * v1) * 0.5f;
			Normalise();
		}
	}
}

void Quaternion::Rotate(const Normal3R& v, float a)
{	
	if (a != 0.f)
	{
		*this *= Quaternion(Maths::Cos(a * 0.5f), v * Maths::Sin(-a * 0.5f));
		Normalise();
	}
}

Quaternion Quaternion::Conjugate() const
{
	return Quaternion(w, -a);
}

float Quaternion::GetRotationAngle() const
{
	float cosHalfAngle = w / Norm();
	if (cosHalfAngle > 1.f)
	{
		cosHalfAngle = 1.f;
	}
	else if (cosHalfAngle < -1.f)
	{
		cosHalfAngle = -1.f;
	}
	return 2.f * acosf(cosHalfAngle);
}

Normal3R Quaternion::GetRotationAxis() const
{
	const float EPS = 1.f / 65536.f;
	if (Magnitude(a) < EPS)
	{
		return Maths::Constants<Normal3R>::X();
	}
	else
	{
		return Normalised(a);
	}
}


void Quaternion::Negate()
{
	w = -w;
	a = -a;
}

Quaternion::Quaternion(const RotationMatrix& m) 
{
	SetRotation(m);
}

void Quaternion::SetRotation(const RotationMatrix& rotationMatrix)
{
	const Vector3R& ax = rotationMatrix.Matrix3().AX();
	const Vector3R& ay = rotationMatrix.Matrix3().AY();
	const Vector3R& az = rotationMatrix.Matrix3().AZ();
	float t = Trace(rotationMatrix.Matrix3()) + 1.f;

	if (t > 0.5f)
	{
		float s = Maths::Sqrt(t) * 2.f;

		w = s / 4.f;

		a.X((az.Y() - ay.Z()) / s);
		a.Y((ax.Z() - az.X()) / s);
		a.Z((ay.X() - ax.Y()) / s);
	}
	else
	{
		if (ax.X() > ay.Y() && ax.X() > az.Z())
		{
			float s = Maths::Sqrt(1.f + ax.X() - ay.Y() - az.Z()) * 2.f;

			a.X(s / 4.f);
			a.Y((ax.Y() + ay.X()) / s);
			a.Z((ax.Z() + az.X()) / s);
			w = (az.Y() - ay.Z()) / s;
		}
		else if (ay.Y() > az.Z())
		{
			float s = Maths::Sqrt(1.f + ay.Y() - ax.X() - az.Z()) * 2.f;
			RP_ASSERT(s > 0.5f);

			a.X((ay.X() + ax.Y()) / s);
			a.Y(s / 4.f);
			a.Z((ay.Z() + az.Y()) / s);
			w = (ax.Z() - az.X()) / s;
		}
		else
		{
			float s = Maths::Sqrt(1.f + az.Z() - ax.X() - ay.Y()) * 2.f;

			a.X((az.X() + ax.Z()) / s);
			a.Y((az.Y() + ay.Z()) / s);
			a.Z(s / 4.f);
			w = (ay.X() - ax.Y()) / s;
		}

		// want to guarantee w > 0
		if (w < 0.f)
		{
			a *= -1.f;
			w *= -1.f;
		}
	}
} 

// fast interpolation (good if q1 and q2 are close together)
void Quaternion::BlendFast(const Quaternion& q1, const Quaternion& q2, float f)
{
	if ((q1 | q2) > 0.f)
		*this = q1 * f + q2 * (1.f - f);
	else
		*this = q1 * f - q2 * (1.f - f);

	Normalise();
}

}

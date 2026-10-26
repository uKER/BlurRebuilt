#include "stdafx.h"
#include "racingphysics/physics/rotationmatrix.h"
#include "racingphysics/physics/quaternion.h"

namespace RacingPhysics
{

void RotationMatrix::LookAlong(const Normal3R& dir, const Normal3R& up)
{
	matrix.AZ(dir);
	matrix.AX(Normalised(up ^ matrix.AZ()));
	matrix.AY(matrix.AZ() ^ matrix.AX());
}

void RotationMatrix::SetRotation(const Normal3R& v, FloatVR a)
{
	FloatVR s, c;
	Maths::SinCos(&s, &c, a);
	FloatVR c2 = Maths::Constants<FloatVR>::One() - c;

	matrix.AX(Vector3R(c + v.X() * v.X() * c2, s * v.Z() + v.Y() * v.X() * c2, -s * v.Y() + v.Z() * v.X() * c2));
	matrix.AY(Vector3R(-s * v.Z() + v.X() * v.Y() * c2, c + v.Y() * v.Y() * c2, s * v.X() + v.Z() * v.Y() * c2));
	matrix.AZ(Vector3R(s * v.Y() + v.X() * v.Z() * c2, -s * v.X() + v.Y() * v.Z() * c2, c + v.Z() * v.Z() * c2));
}

// construct matrix corresponding to rotation of angle a around direction v
RotationMatrix::RotationMatrix(const Normal3R& v, FloatVR a)
{
	SetRotation(v, a);
}

// construct matrix from Euler angles
RotationMatrix::RotationMatrix(float heading, float pitch, float roll) 
{       
	float c1 = (float) Maths::Cos(heading);   
	float s1 = (float) Maths::Sin(heading);    
	float c2 = (float) Maths::Cos(pitch);    
	float s2 = (float) Maths::Sin(pitch);   
	float c3 = (float) Maths::Cos(roll);    
	float s3 = (float) Maths::Sin(roll);

	matrix.AX(Vector3R(c1 * c2, s1 * c2, -s2));
	matrix.AY(Vector3R(-(s1 * c3) + (c1 * s2 * s3), (c1 * c3) + (s1 * s2 * s3), c2 * s3));
	matrix.AZ(Vector3R((s1 * s3) + (c1 * s2 * c3), -(c1 * s3) + (s1 * s2 * c3), c2 * c3));
}

// rotate angle a about normalized vector v
void RotationMatrix::Rotate(const Normal3R& v, FloatVR a)
{
	matrix = matrix * RotationMatrix(v, a).matrix;
	Normalise(&matrix);
}

RotationMatrix RotationMatrix::Rotated(const Normal3R& v, FloatVR a) const
{
	RotationMatrix r = *this;
	r.Rotate(v, a);
	return r;
}

RotationMatrix RotationMatrix::Rotated(const Vector3R& v) const
{
	RotationMatrix r = *this;
	r.Rotate(v);
	return r;
}

RotationMatrix RotationMatrix::RotationMatrixZ(float a) 
{
	RotationMatrix m;

	float s = Maths::Sin(a), c = Maths::Cos(a);

	m.matrix.AX(Vector3R(c, s, 0.f));
	m.matrix.AY(Vector3R(-s, c, 0.f));
	m.matrix.AZ(Vector3R(0.f, 0.f, 1.f));

	return m;
}

void RotationMatrix::SetRotation(const Quaternion& q)
{
	const float& w = q.W();
	const float& x = q.A().X();
	const float& y = q.A().Y();
	const float& z = q.A().Z();

	matrix.AX(Vector3R(1.f - 2.f * (y * y + z * z), 2.f * (x * y - w * z), 2.f * (z * x + w * y))); 
	matrix.AY(Vector3R(2.f * (x * y + w * z), 1.f - 2.f * (x * x + z * z), 2.f * (y * z - w * x)));
	matrix.AZ(Vector3R(2.f * (z * x - w * y), 2.f * (y * z + w * x), 1.f - 2.f * (x * x + y * y)));
}

RotationMatrix RotationMatrix::RotatedLocalY180() const
{
	return RotationMatrix(-matrix.AX(), matrix.AY(), -matrix.AZ()); 
}

}

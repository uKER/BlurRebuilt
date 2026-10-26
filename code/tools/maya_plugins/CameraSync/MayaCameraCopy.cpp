
#include "stdafx.h"
#include <camera2/Camera.h>
#include "MayaCameraCopy.h"

C_MayaCameraCopy::C_MayaCameraCopy()
{
	m_angle[0] = m_angle[1] = m_angle[2] = 0.f;
}

// Convert a Quaternion to Euler angles:
static void QuatToEuler(F32 *pitch, F32 *yaw, F32 *roll, Maths::C_Quaternion quat)
{
	Maths::HMat43 mat(quat);
	*pitch=(F32)asin (mat[2][1]);
	*yaw  =(F32)atan2(mat[2][0], mat[2][2]);
	*roll =(F32)atan2(mat[0][1], mat[1][1]);
}

static F32 AngleDiff(F32 a[3], F32 b[3])
{
	F32 total = 0.f;
	int i = 0;
	// Add up the difference between the angles:
	for (i = 0; i < 3; i++)
	{
		F32 d = b[i] - a[i];
		if (d <= -(F32)M_PI) d += (F32)(2.0 * M_PI);
		if (d >=  (F32)M_PI) d -= (F32)(2.0 * M_PI);
		if (d < 0.f) d = -d;
		total += d;
	}
	
	return total;
}

void C_MayaCameraCopy::SetCamera(const Cam2::C_Camera& cam)
{
	m_camera = cam;

	F32 pitch = 0.f, yaw = 0.f, roll = 0.f;
	QuatToEuler(&pitch, &yaw, &roll, m_camera.Orient());
	yaw = -yaw;

	// There's 2 sets of euler angles which will work
	// Calculate both and pick the nearest one to return:
	F32 ang[3] = {pitch, yaw, roll};
	F32 diff = AngleDiff(m_angle, ang);

	ang[0] = (F32)M_PI - pitch;
	ang[1] = yaw - (F32)M_PI;
	if (ang[1] < -(F32)M_PI) ang[1] += (F32)(2.0 * M_PI);
	
	ang[2] = roll + (F32)M_PI;
	if (ang[2] >= (F32)M_PI) ang[2] -= (F32)(2.0 * M_PI);
	F32 diff2 = AngleDiff(m_angle, ang);
	
	if (diff2 < diff)
	{
		pitch = ang[0];
		yaw   = ang[1];
		roll  = ang[2];
	}

	m_angle[0] = pitch;
	m_angle[1] = yaw;
	m_angle[2] = roll;
}

MDoubleArray C_MayaCameraCopy::GetResult() const
{
	MDoubleArray result;
	result.append( (F32)m_camera.Position().X());
	result.append( (F32)m_camera.Position().Y());
	result.append(-(F32)m_camera.Position().Z());

	result.append(Maths::RadToDeg(m_angle[0]));
	result.append(Maths::RadToDeg(m_angle[1]));
	result.append(Maths::RadToDeg(m_angle[2]));
	result.append(Maths::RadToDeg(m_camera.Fov()));

	return result;
}

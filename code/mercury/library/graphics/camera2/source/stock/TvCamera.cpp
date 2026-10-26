
#include "stdafx.h"
#include "camera2/CameraType.h"
#include "camera2/stock/TvCamera.h"
#include "CameraInt.h"

namespace Cam2
{

static Debug::Bool  Override2("Camera2.TvCamera.Override", false);
static Debug::Float CustomViewRange("Camera2.TvCamera.ViewRange", 20.0f, Debug::Range( 0.f, 20.f));
static Debug::Float OffsetY("Camera2.TvCamera.OffsetY", 0.f, Debug::Range(-2.f, 2.f));

C_TvCamera::C_TvCamera()
{
	SetType(E_TvCamera);
	SetName("Tv");
	m_viewRange = 20.f;
}

void C_TvCamera::SetViewRange(F32 range)
{
	m_viewRange = range;
}

// A more accurate Matrix to Quaternion (avoids problems of jittering when looking south)
static Maths::C_Quaternion MatToQuat(const Maths::HMat33& mat)
{
	Maths::C_Quaternion quat;

	F32 trace  = 1.f + mat.AX().X() + mat.AY().Y() + mat.AZ().Z();
	F32 tracex = 1.f + mat.AX().X() - mat.AY().Y() - mat.AZ().Z();
	F32 tracey = 1.f - mat.AX().X() + mat.AY().Y() - mat.AZ().Z();
	F32 tracez = 1.f - mat.AX().X() - mat.AY().Y() + mat.AZ().Z();

	F32 x = 0.f, y = 0.f, z = 0.f, w = 1.f;
	
	if (trace > tracex && trace > tracey && trace > tracex)
	{
		w = sqrt(trace) * 0.5f;
		F32 s = 0.25f / w;
		x = (mat.AZ().Y() - mat.AY().Z()) * s;
		y = (mat.AX().Z() - mat.AZ().X()) * s;
		z = (mat.AY().X() - mat.AX().Y()) * s;
	}
	else if (tracex > tracey && tracex > tracez)
	{
		x = sqrt(tracex) * 0.5f;
		F32 s = 0.25f / x;
		w = (mat.AZ().Y() - mat.AY().Z()) * s;
		y = (mat.AX().Y() + mat.AY().X()) * s;
		z = (mat.AX().Z() + mat.AZ().X()) * s;
	}
	else if (tracey > tracez)
	{
		y = sqrt(tracey) * 0.5f;
		F32 s = 0.25f / y;
		w = (mat.AX().Z() - mat.AZ().X()) * s;
		x = (mat.AX().Y() + mat.AY().X()) * s;
		z = (mat.AY().Z() + mat.AZ().Y()) * s;
	}
	else
	{
		z = sqrt(tracez) * 0.5f;
		F32 s = 0.25f / z;
		w = (mat.AY().X() - mat.AX().Y()) * s;
		x = (mat.AX().Z() + mat.AZ().X()) * s;
		y = (mat.AY().Z() + mat.AZ().Y()) * s;
	}

	quat = Maths::C_Quaternion(x, y, z, w);

	return quat;
}

void C_TvCamera::OnUpdate()
{
	F32 viewRange = m_viewRange;

	Maths::HVec3 rel = Object().position - Position();

	if (Override2)
	{
		viewRange = CustomViewRange;
		rel += Maths::HVec3(0.f, (F32)OffsetY, 0.f);
	}	
	
	Maths::HVec3 up(0.f, 1.f, 0.f);
	Maths::HVec3 toz = Normalise(rel);
	Maths::HVec3 tox = Normalise(up ^ toz);
	Maths::HVec3 toy = Normalise(toz ^ tox);
	Maths::HMat33 look(tox, toy, toz);
	Maths::C_Quaternion quat;
	quat = MatToQuat(look);
	
	SetOrient(quat);

	F32 objectDistance = Length(rel);

	// Find out the fov to frame the focus object:
	F32 fov = 1.025f;
	if (objectDistance > 0.01f)
	{
		fov = atan(viewRange / objectDistance);
	}
	if (fov > 1.2f) fov = 1.2f; // Limit fov so we don't get crazy fisheye view
	SetFov( fov );
}

} // End of namespace Cam2

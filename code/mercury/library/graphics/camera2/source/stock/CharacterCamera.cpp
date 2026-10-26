
#include "stdafx.h"
#include "camera2/stock/CharacterCamera.h"
#include "camera2/CameraType.h"
#include "CameraInt.h"

namespace Cam2
{

// I've taken these out as debug variables to stop them cluttering up DebugView for now:
//static Debug::Float Height    ("Camera2.Character.Height",   1.2f, Debug::Range(0.f, 10.f));
//static Debug::Float Distance  ("Camera2.Character.Distance", 3.5f, Debug::Range(1.f, 10.f));
//static Debug::Float CharCustomTilt("Camera2.Character.Tilt", -5.f, Debug::Range( -20.f, 20.f));
static F32 Height = 1.2f, Distance = 3.5f, CharCustomTilt = -5.f;

C_CharacterCamera::C_CharacterCamera()
{
	SetType(E_CharacterCamera);
	m_reset = true;
	m_lookX = m_lookY = 0.f;
	m_physics = 0;
	m_distance = 1.f;
}

void C_CharacterCamera::OnUpdate()
{
	if (m_reset)
	{
		m_trailPos = Object().position + Maths::HVec3(0.f, (F32)Height, -(F32)Distance);
		m_distance = 1.f;
		m_reset = false;
	}

	ApplyLookAround();
	ClipToCone();

	// Get follow position:
	Maths::HVec3 opos = Object().position;
	Maths::HVec3 follow = m_trailPos - opos;
	F32 scale = follow.Length();
	if (scale > 1e-5f)
		scale = Distance / scale;
	follow *= scale;

	m_trailPos = opos + follow;

	// Find a 'safe' camera position, and a wanted camera position
	Maths::HVec3 safe = Object().position + Maths::HVec3(0.f, (F32)Height, 0.f);
	Maths::HVec3 high = Maths::HVec3(0.f, (F32)Height, 0.f);
	Maths::HVec3 want = m_trailPos + high;
	
	// Clip to the scenery:
	Maths::HVec3 clipped = FindClippedPosition(safe, want);
	SetPosition(clipped);
	
	SetOrient(GetLookQuat(opos - m_trailPos));

	// Tilt down to look at the character:
	F32 tilt = Maths::DegToRad((F32)CharCustomTilt) * 0.5f;
	SetOrient( Orient() * Maths::C_Quaternion(sin(tilt), 0.f, 0.f, cos(tilt)) );
}

void C_CharacterCamera::SetLookAround(F32 x, F32 y)
{
	m_lookX = x;
	m_lookY = y;
}

void C_CharacterCamera::ApplyLookAround()
{
	Maths::HMat33 mat = Orient();
	m_trailPos -= m_lookX * Distance * Period() * mat.AX();
	m_trailPos -= m_lookY * Distance * Period()* mat.AY();
}

// To stop looking directly up or down and getting gimble lock, push position onto the outside a cone:
void C_CharacterCamera::ClipToCone()
{
	Maths::HVec3 offset = m_trailPos - Object().position;
	F32 limit = offset.Y() * 0.364f; // 20 degrees
	if (limit < 0.f)
		limit = -limit;
	offset.Y(0.f);

	F32 radius = offset.Length();
	if (radius > 1e-3f && radius < limit)
		m_trailPos += offset * (limit / radius - 1.f);
}

Maths::HVec3 C_CharacterCamera::FindClippedPosition(const Maths::HVec3& safe, const Maths::HVec3& want)
{
	// Raytrace from the safe camera position to the wanted camera position to see if anything
	// is in the way of the camera:
	Maths::HVec3 dir = want - safe;
	F32 length = dir.Length();
	if (m_physics == 0 || length < 1e-5f)
		return want;
	
	F32 radius = 0.15f;
	// Raytrace 'radius' further
	dir += dir * radius / length;
	
	// Get right vector:
	Maths::HVec3 right = dir ^ Maths::HVec3(0.f, 1.f, 0.f);
	right = Normalise(right);

	F32 ang = Maths::DegToRad(10.f);
	F32 hit0 = m_physics->RayTrace(safe + right * radius * -2.0f, safe + dir * cos(ang * -2.f) + right * length * sin(ang * -2.f));
	F32 hit1 = m_physics->RayTrace(safe + right * radius * -1.0f, safe + dir * cos(ang * -1.f) + right * length * sin(ang * -1.f));
	F32 hit2 = m_physics->RayTrace(safe                         , safe + dir);
	F32 hit3 = m_physics->RayTrace(safe + right * radius *  1.0f, safe + dir * cos(ang       ) + right * length * sin(ang       ));
	F32 hit4 = m_physics->RayTrace(safe + right * radius *  2.0f, safe + dir * cos(ang *  2.f) + right * length * sin(ang *  2.f));
	F32 min = hit0;
	if (min > hit1) min = hit1;
	if (min > hit2) min = hit2;
	if (min > hit3) min = hit3;
	if (min > hit4) min = hit4;

	min -= radius / length;
	m_distance += (min - m_distance) * 0.1f;
	if (m_distance > hit1)
		m_distance = hit1;
	if (m_distance > hit2)
		m_distance = hit2;
	if (m_distance > hit3)
		m_distance = hit3;

	Maths::HVec3 clipped = safe + dir * m_distance;
	return clipped;
}

} // End of namespace Cam2

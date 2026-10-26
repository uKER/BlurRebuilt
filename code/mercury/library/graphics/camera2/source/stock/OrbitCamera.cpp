
#include "stdafx.h"
#include "camera2/CameraType.h"
#include "camera2/stock/OrbitCamera.h"

namespace Cam2
{

using namespace Maths;

static Debug::Float g_OrbitRadius ("Camera2.OrbitCamera.Radius", 4.0f, Debug::Range(0.1f, 10.f));
static Debug::Float g_OrbitSpeed  ("Camera2.OrbitCamera.Speed",  0.05f, Debug::Range(0.f, 1.f));
static Debug::Float g_OrbitFov    ("Camera2.OrbitCamera.Fov",    60.f, Debug::Range(0.f, 90.f));
static Debug::Float g_OrbitXOffset("Camera2.OrbitCamera.XOffset", 0.0f, Debug::Range(-2.f, 2.f));
static Debug::Float g_OrbitYOffset("Camera2.OrbitCamera.YOffset", 1.5f, Debug::Range(-2.f, 2.f));
static Debug::Float g_OrbitZOffset("Camera2.OrbitCamera.ZOffset", 0.0f, Debug::Range(-2.f, 2.f));
static Debug::Float g_OrbitXAim   ("Camera2.OrbitCamera.XAim",    0.0f, Debug::Range(-2.f, 2.f));
static Debug::Float g_OrbitYAim   ("Camera2.OrbitCamera.YAim",   -1.5f, Debug::Range(-2.f, 2.f));
static Debug::Float g_OrbitZAim   ("Camera2.OrbitCamera.ZAim",    0.0f, Debug::Range(-2.f, 2.f));

C_OrbitCamera::C_OrbitCamera()
{
	SetType(E_OrbitCamera);
	SetName("Orbit");
	m_Angle = 0.f;
}

C_OrbitCamera::~C_OrbitCamera()
{
}

void C_OrbitCamera::OnUpdate()
{
	HVec3 offset((F32)g_OrbitXOffset, (F32)g_OrbitYOffset, (F32)g_OrbitZOffset);
	HVec3 circle = HVec3(sin(m_Angle), 0.f, cos(m_Angle)) * g_OrbitRadius;
	HVec3 aim((F32)g_OrbitXAim, (F32)g_OrbitYAim, (F32)g_OrbitZAim);

	SetPosition(Object().ToWorldSpace(offset + circle));
	LookAt(Object().ToWorldSpace(offset + aim));
	
	m_Angle += Period() * Pi * g_OrbitSpeed;
	if (m_Angle >  Pi * 2.f)
		m_Angle -= Pi * 2.f;
	
	SetFov(DegToRad((F32)g_OrbitFov));
}

} // End of namespace

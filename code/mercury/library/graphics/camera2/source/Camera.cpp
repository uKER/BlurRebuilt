
#include "stdafx.h"
#include <camera2/Feed.h>	// [Blur2PC] tick counter
#include <camera2/Camera.h>
#include <camera2/CameraType.h>
#include <camera2/EffectInfo.h>
#include <camera2/stock/ShakeCamera.h>
#include <camera2/shake/ShakeWorld.h>

namespace Cam2
{

using namespace Maths;

static Debug::Bool InvertYAxisOption("Camera2.FlyCamera.InvertYAxis", false, Debug::Persist());
C_Camera NullCamera; // Global null camera

Debug::Bool ShakeOverride("Camera2.Shake.Override", false);

Debug::Bool g_ShutterOverride("Camera2.PostEffects.ShutterOverride", false);
Debug::Float g_ShutterAngle("Camera2.PostEffects.ShutterAngle", 0.5f, Debug::Range(0.f, 1.f));
static Debug::Bool g_DofOverride("Camera2.PostEffects.DofOverride", false);
static Debug::Float g_Idepth("Camera2.PostEffects.Idepth", 0.f, Debug::Range(0.f, 1.f));
static Debug::Float g_Aperture("Camera2.PostEffects.Aperture", 0.f, Debug::Range(0.f, 4.f));
static Debug::Bool g_HQDofOverride("Camera2.PostEffects.HQDof.Override", false);
static Debug::Float g_NearStart("Camera2.PostEffects.HQDof.NearStart", 0.00f,   Debug::Range(0.0f, 500.f));
static Debug::Float g_NearEnd  ("Camera2.PostEffects.HQDof.NearEnd",   0.01f,   Debug::Range(0.0f, 500.f));
static Debug::Float g_FarStart ("Camera2.PostEffects.HQDof.FarStart",  499.99f, Debug::Range(0.0f, 500.f));
static Debug::Float g_FarEnd   ("Camera2.PostEffects.HQDof.FarEnd",    500.00f, Debug::Range(0.0f, 500.f));
static Debug::Float g_FarScale ("Camera2.PostEffects.HQDof.FarScale",  1.0f,    Debug::Range(0.0f, 1.0f));

C_Camera::C_Camera()
{
	SetFov(DegToRad(60.f));
	m_NearClip = 0.f;
	m_FarClip = 4000.f;
	m_ShutterAngle = 0.5f;
	m_Idepth = m_Aperture = 0.f;
	m_pShakeWorld = 0;
	m_pShakeCamera = 0;
	m_Period = 0.f;
	m_FovShake = 0.f;
	m_EffectInfo = 0;
	SetName("-");
	GetPrevPosition();
}

C_Camera::~C_Camera()
{
	SAFE_DELETE_PTR(m_EffectInfo)
	SAFE_DELETE_PTR(m_pShakeCamera)
}

void C_Camera::SetShutterAngle(F32 angle)
{
	m_ShutterAngle = angle;
}

F32 C_Camera::ShutterAngle() const
{
	return g_ShutterOverride ? g_ShutterAngle : m_ShutterAngle;
}

void C_Camera::Update(F32 period)
{
	StartUpdate(period);
	OnUpdate();
	EndUpdate();
}

void C_Camera::StartUpdate(F32 period)
{
	m_Period = period;
	GetPrevPosition();
}

void C_Camera::EndUpdate()
{
	if (m_pShakeCamera)
		m_pShakeCamera->Update(Period());

	C_Quaternion shake;
	GetCameraShake(shake, m_FovShake);
	
	SetShaken(Orient() * shake);
}

void C_Camera::GetPrevPosition() // todo - change this method name to 'StorePrevPosition'
{
	// Get motion blur before:
	m_PrevPosition = Position();
	m_PrevShaken = Shaken();
	m_PrevFov = Fov();
	m_Blur2PCTick = I_Feed::Blur2PCTick();	// [Blur2PC]
	m_Blur2PCPrevPosition = m_PrevPosition;	// [Blur2PC]
	m_Blur2PCPrevShaken = m_PrevShaken;
	m_Blur2PCPrevFov = m_PrevFov;
}

void C_Camera::GetCameraShake(C_Quaternion& rotate, F32& fov) const
{
	if (ShakeOverride)
	{
		if (C_ShakeWorld::Instance().m_pGlobalShake)
			rotate = C_ShakeWorld::Instance().m_pGlobalShake->Orient();
	}
	else
	{
		rotate = C_Quaternion();

		if (m_pShakeWorld)
			m_pShakeWorld->GetCameraShake(Position(), &rotate, 0, &fov);

		if (m_pShakeCamera)
			rotate = rotate * m_pShakeCamera->Orient();
	}
}

void C_Camera::OnUpdate()
{
	// Application defined
}

void C_Camera::CopyFrom(const C_Camera& other)
{
	HVec3 pos = other.Position();
	if (pos.Finite())
		SetPosition(pos);
	
	C_Quaternion orient = other.Orient();
	if (orient.Length() > 0.5f && orient.Length() < 1.5f)
		SetOrient(orient);

	SetFov   ( other.Fov() );
	SetShutterAngle( other.ShutterAngle() );
	SetIdepth  ( other.Idepth() );
	SetAperture( other.Aperture() );
	
	pos = other.m_PrevPosition;
	if (pos.Finite())
		m_PrevPosition = pos;
	m_PrevShaken = other.m_PrevShaken;
	m_PrevFov = other.m_PrevFov;
	m_Blur2PCTick = other.m_Blur2PCTick;	// [Blur2PC]
	m_Blur2PCPrevPosition = other.m_Blur2PCPrevPosition;	// [Blur2PC]
	m_Blur2PCPrevShaken = other.m_Blur2PCPrevShaken;
	m_Blur2PCPrevFov = other.m_Blur2PCPrevFov;
}

// Get the look quaternion for looking in a direction
C_Quaternion C_Camera::GetLookQuat(const HVec3& direction)
{
	HVec3 toz = direction;
	toz.Normalise();
	HVec3 tox = HVec3(0.f,1.f,0.f) ^ toz;
	tox.Normalise();
	HVec3 toy = toz ^ tox;
	toy.Normalise();

	HMat33 look;
	look.AX(tox);
	look.AY(toy);
	look.AZ(toz);

	return C_Quaternion(look);
}

void C_Camera::LookAt(const HVec3& point)
{
	SetOrient(GetLookQuat(point - Position()));
}

void C_Camera::GetEuler(F32* pitch, F32* yaw, F32* roll) const
{
	QuatToEuler(Orient(), pitch, yaw, roll);
}

void QuatToEuler(const C_Quaternion& quat, F32* pitch, F32* yaw, F32* roll)
{
	HMat33 mat(quat);

	if (pitch)
		*pitch = asin (mat.AZ().Y());
	if (yaw)
		*yaw = atan2( mat.AZ().X(), mat.AZ().Z());
	if (roll)
		*roll = atan2(-mat.AX().Y(), mat.AY().Y());
}

// Functions to interpolate the position and orientation over the previous frame (0.0 = previous frame, 1.0 = current frame)
HVec3 C_Camera::InterPosition(F32 timeVal) const
{
	return m_PrevPosition + (Position() - m_PrevPosition) * timeVal;
}

C_Quaternion C_Camera::InterShaken(F32 timeVal) const
{
	return m_PrevShaken.ShortestSlerp(Shaken(), timeVal);
}

HMat43 C_Camera::InterMat43(F32 timeVal) const
{
	if (timeVal == 1.f)
		return Mat43();

	return HMat43(InterShaken(timeVal), InterPosition(timeVal));
}

F32 C_Camera::InterFov(F32 timeVal) const
{
	return m_PrevFov + (Fov() - m_PrevFov) * timeVal;
}

bool InvertYAxis()
{
	return InvertYAxisOption;
}

void SetInvertYAxis(bool invert)
{
	InvertYAxisOption = invert;
}

// Convenience function - sets same shake amounts on all 3 axes:
void C_Camera::SetShake(F32 amount, F32 frequency)
{
	SetShakeXYZ(amount, amount, amount, frequency);
}

void C_Camera::SetShake(const HVec3& amount, F32 frequency)
{
	SetShakeXYZ(amount.X(), amount.Y(), amount.Z(), frequency);
}

void C_Camera::SetShakeXYZ(F32 amountX, F32 amountY, F32 amountZ, F32 frequency)
{
	if (m_pShakeCamera == 0)
	{
		m_pShakeCamera = new C_ShakeCamera;
		if (m_pShakeCamera == 0)
			return;
	}
	
	m_pShakeCamera->SetParamXYZ(amountX, amountY, amountZ, frequency);
}

void C_Camera::SetShakeWorld(C_ShakeWorld* world)
{
	m_pShakeWorld = world;
}

void C_Camera::SetIdepth(F32 idepth)
{
	m_Idepth = idepth;
}

F32 C_Camera::Idepth() const
{
	return g_DofOverride ? g_Idepth : m_Idepth;
}

void C_Camera::SetAperture(F32 aperture)
{
	m_Aperture = aperture;
}

F32 C_Camera::Aperture() const
{
	return g_DofOverride ? g_Aperture : m_Aperture;
}

HVec3 C_Camera::Velocity() const
{
	F32 invPeriod = Period() > 1e-5f ? 1.f / Period() : 0.f;

	HVec3 diff = Position() - m_PrevPosition;
	diff *= invPeriod;
	return diff;
}

C_Camera C_Camera::Lerp(const C_Camera& target, F32 amount) const
{
	HVec3 pos = Position() + (target.Position() - Position()) * amount;
	C_Quaternion orient = Orient().ShortestLerp(target.Orient(), amount);
	orient.Normalise();
	
	F32 fov = Fov() + (target.Fov() - Fov()) * amount;

	Cam2::C_Camera camera;
	camera.SetPosition(pos);
	camera.SetOrient(orient);
	camera.SetFov(fov);
	return camera;
}

// World space position to camera space:
HVec3 C_Camera::ToCameraSpace(const HVec3& wspos) const
{
	return (wspos - Position()) * HMat33(!Orient()) ;
}

void C_Camera::SetPrevPosition(const HVec3& pos)
{
	m_PrevPosition = pos;
}

void C_Camera::SetPrevShaken(const C_Quaternion& orient)
{
	m_PrevShaken = orient;
}

void C_Camera::SetName(const char* name)
{
#ifdef CFG_GOLD
	(void)name;
#else
	m_Name = name;
#endif
}

const char* C_Camera::Name() const
{
#ifdef CFG_GOLD
	return "";
#else
	return m_Name;
#endif
}

const F32 g_FilmHeight = 0.024f; // 36 by 24mm film

// Helper function for setting field of view:
void C_Camera::SetFovByFocalLength(F32 focalLength)
{
	F32 vfov = atan(g_FilmHeight * 0.5f / focalLength) * 2.f;
	SetFov(vfov);
}

// Helper function for getting focal length:
F32 C_Camera::FovToFocalLength(F32 fov)
{
	F32 denom = tan(fov * 0.5f) * 2.f;
	if (denom > 1e-5f)
		return g_FilmHeight / denom;
	else
		return 0.f;
}

// Based on the current Idepth value, get the imaging length:
F32 C_Camera::ImagingLength() const
{
	F32 focalLength = FovToFocalLength(Fov());
	if (focalLength < 1e-5f)
		return 0.f;
	
	return 1.f / ( (1.f / focalLength) - Idepth());
}

F32 C_Camera::CalculateBokeh(F32 invObjectDistance) const
{
	F32 focalLength   = FovToFocalLength(Fov());
	F32 imagingLength = ImagingLength();

	// let's consider Aperture to be F-Stop (so we don't have to rename it :-)
	return Aperture() * (imagingLength * (1.f - focalLength * invObjectDistance) - focalLength) / g_FilmHeight;
}

F32 C_Camera::FindBokehLimit(F32 diameter) const
{
	F32 focalLength   = FovToFocalLength(Fov());
	F32 imagingLength = ImagingLength();
	F32 aperture = Aperture();
	if (aperture < 1e-5f || imagingLength < 1e-5f)
		return 0.f;
	
	F32 denom = 1.f - (diameter * g_FilmHeight / Aperture() + focalLength) / imagingLength;
	if (denom > -1e-5f && denom < 1e-5f)
		return 0.f;

	return focalLength / denom;
}

Util::C_String C_Camera::InfoString(bool extraInfo) const
{
#ifndef CFG_GOLD
	HVec3 pos = Position();

	F32 pitch = 0.f, heading = 0.f, roll = 0.f;
	GetEuler(&pitch, &heading, &roll);
	F32 fov = Fov();

	pitch   = RadToDeg(pitch);
	heading = RadToDeg(heading);
	roll    = RadToDeg(roll);
	fov     = RadToDeg(fov);
	
	if (heading < 0.f)
		heading += 360.f;
	
	Util::C_String string;
	if (extraInfo)
	{
		// Show angle and FOV info
		string.Printf("%.1f %.1f %.1f (%.2f %.2f %.2f) %.2f",
			(F32)pos.X(), (F32)pos.Y(), (F32)pos.Z(), heading, pitch, roll, fov);
	}
	else
	{
		// Just show Heading angle:
		string.Printf("%.1f %.1f %.1f (%.1f)", (F32)pos.X(), (F32)pos.Y(), (F32)pos.Z(), heading);
	}

	return string;
#else
	return Util::C_String();
#endif
}

void C_Camera::SetEffectInfo(const C_EffectInfo& effect)
{
	if (!m_EffectInfo)
		m_EffectInfo = new C_EffectInfo;
	
	// Copy effect info:
	if (m_EffectInfo)
		*m_EffectInfo = effect;
}

const C_EffectInfo* C_Camera::EffectInfo() const
{
	static C_EffectInfo s_Override;
	s_Override.SetInfo(g_NearStart, g_NearEnd, g_FarStart, g_FarEnd, g_FarScale);

	if (g_HQDofOverride)
		return &s_Override;
	else if (g_DofOverride)
		return 0;
	else
		return m_EffectInfo;
}

bool C_Camera::ProjectPoint(const HVec3& worldPos, F32& sx, F32& sy, F32 aspect) const
{
	sx = sy = 0.f;

	HVec3 rel = (worldPos - Position()) * HMat33(!Shaken());
	HF32 rz = rel.Z();
	if (rz < 1e-5f)
		return false;

	F32 height = tan(Fov() * 0.5f);
	if (height < 1e-5f)
		return false;
	F32 width = height * aspect;

	sx = rel.X() / rz / width;
	sy = rel.Y() / rz / height;
	
	return true;
}

bool C_Camera::ProjectPointToUI(const HVec3& worldPos, F32& sx, F32& sy, F32 aspect) const
{
	if (!ProjectPoint(worldPos, sx, sy, aspect))
		return false;
	
	// Convert to 0,0 - 1024, 768 UI coordinates:
	sx = 512.f + sx *  512.f;
	sy = 384.f + sy * -384.f;
	return true;
}

// This function returns a lerp value independent of framerate:
F32 LerpValue(F32 period, F32 lag)
{
	F32 denom = Max(lag + period, 0.00001f);
	return Min(1.f - (lag / denom), 1.f);
}

} // End of namespace

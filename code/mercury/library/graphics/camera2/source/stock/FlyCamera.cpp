
#include "stdafx.h"
#include "camera2/CameraType.h"
#include "camera2/stock/FlyCamera.h"
#include <System/Serialise/I_FileSystem.h>
#include <Framework/application/CoreSystems.h>

namespace Cam2
{

using namespace Maths;

// A standard fly camera with support for adding keyframes to a camscript.txt file

static Debug::Float FlyCameraMoveSpeed("Camera2.FlyCamera.MoveSpeed", 12.f, Debug::Range(5.f, 100.f));
static Debug::Float FlyCameraMoveSpeedY("Camera2.FlyCamera.MoveSpeedY", 2.f, Debug::Range(1.f, 5.f));
static Debug::Float FlyCameraRotateSpeed("Camera2.FlyCamera.RotateSpeed", 1.f, Debug::Range(0.f, 4.f));

Debug::Float KeyFramePeriod("Camera2.FlyCamera.KeyFramePeriod", 2.f, Debug::Range(0.f, 10.f));

static S32 AddKey = 0; // 1 if we should add a keyframe
static void AddKeyFunc(void*)
{
	AddKey = 1;
}
static Debug::Function AddKeyFrame("Camera2.FlyCamera.AddKeyFrame", AddKeyFunc, 0);
static void ClearKeyFramesFunc(void*)
{
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	fs.DeleteFile("camscript.txt");

	Debug::Logger& log = Debug::GetLogger("mercury.camera2");
	LOGINFO(log, "Cleared keyframes");
}
static Debug::Function ClearKeyFrames("Camera2.FlyCamera.ClearKeyFrames", ClearKeyFramesFunc, 0);

static Debug::Float g_SmoothValue("Camera2.FlyCamera.SmoothValue", 0.f, Debug::Range(0.f, 0.25f));
static Debug::Float g_GameSpeed  ("Camera2.FlyCamera.GameSpeed", 1.0f, Debug::Range(0.0f, 1.0f));

C_FlyCamera::C_FlyCamera()
{
	SetType(E_FlyCamera);
	SetName("Fly");
	m_Debounce = 0;
	m_RotateA = m_RotateB = m_RotateC = 0.f;
	m_Zoom = 0.f;
}

void C_FlyCamera::SetInput(const CamInput& input)
{
	m_input = input;
}

void C_FlyCamera::OnUpdate()
{
	// Get input values:
	F32 rota = 0.f, rotb = 0.f, rotc = 0.f;
	F32 right = 0.f, forw = 0.f, up = 0.f, zoom = 0.f;
	LookAxis(rota, rotb);
	MoveAxis(right, up, forw);

	rotc = m_input.val[E_Input_FlyRoll];
	zoom = m_input.val[E_Input_FlyZoom];

	if (rotc != 0.f || zoom != 0.f)
	{
		rota = rotb = forw = right = 0.f;
	}

	F32 period = Period();
	
	F32 fov = Fov();
	F32 rotateSpeed = fov >= 0.f ? fov : 1.f;
	
	rota *= FlyCameraRotateSpeed * rotateSpeed; // Rotate less if we are zoomed in
	rotb *= FlyCameraRotateSpeed * rotateSpeed;

	F32 speed = FlyCameraMoveSpeed;
	F32 slow = 1.f;
	speed *= 1.f + m_input.val[E_Input_FlyFast] * 10.f; // Speed up
	if (m_input.val[E_Input_FlyFast] < 0.5f)
	{
		// LT - Slow down camera:
		slow /= 1.f + m_input.val[E_Input_FlySlow] * 5.f;
	}
	else
	{
		speed *= 1.f + m_input.val[E_Input_FlySlow] * 10.f; // RT + LT: Speed up more
	}
	speed *= slow;
	zoom *= slow;
	rota *= slow;
	rotb *= slow;
	rotc *= slow;

	// Work out how much to smooth out (damp) the values:
	F32 lerp = LerpValue(period, g_SmoothValue);
	m_RotateA += (rota - m_RotateA) * lerp;
	m_RotateB += (rotb - m_RotateB) * lerp;
	m_RotateC += (rotc - m_RotateC) * lerp;
	m_Zoom += (zoom - m_Zoom) * lerp;

	// Rotate:
	C_Quaternion orient = Orient();
	F32 ra = m_RotateA * period;
	F32 rb = m_RotateB * period;
	F32 rc = m_RotateC * period;
	orient = orient * C_Quaternion(0.f, 0.f, sin(rc), cos(rc));
	orient = orient * C_Quaternion(sin(rb), 0.f, 0.f, cos(rb));
	orient = C_Quaternion(0.f, sin(-ra), 0.f, cos(-ra)) * orient;
	orient.Normalise();
	SetOrient(orient);

	SetFov( Fov() - m_Zoom * period * Fov() );
	if (Fov() > 2.f) SetFov( 2.f );

	// Move position
	HVec3 movement(right * speed, up * FlyCameraMoveSpeedY, forw * speed);
	movement = movement * HMat33(Orient());

	m_Velocity += (movement - m_Velocity) * lerp;
	HVec3 pos = Position() + m_Velocity * period;
	SetPosition(pos);

	CheckKeys();

	m_input = CamInput(); // Blank input
}

// Get the movement input values:
void C_FlyCamera::MoveAxis(F32& sx, F32& sy, F32& sz)
{
	sx = m_input.val[E_Input_FlyMoveX];
	sy = m_input.val[E_Input_FlyMoveY];
	sz = m_input.val[E_Input_FlyMoveZ];
}

// Get the look around input values:
void C_FlyCamera::LookAxis(F32& sx, F32& sy)
{
	sx = m_input.val[E_Input_FlyRotateX];
	sy = m_input.val[E_Input_FlyRotateY];
	if (InvertYAxis()) sy = -sy;
}

void C_FlyCamera::CheckKeys()
{
	S32 key = -1;

	if (m_input.val[E_Input_Keyframe] >= 0.5f)
		key = E_Input_Keyframe;

	if (m_Debounce != E_Input_Keyframe && key == E_Input_Keyframe)
		AddKey = 1;

	if (AddKey)
		KeyNow();
	
	AddKey = 0;

	m_Debounce = key;
}

// Virtual function, overridden in RelativeFly:
void C_FlyCamera::KeyNow()
{
	AddKeyFrame(false, Position(), Orient());
}

void QuatToEuler(const C_Quaternion& q, F32& pitch, F32& yaw, F32& roll)
{
	HMat33 mat = HMat33(q);
	
	pitch= asin (mat.AZ().Y());
	yaw  = atan2(mat.AZ().X(), mat.AZ().Z());
	roll = atan2(-mat.AX().Y(), mat.AY().Y());
}

void C_FlyCamera::AddKeyFrame(bool relative, const HVec3& pos, const C_Quaternion& orient)
{
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	
	Sys::I_File* file = fs.Open("camscript.txt", Sys::E_FileAccess_Append);
	
	if (file)
	{
		char text[128] = {0};
		
		F32 pitch = 0.f, yaw = 0.f, roll = 0.f;
		QuatToEuler(orient, pitch, yaw, roll);
		
		sprintf(text, "%d %5.2f %8.3f %8.3f %8.3f %7.2f %7.2f %7.2f %5.2f %6.4f %4.2f %f\r\n",
			relative ? 1 : 0, (F32)KeyFramePeriod, (F32)pos.X(),(F32)pos.Y(),(F32)pos.Z(),
			RadToDeg(pitch), RadToDeg(yaw), RadToDeg(roll),
			RadToDeg(Fov()), Idepth(), Aperture(), (float)g_GameSpeed
		);

		file->Write(text, (U32)strlen(text));

		Debug::Logger& log = Debug::GetLogger("mercury.camera2");
		LOGINFO(log, "Added %skeyframe: %.3f %.3f %.3f (%.2f)",
			relative ? "relative " : "",
			(F32)pos.X(), (F32)pos.Y(), (F32)pos.Z(), RadToDeg(yaw));
	}

	delete file;
}

float C_FlyCamera::GameSpeed() const
{
	return g_GameSpeed;
}

} // End of namespace Cam2

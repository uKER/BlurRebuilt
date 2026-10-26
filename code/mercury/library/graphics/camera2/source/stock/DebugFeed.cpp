
#include "stdafx.h"

#include <camera2.h>
#include <Camera2/CameraFactory.h>
#include <camera2/shake/ShakeWorld.h>
#include <camera2/stock/FlyCamera.h>
#include <camera2/stock/ScriptCamera.h>

namespace Cam2
{

static Debug::Float g_GameBlur("Camera2.PhotoMode.GameBlur", 0.f, Debug::Range(0.f, 1.f));
static Debug::Bool g_ShakeEventsAffectsDebug("Camera2.Shake.ShakeEventsAffectsDebug", true);

// A DebugFeed either shows a game feed, or a temporary debug camera
// Todo - debugfeed should be derived from C_Feed as well as I_Feed

C_DebugFeed::C_DebugFeed(I_Feed* feed)
{
	m_DebugCamera = 0;
	m_AppFeed = 0;
	m_LookCamera = 0;
	m_PadUsed = -1;
	ShowFeed(feed);
}

C_DebugFeed::~C_DebugFeed()
{
	SAFE_DELETE_PTR(m_LookCamera)
	SAFE_DELETE_PTR(m_DebugCamera)
	m_AppFeed = 0;
}

void C_DebugFeed::Update(F32 period)
{
	if (m_DebugCamera)
	{
		m_DebugCamera->SetObject(Object());
		m_DebugCamera->SetShakeWorld(g_ShakeEventsAffectsDebug ? &C_ShakeWorld::Instance() : 0);
		m_DebugCamera->Update(period);
		
		if (g_GameBlur > 0.f)
		{
			// Get motion blur from GameFeed:
			HVec3 blur = m_AppMotion * g_GameBlur;
			m_DebugCamera->SetPrevPosition(m_DebugCamera->InterPosition(0.f) - blur);
		}
	}
	
	if (m_LookCamera)
	{
		// Base LookCamera off the current feed, but looking at a point:
		const C_Camera& live = AppOrDebugLive();

		m_LookCamera->StartUpdate(live.Period());
		m_LookCamera->SetPosition(live.Position());
		m_LookCamera->LookAt(m_LookTarget);
		m_LookCamera->SetFov(m_LookFov);
		m_LookCamera->EndUpdate();
	}
}

const C_Camera& C_DebugFeed::Live() const
{
	if (m_LookCamera)
		return *m_LookCamera;
	else
		return AppOrDebugLive();
}

const C_Camera& C_DebugFeed::AppOrDebugLive() const
{
	if (m_AppFeed)
		return m_AppFeed->Live();

	if (m_DebugCamera)
		return *m_DebugCamera;

	return NullCamera;
}

void C_DebugFeed::SetInput(const CamInput& input)
{
	if (m_DebugCamera == 0)
		return;
	
	if (
		m_DebugCamera->Type() == E_FlyCamera ||
		m_DebugCamera->Type() == E_RelativeFlyCamera
	)
	{
		C_FlyCamera* fly = (C_FlyCamera*)m_DebugCamera;
		fly->SetInput(input);
		
		m_PadUsed = input.pad;
	}
}

void C_DebugFeed::ShowFeed(Cam2::I_Feed* feed)
{
	m_AppFeed = feed;

	delete m_DebugCamera;
	m_DebugCamera = 0;
}

void C_DebugFeed::SetDebugType(S32 type)
{
	// Create a new debug camera:
	C_Camera* newCam = g_CameraFactory.Create(type);
	if (newCam == 0)
		return;
	
	newCam->SetObject(Object());

	// Copy the debug camera position from the last shown camera:
	newCam->CopyFrom(Live());
	newCam->Update(0.f); // Do an initial update
	m_AppMotion = Live().Position() - Live().InterPosition(0.f);
	// Reset shutter angle:
	newCam->SetShutterAngle(0.5f);

	m_AppFeed = 0;

	delete m_DebugCamera;
	m_DebugCamera = newCam;
}

// Return whether the current debug mode uses input
bool C_DebugFeed::UsesInput() const
{
	if (m_DebugCamera == 0)
		return false;
	
	switch (m_DebugCamera->Live().Type())
	{
		case E_FlyCamera:
		case E_RelativeFlyCamera:
			return true;
		break;
	}
	return false;
}

// Return which pad the current debug mode is using
S32 C_DebugFeed::PadUsed() const
{
	if (!UsesInput())
		return -1;
	
	return m_PadUsed;
}

void C_DebugFeed::ShowLook(bool look)
{
	if (look && !m_LookCamera)
	{
		m_LookCamera = new C_Camera;
	}
	
	if (!look)
	{
		SAFE_DELETE_PTR(m_LookCamera)
	}

	if (m_LookCamera)
		m_LookCamera->SetName("Look");
}

void C_DebugFeed::CopyFrom(const C_Camera& camera)
{
	if (m_DebugCamera)
		m_DebugCamera->CopyFrom(camera);
}

// This gamespeed value can be used to slow down the speed of the game from a debug camera:
float C_DebugFeed::GameSpeed() const
{
	if (m_DebugCamera)
	{
		switch (m_DebugCamera->Type())
		{
			case E_FlyCamera:
			case E_RelativeFlyCamera:
				return ((C_FlyCamera*)m_DebugCamera)->GameSpeed();

			case E_ScriptCamera:
				return ((C_ScriptCamera*)m_DebugCamera)->GameSpeed();
		}
	}
	
	return 1.0f;
}

} // End of namespace Cam2

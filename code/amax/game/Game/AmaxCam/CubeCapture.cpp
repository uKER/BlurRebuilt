
#include "stdafx.h"
#include "CubeCapture.h"

#ifndef CFG_GOLD

#include "AppPhotoCapture.h"
#include "CameraSwitcher.h"

#include <camera2/stock/DebugFeed.h>
#include <framework/application/ApplicationBase.h>

namespace AmaxCam
{
using namespace Maths;

static Debug::Bool g_CubeCapture("Camera2.PhotoMode.CubeCapture", false);

C_CubeCapture::C_CubeCapture()
{
	m_Counter = -1;
}

C_CubeCapture::~C_CubeCapture()
{
}

void C_CubeCapture::RealUpdate()
{
	if (g_CubeCapture)
	{
		m_Counter = 0;
		g_CubeCapture = false;
	}

	if (m_Counter < 0)
		return;

	if (m_Counter == 0)
		Begin();

	// Point the camera in a direction:
	if ((m_Counter & 7) == 0)
		PointInDirection(m_Counter >> 3);
	
	// After 7 frames, take a Photo Mode shot:
	if ((m_Counter & 7) == 7)
		PhotoMode::AppPhotoCapture().TakePhoto(0, false, false, true);

	if (m_Counter < 6 * 8)
	{
		m_Counter++;
	}
	else
	{
		End();
		m_Counter = -1;
	}
}

void C_CubeCapture::Begin()
{
	Framework::C_ApplicationBase::GetInstance()->Pause( Framework::E_LayerPause_Freeze );
	m_OldCamera.CopyFrom(CameraSystem().DebugFeed().Live());
	Switcher().SetType(E_DebugCam_FixedCamera);
}

void C_CubeCapture::End()
{
	Framework::C_ApplicationBase::GetInstance()->Resume( Framework::E_LayerPause_Freeze );
	Switcher().SetType(E_DebugCam_FlyCamera);
	CameraSystem().DebugFeed().CopyFrom(m_OldCamera);
}

void C_CubeCapture::PointInDirection(int dir)
{
	Cam2::C_Camera tmp;
	tmp.CopyFrom(m_OldCamera);

	C_Quaternion orient;
	switch (dir)
	{
		case 0: orient = C_Quaternion( 0.000f, 0.000f, 0.000f, 1.000f); break;
		case 1: orient = C_Quaternion( 0.000f,-0.707f, 0.000f, 0.707f); break;
		case 2: orient = C_Quaternion( 0.000f, 1.000f, 0.000f, 0.000f); break;
		case 3: orient = C_Quaternion( 0.000f, 0.707f, 0.000f, 0.707f); break;
		case 4: orient = C_Quaternion( 0.707f, 0.000f, 0.000f, 0.707f); break;
		case 5: orient = C_Quaternion(-0.707f, 0.000f, 0.000f, 0.707f); break;
	}
	orient.Normalise();

	tmp.SetOrient(orient);
	tmp.SetFov(DegToRad(90.0f));
	CameraSystem().DebugFeed().CopyFrom(tmp);
}

} // End of namespace

#endif

#ifndef CAMERA_SWITCHER_H
#define CAMERA_SWITCHER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <camera2/CameraType.h>
#include <camera2/debug/CameraSwitch.h>

namespace Cam2
{
	class C_Camera;
};

namespace AmaxCam
{

enum E_DebugCam
{
	E_DebugCam_GameFeed = 1,
	E_DebugCam_FatalCamera,
	E_DebugCam_FutureFeed,
	E_DebugCam_GravGun,
	E_DebugCam_HOGResetCamera,
	E_DebugCam_OrbitCamera,
	E_DebugCam_TvFeed,
	E_DebugCam_VehicleFeed,

	E_DebugCam_AnimCamera = Cam2::E_AnimCamera,
	E_DebugCam_ChaseCamera2 = Cam2::E_ChaseCamera2,
	E_DebugCam_FlyCamera = Cam2::E_FlyCamera,
	E_DebugCam_FixedCamera = Cam2::E_FixedCamera,
	E_DebugCam_RelativeFlyCamera = Cam2::E_RelativeFlyCamera,
	E_DebugCam_RelativeCamera = Cam2::E_RelativeCamera,
	E_DebugCam_ScriptCamera = Cam2::E_ScriptCamera,
	E_DebugCam_SimpleChaseCamera = Cam2::E_SimpleChaseCamera,
	E_DebugCam_TvCamera = Cam2::E_TvCamera,
};

class C_CameraSwitcher : public Cam2::C_CameraSwitch
{
public:
	C_CameraSwitcher();
	void Init();
	void Exit();
	void Update(F32 period);
	void SetType(E_DebugCam type);
	S32 DebugPad() const { return m_DebugPad; }
	S32 DebugViewport() const;
	
private:
	S32 m_DebugPad;
};

// C_CameraSwitcher Singleton:
C_CameraSwitcher& Switcher();

}

#endif // CAMERA_SWITCHER_H

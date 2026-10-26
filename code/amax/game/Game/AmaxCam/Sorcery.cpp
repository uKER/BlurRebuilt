
#include "stdafx.h"
#include "CameraActor.h"
#include "DirectorActor.h"
#include <Xt/XtGame.h>

namespace AmaxCam
{
	class C_AccelEffect;
	class C_AmaxChaseParam;
	class C_BoltedParam;
	class C_CameraActorCreate;
	class C_CameraKeyFrame;
	class C_DirectorActorCreate;
	class C_ShutterCurve;
	class C_VehicleFeedParam;
	class C_ViewportFovScale;
	class C_ViewportScreenOffset;
	class C_XtShowGameCameraMsg;
	class C_XtShowLevelCameraMsg;
	class C_XtShowEffectMsg;
}

XT_LIBRARY(AmaxCam)
	XT_REGISTER_TYPE(AmaxCam::C_AccelEffect)
	XT_REGISTER_TYPE(AmaxCam::C_AmaxChaseParam)
	XT_REGISTER_TYPE(AmaxCam::C_BoltedParam)
	XT_REGISTER_TYPE(AmaxCam::C_CameraActorCreate)
	XT_REGISTER_TYPE(AmaxCam::C_CameraKeyFrame)
	XT_REGISTER_TYPE(AmaxCam::C_DirectorActorCreate)
	XT_REGISTER_TYPE(AmaxCam::C_ShutterCurve)
	XT_REGISTER_TYPE(AmaxCam::C_VehicleFeedParam)
	XT_REGISTER_TYPE(AmaxCam::C_ViewportFovScale)
	XT_REGISTER_TYPE(AmaxCam::C_ViewportScreenOffset)
	XT_REGISTER_TYPE(AmaxCam::C_XtShowGameCameraMsg)
	XT_REGISTER_TYPE(AmaxCam::C_XtShowLevelCameraMsg)
	XT_REGISTER_TYPE(AmaxCam::C_XtShowEffectMsg)
	XT_REGISTER_TYPE(AmaxCam::E_Effect)
	XT_REGISTER_TYPE(AmaxCam::E_GameCam)
	XT_REGISTER_TYPE(AmaxCam::E_WhichFeed)
XT_END_LIBRARY


#include "stdafx.h"
#include "CameraActor.h"
#include "DirectorActor.h"
#include <Xt/XtGame.h>

namespace MGCam
{
	class C_CameraActorCreate;
	class C_DirectorActorCreate;
	class C_MovementDualSplineCreate;
}

XT_LIBRARY(MGCam)
	XT_REGISTER_TYPE(MGCam::C_CameraActorCreate)
	XT_REGISTER_TYPE(MGCam::C_DirectorActorCreate)
	XT_REGISTER_TYPE(MGCam::C_MovementDualSplineCreate)
	XT_REGISTER_TYPE(MGCam::C_XtShowGameCameraMsg)
	XT_REGISTER_TYPE(MGCam::C_XtShowLevelCameraMsg)
XT_END_LIBRARY

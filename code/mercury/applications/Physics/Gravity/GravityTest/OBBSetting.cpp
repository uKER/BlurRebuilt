#include "stdafx.h"
#include "OBBSetting.h"


Debug::Float obb_cX("OBB.c.X", 4.f, Debug::Range(-100.f, 100.f) );
Debug::Float obb_cY("OBB.c.Y", -4.7f, Debug::Range(-100.f, 100.f) );
Debug::Float obb_cZ("OBB.c.Z", -4.f, Debug::Range(-100.f, 100.f) );

Debug::Float obb_eX("OBB.e.X", 1.f, Debug::Range(0.f, 100.f) );
Debug::Float obb_eY("OBB.e.Y", 0.5f, Debug::Range(0.f, 100.f) );
Debug::Float obb_eZ("OBB.e.Z", 2.f, Debug::Range(0.f, 100.f) );

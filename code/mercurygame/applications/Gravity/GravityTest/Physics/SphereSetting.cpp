#include "stdafx.h"
#include "SphereSetting.h"


Debug::Float sX("Sphere.c.X", 2.f, Debug::Range(-100.f, 100.f) );
Debug::Float sY("Sphere.c.Y", 2.f, Debug::Range(-100.f, 100.f) );
Debug::Float sZ("Sphere.c.Z", -1.f, Debug::Range(-100.f, 100.f) );

Debug::Float r("Sphere.r", 4.6f, Debug::Range(-100.f, 100.f) );

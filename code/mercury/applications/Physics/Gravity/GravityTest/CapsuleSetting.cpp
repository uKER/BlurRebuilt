#include "stdafx.h"
#include "CapsuleSetting.h"

Debug::Float capsuleSX("Capsule.Start.X", 3.33f, Debug::Range(-100.f, 100.f));
Debug::Float capsuleSY("Capsule.Start.Y", 3.33f, Debug::Range(-100.f, 100.f));
Debug::Float capsuleSZ("Capsule.Start.Z", -4.66f, Debug::Range(-100.f, 100.f));

Debug::Float capsuleEX("Capsule.End.X", -2.f, Debug::Range(-100.f, 100.f));
Debug::Float capsuleEY("Capsule.End.Y", 4.66f, Debug::Range(-100.f, 100.f));
Debug::Float capsuleEZ("Capsule.End.Z", -10.f, Debug::Range(-100.f, 100.f));

Debug::Float capsuleRadius("Capsule.Radius", 1.f, Debug::Range(0.f, 10.f));

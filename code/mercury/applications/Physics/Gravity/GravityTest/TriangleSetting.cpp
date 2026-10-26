#include "stdafx.h"
#include "TriangleSetting.h"

Debug::Float triAX("Triangle.A.X",  3.4f, Debug::Range(-100.f, 100.f), Debug::Persist());
Debug::Float triAY("Triangle.A.Y",  2.7f, Debug::Range(-100.f, 100.f), Debug::Persist());
Debug::Float triAZ("Triangle.A.Z",  -17.f, Debug::Range(-100.f, 100.f), Debug::Persist());

Debug::Float triBX("Triangle.B.X",  2.f, Debug::Range(-100.f, 100.f), Debug::Persist());
Debug::Float triBY("Triangle.B.Y",  3.f, Debug::Range(-100.f, 100.f), Debug::Persist());
Debug::Float triBZ("Triangle.B.Z",  4.f, Debug::Range(-100.f, 100.f), Debug::Persist());

Debug::Float triCX("Triangle.C.X",  -20.f, Debug::Range(-100.f, 100.f), Debug::Persist());
Debug::Float triCY("Triangle.C.Y",  18.66f, Debug::Range(-100.f, 100.f), Debug::Persist());
Debug::Float triCZ("Triangle.C.Z",  -55.33f, Debug::Range(-100.f, 100.f), Debug::Persist());

Debug::Float triLambda("Triangle.Lambda", 0.f, Debug::Range(0.f, 1.f), Debug::Persist());

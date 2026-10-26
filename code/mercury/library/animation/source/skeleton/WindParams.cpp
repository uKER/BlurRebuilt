#include "stdafx.h"
#include <animation/skeleton/WindParams.h>

namespace Animation
{

C_WindParams::C_WindParams()
{
	SetWindDirection(Maths::MakeVector3(0.f, 0.f, 1.f));
	SetWindDirectionVariation(2.f);
	SetWindStrength(28.f);
	SetWindStrengthVariation(2.f);
	SetWindVariationRate(4.f);
}

} // end Animation

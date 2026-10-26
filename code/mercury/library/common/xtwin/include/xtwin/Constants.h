#ifndef CONSTANTS_H
#define CONSTANTS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Core
{
	namespace Constants
	{
		const float kPi		= 3.1415926536f;
		const float k2Pi	= 2.0f * kPi;
		const float k1OnPi	= 1.0f / kPi;
		const float k1On2Pi	= 1.0f / k2Pi;
	};
};

#endif // CONSTANTS_H
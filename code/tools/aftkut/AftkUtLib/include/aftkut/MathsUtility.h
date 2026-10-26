#ifndef AFTK_UT_LIB_MATHS_UTILITY_H
#define AFTK_UT_LIB_MATHS_UTILITY_H

namespace AftkUt
{
	template<class T>
	inline T DegToRad(T deg)
	{
		return deg * PI/180;
	}

	template<class T>
	inline T RadToDeg(T rad)
	{
		return rad * 180/PI;
	}
}

#endif

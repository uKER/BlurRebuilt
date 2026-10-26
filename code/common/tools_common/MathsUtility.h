#ifndef TU_MATHS_UTILITY_H
#define TU_MATHS_UTILITY_H

namespace Tu
{
	template<typename T>
	__forceinline T Round(T& val)
	{
		return (T)floor((F32)val+0.5f);
	}

	template<typename T>
	__forceinline T DegsToRads(const T& degrees)
	{
		return degrees * PI / 180.0f;
	}

	template<typename T>
	__forceinline T RadsToDegs(const T& radians)
	{
		return radians * 180.0f / PI;
	}

	template<typename T>
	__forceinline S32 Sgn (const T& value)
	{ 
		if (value < 0) 
			return -1; 
		else 
			return 1; 
	}

	template<typename T>
	__forceinline T Max (const T& x, const T& y)
	{
		if (x > y) {
			return x;
		} else {
			return y;
		}
	}

	template<typename T>
	__forceinline T Min (const T& x, const T& y)
	{
		if (x < y) {
			return x;
		} else {
			return y;
		}
	}
}
#endif


#include "MiscSharedPch.h"

#include <misc_shared/types/Float16.h>

namespace Half
{

F16::F16(const F32 f32)
{
	FloatDecomp::C_F32 f(f32);	
	FloatDecomp::C_F16 resultF16;
	resultF16.m_U16 = 0;

	U32 sign = f.GetSign();
	U32 mantissa = f.GetMantissa();
	U32 exponent = f.GetExponent();

	resultF16.SetSign(sign);

	if(exponent == 0xff)
	{
		// special cases
		resultF16.SetExponent(31);

		if(mantissa != 0)	// NAN
			resultF16.SetMantissa(1);
	}
	else if(exponent != 0)
	{
		// Regular number
		int newExponent = exponent - 127;
		if(newExponent < -14)
		{
			// This maps to a denorm
			U32 e = (U32) (-14 - newExponent); // 2 ^ -exponent
			if(e < 10)
				resultF16.SetMantissa((0x800000 | mantissa) >> (13 + e));
			else if(e == 10)
				resultF16.SetMantissa(1);
		}
		else if(newExponent > 15)
		{
			// Map this value to infinity
			resultF16.SetExponent(31);
		}
		else
		{
			resultF16.SetMantissa(mantissa >> 13);
			resultF16.SetExponent(newExponent + 15);
		}
	}	

	m_U16 = resultF16.m_U16;
}

F16& F16::operator = (const F32 f32In)
{
	*this = F16(f32In);
	return *this;
}

F16::operator const F32() const
{
	FloatDecomp::C_F16 h(m_U16);
	FloatDecomp::C_F32 resultF32;

	U32 sign = h.GetSign();
	U32 mantissa = h.GetMantissa();
	U32 exponent = h.GetExponent();
	
	resultF32.SetSign(sign);

	if(exponent == 0)
	{
		if(mantissa != 0)
		{
			// Denorm -- denorm half will fit in non-denorm single
			const F32 halfDenorm = 1.0f / 16384.0f; // 2^-14
			F32 m = ((F32)(mantissa)) / 1024.0f;
			F32 sgn = sign ? -1.0f : 1.0f;
			resultF32.m_F32 = sgn * m * halfDenorm;
		}
	}
	else if(exponent == 31)
	{
		resultF32.SetExponent(0xff);
		if(mantissa != 0)	// NAN
			resultF32.SetMantissa(1);
	}
	else
	{
		resultF32.SetExponent(exponent + 112);
		resultF32.SetMantissa(mantissa << 13);
	}

	return resultF32.m_F32;
}

} // namespace Half

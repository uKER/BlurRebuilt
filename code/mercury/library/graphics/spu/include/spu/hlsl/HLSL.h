#ifndef _SPUHLSL_H_INCLUDED_
#define _SPUHLSL_H_INCLUDED_
#ifdef PLATFORM_SPU

#include "Spu/hlsl/_SwizzleHelper.h"

#define SPUHLSL_FAST_CLAMP				//!< Less accurate clamp, causes off by 1 error at top of the range.

//! Float 4x4: Matrix structure.
struct float4x4
{
	qword x;			//!< X row.
	qword y;			//!< Y row.
	qword z;			//!< Z row.
	qword w;			//!< W row.
};

//! 4 element float.
struct float4
{
	inline float4()
		{ return; }
		
	//! Conversion from qword.
	inline float4(const qword q) : m_v(q)
		{ return; }
		
	//! Conversion from float.
	inline float4(const float f)
	{
		const vector unsigned char _shuf_AAAA =
		{
			0x00, 0x01, 0x02, 0x03,
			0x00, 0x01, 0x02, 0x03,
			0x00, 0x01, 0x02, 0x03,
			0x00, 0x01, 0x02, 0x03
			};

		qword val_swiz	= (const qword)si_from_ptr(&f);
			  val_swiz	= si_lqd(val_swiz, 0);
			  m_v		= si_shufb(val_swiz, val_swiz, (const qword)_shuf_AAAA);
	}
			
	//! Addition operator.
	//@param val RHS vector to add.
	//@return Result of addition.
	const float4 operator + (const float4 val) const __restrict __attribute__((pure, const, always_inline))
	{
		return si_fa(m_v, val.m_v);
	}
	
	//! Subtraction operator.
	//@param val RHS vector to sub.
	//@return Result of subtraction.
	const float4 operator - (const float4 val) const __restrict __attribute__((pure, const, always_inline))
	{
		return si_fs(m_v, val.m_v);
	}
	
	//! Assignment operator.
	//@param val RHS vector to assignment.
	//@return current vector;
	const float4 operator = (const float4 val) __restrict __attribute((pure, const, always_inline))
	{
		m_v = val.m_v;
		return *this;
	}
	
	//! Assignment operator.
	//@param val RHS scalar to assignment.
	//@return current vector;
	const float4 operator = (const float val) __restrict __attribute((pure, const, always_inline))
	{
		const vector unsigned char _shuf_AAAA =
		{
			0x00, 0x01, 0x02, 0x03,
			0x00, 0x01, 0x02, 0x03,
			0x00, 0x01, 0x02, 0x03,
			0x00, 0x01, 0x02, 0x03
		};
		
		qword val_swiz	= (const qword)si_from_ptr(&val);
			  val_swiz	= si_lqd(val_swiz, 0);
			  m_v		= si_shufb(val_swiz, val_swiz, (const qword)_shuf_AAAA);
		return *this;
	}
	
	//! Swizzle 4 element target.
	inline const float4 swizzle4(const qword pattern) const __restrict __attribute__((pure, const, always_inline))
	{
		return si_shufb(m_v, m_v, (const qword)pattern);
	}
	
	//! Swizzle 3 element target.
	inline const float swizzle3(qword pattern) const __restrict __attribute__((pure, const, always_inline))
	{
		// TODO: [Stove] Implement swizzle1.
		return 0.0f;
	}
	
	//! Swizzle two element target.
	inline const float swizzle2(qword pattern) const __restrict __attribute__((pure, const, always_inline)) 
	{
		// TODO: [Stove] Implement swizzle1.
		return 0.0f;
	}
	
	//! Swizzle scalar target.
	inline const float swizzle1(qword pattern) const __restrict __attribute__((pure, const, always_inline)) 
	{
		// TODO: [Stove] Implement swizzle1.
		return 0.0f;
	}
	
	qword m_v;
};

//typedef qword float4;	//!< Float 4.
//typedef qword float3;	//!< Float 3.
//typedef qword float2;	//!< Float 2.

//! Absolute value of x.
//@param x Value to compute absolute value of.
//@return Absolute value.
static inline const qword abs(const float4 x) __attribute__((pure, const, always_inline));
static inline const qword abs(const float4 x)
{
	const vector unsigned char _mask =
		{
			0x7f, 0xff, 0xff, 0xff,
			0x7f, 0xff, 0xff, 0xff,
			0x7f, 0xff, 0xff, 0xff,
			0x7f, 0xff, 0xff, 0xff
		};
	
	const qword mask		= (const qword)_mask;								// ODD, 6
		  qword result		= si_and(x.m_v, mask);								// EVEN, 2
	return result;
}

//! Return true if any value in x is non-zero.
//@param x Value to check for non-zero.
//@return Boolean value, true if all components of x are non-zero.
static inline const bool any(const float4 x) __attribute__((pure, const, always_inline));
static inline const bool any(const float4 x)
{

}

//! Returns the distance between 2 points.
//@param x 1st point.
//@param y 2nd point.
//@return The distance between two values.
static inline const float distance(const float4 x, const float4 y) __attribute__((pure, const, always_inline));
static inline const float distance(const float4 x, const float4 y)
{
	qword x_mul_y			= si_fs(x.m_v, y.m_v);								// EVEN, 6
	qword dot_x_mul_y		= si_fm(x_mul_y, x_mul_y);							// EVEN, 6
	qword dot_x_mul_y_r4	= si_rotqbyi(dot_x_mul_y, 4);						// ODD, 4
		  dot_x_mul_y		= si_fa(dot_x_mul_y, dot_x_mul_y_r4);				// EVEN, 6
	qword dot_x_mul_y_r8	= si_rotqbyi(dot_x_mul_y_r4, 4);					// ODD, 4
		  dot_x_mul_y		= si_fa(dot_x_mul_y, dot_x_mul_y_r8);				// EVEN, 6
	qword dot_x_mul_y_sqrt	= si_frsqest(dot_x_mul_y);							// ODD, 4
		  dot_x_mul_y_sqrt	= si_frest(dot_x_mul_y_sqrt);						// ODD, 4
	return si_to_float(dot_x_mul_y_sqrt);
}

//! Return the dot product of two vectors.
//@param x 1st vector.
//@param y 2nd vector.
//@return The dot product of two vectors.
static inline const float dot(const float4 x, const float4 y) __attribute__((pure, const, always_inline));
static inline const float dot(const float4 x, const float4 y)
{
	qword dot_xy			= si_fm(x.m_v, y.m_v);								// EVEN, 6
	qword dot_xy_r4			= si_rotqbyi(dot_xy, 4);							// ODD, 4
		  dot_xy			= si_fa(dot_xy, dot_xy_r4);							// EVEN, 6
	qword dot_xy_r8			= si_rotqbyi(dot_xy_r4, 4);							// ODD, 4
		  dot_xy			= si_fa(dot_xy, dot_xy_r8);							// EVEN, 6
	return si_to_float(dot_xy);
}

//! Return the dot product of two vectors.
//@param x 1st vector.
//@return The fraction part of each component of the float4.
static inline const float frac(const float4 x) __attribute__((pure, const, always_inline));
static inline const float frac(const float4 x)
{
	qword frac				= si_cfltu(x.m_v, 0x0);
	qword frac_f			= si_cuflt(frac, 0x0);
	qword result			= si_fs(x.m_v, frac_f);
	return result;
}

//! Return the length of the vector.
//@param x 1st vector.
//@return The length of the vector.
static inline const float length(const float4 x) __attribute__((pure, const, always_inline));
static inline const float length(const float4 x)
{
	qword dot_xy			= si_fm(x.m_v, x.m_v);								// EVEN, 6
	qword dot_xy_r4			= si_rotqbyi(dot_xy, 4);							// ODD, 4
		  dot_xy			= si_fa(dot_xy, dot_xy_r4);							// EVEN, 6
	qword dot_xy_r8			= si_rotqbyi(dot_xy_r4, 4);							// ODD, 4
		  dot_xy			= si_fa(dot_xy, dot_xy_r8);							// EVEN, 6
	qword dot_xy_sqrt		= si_frsqest(dot_xy);								// ODD, 4
		  dot_xy_sqrt		= si_frest(dot_xy_sqrt);							// ODD, 4
	return si_to_float(dot_xy_sqrt);
}

//! Compute the max of x and y.
//@param x First vector.
//@param y Second vector.
//@return Either x or y depending on which is larger.
static inline const qword max(const float4 x, const float4 y) __attribute__((pure, const, always_inline));
static inline const qword max(const float4 x, const float4 y)
{
	qword mask 				= si_fcgt(x.m_v, y.m_v);							// EVEN, 2
	qword result			= si_selb(x.m_v, y.m_v, mask);						// EVEN, 2
	return result;
}

//! Compute the min of x and y.
//@param x First vector.
//@param y Second vector.
//@return Either x or y depending on which is smaller.
static inline const qword min(const float4 x, const float4 y) __attribute__((pure, const, always_inline));
static inline const qword min(const float4 x, const float4 y)
{
	qword mask 				= si_fcgt(x.m_v, y.m_v);							// EVEN, 2
	qword result			= si_selb(y.m_v, x.m_v, mask);						// EVEN, 2
	return result;
}

//! Transform a vector by a matrix.
//@param x Vector.
//@param m Matrix.
//@return Transformed vector.
static inline const qword mul(const float4 x, const float4 a, const float4 b, const float4 c, const float4 d) __attribute__((pure, const, always_inline));
static inline const qword mul(const float4 x, const float4 a, const float4 b, const float4 c, const float4 d)
{
	const vector unsigned char _ABab = 
		{
			0x00, 0x01, 0x02, 0x03,
			0x04, 0x05, 0x06, 0x07,
			0x10, 0x11, 0x12, 0x13,
			0x14, 0x15, 0x16, 0x17
		};
	const vector unsigned char _ACac = 
		{
			0x00, 0x01, 0x02, 0x03,
			0x08, 0x09, 0x0a, 0x0b,
			0x10, 0x11, 0x12, 0x13,
			0x18, 0x19, 0x1a, 0x1b
		};
	const vector unsigned char _BDbd = 
		{
			0x04, 0x05, 0x06, 0x07,
			0x0c, 0x0d, 0x0e, 0x0f,
			0x14, 0x15, 0x16, 0x17,
			0x1c, 0x1d, 0x1e, 0x1f
		};          
	const vector unsigned char _CDcd = 
		{
			0x08, 0x09, 0x0a, 0x0b,
			0x0c, 0x0d, 0x0e, 0x0f,
			0x18, 0x19, 0x1a, 0x1b,
			0x1c, 0x1d, 0x1e, 0x1f
		};          

		  qword mul_xx      = si_fm(x.m_v, a.m_v);								// 6 EVEN
	const qword shuf_ABab   = (const qword)_ABab;                           	// 6 ODD
		  qword mul_xy      = si_fm(x.m_v, b.m_v);								// 6 EVEN
	const qword shuf_ACac   = (const qword)_ACac;                           	// 6 ODD
		  qword mul_xz      = si_fm(x.m_v, c.m_v);								// 6 EVEN
	const qword shuf_BDbd   = (const qword)_BDbd;                           	// 6 ODD
		  qword mul_xw      = si_fm(x.m_v, d.m_v);								// 6 EVEN
	const qword shuf_CDcd   = (const qword)_CDcd;                           	// 6 ODD
		  qword dot_abcd_xy = si_shufb(mul_xx, mul_xy, shuf_ABab);              // 4 ODD
		  qword dot_efgh_xy = si_shufb(mul_xz, mul_xw, shuf_ABab);              // 4 ODD
		  qword dot_all_x   = si_shufb(dot_abcd_xy, dot_efgh_xy, shuf_ACac);    // 4 ODD
		  qword dot_all_y   = si_shufb(dot_abcd_xy, dot_efgh_xy, shuf_BDbd);    // 4 ODD
		  qword dot_abcd_zw = si_shufb(mul_xx, mul_xy, shuf_CDcd);              // 4 ODD
		  qword dot_efgh_zw = si_shufb(mul_xz, mul_xw, shuf_CDcd);              // 4 ODD
		  qword dot_all_z   = si_shufb(dot_abcd_zw, dot_efgh_zw, shuf_ACac);    // 4 ODD
		  qword dot_all_w	= si_shufb(dot_abcd_zw, dot_efgh_zw, shuf_BDbd);	// 4 ODD
		  qword dot_all_xy  = si_fa(dot_all_x, dot_all_y);                      // 6 EVEN
		  qword dot_all_zw  = si_fa(dot_all_z, dot_all_w);						// 6 EVEN
		  qword dot_xyzw    = si_fa(dot_all_xy, dot_all_zw);					// 6 EVEN
	return dot_xyzw;
}

//! Normalize vector x.
//@param x Vector.
//@return Normalized version of x.
static inline const qword normalize(const float4 x) __attribute__((pure, const, always_inline));
static inline const qword normalize(const float4 x)
{
	qword dot_xy		= si_fm(x.m_v, x.m_v);									// EVEN, 6
	qword dot_xy_r4		= si_rotqbyi(dot_xy, 4);								// ODD, 4
		  dot_xy		= si_fa(dot_xy, dot_xy_r4);								// EVEN, 6
	qword dot_xy_r8		= si_rotqbyi(dot_xy_r4, 4);								// ODD, 4
		  dot_xy		= si_fa(dot_xy, dot_xy_r8);								// EVEN, 6
	qword dot_xy_sqrt	= si_frsqest(dot_xy);									// ODD, 4
	qword result		= si_fm(dot_xy_sqrt, x.m_v);							// EVEN, 6
	return result;
}

//! Compute x^y.
//@param x Base.
//@param y Exponent.
//@return x^y for each component.
static inline const qword pow(const float4 x, const float4 y) __attribute__((pure, const, always_inline));
static inline const qword pow(const float4 x, const float4 y)
{
	// TODO: [Stove] Finish this.
	qword result = si_il(0x0000);
	return result;	
}

//! Reflect a vector about it's normal.
//@param x Vector to reflect.
//@param n Normal to reflect about.
//@return Vector reflected about normal.
static inline const qword reflect(const float4 i, const float4 n) __attribute__((pure, const, always_inline));
static inline const qword reflect(const float4 i, const float4 n)
{
	qword shuf_xxxx			= si_il(0x00010203);								// ODD, 2
	qword two				= si_ilh(0x4000);									// ODD, 2
	qword i_dot_n			= si_fm(i.m_v, n.m_v);								// EVEN, 6
	qword i_dot_n_sh		= si_rotqbyi(i_dot_n, 4);							// ODD, 4
		  i_dot_n			= si_fa(i_dot_n, i_dot_n_sh);						// EVEN, 6
		  i_dot_n_sh		= si_rotqbyi(i_dot_n_sh, 4);						// ODD, 4
		  i_dot_n			= si_fa(i_dot_n, i_dot_n_sh);						// EVEN, 6
		  i_dot_n			= si_shufb(i_dot_n, i_dot_n, shuf_xxxx);			// ODD, 4
	qword dot_2				= si_fm(i_dot_n, two);								// EVEN, 6
	qword dot_2_i			= si_fm(dot_2, i.m_v);								// EVEN, 6
	qword i_sub_dot_2_i		= si_fs(i.m_v, dot_2_i);							// EVEN, 6
	return i_sub_dot_2_i;
}

//! Returns a refraction vector using an entering ray, a surface normal, and a refraction index.
//@param x A floating-point, ray direction vector.
//@param n A floating-point, surface normal vector.
//@param r A floating-point, refraction index scalar.
//@return A floating-point, refraction vector. If the angle between the entering ray i and the surface normal n is too great for a given refraction index, the return value is (0,0,0).
static inline const qword refract(const float4 i, const float4 n, const float r) __attribute__((pure, const, always_inline));
static inline const qword refract(const float4 i, const float4 n, const float r)
{	
	const vector unsigned char _shuf_xxxx =
		{
			0x00, 0x01, 0x02, 0x03,
			0x00, 0x01, 0x02, 0x03,
			0x00, 0x01, 0x02, 0x03,
			0x00, 0x01, 0x02, 0x03
		};
	const vector unsigned char _mask =
		{
			0x7f, 0xff, 0xff, 0xff,
			0x7f, 0xff, 0xff, 0xff,
			0x7f, 0xff, 0xff, 0xff,
			0x7f, 0xff, 0xff, 0xff			
		};

	qword negone			= si_ilhu(0xbf80);										// ODD, 2
	qword invi				= si_fm(negone, i.m_v);									// EVEN, 6
	qword shuf_xxxx			= (const qword)_shuf_xxxx;								// ODD, 6
	qword dot_invi_n		= si_fm(invi, n.m_v);									// EVEN, 6
	qword dot_invi_n_r4		= si_rotqbyi(dot_invi_n, 4);							// ODD, 4
		  dot_invi_n		= si_fa(dot_invi_n, dot_invi_n_r4);						// EVEN, 6
	qword dot_invi_n_r8		= si_rotqbyi(dot_invi_n, 4);							// ODD, 4
	qword cosI				= si_fa(dot_invi_n, dot_invi_n_r8);						// EVEN, 6		-- float cosI = dot(-I, N);
	qword one				= si_ilhu(0x3f80);										// ODD, 2
	qword eta_ratio			= si_from_ptr(&r);										// ODD, 6
		  eta_ratio			= si_lqd(eta_ratio, 0x0);								// ODD, 6
	qword cosI_sq			= si_fm(cosI, cosI);									// EVEN, 6 
	qword inv_cos_sq		= si_fs(one, cosI_sq);									// EVEN, 6
	qword zero				= si_il(0x0000);										// ODD, 2
	qword eta_ratio_sq		= si_fm(eta_ratio, eta_ratio);							// EVEN, 6
	qword cosIshuf			= si_shufb(cosI, cosI, shuf_xxxx);						// ODD, 4
	qword eta_mul_cosI		= si_fm(inv_cos_sq, eta_ratio_sq);						// EVEN, 6
	qword inv_eta_m_cosI	= si_fs(one, eta_mul_cosI);								// EVEN, 6
	qword cosT2				= si_shufb(inv_eta_m_cosI, inv_eta_m_cosI, shuf_xxxx);	// ODD, 4		-- float cosT2 = 1.0f - etaRatio * etaRatio * (1.0f – cosI * cosI);
	qword eta_ratio_shuf	= si_shufb(eta_ratio, eta_ratio, shuf_xxxx);			// ODD, 4
	qword mask				= (const qword)_mask;									// ODD, 6
	qword abs_cosT2			= si_and(cosT2, mask);									// EVEN, 2
	qword sqrt_abs_cosT2	= si_frsqest(abs_cosT2);								// ODD, 4
		  sqrt_abs_cosT2	= si_frest(sqrt_abs_cosT2);								// ODD, 4
	qword eta_mul_cosI_shuf	= si_fm(eta_ratio_shuf, cosIshuf);						// EVEN, 6
	qword e_m_cosI_s_cosT2	= si_fs(eta_mul_cosI_shuf, sqrt_abs_cosT2);				// EVEN, 6
	qword emcosIcosT2_n		= si_fm(e_m_cosI_s_cosT2, n.m_v);						// EVEN, 6
	qword eta_mul_i			= si_fm(eta_ratio_shuf, i.m_v);							// EVEN, 6
	qword t					= si_fa(eta_mul_i, emcosIcosT2_n);						// EVEN, 6		-- float3 T = etaRatio * I +((etaRatio * cosI - sqrt(abs(cosT2))) * N);
		  mask				= si_fcgt(cosT2, zero);									// EVEN, 2
	qword multiplier		= si_selb(one, zero, mask);								// EVEN, 2
	qword result			= si_fm(multiplier, t);									// EVEN, 6		-- T * (float3)(cosT2 > 0);
	return result;
}

//! Returns 1 / sqrt(x).
//@param x Input vector.
//@return 1 / sqrt(x).
static inline const qword rsqrt(const float4 x) __attribute__((pure, const, always_inline));
static inline const qword rsqrt(const float4 x)
{
	return si_frsqest(x.m_v);													// ODD, 4
}

//! Saturate a value to [0.f, 1.0f] range.
//@param x Vector to saturate.
//@return Saturated vector.
static inline const qword saturate(const float4 x) __attribute__((pure, const, always_inline));
static inline const qword saturate(const float4 x)
{
#ifdef SPUHLSL_FAST_CLAMP
	qword a					= si_cfltu(x.m_v, 0x20);							// EVEN, 7
	qword b					= si_cuflt(a, 0x20);								// EVEN, 7
#else
	qword zero				= si_il(0x0000);									// ODD, 2
	qword one				= si_ilh(0x3f80);									// ODD, 2
	qword a_mask			= si_fcgt(x.m_v, zero);								// ODD, 4
	qword a					= si_selb(x.m_v, zero, a_mask);						// EVEN, 2
	qword b_mask			= si_fcgt(a, one);									// ODD, 4
	qword b					= si_selb(a, one, b_mask);							// EVEN, 2
#endif // #ifdef SPUHLSL_FAST_CLAMP
	return b;
}

//! Compute the sign of x.
//@param x Vector for which to compute signs.
//@return Vector containing sign of each element.
static inline const qword sign(const float4 x) __attribute__((pure, const, always_inline));
static inline const qword sign(const float4 x)
{
	const qword zero		= si_il(0x0000);									// ODD, 2
		  qword mask		= si_fcgt(x.m_v, zero);								// EVEN, 2
	const qword one			= si_ilh(0x3f80);									// ODD, 2
	const qword negone 		= si_ilh(0xbf80);									// ODD, 2
		  qword result		= si_selb(one, negone, mask);						// EVEN, 2
	return result;
}

//! Returns the square root of each component of x.
//@param x Input vector.
//@return Square root of each component of x.
static inline const qword sqrt(const float4 x) __attribute__((pure, const, always_inline));
static inline const qword sqrt(const float4 x)
{
	qword rsqrt_x 			= si_frsqest(x.m_v);								// ODD, 4
	qword result			= si_frest(rsqrt_x);								// ODD, 4
	return result;	
}

//! Returns (x >= a) ? 1 : 0.
//@param x Vector to check against a.
//@param a Vector to check x against.
//@return (x >= a) ? 1 : 0.
static inline const qword step(const float4 x, const float4 a) __attribute__((pure, const, always_inline));
static inline const qword step(const float4 x, const float4 a)
{
	qword mask_a 			= si_fcgt(x.m_v, a.m_v);							// EVEN, 2
	qword zero				= si_il(0x0000);									// ODD, 2
	qword mask_b			= si_fceq(x.m_v, a.m_v);							// EVEN, 2
	qword one				= si_ilh(0x3f80);									// ODD, 2
	qword mask				= si_or(mask_a, mask_b);							// EVEN, 2
	qword result 			= si_selb(zero, one, mask);							// EVEN, 2
	return result;
}

#endif	// #ifndef PLATFORM_SPU
#endif	// #ifndef _SPUHLSL_H_INCLUDED_

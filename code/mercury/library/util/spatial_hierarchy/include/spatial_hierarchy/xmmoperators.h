#ifndef __XMM_OPERATORS_H__
#define __XMM_OPERATORS_H__

// intrinsics operators
inline __m128 operator +(__m128 a, __m128 b)
{
	return _mm_add_ps(a,b);
}

inline __m128 operator -(__m128 a, __m128 b)
{
	return _mm_sub_ps(a,b);
}

inline __m128 operator *(__m128 a, __m128 b)
{
	return _mm_mul_ps(a,b);
}

inline __m128 operator /(__m128 a, __m128 b)
{
	return _mm_div_ps(a,b);
}

inline __m128 operator >=(__m128 a, __m128 b)
{
	return _mm_cmpge_ps(a,b);
}

inline __m128 operator <=(__m128 a, __m128 b)
{
	return _mm_cmple_ps(a,b);
}

inline __m128 operator >(__m128 a, __m128 b)
{
	return _mm_cmpgt_ps(a,b);
}

inline __m128 operator &(__m128 a, __m128 b)
{
	return _mm_and_ps(a,b);
}


#endif

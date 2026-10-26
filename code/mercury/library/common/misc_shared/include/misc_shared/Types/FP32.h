#ifndef FP32_H
#define FP32_H

#ifdef _MSC_VER
#pragma once
#endif

// 16.16 fixed point, range of 0.0 - 65536.0

class FP32
{
public:
	FP32() : v(0) {};
	FP32(S32 x) { v = x << 16; }
	FP32(F32 x) { v = (S32)(x * 65536.0f + 0.5f); }
	FP32(F64 x) { v = (S32)(x * 65536.0 + 0.5); }
	~FP32() {};

	F32 Frac() const;

	const FP32 operator + ( const S32& rhs) const;
	const FP32 operator + ( const F32& rhs) const;
	const FP32 operator + ( const F64& rhs) const;
	const FP32 operator + ( const FP32& rhs) const;

	const FP32 operator - ( const S32& rhs) const;
	const FP32 operator - ( const F32& rhs) const;
	const FP32 operator - ( const F64& rhs) const;
	const FP32 operator - ( const FP32& rhs) const;

	void operator += (const S32& rhs);
	void operator += (const F32& rhs);
	void operator += (const F64& rhs);
	void operator += (const FP32& rhs);

	void operator -= (const S32& rhs);
	void operator -= (const F32& rhs);
	void operator -= (const F64& rhs);
	void operator -= (const FP32& rhs);
	
	operator const S32 () const __restrict; // JASOND: Intel compiler requires declaration & definition to be identical
	operator const F32 () const __restrict;
	operator const F64 () const __restrict;
private:
	S32 v;
};

FORCE_INLINE F32 FP32::Frac() const
{
	return (v & 0xffff)/65536.f;
};

FORCE_INLINE const FP32 FP32::operator + (const FP32& rhs) const
{
	FP32 result;
	result.v = v + rhs.v;
	return result;
}

FORCE_INLINE const FP32 FP32::operator + (const S32& rhs) const
{
	FP32 src(rhs);
	FP32 result;
	result.v = v + src.v;
	return result;
}

FORCE_INLINE const FP32 FP32::operator + (const F32& rhs) const
{
	FP32 src(rhs);
	FP32 result;
	result.v = v + src.v;
	return result;
}

FORCE_INLINE const FP32 FP32::operator + (const F64& rhs) const
{
	FP32 src(rhs);
	FP32 result;
	result.v = v + src.v;
	return result;
}

FORCE_INLINE const FP32 FP32::operator - (const FP32& rhs) const
{
	FP32 result;
	result.v = v - rhs.v;
	return result;
}

FORCE_INLINE const FP32 FP32::operator - (const S32& rhs) const
{
	FP32 src(rhs);
	FP32 result;
	result.v = v - src.v;
	return result;
}

FORCE_INLINE const FP32 FP32::operator - (const F32& rhs) const
{
	FP32 src(rhs);
	FP32 result;
	result.v = v - src.v;
	return result;
}

FORCE_INLINE const FP32 FP32::operator - (const F64& rhs) const
{
	FP32 src(rhs);
	FP32 result;
	result.v = v - src.v;
	return result;
}

FORCE_INLINE void FP32::operator += (const S32& rhs)
{
	FP32 t(rhs);
	v += t.v;
};

FORCE_INLINE void FP32::operator += (const F32& rhs)
{
	FP32 t(rhs);
	v += t.v;
};

FORCE_INLINE void FP32::operator += (const F64& rhs)
{
	FP32 t(rhs);
	v += t.v;
};

FORCE_INLINE void FP32::operator -= (const FP32& rhs)
{
	v -= rhs.v;
};

FORCE_INLINE void FP32::operator -= (const S32& rhs)
{
	FP32 t(rhs);
	v -= t.v;
};

FORCE_INLINE void FP32::operator -= (const F32& rhs)
{
	FP32 t(rhs);
	v -= t.v;
};

FORCE_INLINE void FP32::operator -= (const F64& rhs)
{
	FP32 t(rhs);
	v -= t.v;
};

FORCE_INLINE FP32::operator const F32() const __restrict
{
	F32 a = (F32)(v >> 16);
	F32 b = (F32)(v & 0xffff)/65536.f;
	return a + b;
};

FORCE_INLINE FP32::operator const S32() const __restrict
{
	return (v >> 16);
};

FORCE_INLINE FP32::operator const F64() const __restrict
{
	F64 a = (F64)(v >> 16);
	F64 b = (F64)(v & 0xffff)/65536.;
	return a + b;
};

#endif // FP32_H

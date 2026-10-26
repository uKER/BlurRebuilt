#ifndef FLOAT16_H
#define FLOAT16_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Half
{

// 16 bit float conversion class

class F16
{
public:
	F16() {}

	// cast constructor from F32
	F16(const F32 f32In);

	// operator to cast to an F32
    operator const F32() const;

	F16& operator = (const F32 f32In);

public:
	U16	m_U16;
};

} // namespace Half

namespace FloatDecomp
{

//    IEEE 754 single-precision floating point number
//    bits are arranged as follows:
//
//        31 (msb)
//        | 
//        | 30     23
//        | |      | 
//        | |      | 22                    0 (lsb)
//        | |      | |                     |
//        X XXXXXXXX XXXXXXXXXXXXXXXXXXXXXXX
//
//        s e        m
//
//    S is the sign-bit, e is the exponent and m is the mantissa.
//
//    If e is between 1 and 254, f is a normalized number:
//
//                s    e-127
//        f = (-1)  * 2      * 1.m
//
//    If e is 0, and m is not zero, f is a denormalized number:
//
//                s    -126
//        f = (-1)  * 2      * 0.m
//
//    If e and m are both zero, f is zero:
//
//        f = 0.0
//
//    If e is 255, f is an "infinity" or "not a number" (NAN),
//    depending on whether m is zero or not.
//
//    Examples:
//
//        0 00000000 00000000000000000000000 = 0.0
//        0 01111110 00000000000000000000000 = 0.5
//        0 01111111 00000000000000000000000 = 1.0
//        0 10000000 00000000000000000000000 = 2.0
//        0 10000000 10000000000000000000000 = 3.0
//        1 10000101 11110000010000000000000 = -124.0625
//        0 11111111 00000000000000000000000 = +infinity
//        1 11111111 00000000000000000000000 = -infinity
//        0 11111111 10000000000000000000000 = NAN
//        1 11111111 11111111111111111111111 = NAN

// decompose a 32bit floating point value into its separate parts

class C_F32
{
public:
	C_F32() { m_U32 = 0; }
	C_F32( const F32 f32in ) { m_F32 = f32in; }

	U32 GetMantissa() const { return (m_U32 >>  0) & 0x7fffff; }
	U32 GetExponent() const { return (m_U32 >> 23) & 0xff; }
	U32 GetSign() const     { return (m_U32 >> 31) & 0x1; }

	void SetMantissa( const U32 mantissa )	{ m_U32 = (U32)((m_U32 & ~0x007fffff) | (mantissa << 0)); }
	void SetExponent( const U32 exponent )	{ m_U32 = (U32)((m_U32 & ~0x7f800000) | (exponent << 23)); }
	void SetSign( const U32 sign )			{ m_U32 = (U32)((m_U32 & ~0x80000000) | (sign << 31)); }

	union
	{
		F32	m_F32;
		U32 m_U32;
	};
};

//    Half float bit-layout:
//
//        15 (msb)
//        | 
//        | 14  10
//        | |   |
//        | |   | 9        0 (lsb)
//        | |   | |        |
//        X XXXXX XXXXXXXXXX
//
//        s e     m
//
//    S is the sign-bit, e is the exponent and m is the mantissa.
//
//    If e is between 1 and 30, h is a normalized number:
//
//                s    e-15
//        h = (-1)  * 2     * 1.m
//
//    If e is 0, and m is not zero, h is a denormalized number:
//
//                S    -14
//        h = (-1)  * 2     * 0.m
//
//    If e and m are both zero, h is zero:
//
//        h = 0.0
//
//    If e is 31, h is an "infinity" or "not a number" (NAN),
//    depending on whether m is zero or not.
//
//    Examples:
//
//        0 00000 0000000000 = 0.0
//        0 01110 0000000000 = 0.5
//        0 01111 0000000000 = 1.0
//        0 10000 0000000000 = 2.0
//        0 10000 1000000000 = 3.0
//        1 10101 1111000001 = -124.0625
//        0 11111 0000000000 = +infinity
//        1 11111 0000000000 = -infinity
//        0 11111 1000000000 = NAN
//        1 11111 1111111111 = NAN

// decompose a 16bit floating point value into its separate parts

class C_F16
{
public:
	C_F16() { m_U16 = 0; }
	C_F16( const U16 u16In ) { m_U16 = u16In; }

	U32 GetMantissa() const { return (m_U16 >> 0)  & 0x3ff; }
	U32 GetExponent() const { return (m_U16 >> 10) & 0x1f; }
	U32 GetSign() const     { return (m_U16 >> 15) & 0x1; }

	void SetMantissa( const U32 mantissa )	{ m_U16 = (U16)((m_U16 & ~0x03ff) | (mantissa << 0)); }
	void SetExponent( const U32 exponent )	{ m_U16 = (U16)((m_U16 & ~0x7c00) | (exponent << 10)); }
	void SetSign( const U32 sign )			{ m_U16 = (U16)((m_U16 & ~0x8000) | (sign << 15)); }

	U16 m_U16;
};

} // namespace FloatDecomp

#endif

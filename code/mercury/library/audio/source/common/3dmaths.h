#ifndef _3D_MATHS_H_
#define _3D_MATHS_H_

#pragma once

#include "types.h"
#ifdef PLATFORM_PS3
#include <fastmath.h>
#else
#include <math.h>
#endif


#if !defined( __INTEL_COMPILER ) && ( defined( COMPILER_VC ) || defined( _MSC_VER ) )

extern "C" void _ReadWriteBarrier();
extern "C" void _ReadBarrier();
extern "C" void _WriteBarrier();

#pragma intrinsic( _ReadWriteBarrier )
#pragma intrinsic( _ReadBarrier )
#pragma intrinsic( _WriteBarrier )

#else

#define _ReadWriteBarrier()
#define _ReadBarrier()
#define _WriteBarrier()

#endif

namespace Audio
{
namespace Maths
{

#if defined( PLATFORM_PS3 )
#define _AUDIO_MATHS_PPU_
#endif

#if defined( PLATFORM_SPU )
#define _AUDIO_MATHS_SPU_
#endif


#if defined( PLATFORM_PC )
#define _AUDIO_MATHS_SSE_
#endif

#if !defined( AUDIO_PASSINREG )
#if defined( _AUDIO_MATHS_360_ )
#define AUDIO_PASSINREG __declspec( passinreg )
#else
#define AUDIO_PASSINREG
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

#if defined( _AUDIO_MATHS_360_ )

// VMX mask insert flags
enum MASK_INSERT
{
    MASK_INSERT_X    = 0x8,
    MASK_INSERT_Y    = 0x4,
    MASK_INSERT_Z    = 0x2,
    MASK_INSERT_W    = 0x1,
    
    MASK_INSERT_XY   = MASK_INSERT_X | MASK_INSERT_Y,
    MASK_INSERT_XZ   = MASK_INSERT_X | MASK_INSERT_Z,
    MASK_INSERT_XW   = MASK_INSERT_X | MASK_INSERT_W,
    MASK_INSERT_YZ   = MASK_INSERT_Y | MASK_INSERT_Z,
    MASK_INSERT_YW   = MASK_INSERT_Y | MASK_INSERT_W,
    MASK_INSERT_ZW   = MASK_INSERT_Z | MASK_INSERT_W,
    
    MASK_INSERT_XYZ  = MASK_INSERT_X | MASK_INSERT_Y | MASK_INSERT_Z,
    MASK_INSERT_XYW  = MASK_INSERT_X | MASK_INSERT_Y | MASK_INSERT_W,
    MASK_INSERT_XZW  = MASK_INSERT_X | MASK_INSERT_Z | MASK_INSERT_W,
    MASK_INSERT_YZW  = MASK_INSERT_Y | MASK_INSERT_Z | MASK_INSERT_W,
    
    MASK_INSERT_XYZW = MASK_INSERT_X | MASK_INSERT_Y | MASK_INSERT_Z | MASK_INSERT_W
};

// VMX rotate left word
enum ROTATE_LEFT
{
    ROTATE_LEFT_NONE   = 0,
    
    ROTATE_LEFT_X_TO_W = 1,
    ROTATE_LEFT_X_TO_Z = 2,
    ROTATE_LEFT_X_TO_Y = 3,
    
    ROTATE_LEFT_Y_TO_X = 1,
    ROTATE_LEFT_Y_TO_W = 2,
    ROTATE_LEFT_Y_TO_Z = 3,
    
    ROTATE_LEFT_Z_TO_Y = 1,
    ROTATE_LEFT_Z_TO_X = 2,
    ROTATE_LEFT_Z_TO_W = 3,
    
    ROTATE_LEFT_W_TO_Z = 1,
    ROTATE_LEFT_W_TO_Y = 2,
    ROTATE_LEFT_W_TO_X = 3
};

// VMX Condition Register Bit Masks & Shifts
#define CR6_ALL_TRUE_BIT    7
#define CR6_ALL_FALSE_BIT   5
#define CR6_ALL_TRUE        ( 1 << CR6_ALL_TRUE_BIT )
#define CR6_ALL_FALSE       ( 1 << CR6_ALL_FALSE_BIT )

#endif

enum VectorElementIndex
{
    VX = 0,
    VY = 1,
    VZ = 2,
    VW = 3
};

inline const F32 IntBitsToFloat( const U32 bits )
{
	typedef union { U32 i; F32 f; } I2F;
	I2F convert;
	convert.i = bits;
	return convert.f;
}

inline const U32 IntFloatToBits( const F32 fValue )
{
	typedef union { U32 i; F32 f; } F2I;
	F2I convert;
	convert.f = fValue;
	return convert.i;
}

extern const F32 fPi;           // = 3.14159274f;
extern const F32 fHalfPi;       // = 1.57079633f;
extern const F32 fReciprocalPi; // = 1.0f / 3.14159274f;
extern const F32 f2Pi;          // = 6.28318548f;
extern const F32 fMin;          // = 1.175494351e-38f;
extern const F32 fMax;          // = 3.402823466e+38f;
extern const F32 fEpsilon;      // = 0.0001f;
extern const F32 fMinus3dB;     // = 0.707106769f;
extern const F32 fMinus48dB;    // = 0.00390625f;
extern const F32 fLog10;        // = 2.302585093f;
extern const F32 fLog2;         // = 0.69314718056f;
extern const F32 fInvLog2;      // = 1.442695040889f;
extern const F32 fInv2pow15;    // = 3.0517578125e-5f;
extern const F32 fRoot2;        // = 1.41421356237f;
extern const F32 fInfinity;     // = 0x7f800000;
extern const F32 fMinusInfinity;// = 0xff800000;
extern const F32 fInvalid;      // = 0xffffffff;


inline const bool IsValid( F32 fValue )
{
	U32 uBits = IntFloatToBits( fValue ) & 0x7fffffff;
	return uBits > IntFloatToBits( fInfinity );
}

inline const bool IsInfinity( F32 fValue )
{
	U32 uBits = IntFloatToBits( fValue ) & 0x7fffffff;
	return uBits == IntFloatToBits( fInfinity );
}

inline const F32 MakeMinusOneToOne( F32 fValue )
{
#if 0
    if (fValue < -1.f)
        fValue -= 2.f * (F32)(((S32)fValue - 1) / 2);
    else if (fValue > 1.f)
        fValue -= 2.f * (F32)(((S32)fValue + 1) / 2);
#else
	if (fValue < -1.f)
		fValue = fmod(fValue - 1.f, 2.f) + 1.f;
	else if (fValue > 1.f)
		fValue = fmod(fValue + 1.f, 2.f) - 1.f;
#endif
	return fValue;
}

//	Uses the absolute value of level and rounds down to nearest 6dB, faster than a float compare
inline bool IsVolumeLessThan( F32 fLevel, S32 dB )
{
	return ( (S32)( ( *(U32*)&fLevel & ~0x80000000 ) >> 23 ) - 127 ) < dB / 6;
}

#if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )

extern const vector float g_VectorUnitX;
extern const vector float g_VectorUnitY;
extern const vector float g_VectorUnitZ;
extern const vector float g_VectorUnitW;

extern const vector float g_VectorZero;
extern const vector float g_VectorHalf;
extern const vector float g_VectorOne;
extern const vector float g_VectorMinusOne;

extern const vector float g_VectorTwo;
extern const vector float g_VectorThree;
extern const vector float g_VectorFour;
extern const vector float g_VectorFive;
extern const vector float g_VectorEight;
extern const vector float g_VectorSixteen;
extern const vector float g_VectorThirtyTwo;

extern const vector float g_VectorFloat_00_01_02_03;
extern const vector float g_VectorFloat_04_05_06_07;
extern const vector float g_VectorFloat_08_09_0A_0B;
extern const vector float g_VectorFloat_0C_0D_0E_0F;
extern const vector float g_VectorFloat_10_11_12_13;
extern const vector float g_VectorFloat_14_15_16_17;
extern const vector float g_VectorFloat_18_19_1A_1B;
extern const vector float g_VectorFloat_1C_1D_1E_1F;

extern const vector float g_VectorEpsilon;

extern const vector unsigned char g_VectorPermuteYZX;

extern const vector unsigned char g_VectorPermuteZXY;

#elif defined( _AUDIO_MATHS_360_ )

extern const __vector4 g_VectorAbsMask;
extern const __vector4 g_VectorSignMask;

extern const __vector4 g_VectorSignMaskXY;
extern const __vector4 g_VectorSignMaskXZ;
extern const __vector4 g_VectorSignMaskYZ;

extern const __vector4 g_VectorUnitX;
extern const __vector4 g_VectorUnitY;
extern const __vector4 g_VectorUnitZ;
extern const __vector4 g_VectorUnitW;

extern const __vector4 g_VectorZero;
extern const __vector4 g_VectorHalf;
extern const __vector4 g_VectorOne;
extern const __vector4 g_VectorMinusOne;

extern const __vector4 g_VectorTwo;
extern const __vector4 g_VectorThree;
extern const __vector4 g_VectorFour;
extern const __vector4 g_VectorFive;
extern const __vector4 g_VectorEight;
extern const __vector4 g_VectorSixteen;
extern const __vector4 g_VectorThirtyTwo;

extern const __vector4 g_VectorFloat_00_01_02_03;
extern const __vector4 g_VectorFloat_04_05_06_07;
extern const __vector4 g_VectorFloat_08_09_0A_0B;
extern const __vector4 g_VectorFloat_0C_0D_0E_0F;
extern const __vector4 g_VectorFloat_10_11_12_13;
extern const __vector4 g_VectorFloat_14_15_16_17;
extern const __vector4 g_VectorFloat_18_19_1A_1B;
extern const __vector4 g_VectorFloat_1C_1D_1E_1F;

extern const __vector4 g_VectorEpsilon;

extern const __vector4 g_VectorFltMin;
extern const __vector4 g_VectorFltMax;

#else

extern const F32 g_VectorUnitX[4];
extern const F32 g_VectorUnitY[4];
extern const F32 g_VectorUnitZ[4];
extern const F32 g_VectorUnitW[4];

extern const F32 g_VectorZero[4];
extern const F32 g_VectorHalf[4];
extern const F32 g_VectorOne[4];
extern const F32 g_VectorMinusOne[4];

extern const F32 g_VectorTwo[4];
extern const F32 g_VectorThree[4];
extern const F32 g_VectorFour[4];
extern const F32 g_VectorFive[4];
extern const F32 g_VectorEight[4];
extern const F32 g_VectorSixteen[4];
extern const F32 g_VectorThirtyTwo[4];

extern const F32 g_VectorFloat_00_01_02_03[4];
extern const F32 g_VectorFloat_04_05_06_07[4];
extern const F32 g_VectorFloat_08_09_0A_0B[4];
extern const F32 g_VectorFloat_0C_0D_0E_0F[4];
extern const F32 g_VectorFloat_10_11_12_13[4];
extern const F32 g_VectorFloat_14_15_16_17[4];
extern const F32 g_VectorFloat_18_19_1A_1B[4];
extern const F32 g_VectorFloat_1C_1D_1E_1F[4];

extern const F32 g_VectorEpsilon[4];

extern const F32 g_VectorFltMin[4];
extern const F32 g_VectorFltMax[4];

#endif


#if defined( _AUDIO_MATHS_SPU_ )

extern const vector unsigned int g_VectorSignMask;
extern const vector float        g_VectorAbsMask;

extern const vector unsigned int g_VectorMaskX;
extern const vector unsigned int g_VectorMaskY;
extern const vector unsigned int g_VectorMaskZ;
extern const vector unsigned int g_VectorMaskW;

extern const vector unsigned int g_VectorMaskXYZ;

extern const vector unsigned char g_VectorMergeXY;

extern const vector unsigned char g_VectorUnpackHi16;
extern const vector unsigned char g_VectorUnpackLo16;

extern const vector unsigned char g_VectorLoadUnaligned;

#elif defined( _AUDIO_MATHS_PPU_ )

extern const vector bool int g_VectorSignMask;

extern const vector bool int g_VectorMaskX;
extern const vector bool int g_VectorMaskY;
extern const vector bool int g_VectorMaskZ;
extern const vector bool int g_VectorMaskW;

extern const vector bool int g_VectorMaskXYZ;

#endif

////////////////////////////////////////////////////////////////////////////////
//	helper functions
////////////////////////////////////////////////////////////////////////////////
template<class T>
inline const T& Max(const T& a, const T& b) { return a > b ? a : b; }

template<class T>
inline const T& Min(const T& a, const T& b) { return a < b ? a : b; }

template<class T>
inline const T& Clamp(const T& fVal, const T& fMin, const T& fMax)
{
	return fVal >= fMax ? fMax : fVal >= fMin ? fVal : fMin;
}



#if defined( PLATFORM_PS3 )

inline const F32 Abs( const F32 fValue )
{
    return __fabsf( fValue );
}

inline const F64 Floor( const F64 dValue )
{
    return __fcfid( __fctidz( dValue ) );
}

inline const F32 Floor( const F32 fValue )
{
    return (F32) __fcfid( __fctidz( (F64) fValue ) );
}

inline const F32 Select( const F32 fComp, const F32 fPos, const F32 fNeg )
{
    return __fsels( fComp, fPos, fNeg );
}

inline const F32 RcpEst( const F32 fValue )
{
    return __fres( fValue );
}

inline const F32 Rcp( const F32 fValue )
{
    const float one( 1.0f );
    const float B( fValue );
    const float y0( __fres( fValue ) );                   // Estimate: 8 bit accuracy
    const float y1( y0 + ( y0 * ( one - ( B * y0 ) ) ) ); // NR iteration 1: 16 bit accuracy
    const float y2( y1 + ( y1 * ( one - ( B * y1 ) ) ) ); // NR iteration 2: 24 bit accuracy
    return __fsels( __fabsf( y2 ), y2, 0.0f ); // Returns 0.0 if y2 is a NAN
}

inline const F32 RcpSqrt( const F32 fValue )
{
    const float half( 0.5f );
    const float one( 1.0f );
    const float B( fValue );
    const float y0( (F32) __frsqrte( B ) );                          // Estimate: 8 bit accuracy
    const float y1( y0 + ( half * y0 * ( one - ( B * y0 * y0 ) ) ) ); // NR iteration 1: 16 bit accuracy
    const float y2( y1 + ( half * y1 * ( one - ( B * y1 * y1 ) ) ) ); // NR iteration 2: 24 bit accuracy
    return __fsels( y2, y2, 0.0f ); // Returns 0.0 if y2 is a NAN
}

inline const F32 Sqrt( const F32 fValue )
{
    return __fsqrts( fValue );
}

#else

inline const F32 Abs( const F32 fValue )
{
    return fabs( fValue );
}

inline const F64 Floor( const F64 dValue )
{
    return floor( dValue );
}

inline const F32 Floor( const F32 fValue )
{
    return floor( fValue );
}

inline const F32 Select( const F32 fComp, const F32 fPos, const F32 fNeg )
{
    return fComp >= 0.0f ? fPos : fNeg;
}

inline const F32 RcpEst( const F32 fValue )
{
    return 1.0f / fValue;
}

inline const F32 Rcp( const F32 fValue )
{
    return 1.0f / fValue;
}

inline const F32 RcpSqrt( const F32 fValue )
{
    return 1.0f / sqrtf( fValue );
}

inline const F32 Sqrt( const F32 fValue )
{
    return sqrtf( fValue );
}

#endif


inline F32 SlowSqrt( F32 fValue ) { return sqrtf( fValue ); }
inline F32 Sin( F32 fValue ) { return sinf( fValue ); }
inline F32 Cos( F32 fValue ) { return cosf( fValue ); }
inline F32 Tan( F32 fValue ) { return tanf( fValue ); }
inline F32 ArcSin( F32 fValue ) { return asinf( fValue ); }
inline F32 ArcCos( F32 fValue ) { return acosf( fValue ); }
inline F32 ArcTan( F32 fValue ) { return atanf( fValue ); }
inline F32 ArcTan2( F32 fValX, F32 fValY ) { return atan2f( fValX, fValY ); }
inline F32 Exp( F32 fValue ) { return expf( fValue ); }
inline F32 Power( F32 fValue, F32 fPower ) { return powf( fValue, fPower ); }
inline F32 Log( F32 fValue ) { return logf( fValue ); }

#ifdef PLATFORM_PC
inline const F32 dBToFloat( const F32& fVal )
{
	if( fVal == Maths::fMinusInfinity )
		return 0.f;
	else
		return Exp( Maths::fLog10 * fVal * 0.05f );
}
#else
inline const F32 dBToFloat( const F32& fVal ) { return Exp( Maths::fLog10 * fVal * 0.05f ); }
#endif
inline const F32 FloatTodB( const F32& fVal ) { return 20.f * log10f( fVal ); }

inline const F32 OctToHz( const F32& fVal ) { return Power( 2.f, fVal ); }
inline const F32 HzToOct( const F32& fVal ) { return Log( fVal ) * fInvLog2; }

inline const F32 Round( const F32& fVal ) { return Floor( fVal + 0.5f ); }


#if defined (PLATFORM_SPU)
// TODO: this properly
inline bool IsFinite(F32 fValue) { return true; }
#else
inline bool IsFinite(F32 fValue) { return (_finite(fValue) != 0) ? true : false; }
#endif

#if defined( _AUDIO_MATHS_SPU_ )

inline const vector signed short vector_load_unaligned( S16 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector signed short vector_load_unaligned( S16 const * const __restrict pSrc )
{
    vector signed short const * const __restrict pVecSrc = (vector signed short*)pSrc;
    const vector signed short a = pVecSrc[0];
    const vector signed short b = pVecSrc[1];
    #if 0
    const vector unsigned char pattern = (vector unsigned char)spu_add( (vector unsigned short)g_VectorLoadUnaligned,
                                                                        (vector unsigned short)spu_splats( (unsigned char)( (U32)pSrc & 0xF ) ) );
    return spu_shuffle( a, b, pattern );
    #else
    const unsigned int shift = (unsigned int)pSrc & 0xF;
    return spu_or( spu_slqwbyte( a, shift ),
                   spu_rlmaskqwbyte( b, shift-16 ) );
    #endif
}

//inline const vector float make_vector3( const float x, const float y, const float z ) __attribute__(( const, always_inline ));
inline const vector float make_vector3( const float x, const float y, const float z ) __attribute__(( pure, always_inline ));
inline const vector float make_vector3( const float x, const float y, const float z )
{
    //return spu_insert( z, spu_insert( y, spu_insert( x, g_VectorZero, 0 ), 1 ), 2 );
    return spu_insert( z, spu_insert( y, spu_promote( x, 0 ), 1 ), 2 ); // W is undefined!
}

inline const vector float make_vector4( const float x, const float y, const float z, const float w ) __attribute__(( const, always_inline ));
inline const vector float make_vector4( const float x, const float y, const float z, const float w )
{
    return spu_insert( w, spu_insert( z, spu_insert( y, spu_promote( x, VX ), VY ), VZ ), VW );
}

inline const vector float make_vector4( const vector float sx, const vector float sy, const vector float sz, const vector float sw ) __attribute__(( const, always_inline ));
inline const vector float make_vector4( const vector float sx, const vector float sy, const vector float sz, const vector float sw )
{
    const float x = spu_extract( sx, VX );
    const float y = spu_extract( sx, VY );
    const float z = spu_extract( sx, VZ );
    const float w = spu_extract( sx, VW );
    return spu_insert( w, spu_insert( z, spu_insert( y, spu_promote( x, VX ), VY ), VZ ), VW );
}

inline const float scalar_to_float( const vector float s ) __attribute__(( const, always_inline ));
inline const float scalar_to_float( const vector float s )
{
    return spu_extract( s, VX );
}

inline const vector float vector_insert_x( const vector float v, const float x ) __attribute__(( const, always_inline ));
inline const vector float vector_insert_x( const vector float v, const float x )
{                                                                
    return spu_insert( x, v, VX );                                
}                                                                
                                                                 
inline const vector float vector_insert_y( const vector float v, const float y ) __attribute__(( const, always_inline ));
inline const vector float vector_insert_y( const vector float v, const float y )
{                                                                
    return spu_insert( y, v, VY );                                
}                                                                
                                                                 
inline const vector float vector_insert_z( const vector float v, const float z ) __attribute__(( const, always_inline ));
inline const vector float vector_insert_z( const vector float v, const float z )
{                                                                
    return spu_insert( z, v, VZ );                                
}                                                                
                                                                 
inline const vector float vector_insert_w( const vector float v, const float w ) __attribute__(( const, always_inline ));
inline const vector float vector_insert_w( const vector float v, const float w )
{
    return spu_insert( w, v, VW );
}

inline const vector float vector_insert_scalar_x( const vector float v, const vector float s ) __attribute__(( const, always_inline ));
inline const vector float vector_insert_scalar_x( const vector float v, const vector float s )
{                                                                
    return spu_insert( spu_extract( s, VX ), v, VX );                                
}                                                                
                                                                 
inline const vector float vector_insert_scalar_y( const vector float v, const vector float s ) __attribute__(( const, always_inline ));
inline const vector float vector_insert_scalar_y( const vector float v, const vector float s )
{                                                                
    return spu_insert( spu_extract( s, VY ), v, VY );                                
}                                                                
                                                                 
inline const vector float vector_insert_scalar_z( const vector float v, const vector float s ) __attribute__(( const, always_inline ));
inline const vector float vector_insert_scalar_z( const vector float v, const vector float s )
{                                                                
    return spu_insert( spu_extract( s, VZ ), v, VZ );                                
}                                                                
                                                                 
inline const vector float vector_insert_scalar_w( const vector float v, const vector float s ) __attribute__(( const, always_inline ));
inline const vector float vector_insert_scalar_w( const vector float v, const vector float s )
{
    return spu_insert( spu_extract( s, VW ), v, VW );
}

inline const float vector_extract_x( const vector float v ) __attribute__(( const, always_inline ));
inline const float vector_extract_x( const vector float v )
{
    return spu_extract( v, VX );
}

inline const float vector_extract_y( const vector float v ) __attribute__(( const, always_inline ));
inline const float vector_extract_y( const vector float v )
{
    return spu_extract( v, VY );
}

inline const float vector_extract_z( const vector float v ) __attribute__(( const, always_inline ));
inline const float vector_extract_z( const vector float v )
{
    return spu_extract( v, VZ );
}

inline const float vector_extract_w( const vector float v ) __attribute__(( const, always_inline ));
inline const float vector_extract_w( const vector float v )
{
    return spu_extract( v, VW );
}

inline const unsigned int vector_extract_x( const vector unsigned int v ) __attribute__(( const, always_inline ));
inline const unsigned int vector_extract_x( const vector unsigned int v )
{
    return spu_extract( v, VX );
}

inline const unsigned int vector_extract_y( const vector unsigned int v ) __attribute__(( const, always_inline ));
inline const unsigned int vector_extract_y( const vector unsigned int v )
{
    return spu_extract( v, VY );
}

inline const unsigned int vector_extract_z( const vector unsigned int v ) __attribute__(( const, always_inline ));
inline const unsigned int vector_extract_z( const vector unsigned int v )
{
    return spu_extract( v, VZ );
}

inline const unsigned int vector_extract_w( const vector unsigned int v ) __attribute__(( const, always_inline ));
inline const unsigned int vector_extract_w( const vector unsigned int v )
{
    return spu_extract( v, VW );
}

inline const vector float vector_splat_x( const vector float v ) __attribute__(( const, always_inline ));
inline const vector float vector_splat_x( const vector float v )
{
    return spu_splats( spu_extract( v, VX ) );
}

inline const vector float vector_splat_y( const vector float v ) __attribute__(( const, always_inline ));
inline const vector float vector_splat_y( const vector float v )
{
    return spu_splats( spu_extract( v, VY ) );
}

inline const vector float vector_splat_z( const vector float v ) __attribute__(( const, always_inline ));
inline const vector float vector_splat_z( const vector float v )
{
    return spu_splats( spu_extract( v, VZ ) );
}

inline const vector float vector_splat_w( const vector float v ) __attribute__(( const, always_inline ));
inline const vector float vector_splat_w( const vector float v )
{
    return spu_splats( spu_extract( v, VW ) );
}

inline const vector signed int vector_unpack_hi( const vector signed short v ) __attribute__(( pure, always_inline ));
inline const vector signed int vector_unpack_hi( const vector signed short v )
{
    //return spu_extend( spu_shuffle( v, v, (vec_uchar16)( 0, 0, 0, 1, 0, 0, 2, 3, 0, 0, 4, 5, 0, 0, 6, 7 ) ) );
    return spu_extend( spu_shuffle( v, v, g_VectorUnpackHi16 ) );
}

inline const vector signed int vector_unpack_lo( const vector signed short v ) __attribute__(( pure, always_inline ));
inline const vector signed int vector_unpack_lo( const vector signed short v )
{
    //return spu_extend( spu_shuffle( v, v, (vec_uchar16)( 0, 0, 8, 9, 0, 0, 10, 11, 0, 0,12,13, 0, 0, 14, 15 ) ) );
    return spu_extend( spu_shuffle( v, v, g_VectorUnpackHi16 ) );
}

//inline const vector float vector_abs( const vector float v ) __attribute__(( const, always_inline ));
inline const vector float vector_abs( const vector float v ) __attribute__(( pure, always_inline ));
inline const vector float vector_abs( const vector float v )
{
    return spu_and( v, g_VectorAbsMask );
}

inline const vector float vector3_sum( const vector float v ) __attribute__(( const, always_inline ));
inline const vector float vector3_sum( const vector float v )
{
    return spu_add( spu_splats( spu_extract( v, VX ) ),
           spu_add( spu_splats( spu_extract( v, VY ) ),
                    spu_splats( spu_extract( v, VZ ) ) ) );
}

inline const vector float vector3_dot_product( const vector float v1, const vector float v2 ) __attribute__(( const, always_inline ));
inline const vector float vector3_dot_product( const vector float v1, const vector float v2 )
{
    const vector float v1_x_v2( spu_mul( v1, v2 ) );
    return spu_add( spu_splats( spu_extract( v1_x_v2, VX ) ),
           spu_add( spu_splats( spu_extract( v1_x_v2, VY ) ),
                    spu_splats( spu_extract( v1_x_v2, VZ ) ) ) );
}

inline const vector float vector4_dot_product( const vector float v1, const vector float v2 ) __attribute__(( const, always_inline ));
inline const vector float vector4_dot_product( const vector float v1, const vector float v2 )
{
    const vector float v1_x_v2( spu_mul( v1, v2 ) );
    return spu_add( spu_add( spu_splats( spu_extract( v1_x_v2, VX ) ),
                             spu_splats( spu_extract( v1_x_v2, VY ) ) ),
                    spu_add( spu_splats( spu_extract( v1_x_v2, VZ ) ),
                             spu_splats( spu_extract( v1_x_v2, VW ) ) ) );
}

//inline const vector float vector3_cross_product( const vector float v1, const vector float v2 ) __attribute__(( const, always_inline ));
inline const vector float vector3_cross_product( const vector float v1, const vector float v2 ) __attribute__(( pure, always_inline ));
inline const vector float vector3_cross_product( const vector float v1, const vector float v2 )
{
        const vector float v1_yzx( spu_shuffle( v1, v1, g_VectorPermuteYZX ) );
        const vector float v1_zxy( spu_shuffle( v1, v1, g_VectorPermuteZXY ) );
        const vector float v2_yzx( spu_shuffle( v2, v2, g_VectorPermuteYZX ) );
        const vector float v2_zxy( spu_shuffle( v2, v2, g_VectorPermuteZXY ) );
        return spu_msub( v1_yzx, v2_zxy, spu_mul( v2_yzx, v1_zxy ) );
}

//inline const vector float vector_reciprocal_sqrt( const vector float B ) __attribute__(( const, always_inline ));
inline const vector float vector_reciprocal_sqrt( const vector float B ) __attribute__(( pure, always_inline ));
inline const vector float vector_reciprocal_sqrt( const vector float B )
{
    const vector float        y0( spu_rsqrte( B ) ); // A / sqrt(B), A == 1.0f
    const vector float        zero( g_VectorZero );
    const vector float        half( g_VectorHalf );
    const vector float        one( g_VectorOne );
    const vector unsigned int B_GT_zero( spu_cmpgt( B, zero ) );
    const vector float        t0( spu_mul( y0, y0 ) );
    const vector float        t1( spu_mul( y0, half ) );
    const vector float        t2( spu_nmsub( B, t0, one ) );
    const vector float        y1( spu_madd( t2, t1, y0 ) );
    const vector float        rcp_sqrt_B( spu_sel( zero, y1, B_GT_zero ) );
    return rcp_sqrt_B;
}

inline const vector unsigned int vector_not( const vector unsigned int v ) __attribute__(( const, always_inline ));
inline const vector unsigned int vector_not( const vector unsigned int v )
{
    return spu_nor( v, v );
}

inline const vector unsigned int vector_cmpgt( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector unsigned int vector_cmpgt( const vector float a, const vector float b )
{
    const vector unsigned int a_GT_b = spu_cmpgt( a, b );
    return a_GT_b;
}

inline const vector unsigned int vector_cmpge( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector unsigned int vector_cmpge( const vector float a, const vector float b )
{
    const vector unsigned int a_LT_b = spu_cmpgt( b, a );
    const vector unsigned int a_GE_b = spu_nor( a_LT_b, a_LT_b );
    return a_GE_b;
}

inline const vector unsigned int vector_cmplt( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector unsigned int vector_cmplt( const vector float a, const vector float b )
{
    const vector unsigned int a_LT_b = spu_cmpgt( b, a );
    return a_LT_b;
}

inline const vector unsigned int vector_cmple( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector unsigned int vector_cmple( const vector float a, const vector float b )
{
    const vector unsigned int a_GT_b = spu_cmpgt( a, b );
    const vector unsigned int a_LE_b = spu_nor( a_GT_b, a_GT_b );
    return a_LE_b;
}

inline const vector float vector_min( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector float vector_min( const vector float a, const vector float b )
{
    return spu_sel( a, b, spu_cmpgt( a, b ) );
}

inline const vector float vector_max( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector float vector_max( const vector float a, const vector float b )
{
    return spu_sel( b, a, spu_cmpgt( a, b ) );
}

inline const vector float vector_rfiz( const vector float v ) __attribute__(( const, always_inline ));
inline const vector float vector_rfiz( const vector float v )
{
    return spu_convtf( spu_convts( v, 0 ), 0 );
}

inline const vector float vector_rfin( const vector float v ) __attribute__(( const, always_inline ));
inline const vector float vector_rfin( const vector float v )
{
    return spu_sel( spu_convtf( spu_convts( v + spu_splats( 0.5f ), 0 ), 0 ),
                    spu_convtf( spu_convts( v - spu_splats( 0.5f ), 0 ), 0 ),
                    spu_cmpgt( spu_splats( 0.0f ), v ) );
}

#elif defined( _AUDIO_MATHS_PPU_ )

inline const vector float vector_load_unaligned( F32 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector float vector_load_unaligned( F32 const * const __restrict pSrc )
{
    //lvx vHI,rA,rB ;# load the MSQ
    //lvsl vP,rA,rB ;# set the permute vector
    //addi rB,rB,16 ;# address of LSQ
    //lvx vLO,rA,rB ;# load LSQ component
    //vperm vT,vHI,vLO,vP ;# align the data
    
    const vector float vHI = vec_lvx( 0x00, pSrc ); // load the MSQ
    const vector float vLO = vec_lvx( 0x10, pSrc ); // load LSQ component
    const vector unsigned char vP = vec_lvsl( 0x00, pSrc ); // set the permute vector
    return vec_perm( vHI, vLO, vP ); // align the data
}

inline const vector signed short vector_load_unaligned( S16 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector signed short vector_load_unaligned( S16 const * const __restrict pSrc )
{
    //lvx vHI,rA,rB ;# load the MSQ
    //lvsl vP,rA,rB ;# set the permute vector
    //addi rB,rB,16 ;# address of LSQ
    //lvx vLO,rA,rB ;# load LSQ component
    //vperm vT,vHI,vLO,vP ;# align the data
    
    const vector signed short vHI = vec_lvx( 0x00, pSrc ); // load the MSQ
    const vector signed short vLO = vec_lvx( 0x10, pSrc ); // load LSQ component
    const vector unsigned char vP = vec_lvsl( 0x00, pSrc ); // set the permute vector
    return vec_perm( vHI, vLO, vP ); // align the data
}

inline const vector float vector_load_x( F32 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector float vector_load_x( F32 const * const __restrict pSrc )
{
    const vector float v = vec_lvx( 0x0, pSrc ); // load the MSQ
    const vector unsigned char p = vec_lvsl( 0x0, pSrc ); // set the permute vector
    return vec_perm( v, v, p ); // align the data
}

inline const vector unsigned int vector_load_x( U32 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector unsigned int vector_load_x( U32 const * const __restrict pSrc )
{
    const vector unsigned int v = vec_lvx( 0x0, pSrc ); // load the MSQ
    const vector unsigned char p = vec_lvsl( 0x0, pSrc ); // set the permute vector
    return vec_perm( v, v, p ); // align the data
}

inline const vector signed int vector_load_x( S32 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector signed int vector_load_x( S32 const * const __restrict pSrc )
{
    const vector signed int v = vec_lvx( 0x0, pSrc ); // load the MSQ
    const vector unsigned char p = vec_lvsl( 0x0, pSrc ); // set the permute vector
    return vec_perm( v, v, p ); // align the data
}

//inline const vector float make_vector3( const float x, const float y, const float z ) __attribute__(( const, always_inline ));
inline const vector float make_vector3( const float& x, const float& y, const float& z ) __attribute__(( pure, always_inline ));
inline const vector float make_vector3( const float& x, const float& y, const float& z )
{
    const vector float vx = vector_load_x( &x );
    const vector float vy = vector_load_x( &y );
    const vector float vz = vector_load_x( &z );
    const vector float vw = g_VectorZero;
    return vec_mergeh( vec_mergeh( vx, vz ),
                       vec_mergeh( vy, vw ) );
}

inline const vector float make_vector4( const float& x, const float& y, const float& z, const float& w ) __attribute__(( const, always_inline ));
inline const vector float make_vector4( const float& x, const float& y, const float& z, const float& w )
{
    const vector float vx = vector_load_x( &x );
    const vector float vy = vector_load_x( &y );
    const vector float vz = vector_load_x( &z );
    const vector float vw = vector_load_x( &w );
    return vec_mergeh( vec_mergeh( vx, vz ),
                       vec_mergeh( vy, vw ) );
}

inline const vector float make_vector4( const vector float sx, const vector float sy, const vector float sz, const vector float sw ) __attribute__(( const, always_inline ));
inline const vector float make_vector4( const vector float sx, const vector float sy, const vector float sz, const vector float sw )
{
    return vec_mergeh( vec_mergeh( sx, sz ),
                       vec_mergeh( sy, sw ) );
}

inline const vector float make_scalar( const float& f ) __attribute__(( const, always_inline ));
inline const vector float make_scalar( const float& f )
{
    return vec_splat( vector_load_x( &f ), VX );
}

inline const vector unsigned int make_scalar( const U32& i ) __attribute__(( const, always_inline ));
inline const vector unsigned int make_scalar( const U32& i )
{
    return vec_splat( vector_load_x( &i ), VX );
}

inline const vector signed int make_scalar( const S32& i ) __attribute__(( const, always_inline ));
inline const vector signed int make_scalar( const S32& i )
{
    return vec_splat( vector_load_x( &i ), VX );
}

inline const float scalar_to_float( const vector float scalar ) __attribute__(( const, always_inline ));
inline const float scalar_to_float( const vector float scalar ) // NOTE: scalar MUST be same float value splatted into all elements!
{
    float result;
    vec_ste( scalar, 0, &result );
    return result; // Warning: LHS!
}

inline const vector float vector_splat_x( const vector float v ) __attribute__(( const, always_inline ));
inline const vector float vector_splat_x( const vector float v )
{
    return vec_splat( v, 0 );
}

inline const vector float vector_splat_y( const vector float v ) __attribute__(( const, always_inline ));
inline const vector float vector_splat_y( const vector float v )
{
    return vec_splat( v, 1 );
}

inline const vector float vector_splat_z( const vector float v ) __attribute__(( const, always_inline ));
inline const vector float vector_splat_z( const vector float v )
{
    return vec_splat( v, 2 );
}

inline const vector float vector_splat_w( const vector float v ) __attribute__(( const, always_inline ));
inline const vector float vector_splat_w( const vector float v )
{
    return vec_splat( v, 3 );
}

inline const vector float vector_insert_x( const vector float v, const float& x ) __attribute__(( pure, always_inline ));
inline const vector float vector_insert_x( const vector float v, const float& x )
{
    return vec_sel( v, make_scalar( x ), g_VectorMaskX );
}

inline const vector float vector_insert_y( const vector float v, const float& y ) __attribute__(( pure, always_inline ));
inline const vector float vector_insert_y( const vector float v, const float& y )
{
    return vec_sel( v, make_scalar( y ), g_VectorMaskY );
}

inline const vector float vector_insert_z( const vector float v, const float& z ) __attribute__(( pure, always_inline ));
inline const vector float vector_insert_z( const vector float v, const float& z )
{
    return vec_sel( v, make_scalar( z ), g_VectorMaskZ );
}

inline const vector float vector_insert_w( const vector float v, const float& w ) __attribute__(( pure, always_inline ));
inline const vector float vector_insert_w( const vector float v, const float& w )
{
    return vec_sel( v, make_scalar( w ), g_VectorMaskW );
}

inline const vector float vector_insert_scalar_x( const vector float v, const vector float s ) __attribute__(( pure, always_inline ));
inline const vector float vector_insert_scalar_x( const vector float v, const vector float s )
{
    return vec_sel( v, s, g_VectorMaskX );
}

inline const vector float vector_insert_scalar_y( const vector float v, const vector float s ) __attribute__(( pure, always_inline ));
inline const vector float vector_insert_scalar_y( const vector float v, const vector float s )
{
    return vec_sel( v, s, g_VectorMaskY );
}

inline const vector float vector_insert_scalar_z( const vector float v, const vector float s ) __attribute__(( pure, always_inline ));
inline const vector float vector_insert_scalar_z( const vector float v, const vector float s )
{
    return vec_sel( v, s, g_VectorMaskZ );
}

inline const vector float vector_insert_scalar_w( const vector float v, const vector float s ) __attribute__(( pure, always_inline ));
inline const vector float vector_insert_scalar_w( const vector float v, const vector float s )
{
    return vec_sel( v, s, g_VectorMaskW );
}

//inline const bool vector3_all_eq( const vector float v1, const vector float v2 ) __attribute__(( const, always_inline ));
inline const bool vector3_all_eq( const vector float v1, const vector float v2 ) __attribute__(( pure, always_inline ));
inline const bool vector3_all_eq( const vector float v1, const vector float v2 )
{
    const vector float v1_xyz0( vec_and( v1, g_VectorMaskXYZ ) );
    const vector float v2_xyz0( vec_and( v2, g_VectorMaskXYZ ) );
    return vec_all_eq( v1_xyz0, v2_xyz0 );
}

//inline const bool vector3_any_ne( const vector float v1, const vector float v2 ) __attribute__(( const, always_inline ));
inline const bool vector3_any_ne( const vector float v1, const vector float v2 ) __attribute__(( pure, always_inline ));
inline const bool vector3_any_ne( const vector float v1, const vector float v2 )
{
    const vector float v1_xyz0( vec_and( v1, g_VectorMaskXYZ ) );
    const vector float v2_xyz0( vec_and( v2, g_VectorMaskXYZ ) );
    return vec_any_ne( v1_xyz0, v2_xyz0 );
}

//inline const vector float vector3_dot_product( const vector float v1, const vector float v2 ) __attribute__(( const, always_inline ));
inline const vector float vector3_dot_product( const vector float v1, const vector float v2 ) __attribute__(( pure, always_inline ));
inline const vector float vector3_dot_product( const vector float v1, const vector float v2 )
{
    const vector float v1_x_v2( vec_madd( v1, v2, vec_splats( 0.0f ) ) );
    return vec_add( vec_splat( v1_x_v2, VX ),
           vec_add( vec_splat( v1_x_v2, VY ),
                    vec_splat( v1_x_v2, VZ ) ) );
}

inline const vector float vector4_dot_product( const vector float v1, const vector float v2 ) __attribute__(( pure, always_inline ));
inline const vector float vector4_dot_product( const vector float v1, const vector float v2 )
{
    const vector float v1_x_v2( vec_madd( v1, v2, vec_splats( 0.0f ) ) );
    return vec_add( vec_add( vec_splat( v1_x_v2, VX ),
                             vec_splat( v1_x_v2, VY ) ),
                    vec_add( vec_splat( v1_x_v2, VZ ),
                             vec_splat( v1_x_v2, VW ) ) );
}

//inline const vector float vector3_cross_product( const vector float v1, const vector float v2 ) __attribute__(( const, always_inline ));
inline const vector float vector3_cross_product( const vector float v1, const vector float v2 ) __attribute__(( pure, always_inline ));
inline const vector float vector3_cross_product( const vector float v1, const vector float v2 )
{
    const vector float v1_yzx( vec_perm( v1, v1, g_VectorPermuteYZX ) );
    const vector float v1_zxy( vec_perm( v1, v1, g_VectorPermuteZXY ) );
    const vector float v2_yzx( vec_perm( v2, v2, g_VectorPermuteYZX ) );
    const vector float v2_zxy( vec_perm( v2, v2, g_VectorPermuteZXY ) );
    return vec_nmsub( v2_yzx, v1_zxy, vec_madd( v1_yzx, v2_zxy, vec_splats( 0.0f ) ) );
}

//inline const vector float vector_reciprocal_sqrt( const vector float B ) __attribute__(( const, always_inline ));
inline const vector float vector_reciprocal_sqrt( const vector float B ) __attribute__(( pure, always_inline ));
inline const vector float vector_reciprocal_sqrt( const vector float B )
{
    const vector float    y0( vec_rsqrte( B ) ); // A / sqrt(B), A == 1.0f
    const vector float    zero( vec_splats( 0.0f ) );
    const vector float    half( vec_splats( 0.5f ) );
    const vector float    one( vec_splats( 1.0f ) );
    const vector bool int B_GT_zero( vec_cmpgt( B, zero ) );
    const vector float    t0( y0 * y0 );
    const vector float    t1( y0 * half );
    const vector float    t2( vec_nmsub( B, t0, one ) );
    const vector float    y1( vec_madd( t2, t1, y0 ) );
    return vec_sel( zero, y1, B_GT_zero );
}

#elif defined( _AUDIO_MATHS_360_ )

// altivec_pem.pdf: 4.2.2.1 Floating-Point Division & Square-Root
#define VECTOR_RECIPROCAL( B, out_rcp_B )                                       \
{                                                                               \
    const __vector4 y0( __vrefp( B ) );                                         \
    const __vector4 absMask( g_VectorAbsMask );                                 \
    const __vector4 abs_B( __vand( B, absMask ) );                              \
    const __vector4 fltMin( g_VectorFltMin );                                   \
    const __vector4 fltMax( g_VectorFltMax );                                   \
    const __vector4 one( g_VectorOne );                                         \
    const __vector4 abs_B_GT_fltMin( __vcmpgtfp( abs_B, fltMin ) );             \
    const __vector4 abs_B_LT_fltMax( __vcmpgtfp( fltMax, abs_B ) );             \
    const __vector4 valid_B( __vand( abs_B_GT_fltMin, abs_B_LT_fltMax ) );      \
    const __vector4 t( __vnmsubfp( y0, B, one ) );                              \
    const __vector4 y1( __vmaddfp( y0, t, y0 ) );                               \
    out_rcp_B = __vsel( __vzero(), y1, valid_B );                               \
}

// altivec_pem.pdf: 4.2.2.1 Floating-Point Division & Square-Root
#define VECTOR_RECIPROCAL_SQRT( B, out_rcp_sqrt_B )                     \
{                                                                       \
    const __vector4 y0( __vrsqrtefp( B ) );                             \
    const __vector4 fltMin( g_VectorFltMin );                           \
    const __vector4 fltMax( g_VectorFltMax );                           \
    const __vector4 half( g_VectorHalf );                               \
    const __vector4 one( g_VectorOne );                                 \
    const __vector4 B_GT_fltMin( __vcmpgtfp( B, fltMin ) );             \
    const __vector4 B_LT_fltMax( __vcmpgtfp( fltMax, B ) );             \
    const __vector4 valid_B( __vand( B_GT_fltMin, B_LT_fltMax ) );      \
    const __vector4 t0( __vmulfp( y0, y0 ) );                           \
    const __vector4 t1( __vmulfp( y0, half ) );                         \
    const __vector4 t2( __vnmsubfp( B, t0, one ) );                     \
    const __vector4 y1( __vmaddfp( t2, t1, y0 ) );                      \
    out_rcp_sqrt_B = __vsel( __vzero(), y1, valid_B );                  \
}

#endif

////////////////////////////////////////////////////////////////////////////////
// Scalar, Vector3 & Vector4
////////////////////////////////////////////////////////////////////////////////

AUDIO_PASSINREG class Scalar // Single float value [some implementations splatted into all 4 elements of a vector]
{
public:
    
    #if defined( _AUDIO_MATHS_SPU_ ) || defined( _AUDIO_MATHS_PPU_ )
    vector float f;
    #elif defined( _AUDIO_MATHS_360_ )
    __vector4 f;
    #else
    float f;
    #endif
    
} __attribute__(( aligned( 16 ) ));

AUDIO_PASSINREG class Vector3 // Vector of 3 floats, with 4th element undefined
{
public:
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    vector float xyz;
    #elif defined( _AUDIO_MATHS_360_ )
    __vector4 xyz;
    #else
    float x, y, z;
    #endif
    
} __attribute__(( aligned( 16 ) ));

typedef Vector3 Vector;

AUDIO_PASSINREG class Vector4 // Vector of 4 floats
{
public:
    
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    vector float xyzw;
    #elif defined( _AUDIO_MATHS_360_ )
    __vector4 xyzw;
    #else
    float x, y, z, w;
    #endif
    
} __attribute__(( aligned( 16 ) ));

AUDIO_PASSINREG class Normal // Vector of 3 floats, with magnitude = 1.0f and 4th element undefined
{
public:
    
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    vector float xyz;
    #elif defined( _AUDIO_MATHS_360_ )
    __vector4 xyz;
    #else
    float x, y, z;
    #endif
    
} __attribute__(( aligned( 16 ) ));

AUDIO_PASSINREG class Vector4_U32 // Vector of 4 U32's
{
public:
    
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    vector unsigned int xyzw;
    #elif defined( _AUDIO_MATHS_360_ )
    __vector4 xyzw;
    #else
    U32 x, y, z, w;
    #endif
    
} __attribute__(( aligned( 16 ) ));

AUDIO_PASSINREG class Vector4_S32 // Vector of 4 S32's
{
public:
    
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    vector signed int xyzw;
    #elif defined( _AUDIO_MATHS_360_ )
    __vector4 xyzw;
    #else
    S32 x, y, z, w;
    #endif
    
} __attribute__(( aligned( 16 ) ));

AUDIO_PASSINREG class Vector8_S16 // Vector of 8 S16's
{
public:
    
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    vector signed short v8;
    #elif defined( _AUDIO_MATHS_360_ )
    __vector4 v8;
    #else
    S16 s0, s1, s2, s3, s4, s5, s6, s7;
    #endif
    
} __attribute__(( aligned( 16 ) ));



NOALIAS __forceinline const float _float( const Scalar scalar ) __attribute__(( always_inline ));
NOALIAS __forceinline const float _float( const Scalar scalar )
{
    float result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result = spu_extract( scalar.f, VX );
    #elif defined( _AUDIO_MATHS_PPU_ )
    vec_ste( scalar.f, 0x0, &result ); // Warning: LHS!
    #elif defined( _AUDIO_MATHS_360_ )
    __stvewx( scalar.f, &result, 0x0 ); // Warning: LHS!
    #else
    result = scalar.f;
    #endif
    return result;
}

NOALIAS __forceinline const Scalar _Scalar( const float& f ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar _Scalar( const float& f )
{
    Scalar result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.f = spu_splats( f );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.f = vec_splat( vector_load_x( &f ), VX );
    #elif defined( _AUDIO_MATHS_360_ )
    result.f = __vspltw( __lvlx( &f, 0x0 ), VX );
    #else
    result.f = f;
    #endif
    return result;
}


#if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )

inline const Scalar _Scalar( const vector float src ) __attribute__(( always_inline ));
inline const Scalar _Scalar( const vector float src )
{
    Scalar result;
    result.f = src;
    return result;
}

inline const Vector3 MakeVector3( const vector float src ) __attribute__(( always_inline ));
inline const Vector3 MakeVector3( const vector float src )
{
    Vector3 result;
    result.xyz = src;
    return result;
}

inline const Vector4 MakeVector4( const vector float src ) __attribute__(( always_inline ));
inline const Vector4 MakeVector4( const vector float src )
{
    Vector4 result;
    result.xyzw = src;
    return result;
}

#elif defined( _AUDIO_MATHS_360_ )

NOALIAS __forceinline const Scalar _Scalar( const __vector4 src );
NOALIAS __forceinline const Scalar _Scalar( const __vector4 src )
{
    Scalar result;
    result.f = src;
    return result;
}

NOALIAS __forceinline const Vector3 MakeVector3( const __vector4 src );
NOALIAS __forceinline const Vector3 MakeVector3( const __vector4 src )
{
    Vector3 result;
    result.xyz = src;
    return result;
}

NOALIAS __forceinline const Vector4 MakeVector4( const __vector4 src );
NOALIAS __forceinline const Vector4 MakeVector4( const __vector4 src )
{
    Vector4 result;
    result.xyzw = src;
    return result;
}

#else

NOALIAS __forceinline const Scalar _Scalar( const float src[] );
NOALIAS __forceinline const Scalar _Scalar( const float src[] )
{
    Scalar result;
    result.f = src[0];
    return result;
}

NOALIAS __forceinline const Vector3 MakeVector3( const float src[] );
NOALIAS __forceinline const Vector3 MakeVector3( const float src[] )
{
    Vector3 result;
    result.x = src[0];
    result.y = src[1];
    result.z = src[2];
    return result;
}

NOALIAS __forceinline const Vector4 MakeVector4( const float src[] );
NOALIAS __forceinline const Vector4 MakeVector4( const float src[] )
{
    Vector4 result;
    result.x = src[0];
    result.y = src[1];
    result.z = src[2];
    result.w = src[3];
    return result;
}

#endif


/// Make Vector3 from 3 floats
NOALIAS __forceinline const Vector3 MakeVector3( const float& x = 0.0f, const float& y = 0.0f, const float& z = 0.0f ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 MakeVector3( const float& x, const float& y, const float& z )
{
    Vector3 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    //result.xyz = (vector float)( x, y, z, 0.0f );
    result.xyz = make_vector3( x, y, z );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyz = __vmrghw( __vmrghw( __lvlx( &x, 0x0 ), __lvlx( &z, 0x0 ) ),
                           __vmrghw( __lvlx( &y, 0x0 ), __vzero() ) );
    #else
    result.x = x;
    result.y = y;
    result.z = z;
    #endif
    return result;
}

/// Make Vector4 from 4 Scalars
NOALIAS __forceinline const Vector4 MakeVector4( const Scalar x, const Scalar y, const Scalar z, const Scalar w ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 MakeVector4( const Scalar x, const Scalar y, const Scalar z, const Scalar w )
{
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ ) || defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = make_vector4( x.f, y.f, z.f, w.f );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vmrghw( __vmrghw( x.f, z.f ),
                            __vmrghw( y.f, w.f ) );
    #else
    result.x = x.f;
    result.y = y.f;
    result.z = z.f;
    result.w = w.f;
    #endif
    return result;
}

/// Make Vector4 from 4 floats
NOALIAS __forceinline const Vector4 MakeVector4( const float& x, const float& y, const float& z, const float& w ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 MakeVector4( const float& x, const float& y, const float& z, const float& w )
{
    Vector4 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = make_vector4( x, y, z, w );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vmrghw( __vmrghw( __lvlx( &x, 0x0 ), __lvlx( &z, 0x0 ) ),
                            __vmrghw( __lvlx( &y, 0x0 ), __lvlx( &w, 0x0 ) ) );
    #else
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    #endif
    return result;
}

/// Make Vector4 from 1 float splatted into all elements
NOALIAS __forceinline const Vector4 MakeVector4( const float& f = 0.0f ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 MakeVector4( const float& f )
{
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_splats( f );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = make_scalar( f );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vspltw( __lvlx( &f, 0x0 ), VX );
    #else
    result.x = f;
    result.y = f;
    result.z = f;
    result.w = f;
    #endif
    return result;
}

/// Make Vector4 from 1 int splatted into all elements
NOALIAS __forceinline const Vector4 MakeVector4( const S32& i ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 MakeVector4( const S32& i )
{
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_splats( (F32)i );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_ctf( vec_splat( vector_load_x( &i ), VX ), 0 );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vcfsx( __vspltw( __lvlx( &i, 0x0 ), VX ), 0 );
    #else
    result.x = (F32)i;
    result.y = (F32)i;
    result.z = (F32)i;
    result.w = (F32)i;
    #endif
    return result;
}

/// Make Vector4 from 1 int splatted into all elements
NOALIAS __forceinline const Vector4 MakeVector4( const U32& i ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 MakeVector4( const U32& i )
{
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_splats( (F32)i );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_ctf( vec_splat( vector_load_x( &i ), VX ), 0 );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vcfux( __vspltw( __lvlx( &i, 0x0 ), VX ), 0 );
    #else
    result.x = (F32)i;
    result.y = (F32)i;
    result.z = (F32)i;
    result.w = (F32)i;
    #endif
    return result;
}

/// Make Vector4 from 1 float splatted into all elements
template < int divPower2 > NOALIAS __forceinline const Vector4 IntToFloatHi( const Vector8_S16 src ) __attribute__(( always_inline ));
template < int divPower2 > NOALIAS __forceinline const Vector4 IntToFloatHi( const Vector8_S16 src )
{
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_convtf( vector_unpack_hi( src.v8 ), divPower2 );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_ctf( vec_unpackh( src.v8 ), divPower2 );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vcfsx( __vupkhsh( src.v8 ), divPower2 );
    #else
    const float scale = 1.0f / (float)( 1 < divPower2 );
    result.x = (F32)src.s0 * scale;
    result.y = (F32)src.s1 * scale;
    result.z = (F32)src.s2 * scale;
    result.w = (F32)src.s3 * scale;
    #endif
    return result;
}

/// Make Vector4 from 1 float splatted into all elements
template < int divPower2 > NOALIAS __forceinline const Vector4 IntToFloatLo( const Vector8_S16 src ) __attribute__(( always_inline ));
template < int divPower2 > NOALIAS __forceinline const Vector4 IntToFloatLo( const Vector8_S16 src )
{
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_convtf( vector_unpack_lo( src.v8 ), divPower2 );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_ctf( vec_unpackl( src.v8 ), divPower2 );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vcfsx( __vupklsh( src.v8 ), divPower2 );
    #else
    const float scale = 1.0f / (float)( 1 < divPower2 );
    result.x = (F32)src.s4 * scale;
    result.y = (F32)src.s5 * scale;
    result.z = (F32)src.s6 * scale;
    result.w = (F32)src.s7 * scale;
    #endif
    return result;
}

/// Make Vector4_U32 from 1 U32 splatted into all elements
NOALIAS __forceinline const Vector4_U32 _Vector4_U32( const U32& i ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4_U32 _Vector4_U32( const U32& i )
{
    Vector4_U32 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_splats( i );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = make_scalar( i );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vspltw( __lvlx( &i, 0x0 ), VX );
    #else
    result.x = i;
    result.y = i;
    result.z = i;
    result.w = i;
    #endif
    return result;
}

/// Make Vector4_S32 from 1 S32 splatted into all elements
NOALIAS __forceinline const Vector4_S32 _Vector4_S32( const S32& i ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4_S32 _Vector4_S32( const S32& i )
{
    Vector4_S32 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_splats( i );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = make_scalar( i );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vspltw( __lvlx( &i, 0x0 ), VX );
    #else
    result.x = i;
    result.y = i;
    result.z = i;
    result.w = i;
    #endif
    return result;
}

/// Make Vector4_U32 from 1 U32 splatted into all elements
template < U32 I > NOALIAS __forceinline const Vector4_U32 _Vector4_U32() __attribute__(( always_inline ));
template < U32 I > NOALIAS __forceinline const Vector4_U32 _Vector4_U32()
{
    Vector4_U32 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_splats( I );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_splat_u32( I );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vspltisw( I );
    #else
    result.x = I;
    result.y = I;
    result.z = I;
    result.w = I;
    #endif
    return result;
}

/// Load aligned array of 4 floats into a Vector4
NOALIAS __forceinline const Vector4 LoadAligned( float const * const __restrict ptr ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 LoadAligned( float const * const __restrict ptr )
{
    Vector4 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = *( (vector float*)ptr );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = *( (__vector4*)ptr );
    #else
    result.x = ptr[0];
    result.y = ptr[1];
    result.z = ptr[2];
    result.w = ptr[3];
    #endif
    return result;
}

/// Load unaligned array of 8 signed shorts into a Vector8_S16
NOALIAS __forceinline const Vector8_S16 LoadUnaligned( S16 const * const __restrict ptr ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector8_S16 LoadUnaligned( S16 const * const __restrict ptr )
{
    Vector8_S16 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.v8 = vector_load_unaligned( ptr );
    #elif defined( _AUDIO_MATHS_360_ )
    result.v8 = __vor( __lvlx( ptr, 0x00 ),
                       __lvrx( ptr, 0x10 ) );
    #else
    result.s0 = ptr[0];
    result.s1 = ptr[1];
    result.s2 = ptr[2];
    result.s3 = ptr[3];
    result.s4 = ptr[4];
    result.s5 = ptr[5];
    result.s6 = ptr[6];
    result.s7 = ptr[7];
    #endif
    return result;
}

/// Store Vector4 into aligned array of 4 floats
NOALIAS __forceinline void StoreAligned( const Vector4 v, float * const __restrict ptr ) __attribute__(( always_inline ));
NOALIAS __forceinline void StoreAligned( const Vector4 v, float * const __restrict ptr )
{
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    *( (vector float*)ptr ) = v.xyzw;
    #elif defined( _AUDIO_MATHS_360_ )
    *( (__vector4*)ptr ) = v.xyzw;
    #else
    ptr[0] = v.x;
    ptr[1] = v.y;
    ptr[2] = v.z;
    ptr[3] = v.w;
    #endif
}

/// Vector3 Accessor: Get X
NOALIAS __forceinline const Scalar X( const Vector3 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar X( const Vector3 v )
{
    Scalar result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.f = vector_splat_x( v.xyz );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.f = vector_splat_x( v.xyz );
    #elif defined( _AUDIO_MATHS_360_ )
    result.f = __vspltw( v.xyz, VX );
    #else
    result.f = v.x;
    #endif
    return result;
}

/// Vector3 Accessor: Get Y
NOALIAS __forceinline const Scalar Y( const Vector3 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar Y( const Vector3 v )
{
    Scalar result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.f = vector_splat_y( v.xyz );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.f = vector_splat_y( v.xyz );
    #elif defined( _AUDIO_MATHS_360_ )
    result.f = __vspltw( v.xyz, VY );
    #else
    result.f = v.y;
    #endif
    return result;
}

/// Vector3 Accessor: Get Z
NOALIAS __forceinline const Scalar Z( const Vector3 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar Z( const Vector3 v )
{
    Scalar result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.f = vector_splat_z( v.xyz );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.f = vector_splat_z( v.xyz );
    #elif defined( _AUDIO_MATHS_360_ )
    result.f = __vspltw( v.xyz, VZ );
    #else
    result.f = v.z;
    #endif
    return result;
}

/// Vector3 Accessor: Set X
NOALIAS __forceinline void SetX( Vector3 v, const Scalar x ) __attribute__(( always_inline ));
NOALIAS __forceinline void SetX( Vector3 v, const Scalar x )
{
    #if defined( _AUDIO_MATHS_SPU_ )
    v.xyz = vector_insert_scalar_x( v.xyz, x.f );
    #elif defined( _AUDIO_MATHS_PPU_ )
    v.xyz = vector_insert_scalar_x( v.xyz, x.f );
    #elif defined( _AUDIO_MATHS_360_ )
    v.xyz = __vrlimi( v.xyz, x.f, MASK_INSERT_X, 0 );
    #else
    v.x = x.f;
    #endif
}

/// Vector3 Accessor: Set Y
NOALIAS __forceinline void SetY( Vector3 v, const Scalar y ) __attribute__(( always_inline ));
NOALIAS __forceinline void SetY( Vector3 v, const Scalar y )
{
    #if defined( _AUDIO_MATHS_SPU_ )
    v.xyz = vector_insert_scalar_y( v.xyz, y.f );
    #elif defined( _AUDIO_MATHS_PPU_ )
    v.xyz = vector_insert_scalar_y( v.xyz, y.f );
    #elif defined( _AUDIO_MATHS_360_ )
    v.xyz = __vrlimi( v.xyz, y.f, MASK_INSERT_Y, 3 );
    #else
    v.y = y.f;
    #endif
}

/// Vector3 Accessor: Set Z
NOALIAS __forceinline void SetZ( Vector3 v, const Scalar z ) __attribute__(( always_inline ));
NOALIAS __forceinline void SetZ( Vector3 v, const Scalar z )
{
    #if defined( _AUDIO_MATHS_SPU_ )
    v.xyz = vector_insert_scalar_z( v.xyz, z.f );
    #elif defined( _AUDIO_MATHS_PPU_ )
    v.xyz = vector_insert_scalar_z( v.xyz, z.f );
    #elif defined( _AUDIO_MATHS_360_ )
    v.xyz = __vrlimi( v.xyz, z.f, MASK_INSERT_Z, 2 );
    #else
    v.z = z.f;
    #endif
}

/// Vector4 Accessor: Get X
NOALIAS __forceinline const Scalar X( const Vector4 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar X( const Vector4 v )
{
    Scalar result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.f = vector_splat_x( v.xyzw );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.f = vector_splat_x( v.xyzw );
    #elif defined( _AUDIO_MATHS_360_ )
    result.f = __vspltw( v.xyzw, VX );
    #else
    result.f = v.x;
    #endif
    return result;
}

/// Vector4 Accessor: Get Y
NOALIAS __forceinline const Scalar Y( const Vector4 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar Y( const Vector4 v )
{
    Scalar result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.f = vector_splat_y( v.xyzw );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.f = vector_splat_y( v.xyzw );
    #elif defined( _AUDIO_MATHS_360_ )
    result.f = __vspltw( v.xyzw, VY );
    #else
    result.f = v.y;
    #endif
    return result;
}

/// Vector4 Accessor: Get Z
NOALIAS __forceinline const Scalar Z( const Vector4 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar Z( const Vector4 v )
{
    Scalar result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.f = vector_splat_z( v.xyzw );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.f = vector_splat_z( v.xyzw );
    #elif defined( _AUDIO_MATHS_360_ )
    result.f = __vspltw( v.xyzw, VZ );
    #else
    result.f = v.z;
    #endif
    return result;
}

/// Vector4 Accessor: Get W
NOALIAS __forceinline const Scalar W( const Vector4 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar W( const Vector4 v )
{
    Scalar result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.f = vector_splat_w( v.xyzw );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.f = vector_splat_w( v.xyzw );
    #elif defined( _AUDIO_MATHS_360_ )
    result.f = __vspltw( v.xyzw, VW );
    #else
    result.f = v.w;
    #endif
    return result;
}

/// Vector4 Accessor: Set X
NOALIAS __forceinline void SetX( Vector4& v, const Scalar x ) __attribute__(( always_inline ));
NOALIAS __forceinline void SetX( Vector4& v, const Scalar x )
{
    #if defined( _AUDIO_MATHS_SPU_ )
    v.xyzw = vector_insert_scalar_x( v.xyzw, x.f );
    #elif defined( _AUDIO_MATHS_PPU_ )
    v.xyzw = vector_insert_scalar_x( v.xyzw, x.f );
    #elif defined( _AUDIO_MATHS_360_ )
    v.xyzw = __vrlimi( v.xyzw, x.f, MASK_INSERT_X, 0 );
    #else
    v.x = x.f;
    #endif
}

/// Vector4 Accessor: Set Y
NOALIAS __forceinline void SetY( Vector4& v, const Scalar y ) __attribute__(( always_inline ));
NOALIAS __forceinline void SetY( Vector4& v, const Scalar y )
{
    #if defined( _AUDIO_MATHS_SPU_ )
    v.xyzw = vector_insert_scalar_y( v.xyzw, y.f );
    #elif defined( _AUDIO_MATHS_PPU_ )
    v.xyzw = vector_insert_scalar_y( v.xyzw, y.f );
    #elif defined( _AUDIO_MATHS_360_ )
    v.xyzw = __vrlimi( v.xyzw, y.f, MASK_INSERT_Y, 3 );
    #else
    v.y = y.f;
    #endif
}

/// Vector4 Accessor: Set Z
NOALIAS __forceinline void SetZ( Vector4& v, const Scalar z ) __attribute__(( always_inline ));
NOALIAS __forceinline void SetZ( Vector4& v, const Scalar z )
{
    #if defined( _AUDIO_MATHS_SPU_ )
    v.xyzw = vector_insert_scalar_z( v.xyzw, z.f );
    #elif defined( _AUDIO_MATHS_PPU_ )
    v.xyzw = vector_insert_scalar_z( v.xyzw, z.f );
    #elif defined( _AUDIO_MATHS_360_ )
    v.xyzw = __vrlimi( v.xyzw, z.f, MASK_INSERT_Z, 2 );
    #else
    v.z = z.f;
    #endif
}

/// Vector4 Accessor: Set W
NOALIAS __forceinline void SetW( Vector4& v, const Scalar w ) __attribute__(( always_inline ));
NOALIAS __forceinline void SetW( Vector4& v, const Scalar w )
{
    #if defined( _AUDIO_MATHS_SPU_ )
    v.xyzw = vector_insert_scalar_w( v.xyzw, w.f );
    #elif defined( _AUDIO_MATHS_PPU_ )
    v.xyzw = vector_insert_scalar_w( v.xyzw, w.f );
    #elif defined( _AUDIO_MATHS_360_ )
    v.xyzw = __vrlimi( v.xyzw, w.f, MASK_INSERT_W, 1 );
    #else
    v.w = w.f;
    #endif
}

/// Create Unit Vector3 along X Axis
NOALIAS __forceinline const Vector3 Vector3_UnitX() __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 Vector3_UnitX()
{
    Vector3 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ ) || defined( _AUDIO_MATHS_360_ )
    result.xyz = g_VectorUnitX;
    #else
    result.x = 1.0f;
    result.y = 0.0f;
    result.z = 0.0f;
    #endif
    return result;
}

/// Create Unit Vector3 along Y Axis
NOALIAS __forceinline const Vector3 Vector3_UnitY() __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 Vector3_UnitY()
{
    Vector3 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ ) || defined( _AUDIO_MATHS_360_ )
    result.xyz = g_VectorUnitY;
    #else
    result.x = 0.0f;
    result.y = 1.0f;
    result.z = 0.0f;
    #endif
    return result;
}

/// Create Unit Vector3 along Z Axis
NOALIAS __forceinline const Vector3 Vector3_UnitZ() __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 Vector3_UnitZ()
{
    Vector3 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ ) || defined( _AUDIO_MATHS_360_ )
    result.xyz = g_VectorUnitZ;
    #else
    result.x = 0.0f;
    result.y = 0.0f;
    result.z = 1.0f;
    #endif
    return result;
}

/// Vector4 Merge X & Y
NOALIAS __forceinline const Vector4 MergeXY( const Vector4 v1, const Vector4 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 MergeXY( const Vector4 v1, const Vector4 v2 )
{	
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_shuffle( v1.xyzw, v2.xyzw, g_VectorMergeXY );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_mergeh( v1.xyzw, v2.xyzw );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vmrghw( v1.xyzw, v2.xyzw );
    #else
    result.x = v1.x;
    result.y = v2.x;
    result.z = v1.y;
    result.w = v2.y;
    #endif
    return result;
}

/// Vector4 Round To Integer [ Towards Zero ]
NOALIAS __forceinline const Vector4 RoundToIntZero( const Vector4 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 RoundToIntZero( const Vector4 v )
{	
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = vector_rfiz( v.xyzw );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_vrfiz( v.xyzw );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vrfiz( v.xyzw );
    #else
    result.x = (float)_copysign( floorf( fabsf( v.x ) ), v.x );
    result.y = (float)_copysign( floorf( fabsf( v.y ) ), v.y );
    result.z = (float)_copysign( floorf( fabsf( v.z ) ), v.z );
    result.w = (float)_copysign( floorf( fabsf( v.w ) ), v.w );
    #endif
    return result;
}

/// Vector4 Round To Integer [ Towards Nearest ]
NOALIAS __forceinline const Vector4 RoundToIntNearest( const Vector4 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 RoundToIntNearest( const Vector4 v )
{	
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = vector_rfin( v.xyzw );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_vrfin( v.xyzw );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vrfin( v.xyzw );
    #else
    result.x = floorf( v.x + 0.5f );
    result.y = floorf( v.y + 0.5f );
    result.z = floorf( v.z + 0.5f );
    result.w = floorf( v.w + 0.5f );
    #endif
    return result;
}

/// Vector4 Round To Integer [ Towards Positive Infinity ]
#if 0 // until vector_rfip implemented!
NOALIAS __forceinline const Vector4 RoundToIntPosInf( const Vector4 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 RoundToIntPosInf( const Vector4 v )
{	
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = vector_rfip( v.xyzw );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_vrfip( v.xyzw );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vrfip( v.xyzw );
    #else
    result.x = ceilf( v.x );
    result.y = ceilf( v.y );
    result.z = ceilf( v.z );
    result.w = ceilf( v.w );
    #endif
    return result;
}
#endif

/// Vector4 Round To Integer [ Towards Negative Infinity ]
#if 0 // until vector_rfim implemented!
NOALIAS __forceinline const Vector4 RoundToIntNegInf( const Vector4 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 RoundToIntNegInf( const Vector4 v )
{	
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = vector_rfim( v.xyzw );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_vrfim( v.xyzw );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vrfim( v.xyzw );
    #else
    result.x = floorf( v.x );
    result.y = floorf( v.y );
    result.z = floorf( v.z );
    result.w = floorf( v.w );
    #endif
    return result;
}
#endif

/// Vector3 Comparison == / EQ
NOALIAS __forceinline const bool operator == ( const Vector3 v1, const Vector3 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const bool operator == ( const Vector3 v1, const Vector3 v2 )
{	
    #if defined( _AUDIO_MATHS_SPU_ )
    const vector unsigned int vec_eq( spu_cmpeq( v1.xyz, v2.xyz ) );
    const unsigned int eq( vector_extract_x( vec_eq ) &
                           vector_extract_y( vec_eq ) &
                           vector_extract_z( vec_eq ) );
    return (bool)( eq & 1 );
    #elif defined( _AUDIO_MATHS_PPU_ )
    return (bool)vector3_all_eq( v1.xyz, v2.xyz );
    #elif defined( _AUDIO_MATHS_360_ )
    U32 CR;
    const __vector4 a_xyzz( __vpermwi( v1.xyz, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b_xyzz( __vpermwi( v2.xyz, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpeqfpR( a_xyzz, b_xyzz, &CR );
    return ( CR >> CR6_ALL_TRUE_BIT ) & 1;
    #else
    return ( v1.x == v2.x &&
             v1.y == v2.y &&
             v1.z == v2.z );
    #endif
}

/// Vector3 Comparison != / NE
NOALIAS __forceinline const bool operator != ( const Vector3 v1, const Vector3 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const bool operator != ( const Vector3 v1, const Vector3 v2 )
{	
    #if defined( _AUDIO_MATHS_SPU_ )
    const vector unsigned int vec_eq( spu_cmpeq( v1.xyz, v2.xyz ) );
    const vector unsigned int vec_ne( vector_not( vec_eq ) );
    const unsigned int ne( vector_extract_x( vec_ne ) |
                           vector_extract_y( vec_ne ) |
                           vector_extract_z( vec_ne ) );
    return (bool)( ne & 1 );
    #elif defined( _AUDIO_MATHS_PPU_ )
    return (bool)vector3_any_ne( v1.xyz, v2.xyz );
    #elif defined( _AUDIO_MATHS_360_ )
    U32 CR;
    const __vector4 a_xyzz( __vpermwi( v1.xyz, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b_xyzz( __vpermwi( v2.xyz, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpeqfpR( a_xyzz, b_xyzz, &CR );
    return ~( CR >> CR6_ALL_TRUE_BIT ) & 1;
    #else
    return ( v1.x != v2.x ||
             v1.y != v2.y ||
             v1.z != v2.z );
    #endif
}

/// Vector3 Test if Equal: compensates for floating point precision errors
NOALIAS __forceinline const bool IsEqual( const Vector3 v1, const Vector3 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const bool IsEqual( const Vector3 v1, const Vector3 v2 )
{
    #if defined( _AUDIO_MATHS_SPU_ )
    const vector float abs_diff( vector_abs( spu_sub( v1.xyz, v2.xyz ) ) );
    const vector unsigned int vec_gt( spu_cmpgt( abs_diff, g_VectorEpsilon ) );
    const vector unsigned int vec_le( vector_not( vec_gt ) );
    const unsigned int le( vector_extract_x( vec_le ) &
                           vector_extract_y( vec_le ) &
                           vector_extract_z( vec_le ) );
    return (bool)( le & 1 );
    #elif defined( _AUDIO_MATHS_PPU_ )
    const vector float v1_xyz0( vec_and( v1.xyz, g_VectorMaskXYZ ) ); // Ensure W is zero!
    const vector float v2_xyz0( vec_and( v2.xyz, g_VectorMaskXYZ ) );
    return (bool)vec_all_le( vec_abs( vec_sub( v1_xyz0, v2_xyz0 ) ), g_VectorEpsilon );
    #elif defined( _AUDIO_MATHS_360_ )
    U32 CR;
    const __vector4 v1_xyz0( __vrlimi( v1.xyz, __vzero(), MASK_INSERT_W, 0 ) );
    const __vector4 v2_xyz0( __vrlimi( v2.xyz, __vzero(), MASK_INSERT_W, 0 ) );
    const __vector4 abs_diff( __vand( __vsubfp( v1_xyz0, v2_xyz0 ), g_VectorAbsMask ) );
    __vcmpgefpR( g_VectorEpsilon, abs_diff, &CR );
    return ( CR >> CR6_ALL_TRUE_BIT ) & 1;
    #else
    return ( Abs( v1.x - v2.x ) <= fEpsilon ) &&
	       ( Abs( v1.y - v2.y ) <= fEpsilon ) &&
	       ( Abs( v1.z - v2.z ) <= fEpsilon );
    #endif
}

/// Vector4 Select_EQ : returns ( a = b ? c : d ) for each element x, y, z, & w
NOALIAS __forceinline const Vector4 Select_EQ( const Vector4 a, const Vector4 b, const Vector4 c, const Vector4 d ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 Select_EQ( const Vector4 a, const Vector4 b, const Vector4 c, const Vector4 d )
{
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_sel( d.xyzw, c.xyzw, spu_cmpeq( a.xyzw, b.xyzw ) );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_sel( d.xyzw, c.xyzw, vec_cmpeq( a.xyzw, b.xyzw ) );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vsel( d.xyzw, c.xyzw, __vcmpeqfp( a.xyzw, b.xyzw ) );
    #else
    result.x = a.x == b.x ? c.x : d.x;
    result.y = a.y == b.y ? c.y : d.y;
    result.z = a.z == b.z ? c.z : d.z;
    result.w = a.w == b.w ? c.w : d.w;
    #endif
    return result;
}

/// Vector4 Select_GT : returns ( a > b ? c : d ) for each element x, y, z, & w
NOALIAS __forceinline const Vector4 Select_GT( const Vector4 a, const Vector4 b, const Vector4 c, const Vector4 d ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 Select_GT( const Vector4 a, const Vector4 b, const Vector4 c, const Vector4 d )
{
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_sel( d.xyzw, c.xyzw, spu_cmpgt( a.xyzw, b.xyzw ) );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_sel( d.xyzw, c.xyzw, vec_cmpgt( a.xyzw, b.xyzw ) );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vsel( d.xyzw, c.xyzw, __vcmpgtfp( a.xyzw, b.xyzw ) );
    #else
    result.x = a.x > b.x ? c.x : d.x;
    result.y = a.y > b.y ? c.y : d.y;
    result.z = a.z > b.z ? c.z : d.z;
    result.w = a.w > b.w ? c.w : d.w;
    #endif
    return result;
}

/// Vector4 Select_GE : returns ( a >= b ? c : d ) for each element x, y, z, & w
NOALIAS __forceinline const Vector4 Select_GE( const Vector4 a, const Vector4 b, const Vector4 c, const Vector4 d ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 Select_GE( const Vector4 a, const Vector4 b, const Vector4 c, const Vector4 d )
{
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_sel( d.xyzw, c.xyzw, vector_cmpge( a.xyzw, b.xyzw ) );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_sel( d.xyzw, c.xyzw, vec_cmpge( a.xyzw, b.xyzw ) );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vsel( d.xyzw, c.xyzw, __vcmpgefp( a.xyzw, b.xyzw ) );
    #else
    result.x = a.x >= b.x ? c.x : d.x;
    result.y = a.y >= b.y ? c.y : d.y;
    result.z = a.z >= b.z ? c.z : d.z;
    result.w = a.w >= b.w ? c.w : d.w;
    #endif
    return result;
}

/// Vector4 Select_EQ : returns ( a = b ? c : d ) for each element x, y, z, & w
NOALIAS __forceinline const Vector4 Select_EQ( const Vector4_U32 a, const Vector4_U32 b, const Vector4 c, const Vector4 d ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 Select_EQ( const Vector4_U32 a, const Vector4_U32 b, const Vector4 c, const Vector4 d )
{
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_sel( d.xyzw, c.xyzw, spu_cmpeq( a.xyzw, b.xyzw ) );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_sel( d.xyzw, c.xyzw, vec_cmpeq( a.xyzw, b.xyzw ) );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vsel( d.xyzw, c.xyzw, __vcmpequw( a.xyzw, b.xyzw ) );
    #else
    result.x = a.x == b.x ? c.x : d.x;
    result.y = a.y == b.y ? c.y : d.y;
    result.z = a.z == b.z ? c.z : d.z;
    result.w = a.w == b.w ? c.w : d.w;
    #endif
    return result;
}

/// Vector3 Unary Plus [does nothing]
NOALIAS __forceinline const Vector3 operator + ( const Vector3 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 operator + ( const Vector3 v )
{
    return v;
}

/// Vector4 Unary Plus [does nothing]
NOALIAS __forceinline const Vector4 operator + ( const Vector4 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 operator + ( const Vector4 v )
{
    return v;
}

/// Scalar Unary Minus [negation]
NOALIAS __forceinline const Scalar operator - ( const Scalar s ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar operator - ( const Scalar s )
{	
    Scalar result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.f = -s.f;
    #elif defined( _AUDIO_MATHS_360_ )
    result.f = __vxor( s.f, g_VectorSignMask );
    #else
    result.f = -s.f;
    #endif
    return result;
}

/// Vector3 Unary Minus [negation]
NOALIAS __forceinline const Vector3 operator - ( const Vector3 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 operator - ( const Vector3 v )
{	
    Vector3 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.xyz = -v.xyz;
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyz = __vxor( v.xyz, g_VectorSignMask );
    #else
    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;
    #endif
    return result;
}

/// Vector4 Unary Minus [negation]
NOALIAS __forceinline const Vector4 operator - ( const Vector4 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 operator - ( const Vector4 v )
{	
    Vector4 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = -v.xyzw;
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vxor( v.xyzw, g_VectorSignMask );
    #else
    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;
    result.w = -v.z;
    #endif
    return result;
}

/// Scalar Add
NOALIAS __forceinline const Scalar operator + ( const Scalar s1, const Scalar s2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar operator + ( const Scalar s1, const Scalar s2 )
{	
    Scalar result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.f = s1.f + s2.f;
    #elif defined( _AUDIO_MATHS_360_ )
    result.f = __vaddfp( s1.f, s2.f );
    #else
    result.f = s1.f + s2.f;
    #endif
    return result;
}

/// Vector3 Add
NOALIAS __forceinline const Vector3 operator + ( const Vector3 v1, const Vector3 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 operator + ( const Vector3 v1, const Vector3 v2 )
{	
    Vector3 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.xyz = v1.xyz + v2.xyz;
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyz = __vaddfp( v1.xyz, v2.xyz );
    #else
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    #endif
    return result;
}

/// Vector4 Add
NOALIAS __forceinline const Vector4 operator + ( const Vector4 v1, const Vector4 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 operator + ( const Vector4 v1, const Vector4 v2 )
{	
    Vector4 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = v1.xyzw + v2.xyzw;
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vaddfp( v1.xyzw, v2.xyzw );
    #else
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    result.w = v1.w + v2.w;
    #endif
    return result;
}

/// Scalar Add Update
NOALIAS __forceinline Scalar& operator += ( Scalar& s1, const Scalar s2 ) __attribute__(( always_inline ));
NOALIAS __forceinline Scalar& operator += ( Scalar& s1, const Scalar s2 )
{	
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    s1.f = s1.f + s2.f;
    #elif defined( _AUDIO_MATHS_360_ )
    s1.f = __vaddfp( s1.f, s2.f );
    #else
    s1.f += s2.f;
    #endif
    return s1;
}

/// Vector3 Add Update
NOALIAS __forceinline Vector3& operator += ( Vector3& v1, const Vector3 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline Vector3& operator += ( Vector3& v1, const Vector3 v2 )
{	
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    v1.xyz = v1.xyz + v2.xyz;
    #elif defined( _AUDIO_MATHS_360_ )
    v1.xyz = __vaddfp( v1.xyz, v2.xyz );
    #else
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
    #endif
    return v1;
}

/// Vector4 Add Update
NOALIAS __forceinline Vector4& operator += ( Vector4& v1, const Vector4 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline Vector4& operator += ( Vector4& v1, const Vector4 v2 )
{	
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    v1.xyzw = v1.xyzw + v2.xyzw;
    #elif defined( _AUDIO_MATHS_360_ )
    v1.xyzw = __vaddfp( v1.xyzw, v2.xyzw );
    #else
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
    v1.w += v2.w;
    #endif
    return v1;
}

/// Scalar Subtract
NOALIAS __forceinline const Scalar operator - ( const Scalar s1, const Scalar s2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar operator - ( const Scalar s1, const Scalar s2 )
{	
    Scalar result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.f = s1.f - s2.f;
    #elif defined( _AUDIO_MATHS_360_ )
    result.f = __vsubfp( s1.f, s2.f );
    #else
    result.f = s1.f - s2.f;
    #endif
    return result;
}

/// Vector3 Subtract
NOALIAS __forceinline const Vector3 operator - ( const Vector3 v1, const Vector3 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 operator - ( const Vector3 v1, const Vector3 v2 )
{	
    Vector3 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.xyz = v1.xyz - v2.xyz;
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyz = __vsubfp( v1.xyz, v2.xyz );
    #else
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    #endif
    return result;
}

/// Vector4 Subtract
NOALIAS __forceinline const Vector4 operator - ( const Vector4 v1, const Vector4 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 operator - ( const Vector4 v1, const Vector4 v2 )
{	
    Vector4 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = v1.xyzw - v2.xyzw;
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vsubfp( v1.xyzw, v2.xyzw );
    #else
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    result.w = v1.w - v2.w;
    #endif
    return result;
}

/// Scalar Subtract Update
NOALIAS __forceinline Scalar& operator -= ( Scalar& s1, const Scalar s2 ) __attribute__(( always_inline ));
NOALIAS __forceinline Scalar& operator -= ( Scalar& s1, const Scalar s2 )
{	
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    s1.f = s1.f - s2.f;
    #elif defined( _AUDIO_MATHS_360_ )
    s1.f = __vsubfp( s1.f, s2.f );
    #else
    s1.f -= s2.f;
    #endif
    return s1;
}

/// Vector3 Subtract Update
NOALIAS __forceinline Vector3& operator -= ( Vector3& v1, const Vector3 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline Vector3& operator -= ( Vector3& v1, const Vector3 v2 )
{	
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    v1.xyz = v1.xyz - v2.xyz;
    #elif defined( _AUDIO_MATHS_360_ )
    v1.xyz = __vsubfp( v1.xyz, v2.xyz );
    #else
    v1.x -= v2.x;
    v1.y -= v2.y;
    v1.z -= v2.z;
    #endif
    return v1;
}

/// Vector4 Subtract Update
NOALIAS __forceinline Vector4& operator -= ( Vector4& v1, const Vector4 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline Vector4& operator -= ( Vector4& v1, const Vector4 v2 )
{	
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    v1.xyzw = v1.xyzw - v2.xyzw;
    #elif defined( _AUDIO_MATHS_360_ )
    v1.xyzw = __vsubfp( v1.xyzw, v2.xyzw );
    #else
    v1.x -= v2.x;
    v1.y -= v2.y;
    v1.z -= v2.z;
    v1.w -= v2.w;
    #endif
    return v1;
}

/// Scalar Multiply
NOALIAS __forceinline const Scalar operator * ( const Scalar s1, const Scalar s2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar operator * ( const Scalar s1, const Scalar s2 )
{	
    Scalar result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.f = s1.f * s2.f;
    #elif defined( _AUDIO_MATHS_360_ )
    result.f = __vmulfp( s1.f, s2.f );
    #else
    result.f = s1.f * s2.f;
    #endif
    return result;
}

/// Vector3 Multiply
NOALIAS __forceinline const Vector3 operator * ( const Vector3 v1, const Vector3 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 operator * ( const Vector3 v1, const Vector3 v2 )
{	
    Vector3 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.xyz = v1.xyz * v2.xyz;
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyz = __vmulfp( v1.xyz, v2.xyz );
    #else
    result.x = v1.x * v2.x;
    result.y = v1.y * v2.y;
    result.z = v1.z * v2.z;
    #endif
    return result;
}

/// Vector4 Multiply
NOALIAS __forceinline const Vector4 operator * ( const Vector4 v1, const Vector4 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 operator * ( const Vector4 v1, const Vector4 v2 )
{	
    Vector4 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = v1.xyzw * v2.xyzw;
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vmulfp( v1.xyzw, v2.xyzw );
    #else
    result.x = v1.x * v2.x;
    result.y = v1.y * v2.y;
    result.z = v1.z * v2.z;
    result.w = v1.w * v2.w;
    #endif
    return result;
}

/// Scalar * Vector3 Multiply
NOALIAS __forceinline const Vector3 operator * ( const Scalar s1, const Vector3 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 operator * ( const Scalar s1, const Vector3 v2 )
{	
    Vector3 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.xyz = s1.f * v2.xyz;
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyz = __vmulfp( s1.f, v2.xyz );
    #else
    result.x = s1.f * v2.x;
    result.y = s1.f * v2.y;
    result.z = s1.f * v2.z;
    #endif
    return result;
}

/// Vector3 * Scalar Multiply
NOALIAS __forceinline const Vector3 operator * ( const Vector3 v1, const Scalar s2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 operator * ( const Vector3 v1, const Scalar s2 )
{	
    Vector3 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.xyz = v1.xyz * s2.f;
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyz = __vmulfp( v1.xyz, s2.f );
    #else
    result.x = v1.x * s2.f;
    result.y = v1.y * s2.f;
    result.z = v1.z * s2.f;
    #endif
    return result;
}

/// Scalar * Vector4 Multiply
NOALIAS __forceinline const Vector4 operator * ( const Scalar s1, const Vector4 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 operator * ( const Scalar s1, const Vector4 v2 )
{	
    Vector4 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = s1.f * v2.xyzw;
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vmulfp( s1.f, v2.xyzw );
    #else
    result.x = s1.f * v2.x;
    result.y = s1.f * v2.y;
    result.z = s1.f * v2.z;
    result.w = s1.f * v2.w;
    #endif
    return result;
}

/// Vector4 * Scalar Multiply
NOALIAS __forceinline const Vector4 operator * ( const Vector4 v1, const Scalar s2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 operator * ( const Vector4 v1, const Scalar s2 )
{	
    Vector4 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = v1.xyzw * s2.f;
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vmulfp( v1.xyzw, s2.f );
    #else
    result.x = v1.x * s2.f;
    result.y = v1.y * s2.f;
    result.z = v1.z * s2.f;
    result.w = v1.w * s2.f;
    #endif
    return result;
}

/// Scalar Multiply Update
NOALIAS __forceinline Scalar& operator *= ( Scalar& s1, const Scalar s2 ) __attribute__(( always_inline ));
NOALIAS __forceinline Scalar& operator *= ( Scalar& s1, const Scalar s2 )
{	
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    s1.f = s1.f * s2.f;
    #elif defined( _AUDIO_MATHS_360_ )
    s1.f = __vmulfp( s1.f, s2.f );
    #else
    s1.f *= s2.f;
    #endif
    return s1;
}

/// Vector3 Multiply Update
NOALIAS __forceinline Vector3& operator *= ( Vector3& v1, const Vector3 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline Vector3& operator *= ( Vector3& v1, const Vector3 v2 )
{	
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    v1.xyz = v1.xyz * v2.xyz;
    #elif defined( _AUDIO_MATHS_360_ )
    v1.xyz = __vmulfp( v1.xyz, v2.xyz );
    #else
    v1.x *= v2.x;
    v1.y *= v2.y;
    v1.z *= v2.z;
    #endif
    return v1;
}

/// Vector4 Multiply Update
NOALIAS __forceinline Vector4& operator *= ( Vector4& v1, const Vector4 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline Vector4& operator *= ( Vector4& v1, const Vector4 v2 )
{	
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    v1.xyzw = v1.xyzw * v2.xyzw;
    #elif defined( _AUDIO_MATHS_360_ )
    v1.xyzw = __vmulfp( v1.xyzw, v2.xyzw );
    #else
    v1.x *= v2.x;
    v1.y *= v2.y;
    v1.z *= v2.z;
    v1.w *= v2.w;
    #endif
    return v1;
}

/// Vector3 x Scalar Multiply
//NOALIAS __forceinline const Vector3 operator * ( const Vector3 v, const Scalar s ) __attribute__(( always_inline ));
//NOALIAS __forceinline const Vector3 operator * ( const Vector3 v, const Scalar s )
//{	
//    Vector3 result;
//    #if defined( _AUDIO_MATHS_SPU_ )
//    result.xyz = v.xyz * s.f;
//    #elif defined( _AUDIO_MATHS_PPU_ )
//    result.xyz = v.xyz * s.f;
//    #elif defined( _AUDIO_MATHS_360_ )
//    result.xyz = __vmulfp( v.xyz, s.f );
//    #else
//    result.x = v.x * s.f;
//    result.y = v.y * s.f;
//    result.z = v.z * s.f;
//    #endif
//    return result;
//}

/// Vector3 Vector3 x Scalar Multiply Update
NOALIAS __forceinline Vector3& operator *= ( Vector3& v, const Scalar s ) __attribute__(( always_inline ));
NOALIAS __forceinline Vector3& operator *= ( Vector3& v, const Scalar s )
{	
    #if defined( _AUDIO_MATHS_SPU_ )
    v.xyz = v.xyz * s.f;
    #elif defined( _AUDIO_MATHS_PPU_ )
    v.xyz = v.xyz * s.f;
    #elif defined( _AUDIO_MATHS_360_ )
    v.xyz = __vmulfp( v.xyz, s.f );
    #else
    v.x *= s.f;
    v.y *= s.f;
    v.z *= s.f;
    #endif
    return v;
}

/// Scalar Division
NOALIAS __forceinline const Scalar operator / ( const Scalar s1, const Scalar s2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar operator / ( const Scalar s1, const Scalar s2 )
{	
    Scalar result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.f = s1.f / s2.f;
    #elif defined( _AUDIO_MATHS_360_ )
    __vector4 rcp_s2_f;
    VECTOR_RECIPROCAL( s2.f, rcp_s2_f )
    result.f = __vmulfp( s1.f, rcp_s2_f );
    #else
    result.f = s1.f / s2.f;
    #endif
    return result;
}

/// Vector3 Division
NOALIAS __forceinline const Vector3 operator / ( const Vector3 v1, const Vector3 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 operator / ( const Vector3 v1, const Vector3 v2 )
{	
    Vector3 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.xyz = v1.xyz / v2.xyz;
    #elif defined( _AUDIO_MATHS_360_ )
    __vector4 rcp_v2_xyz;
    VECTOR_RECIPROCAL( v2.xyz, rcp_v2_xyz )
    result.xyz = __vmulfp( v1.xyz, rcp_v2_xyz );
    #else
    result.x = v1.x / v2.x;
    result.y = v1.y / v2.y;
    result.z = v1.z / v2.z;
    #endif
    return result;
}

/// Vector4 Division
NOALIAS __forceinline const Vector4 operator / ( const Vector4 v1, const Vector4 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 operator / ( const Vector4 v1, const Vector4 v2 )
{	
    Vector4 result;
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = v1.xyzw / v2.xyzw;
    #elif defined( _AUDIO_MATHS_360_ )
    __vector4 rcp_v2_xyzw;
    VECTOR_RECIPROCAL( v2.xyzw, rcp_v2_xyzw )
    result.xyzw = __vmulfp( v1.xyzw, rcp_v2_xyzw );
    #else
    result.x = v1.x / v2.x;
    result.y = v1.y / v2.y;
    result.z = v1.z / v2.z;
    result.w = v1.w / v2.w;
    #endif
    return result;
}

/// Scalar Division Update
NOALIAS __forceinline Scalar& operator /= ( Scalar& s1, const Scalar s2 ) __attribute__(( always_inline ));
NOALIAS __forceinline Scalar& operator /= ( Scalar& s1, const Scalar s2 )
{	
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    s1.f = s1.f / s2.f;
    #elif defined( _AUDIO_MATHS_360_ )
    __vector4 rcp_s2_f;
    VECTOR_RECIPROCAL( s2.f, rcp_s2_f )
    s1.f = __vmulfp( s1.f, rcp_s2_f );
    #else
    s1.f /= s2.f;
    #endif
    return s1;
}

/// Vector3 Division Update
NOALIAS __forceinline Vector3& operator /= ( Vector3& v1, const Vector3 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline Vector3& operator /= ( Vector3& v1, const Vector3 v2 )
{	
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    v1.xyz = v1.xyz / v2.xyz;
    #elif defined( _AUDIO_MATHS_360_ )
    __vector4 rcp_v2_xyz;
    VECTOR_RECIPROCAL( v2.xyz, rcp_v2_xyz )
    v1.xyz = __vmulfp( v1.xyz, rcp_v2_xyz );
    #else
    v1.x /= v2.x;
    v1.y /= v2.y;
    v1.z /= v2.z;
    #endif
    return v1;
}

/// Vector3 Scalar Division
NOALIAS __forceinline const Vector3 operator / ( const Vector3 v, const Scalar s ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 operator / ( const Vector3 v, const Scalar s )
{	
    Vector3 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyz = v.xyz / s.f;
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyz = v.xyz / s.f;
    #elif defined( _AUDIO_MATHS_360_ )
    __vector4 rcp_s_f;
    VECTOR_RECIPROCAL( s.f, rcp_s_f )
    result.xyz = __vmulfp( v.xyz, rcp_s_f );
    #else
    const float rcp_s_f = 1.0f / s.f;
    result.x = v.x * rcp_s_f;
    result.y = v.y * rcp_s_f;
    result.z = v.z * rcp_s_f;
    #endif
    return result;
}

/// Vector3 Scalar Division Update
NOALIAS __forceinline Vector3& operator /= ( Vector3& v, const Scalar s ) __attribute__(( always_inline ));
NOALIAS __forceinline Vector3& operator /= ( Vector3& v, const Scalar s )
{	
    #if defined( _AUDIO_MATHS_SPU_ )
    v.xyz = v.xyz / s.f;
    #elif defined( _AUDIO_MATHS_PPU_ )
    v.xyz = v.xyz / s.f;
    #elif defined( _AUDIO_MATHS_360_ )
    __vector4 rcp_s_f;
    VECTOR_RECIPROCAL( s.f, rcp_s_f )
    v.xyz = __vmulfp( v.xyz, rcp_s_f );
    #else
    const float rcp_s_f = 1.0f / s.f;
    v.x *= rcp_s_f;
    v.y *= rcp_s_f;
    v.z *= rcp_s_f;
    #endif
    return v;
}

/// Vector3 Reciprocal Estimate
NOALIAS __forceinline const Vector3 RcpEst( const Vector3 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 RcpEst( const Vector3 v )
{	
    Vector3 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyz = spu_re( v.xyz );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyz = vec_re( v.xyz );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyz = __vrefp( v.xyz );
    #else
    result.x = 1.0f / v.x;
    result.y = 1.0f / v.y;
    result.z = 1.0f / v.z;
    #endif
    return result;
}

/// Vector4 Reciprocal Estimate
NOALIAS __forceinline const Vector4 RcpEst( const Vector4 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 RcpEst( const Vector4 v )
{	
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_re( v.xyzw );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_re( v.xyzw );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vrefp( v.xyzw );
    #else
    result.x = 1.0f / v.x;
    result.y = 1.0f / v.y;
    result.z = 1.0f / v.z;
    result.w = 1.0f / v.w;
    #endif
    return result;
}

/// Vector3 Multiply Add : a * b + c
NOALIAS __forceinline const Vector3 MulAdd( const Vector3 a, const Vector3 b, const Vector3 c ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 MulAdd( const Vector3 a, const Vector3 b, const Vector3 c )
{
    Vector3 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyz = spu_madd( a.xyz, b.xyz, c.xyz );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyz = vec_madd( a.xyz, b.xyz, c.xyz );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyz = __vmaddfp( a.xyz, b.xyz, c.xyz );
    #else
    result.x = a.x * b.x + c.x;
    result.y = a.y * b.y + c.y;
    result.z = a.z * b.z + c.z;
    #endif
    return result;
}

/// Vector4 Multiply Add : a * b + c
NOALIAS __forceinline const Vector4 MulAdd( const Vector4 a, const Vector4 b, const Vector4 c ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 MulAdd( const Vector4 a, const Vector4 b, const Vector4 c )
{
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_madd( a.xyzw, b.xyzw, c.xyzw );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_madd( a.xyzw, b.xyzw, c.xyzw );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vmaddfp( a.xyzw, b.xyzw, c.xyzw );
    #else
    result.x = a.x * b.x + c.x;
    result.y = a.y * b.y + c.y;
    result.z = a.z * b.z + c.z;
    result.w = a.w * b.w + c.w;
    #endif
    return result;
}

/// Vector3 Multiply Subtract : a * b - c
NOALIAS __forceinline const Vector3 MulSub( const Vector3 a, const Vector3 b, const Vector3 c ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 MulSub( const Vector3 a, const Vector3 b, const Vector3 c )
{
    Vector3 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyz = spu_msub( a.xyz, b.xyz, c.xyz );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyz = vec_madd( a.xyz, b.xyz, -c.xyz );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyz = __vmaddfp( a.xyz, b.xyz, __vxor( c.xyz, g_VectorSignMask ) );
    #else
    result.x = a.x * b.x - c.x;
    result.y = a.y * b.y - c.y;
    result.z = a.z * b.z - c.z;
    #endif
    return result;
}

/// Vector4 Multiply Subtract : a * b - c
NOALIAS __forceinline const Vector4 MulSub( const Vector4 a, const Vector4 b, const Vector4 c ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 MulSub( const Vector4 a, const Vector4 b, const Vector4 c )
{
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_msub( a.xyzw, b.xyzw, c.xyzw );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_madd( a.xyzw, b.xyzw, -c.xyzw );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vmaddfp( a.xyzw, b.xyzw, __vxor( c.xyzw, g_VectorSignMask ) );
    #else
    result.x = a.x * b.x - c.x;
    result.y = a.y * b.y - c.y;
    result.z = a.z * b.z - c.z;
    result.w = a.w * b.w - c.w;
    #endif
    return result;
}

/// Vector3 Negative Multiply Add : -( a * b + c )
NOALIAS __forceinline const Vector3 NegMulAdd( const Vector3 a, const Vector3 b, const Vector3 c ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 NegMulAdd( const Vector3 a, const Vector3 b, const Vector3 c )
{
    Vector3 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyz = -spu_madd( a.xyz, b.xyz, c.xyz );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyz = -vec_madd( a.xyz, b.xyz, c.xyz );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyz = __vxor( __vmaddfp( a.xyz, b.xyz, c.xyz ), g_VectorSignMask );
    #else
    result.x = -( a.x * b.x + c.x );
    result.y = -( a.y * b.y + c.y );
    result.z = -( a.z * b.z + c.z );
    #endif
    return result;
}

/// Vector4 Negative Multiply Add : -( a * b + c )
NOALIAS __forceinline const Vector4 NegMulAdd( const Vector4 a, const Vector4 b, const Vector4 c ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 NegMulAdd( const Vector4 a, const Vector4 b, const Vector4 c )
{
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = -spu_madd( a.xyzw, b.xyzw, c.xyzw );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = -vec_madd( a.xyzw, b.xyzw, c.xyzw );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vxor( __vmaddfp( a.xyzw, b.xyzw, c.xyzw ), g_VectorSignMask );
    #else
    result.x = -( a.x * b.x + c.x );
    result.y = -( a.y * b.y + c.y );
    result.z = -( a.z * b.z + c.z );
    result.w = -( a.w * b.w + c.w );
    #endif
    return result;
}

/// Vector3 Negative Multiply Subtract : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vector3 NegMulSub( const Vector3 a, const Vector3 b, const Vector3 c ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 NegMulSub( const Vector3 a, const Vector3 b, const Vector3 c )
{
    Vector3 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyz = spu_nmsub( a.xyz, b.xyz, c.xyz );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyz = vec_nmsub( a.xyz, b.xyz, c.xyz );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyz = __vnmsubfp( a.xyz, b.xyz, c.xyz );
    #else
    result.x = c.x - a.x * b.x;
    result.y = c.y - a.y * b.y;
    result.z = c.z - a.z * b.z;
    #endif
    return result;
}

/// Vector4 Negative Multiply Subtract : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vector4 NegMulSub( const Vector4 a, const Vector4 b, const Vector4 c ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 NegMulSub( const Vector4 a, const Vector4 b, const Vector4 c )
{
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_nmsub( a.xyzw, b.xyzw, c.xyzw );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_nmsub( a.xyzw, b.xyzw, c.xyzw );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vnmsubfp( a.xyzw, b.xyzw, c.xyzw );
    #else
    result.x = c.x - a.x * b.x;
    result.y = c.y - a.y * b.y;
    result.z = c.z - a.z * b.z;
    result.w = c.w - a.w * b.w;
    #endif
    return result;
}

/// Vector3 Dot Product
NOALIAS __forceinline const Scalar operator % ( const Vector3 v1, const Vector3 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar operator % ( const Vector3 v1, const Vector3 v2 )
{	
    Scalar result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.f = vector3_dot_product( v1.xyz, v2.xyz );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.f = vector3_dot_product( v1.xyz, v2.xyz );
    #elif defined( _AUDIO_MATHS_360_ )
    result.f = __vmsum3fp( v1.xyz, v2.xyz );
    #else
    result.f = v1.x * v2.x +
               v1.y * v2.y +
               v1.z * v2.z;
    #endif
    return result;
}

/// Vector4 Dot Product
NOALIAS __forceinline const Scalar operator % ( const Vector4 v1, const Vector4 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar operator % ( const Vector4 v1, const Vector4 v2 )
{	
    Scalar result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.f = vector4_dot_product( v1.xyzw, v2.xyzw );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.f = vector4_dot_product( v1.xyzw, v2.xyzw );
    #elif defined( _AUDIO_MATHS_360_ )
    result.f = __vmsum4fp( v1.xyzw, v2.xyzw );
    #else
    result.f = v1.x * v2.x +
               v1.y * v2.y +
               v1.z * v2.z +
               v1.w * v2.w;
    #endif
    return result;
}

/// Vector3 Cross Product
NOALIAS __forceinline const Vector3 operator ^ ( const Vector3 v1, const Vector3 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 operator ^ ( const Vector3 v1, const Vector3 v2 )
{	
    Vector3 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyz = vector3_cross_product( v1.xyz, v2.xyz );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyz = vector3_cross_product( v1.xyz, v2.xyz );
    #elif defined( _AUDIO_MATHS_360_ )
    const __vector4 v1_yzx( __vpermwi( v1.xyz, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
    const __vector4 v1_zxy( __vpermwi( v1.xyz, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
    const __vector4 v2_yzx( __vpermwi( v2.xyz, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
    const __vector4 v2_zxy( __vpermwi( v2.xyz, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
    result.xyz = __vnmsubfp( v1_zxy, v2_yzx, __vmulfp( v1_yzx, v2_zxy ) );
    #else
    result.x = v1.y * v2.z - v2.y * v1.z;
	result.y = v1.z * v2.x - v2.z * v1.x;
	result.z = v1.x * v2.y - v2.x * v1.y;
    #endif
    return result;
}

/// Vector3 Cross Product Update
NOALIAS __forceinline Vector3& operator ^= ( Vector3& v1, const Vector3 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline Vector3& operator ^= ( Vector3& v1, const Vector3 v2 )
{	
    v1 = v1 ^ v2;
    return v1;
}

/// Scalar Min
NOALIAS __forceinline const Scalar Min( const Scalar s1, const Scalar s2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar Min( const Scalar s1, const Scalar s2 )
{	
    Scalar result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.f = vector_min( s1.f, s2.f );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.f = vec_min( s1.f, s2.f );
    #elif defined( _AUDIO_MATHS_360_ )
    result.f = __vminfp( s1.f, s2.f );
    #else
    result.f = s1.f <= s2.f ? s1.f : s2.f;
    #endif
    return result;
}

/// Scalar Max
NOALIAS __forceinline const Scalar Max( const Scalar s1, const Scalar s2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar Max( const Scalar s1, const Scalar s2 )
{	
    Scalar result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.f = vector_max( s1.f, s2.f );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.f = vec_max( s1.f, s2.f );
    #elif defined( _AUDIO_MATHS_360_ )
    result.f = __vmaxfp( s1.f, s2.f );
    #else
    result.f = s1.f >= s2.f ? s1.f : s2.f;
    #endif
    return result;
}

/// Vector3 Min
NOALIAS __forceinline const Vector3 Min( const Vector3 v1, const Vector3 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 Min( const Vector3 v1, const Vector3 v2 )
{	
    Vector3 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyz = vector_min( v1.xyz, v2.xyz );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyz = vec_min( v1.xyz, v2.xyz );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyz = __vminfp( v1.xyz, v2.xyz );
    #else
    result.x = v1.x <= v2.x ? v1.x : v2.x;
    result.y = v1.y <= v2.y ? v1.y : v2.y;
    result.z = v1.z <= v2.z ? v1.z : v2.z;
    #endif
    return result;
}

/// Vector3 Max
NOALIAS __forceinline const Vector3 Max( const Vector3 v1, const Vector3 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 Max( const Vector3 v1, const Vector3 v2 )
{	
    Vector3 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyz = vector_max( v1.xyz, v2.xyz );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyz = vec_max( v1.xyz, v2.xyz );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyz = __vmaxfp( v1.xyz, v2.xyz );
    #else
    result.x = v1.x >= v2.x ? v1.x : v2.x;
    result.y = v1.y >= v2.y ? v1.y : v2.y;
    result.z = v1.z >= v2.z ? v1.z : v2.z;
    #endif
    return result;
}

/// Vector4 Min
NOALIAS __forceinline const Vector4 Min( const Vector4 v1, const Vector4 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 Min( const Vector4 v1, const Vector4 v2 )
{	
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = vector_min( v1.xyzw, v2.xyzw );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_min( v1.xyzw, v2.xyzw );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vminfp( v1.xyzw, v2.xyzw );
    #else
    result.x = v1.x <= v2.x ? v1.x : v2.x;
    result.y = v1.y <= v2.y ? v1.y : v2.y;
    result.z = v1.z <= v2.z ? v1.z : v2.z;
    result.w = v1.w <= v2.w ? v1.w : v2.w;
    #endif
    return result;
}

/// Vector4 Max
NOALIAS __forceinline const Vector4 Max( const Vector4 v1, const Vector4 v2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 Max( const Vector4 v1, const Vector4 v2 )
{	
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = vector_max( v1.xyzw, v2.xyzw );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_max( v1.xyzw, v2.xyzw );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vmaxfp( v1.xyzw, v2.xyzw );
    #else
    result.x = v1.x >= v2.x ? v1.x : v2.x;
    result.y = v1.y >= v2.y ? v1.y : v2.y;
    result.z = v1.z >= v2.z ? v1.z : v2.z;
    result.w = v1.w >= v2.w ? v1.w : v2.w;
    #endif
    return result;
}

/// Scalar Clamp
NOALIAS __forceinline const Scalar Clamp( const Scalar s, const Scalar lo, const Scalar hi ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar Clamp( const Scalar s, const Scalar lo, const Scalar hi )
{
    return Min( Max( s, lo ), hi );
}

/// Vector3 Clamp
NOALIAS __forceinline const Vector3 Clamp( const Vector3 s, const Vector3 lo, const Vector3 hi ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 Clamp( const Vector3 s, const Vector3 lo, const Vector3 hi )
{
    return Min( Max( s, lo ), hi );
}

/// Vector4 Clamp
NOALIAS __forceinline const Vector4 Clamp( const Vector4 s, const Vector4 lo, const Vector4 hi ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 Clamp( const Vector4 s, const Vector4 lo, const Vector4 hi )
{
    return Min( Max( s, lo ), hi );
}

/// Vector3 Abs 
NOALIAS __forceinline const Vector3 Abs( const Vector3 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 Abs( const Vector3 v )
{	
    Vector3 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyz = spu_and( v.xyz, g_VectorAbsMask );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyz = vec_abs( v.xyz );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyz = __vand( v.xyz, g_VectorAbsMask );
    #else
    result.x = Abs( v.x );
    result.y = Abs( v.y );
    result.z = Abs( v.z );
    #endif
    return result;
}

/// Vector4 Abs 
NOALIAS __forceinline const Vector4 Abs( const Vector4 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 Abs( const Vector4 v )
{	
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_and( v.xyzw, g_VectorAbsMask );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_abs( v.xyzw );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vand( v.xyzw, g_VectorAbsMask );
    #else
    result.x = Abs( v.x );
    result.y = Abs( v.y );
    result.z = Abs( v.z );
    result.w = Abs( v.w );
    #endif
    return result;
}

/// Vector3 Sign 
NOALIAS __forceinline const Vector3 Sign( const Vector3 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 Sign( const Vector3 v )
{	
    Vector3 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyz = spu_sel( g_VectorOne, v.xyz, g_VectorSignMask );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyz = vec_sel( g_VectorOne, v.xyz, g_VectorSignMask );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyz = __vsel( g_VectorOne, v.xyz, g_VectorSignMask );
    #else
    result.x = (float)_copysign( 1.0f, v.x );
    result.y = (float)_copysign( 1.0f, v.y );
    result.z = (float)_copysign( 1.0f, v.z );
    #endif
    return result;
}

/// Vector4 Sign 
NOALIAS __forceinline const Vector4 Sign( const Vector4 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector4 Sign( const Vector4 v )
{	
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyzw = spu_sel( g_VectorOne, v.xyzw, g_VectorSignMask );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_sel( g_VectorOne, v.xyzw, g_VectorSignMask );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vsel( g_VectorOne, v.xyzw, g_VectorSignMask );
    #else
    result.x = (float)_copysign( 1.0f, v.x );
    result.y = (float)_copysign( 1.0f, v.y );
    result.z = (float)_copysign( 1.0f, v.z );
    result.w = (float)_copysign( 1.0f, v.w );
    #endif
    return result;
}

///	Magnitude Squared
NOALIAS __forceinline const Scalar Magnitude2( const Vector3 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar Magnitude2( const Vector3 v )
{	
    Scalar result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.f = vector3_dot_product( v.xyz, v.xyz );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.f = vector3_dot_product( v.xyz, v.xyz );
    #elif defined( _AUDIO_MATHS_360_ )
    result.f = __vmsum3fp( v.xyz, v.xyz );
    #else
    result.f = v.x * v.x +
               v.y * v.y +
               v.z * v.z;
    #endif
    return result;
}

/// Magnitude
NOALIAS __forceinline const Scalar operator ! ( const Vector3 v ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar operator ! ( const Vector3 v )
{	
    Scalar result;
    #if defined( _AUDIO_MATHS_SPU_ )
    const vector float magnitude2( vector3_dot_product( v.xyz, v.xyz ) );
    const vector float rcp_magnitude( vector_reciprocal_sqrt( magnitude2 ) );
    result.f = spu_mul( magnitude2, rcp_magnitude );
    #elif defined( _AUDIO_MATHS_PPU_ )
    const vector float magnitude2( vector3_dot_product( v.xyz, v.xyz ) );
    const vector float rcp_magnitude( vector_reciprocal_sqrt( magnitude2 ) );
    result.f = magnitude2 * rcp_magnitude;
    #elif defined( _AUDIO_MATHS_360_ )
    const __vector4 magnitude2( __vmsum3fp( v.xyz, v.xyz ) );
    __vector4 rcp_magnitude;
    VECTOR_RECIPROCAL_SQRT( magnitude2, rcp_magnitude );
    result.f = __vmulfp( magnitude2, rcp_magnitude );
    #else
    result.f = Sqrt( v.x * v.x +
                     v.y * v.y +
                     v.z * v.z );
    #endif
    return result;
}

/// Set Length ( 1 = normalise )
NOALIAS __forceinline const Vector3 operator | ( const Vector3 v, const Scalar dLength ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 operator | ( const Vector3 v, const Scalar dLength )
{	
    #if defined( _AUDIO_MATHS_SPU_ )
    Vector3 result;
    const vector float magnitude2( vector3_dot_product( v.xyz, v.xyz ) );
    const vector float rcp_magnitude( vector_reciprocal_sqrt( magnitude2 ) );
    result.xyz = v.xyz * dLength.f * rcp_magnitude;
    return result;
    #elif defined( _AUDIO_MATHS_PPU_ )
    Vector3 result;
    const vector float magnitude2( vector3_dot_product( v.xyz, v.xyz ) );
    const vector float rcp_magnitude( vector_reciprocal_sqrt( magnitude2 ) );
    result.xyz = v.xyz * dLength.f * rcp_magnitude;
    return result;
    #elif defined( _AUDIO_MATHS_360_ )
    Vector3 result;
    const __vector4 magnitude2( __vmsum3fp( v.xyz, v.xyz ) );
    __vector4 rcp_magnitude;
    VECTOR_RECIPROCAL_SQRT( magnitude2, rcp_magnitude );
    __vmulfp( magnitude2, rcp_magnitude );
    result.xyz = __vmulfp( v.xyz, __vmulfp( dLength.f, rcp_magnitude ) );
    return result;
    #else
    return v * ( dLength / !v );
    #endif
}

/// Set Length Update ( 1 = normalise )
NOALIAS __forceinline Vector3& operator |= ( Vector3& v, const Scalar dLength ) __attribute__(( always_inline ));
NOALIAS __forceinline Vector3& operator |= ( Vector3& v, const Scalar dLength )
{
    v = v | dLength;
    return v;
}

/// Angle between two Vectors in radians ( BOTH parameters must be normalised! )
NOALIAS __forceinline const float Angle( const Vector3 normal1, const Vector3 normal2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const float Angle( const Vector3 normal1, const Vector3 normal2 )
{	
    // TODO: PPU/SPU versions!
    
    const F32 fDot = _float( normal1 % normal2 ); // TODO: Optimisation - keep this in a Scalar!
    if ( fDot > 1.f )
        return 0.f;
    else if ( fDot < -1.f )
        return 3.1415926535897932384626433832795f;
    else
        return ArcCos( fDot );
}

/// Reflect vector in plane of normal
NOALIAS __forceinline const Vector3 Reflect( const Vector3 vector, const Vector3 normal ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 Reflect( const Vector3 vector, const Vector3 normal )
{	
    // TODO: PPU/SPU versions!
    
    //return vector - normal * 2.0f * ( vector % normal );
    return vector - normal * _Scalar( g_VectorTwo ) * ( vector % normal );
}

/// Rotate dAngle radians Around Normal
NOALIAS __forceinline const Vector3 Rotate( const Vector3 vector, const float dAngle, const Vector3 normal ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 Rotate( const Vector3 vector, const float dAngle, const Vector3 normal )
{	
    const Scalar dCos = _Scalar( Cos( dAngle ) );
    const Scalar dSin = _Scalar( Sin( dAngle ) );
    const Scalar one  = _Scalar( g_VectorOne );
    
    return vector * dCos +
           normal * ( ( vector % normal ) * ( one - dCos ) ) +
           ( vector ^ normal ) * dSin;
}

/// RotateLeft: Rotate the concatenation of 2 vectors left by N elements, returning the leftmost 4 elements as the result vector
template < unsigned char N > NOALIAS __forceinline const Vector4 RotateLeft( const Vector4 v1, const Vector4 v2 ) __attribute__(( always_inline ));
template < unsigned char N > NOALIAS __forceinline const Vector4 RotateLeft( const Vector4 v1, const Vector4 v2 )
{
    Vector4 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    const vector unsigned char shuffle_pattern = 
    {
         0+N,  1+N,  2+N,  3+N,
         4+N,  5+N,  6+N,  7+N,
         8+N,  9+N, 10+N, 11+N,
        12+N, 13+N, 14+N, 15+N
    };
    result.xyzw = spu_shuffle( v1.xyzw, v2.xyzw, shuffle_pattern );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyzw = vec_sld( v1.xyzw, v2.xyzw, N*4 );
    #elif defined( _AUDIO_MATHS_360_ )
    result.xyzw = __vsldoi( v1.xyzw, v2.xyzw, N*4 );
    #else
    //assert( 0 ); // TODO!
    F32 temp[8] = { v1.x, v1.y, v1.z, v1.w, v2.x, v2.y, v2.z, v2.w };
    result.x = temp[N+0];
    result.y = temp[N+1];
    result.z = temp[N+2];
    result.w = temp[N+3];
    #endif
    return result;
}

///// 2* fast downsample
//NOALIAS __forceinline const Vector3 Downsample( const Vector3 v1, const Vector3 v2 ) __attribute__(( always_inline ));
//NOALIAS __forceinline const Vector3 Downsample( const Vector3 v1, const Vector3 v2 )
//{	
//    const Scalar dCos = _Scalar( Cos( dAngle ) );
//    const Scalar dSin = _Scalar( Sin( dAngle ) );
//    const Scalar one  = _Scalar( g_VectorOne );
//    
//    return vector * dCos +
//           normal * ( ( vector % normal ) * ( one - dCos ) ) +
//           ( vector ^ normal ) * dSin;
//}

////////////////////////////////////////////////////////////////////////////////
//	Matrix43
////////////////////////////////////////////////////////////////////////////////

class Matrix43
{
public:
    
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    vector float AX;
    vector float AY;
    vector float AZ;
    vector float Pos;
    #elif defined( _AUDIO_MATHS_360_ )
    Vector4 CX; // Stored in column major format for Xenon/360!
    Vector4 CY;
    Vector4 CZ;
    #else
    Vector3 AX;
	Vector3 AY;
	Vector3 AZ;
	Vector3 Pos;
	#endif
	
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    /// PS3 vector float constructor
    inline explicit Matrix43( const vector float pos,
                              const vector float ax,
                              const vector float ay,
                              const vector float az ) __attribute__(( always_inline ));
    #elif defined( _AUDIO_MATHS_360_ )
    NOALIAS __forceinline explicit Matrix43( const __vector4 cx,
                                             const __vector4 cy,
                                             const __vector4 cz );
    #endif
    
    NOALIAS __forceinline Matrix43() __attribute__(( always_inline ));
    
    NOALIAS __forceinline Matrix43( const float& x, const float& y, const float& z ) __attribute__(( always_inline ));
    
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ ) || defined( _AUDIO_MATHS_360_ )
    NOALIAS __forceinline Matrix43( const Vector3 pos,
                                    const Vector3 ax = MakeVector3( g_VectorUnitX ),
                                    const Vector3 ay = MakeVector3( g_VectorUnitY ),
                                    const Vector3 az = MakeVector3( g_VectorUnitZ ) ) __attribute__(( always_inline ));
    #else
    NOALIAS __forceinline Matrix43( const Vector3 pos,
                                    const Vector3 ax = MakeVector3( 1.0f, 0.0f, 0.0f ),
                                    const Vector3 ay = MakeVector3( 0.0f, 1.0f, 0.0f ),
                                    const Vector3 az = MakeVector3( 0.0f, 0.0f, 1.0f ) );
    #endif
	
    NOALIAS __forceinline Matrix43( const Matrix43& matrix ) __attribute__(( always_inline ));

    /// Matrix43 Assignment
    NOALIAS __forceinline const Matrix43& operator = ( const Matrix43& m ) __attribute__(( always_inline ));
    
    /// Vector3 Assignment
    NOALIAS __forceinline const Matrix43& operator = ( const Vector3 v ) __attribute__(( always_inline ));
    
    //	boolean operators
    //const bool operator == ( const Matrix43& matrix ) const;
    //const bool operator != ( const Matrix43& matrix ) const;

    //	+, -
    const Matrix43 operator + ( const Vector3 vector ) const;
    const Matrix43 operator - ( const Vector3 vector ) const;
    const Matrix43& operator += ( const Vector3 vector );
    const Matrix43& operator -= ( const Vector3 vector );

    //	*
    const Matrix43 operator * ( const Matrix43& matrix ) const;
    const Matrix43 operator * ( const Vector3 vector ) const;
    const Matrix43 operator * ( const float& scale ) const;
    const Matrix43& operator *= ( const Matrix43& matrix );
    const Matrix43& operator *= ( const Vector3 vector );
    const Matrix43& operator *= ( const float& scale );

    const Matrix43 operator ! () const;	//	inverse

    void SetIdentity()
    {
        #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
        AX  = g_VectorUnitX;
        AY  = g_VectorUnitY;
        AZ  = g_VectorUnitZ;
        Pos = g_VectorZero;
        #elif defined( _AUDIO_MATHS_360_ )
        CX.xyzw = g_VectorUnitX;
        CY.xyzw = g_VectorUnitY;
        CZ.xyzw = g_VectorUnitZ;
        #else
        AX  = MakeVector3( 1.0f, 0.0f, 0.0f );
        AY  = MakeVector3( 0.0f, 1.0f, 0.0f );
        AZ  = MakeVector3( 0.0f, 0.0f, 1.0f );
        Pos = MakeVector3( 0.0f, 0.0f, 0.0f );
        #endif
    }
    
    void RotateX180()
    {
        #if defined( _AUDIO_MATHS_360_ )
        CX.xyzw = __vxor( CX.xyzw, g_VectorSignMaskYZ );
        CY.xyzw = __vxor( CY.xyzw, g_VectorSignMaskYZ );
        CZ.xyzw = __vxor( CZ.xyzw, g_VectorSignMaskYZ );
        #else
        AY = -AY;
        AZ = -AZ;
        #endif
    }
    
    void RotateY180()
    {
        #if defined( _AUDIO_MATHS_360_ )
        CX.xyzw = __vxor( CX.xyzw, g_VectorSignMaskXZ );
        CY.xyzw = __vxor( CY.xyzw, g_VectorSignMaskXZ );
        CZ.xyzw = __vxor( CZ.xyzw, g_VectorSignMaskXZ );
        #else
        AX = -AX;
        AZ = -AZ;
        #endif
    }

    void RotateZ180()
    {
        #if defined( _AUDIO_MATHS_360_ )
        CX.xyzw = __vxor( CX.xyzw, g_VectorSignMaskXY );
        CY.xyzw = __vxor( CY.xyzw, g_VectorSignMaskXY );
        CZ.xyzw = __vxor( CZ.xyzw, g_VectorSignMaskXY );
        #else
        AX = -AX;
        AY = -AY;
        #endif
    }

    //	standard matrices
    const Matrix43 RotateX( const float dAngle ) const;
    const Matrix43 RotateY( const float dAngle ) const;
    const Matrix43 RotateZ( const float dAngle ) const;
    const Matrix43 Identity() const;
    const Matrix43 Billboard() const;

    //	tranform a Vector3
    NOALIAS __forceinline const Vector3 Transform( const Vector3 v ) const __attribute__(( always_inline ));
    
    NOALIAS __forceinline const Vector3 Transform( float& x, float& y, float& z ) const __attribute__(( always_inline ));
    
    const Vector3 Rotate( const Vector3 vector ) const;
};

typedef Matrix43 Matrix;

#if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
/// PS3 vector float constructor
inline Matrix43::Matrix43( const vector float pos,
                           const vector float ax,
                           const vector float ay,
                           const vector float az )
    : AX ( ax  )
    , AY ( ay  )
    , AZ ( az  )
    , Pos( pos )
{
}
#elif defined( _AUDIO_MATHS_360_ )
NOALIAS __forceinline Matrix43::Matrix43( const __vector4 cx,
                                          const __vector4 cy,
                                          const __vector4 cz )
    : CX( MakeVector4( cx ) )
    , CY( MakeVector4( cy ) )
    , CZ( MakeVector4( cz ) )
{
}
#endif

NOALIAS __forceinline Matrix43::Matrix43()
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    : AX ( g_VectorUnitX )
    , AY ( g_VectorUnitY )
    , AZ ( g_VectorUnitZ )
    , Pos( g_VectorZero )
    #elif defined( _AUDIO_MATHS_360_ )
    : CX( MakeVector4( g_VectorUnitX ) )
    , CY( MakeVector4( g_VectorUnitY ) )
    , CZ( MakeVector4( g_VectorUnitZ ) )
    #else
    : AX ( MakeVector3( 1.0f, 0.0f, 0.0f ) )
    , AY ( MakeVector3( 0.0f, 1.0f, 0.0f ) )
    , AZ ( MakeVector3( 0.0f, 0.0f, 1.0f ) )
    , Pos( MakeVector3( 0.0f, 0.0f, 0.0f ) )
    #endif
{
}

NOALIAS __forceinline Matrix43::Matrix43( const float& x, const float& y, const float& z )
    #if defined( _AUDIO_MATHS_SPU_ )
    : AX ( g_VectorUnitX )
    , AY ( g_VectorUnitY )
    , AZ ( g_VectorUnitZ )
    , Pos( make_vector3( x, y, z ) )
    #elif defined( _AUDIO_MATHS_PPU_ )
    : AX ( g_VectorUnitX )
    , AY ( g_VectorUnitY )
    , AZ ( g_VectorUnitZ )
    , Pos( make_vector3( x, y, z ) )
    #elif defined( _AUDIO_MATHS_360_ )
    : CX( MakeVector4( __vrlimi( g_VectorUnitX, __lvlx( &x, 0x0 ), MASK_INSERT_W, 1 ) ) )
    , CY( MakeVector4( __vrlimi( g_VectorUnitY, __lvlx( &y, 0x0 ), MASK_INSERT_W, 1 ) ) )
    , CZ( MakeVector4( __vrlimi( g_VectorUnitZ, __lvlx( &z, 0x0 ), MASK_INSERT_W, 1 ) ) )
    #else
    : AX ( MakeVector3( 1, 0, 0 ) )
    , AY ( MakeVector3( 0, 1, 0 ) )
    , AZ ( MakeVector3( 0, 0, 1 ) )
    , Pos( MakeVector3( x, y, z ) )
    #endif
{
}

NOALIAS __forceinline Matrix43::Matrix43( const Vector3 pos,
                                          const Vector3 ax,
                                          const Vector3 ay,
                                          const Vector3 az )
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    : AX ( ax.xyz  )
    , AY ( ay.xyz  )
    , AZ ( az.xyz  )
    , Pos( pos.xyz )
    #elif !defined( _AUDIO_MATHS_360_ )
    : AX ( ax  )
    , AY ( ay  )
    , AZ ( az  )
    , Pos( pos )
    #endif
{
    #if defined( _AUDIO_MATHS_360_ )
    const __vector4 tmp0( __vmrghw( ax.xyz, az.xyz  ) );
    const __vector4 tmp1( __vmrghw( ay.xyz, pos.xyz ) );
    const __vector4 tmp2( __vmrglw( ax.xyz, az.xyz  ) );
    const __vector4 tmp3( __vmrglw( ay.xyz, pos.xyz ) );
    CX.xyzw = __vmrghw( tmp0, tmp1 );
    CY.xyzw = __vmrglw( tmp0, tmp1 );
    CZ.xyzw = __vmrghw( tmp2, tmp3 );
    #endif
}

NOALIAS __forceinline Matrix43::Matrix43( const Matrix43& matrix )
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    : AX ( matrix.AX  )
    , AY ( matrix.AY  )
    , AZ ( matrix.AZ  )
    , Pos( matrix.Pos )
    #elif defined( _AUDIO_MATHS_360_ )
    : CX( matrix.CX )
    , CY( matrix.CY )
    , CZ( matrix.CZ )
    #else
    : AX ( matrix.AX  )
    , AY ( matrix.AY  )
    , AZ ( matrix.AZ  )
    , Pos( matrix.Pos )
    #endif
{
}

NOALIAS __forceinline const Matrix43& Matrix43::operator = ( const Matrix43& m )
{
    #if defined( _AUDIO_MATHS_360_ )
    CX.xyzw = m.CX.xyzw;
    CY.xyzw = m.CY.xyzw;
    CZ.xyzw = m.CZ.xyzw;
    #else
    AX  = m.AX;
    AY  = m.AY;
    AZ  = m.AZ;
    Pos = m.Pos;
    #endif
    return *this;
}

NOALIAS __forceinline const Matrix43& Matrix43::operator = ( const Vector3 v )
{
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    Pos = v.xyz;
    #elif defined( _AUDIO_MATHS_360_ )
    CX.xyzw = __vrlimi( CX.xyzw, v.xyz, MASK_INSERT_W, 1 );
    CY.xyzw = __vrlimi( CY.xyzw, v.xyz, MASK_INSERT_W, 2 );
    CZ.xyzw = __vrlimi( CZ.xyzw, v.xyz, MASK_INSERT_W, 3 );
    #else
    Pos = v;
    #endif
    return *this;
}

NOALIAS __forceinline const bool operator == ( const Matrix43& m1, const Matrix43& m2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const bool operator == ( const Matrix43& m1, const Matrix43& m2 )
{
    #if defined( _AUDIO_MATHS_SPU_ )
    const vector unsigned int vec_eq( spu_and( spu_and( spu_cmpeq( m1.AX,  m2.AX  ),
                                                        spu_cmpeq( m1.AY,  m2.AY  ) ),
                                               spu_and( spu_cmpeq( m1.AZ,  m2.AZ  ),
                                                        spu_cmpeq( m1.Pos, m2.Pos ) ) ) );
    return (bool)( vector_extract_x( vec_eq ) &
                   vector_extract_y( vec_eq ) &
                   vector_extract_z( vec_eq ) & 1 );
    #elif defined( _AUDIO_MATHS_PPU_ )
	return (bool)( vector3_all_eq( m1.AX,  m2.AX  ) &&
		           vector3_all_eq( m1.AY,  m2.AY  ) &&
		           vector3_all_eq( m1.AZ,  m2.AZ  ) &&
		           vector3_all_eq( m1.Pos, m2.Pos ) );
    #else
	return ( m1.AX  == m2.AX  ) &&
		   ( m1.AY  == m2.AY  ) &&
		   ( m1.AZ  == m2.AZ  ) &&
		   ( m1.Pos == m2.Pos );
    #endif
}

NOALIAS __forceinline const bool operator != ( const Matrix& m1, const Matrix& m2 ) __attribute__(( always_inline ));
NOALIAS __forceinline const bool operator != ( const Matrix& m1, const Matrix& m2 )
{
    #if defined( _AUDIO_MATHS_SPU_ )
    const vector unsigned int vec_eq( spu_and( spu_and( spu_cmpeq( m1.AX,  m2.AX  ),
                                                        spu_cmpeq( m1.AY,  m2.AY  ) ),
                                               spu_and( spu_cmpeq( m1.AZ,  m2.AZ  ),
                                                        spu_cmpeq( m1.Pos, m2.Pos ) ) ) );
    const vector unsigned int vec_ne( vector_not( vec_eq ) );
    return (bool)( ( vector_extract_x( vec_ne ) |
                     vector_extract_y( vec_ne ) |
                     vector_extract_z( vec_ne ) ) & 1 );
    #elif defined( _AUDIO_MATHS_PPU_ )
	return (bool)( vector3_any_ne( m1.AX,  m2.AX  ) ||
		           vector3_any_ne( m1.AY,  m2.AY  ) ||
		           vector3_any_ne( m1.AZ,  m2.AZ  ) ||
		           vector3_any_ne( m1.Pos, m2.Pos ) );
    #else
	return ( m1.AX  != m2.AX  ) ||
		   ( m1.AY  != m2.AY  ) ||
		   ( m1.AZ  != m2.AZ  ) ||
		   ( m1.Pos != m2.Pos );
    #endif
}

NOALIAS __forceinline const Vector3 AX( const Matrix43& m ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 AX( const Matrix43& m )
{
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    return MakeVector3( m.AX );
    #elif defined( _AUDIO_MATHS_360_ )
    Vector3 ax;
    //Vector3 ay;
    //Vector3 az;
    //Vector3 aw;
    const __vector4 tmp0( __vmrghw( m.CX.xyzw, m.CZ.xyzw   ) );
    const __vector4 tmp1( __vmrghw( m.CY.xyzw, __vzero() ) );
    //const __vector4 tmp2( __vmrglw( m.CX.xyzw, m.CZ.xyzw   ) );
    //const __vector4 tmp3( __vmrglw( m.CY.xyzw, __vzero() ) );
    ax.xyz = __vmrghw( tmp0, tmp1 );
    //ay.xyz = __vmrglw( tmp0, tmp1 );
    //az.xyz = __vmrghw( tmp2, tmp3 );
    //aw.xyz = __vmrglw( tmp2, tmp3 );
    return ax;
    #else
    return m.AX;
    #endif
}

NOALIAS __forceinline const Vector3 AY( const Matrix43& m ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 AY( const Matrix43& m )
{
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    return MakeVector3( m.AY );
    #elif defined( _AUDIO_MATHS_360_ )
    //Vector3 ax;
    Vector3 ay;
    //Vector3 az;
    //Vector3 aw;
    const __vector4 tmp0( __vmrghw( m.CX.xyzw, m.CZ.xyzw   ) );
    const __vector4 tmp1( __vmrghw( m.CY.xyzw, __vzero() ) );
    //const __vector4 tmp2( __vmrglw( m.CX.xyzw, m.CZ.xyzw   ) );
    //const __vector4 tmp3( __vmrglw( m.CY.xyzw, __vzero() ) );
    //ax.xyz = __vmrghw( tmp0, tmp1 );
    ay.xyz = __vmrglw( tmp0, tmp1 );
    //az.xyz = __vmrghw( tmp2, tmp3 );
    //aw.xyz = __vmrglw( tmp2, tmp3 );
    return ay;
    #else
    return m.AY;
    #endif
}

NOALIAS __forceinline const Vector3 AZ( const Matrix43& m ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 AZ( const Matrix43& m )
{
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    return MakeVector3( m.AZ );
    #elif defined( _AUDIO_MATHS_360_ )
    //Vector3 ax;
    //Vector3 ay;
    Vector3 az;
    //Vector3 aw;
    //const __vector4 tmp0( __vmrghw( m.CX.xyzw, m.CZ.xyzw   ) );
    //const __vector4 tmp1( __vmrghw( m.CY.xyzw, __vzero() ) );
    const __vector4 tmp2( __vmrglw( m.CX.xyzw, m.CZ.xyzw   ) );
    const __vector4 tmp3( __vmrglw( m.CY.xyzw, __vzero() ) );
    //ax.xyz = __vmrghw( tmp0, tmp1 );
    //ay.xyz = __vmrglw( tmp0, tmp1 );
    az.xyz = __vmrghw( tmp2, tmp3 );
    //aw.xyz = __vmrglw( tmp2, tmp3 );
    return az;
    #else
    return m.AZ;
    #endif
}

NOALIAS __forceinline const Vector3 Pos( const Matrix43& m ) __attribute__(( always_inline ));
NOALIAS __forceinline const Vector3 Pos( const Matrix43& m )
{
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    return MakeVector3( m.Pos );
    #elif defined( _AUDIO_MATHS_360_ )
    //Vector3 ax;
    //Vector3 ay;
    //Vector3 az;
    Vector3 aw;
    //const __vector4 tmp0( __vmrghw( m.CX.xyzw, m.CZ.xyzw   ) );
    //const __vector4 tmp1( __vmrghw( m.CY.xyzw, __vzero() ) );
    const __vector4 tmp2( __vmrglw( m.CX.xyzw, m.CZ.xyzw   ) );
    const __vector4 tmp3( __vmrglw( m.CY.xyzw, __vzero() ) );
    //ax.xyz = __vmrghw( tmp0, tmp1 );
    //ay.xyz = __vmrglw( tmp0, tmp1 );
    //az.xyz = __vmrghw( tmp2, tmp3 );
    aw.xyz = __vmrglw( tmp2, tmp3 );
    return aw;
    #else
    return m.Pos;
    #endif
}

NOALIAS __forceinline void SetAX( Matrix43& m, const Vector3 v ) __attribute__(( always_inline ));
NOALIAS __forceinline void SetAX( Matrix43& m, const Vector3 v )
{
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    m.AX = v.xyz;
    #elif defined( _AUDIO_MATHS_360_ )
    m.CX.xyzw = __vrlimi( m.CX.xyzw, v.xyz, MASK_INSERT_X, 0 );
    m.CY.xyzw = __vrlimi( m.CY.xyzw, v.xyz, MASK_INSERT_X, 1 );
    m.CZ.xyzw = __vrlimi( m.CZ.xyzw, v.xyz, MASK_INSERT_X, 2 );
    #else
    m.AX = v;
    #endif
}

NOALIAS __forceinline void SetAY( Matrix43& m, const Vector3 v ) __attribute__(( always_inline ));
NOALIAS __forceinline void SetAY( Matrix43& m, const Vector3 v )
{
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    m.AY = v.xyz;
    #elif defined( _AUDIO_MATHS_360_ )
    m.CX.xyzw = __vrlimi( m.CX.xyzw, v.xyz, MASK_INSERT_Y, 3 );
    m.CY.xyzw = __vrlimi( m.CY.xyzw, v.xyz, MASK_INSERT_Y, 0 );
    m.CZ.xyzw = __vrlimi( m.CZ.xyzw, v.xyz, MASK_INSERT_Y, 1 );
    #else
    m.AY = v;
    #endif
}

NOALIAS __forceinline void SetAZ( Matrix43& m, const Vector3 v ) __attribute__(( always_inline ));
NOALIAS __forceinline void SetAZ( Matrix43& m, const Vector3 v )
{
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    m.AZ = v.xyz;
    #elif defined( _AUDIO_MATHS_360_ )
    m.CX.xyzw = __vrlimi( m.CX.xyzw, v.xyz, MASK_INSERT_Z, 2 );
    m.CY.xyzw = __vrlimi( m.CY.xyzw, v.xyz, MASK_INSERT_Z, 3 );
    m.CZ.xyzw = __vrlimi( m.CZ.xyzw, v.xyz, MASK_INSERT_Z, 0 );
    #else
    m.AZ = v;
    #endif
}

NOALIAS __forceinline void SetPos( Matrix43& m, const Vector3 v ) __attribute__(( always_inline ));
NOALIAS __forceinline void SetPos( Matrix43& m, const Vector3 v )
{
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
    m.Pos = v.xyz;
    #elif defined( _AUDIO_MATHS_360_ )
    m.CX.xyzw = __vrlimi( m.CX.xyzw, v.xyz, MASK_INSERT_W, 1 );
    m.CY.xyzw = __vrlimi( m.CY.xyzw, v.xyz, MASK_INSERT_W, 2 );
    m.CZ.xyzw = __vrlimi( m.CZ.xyzw, v.xyz, MASK_INSERT_W, 3 );
    #else
    m.Pos = v;
    #endif
}

NOALIAS __forceinline const Vector3 Matrix43::Transform( const Vector3 v ) const
{
    Vector3 result;
    #if defined( _AUDIO_MATHS_SPU_ )
    result.xyz = spu_madd( vector_splat_x( v.xyz ), AX,
                 spu_madd( vector_splat_y( v.xyz ), AY,
                 spu_madd( vector_splat_z( v.xyz ), AZ, Pos ) ) );
    #elif defined( _AUDIO_MATHS_PPU_ )
    result.xyz = vec_madd( vector_splat_x( v.xyz ), AX,
                 vec_madd( vector_splat_y( v.xyz ), AY,
                 vec_madd( vector_splat_z( v.xyz ), AZ, Pos ) ) );
	#else
	result.x = v.x * AX.x + v.y * AY.x + v.z * AZ.x + Pos.x;
	result.y = v.x * AX.y + v.y * AY.y + v.z * AZ.y + Pos.y;
	result.z = v.x * AX.z + v.y * AY.z + v.z * AZ.z + Pos.z;
	#endif
	return result;
}

NOALIAS __forceinline const Vector3 Matrix43::Transform( float& x, float& y, float& z ) const
{	
    return Transform( MakeVector3( x, y, z ) );
}





void GenericDspLowPass1stOrder_x16(
    Vector4* __restrict pBuffer,
    const Scalar        vA,
    const Scalar        vB1,
    Vector4&            vPrevIn,
    Vector4&            vPrevOut,
    const U32           uSampleCount );



} // namespace Audio
} // namespace Maths

#endif	// _3D_MATHS_H_

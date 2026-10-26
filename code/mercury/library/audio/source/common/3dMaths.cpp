#include "AudioPch.h"
#include "..\common\3dmaths.h"

//using namespace Audio;
//using namespace Audio::Maths;

namespace Audio {
namespace Maths {

const F32 fPi             = 3.14159274f;
const F32 fHalfPi         = 1.57079633f;
const F32 fReciprocalPi   = 1.0f / 3.14159274f;
const F32 f2Pi            = 6.28318548f;
const F32 fMin            = 1.175494351e-38f;
const F32 fMax            = 3.402823466e+38f;
const F32 fEpsilon        = 0.0001f;
const F32 fMinus3dB       = 0.707106769f;
const F32 fMinus48dB      = 0.00390625f;
const F32 fLog10          = 2.302585093f;
const F32 fLog2           = 0.69314718056f;
const F32 fInvLog2        = 1.442695040889f;
const F32 fInv2pow15      = 3.0517578125e-5f;
const F32 fRoot2          = 1.41421356237f;
const F32 fInfinity       = IntBitsToFloat( 0x7f800000 );
const F32 fMinusInfinity  = IntBitsToFloat( 0xff800000 );
const F32 fInvalid        = IntBitsToFloat( 0xffffffff );

#if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )

const vector float g_VectorUnitX = { 1.0f, 0.0f, 0.0f, 0.0f };
const vector float g_VectorUnitY = { 0.0f, 1.0f, 0.0f, 0.0f };
const vector float g_VectorUnitZ = { 0.0f, 0.0f, 1.0f, 0.0f };
const vector float g_VectorUnitW = { 0.0f, 0.0f, 0.0f, 1.0f };

const vector float g_VectorZero     = {  0.0f,  0.0f,  0.0f,  0.0f };
const vector float g_VectorHalf     = {  0.5f,  0.5f,  0.5f,  0.5f };
const vector float g_VectorOne      = {  1.0f,  1.0f,  1.0f,  1.0f };
const vector float g_VectorMinusOne = { -1.0f, -1.0f, -1.0f, -1.0f };

const vector float g_VectorTwo       = {  2.0f,  2.0f,  2.0f,  2.0f };
const vector float g_VectorThree     = {  3.0f,  3.0f,  3.0f,  3.0f };
const vector float g_VectorFour      = {  4.0f,  4.0f,  4.0f,  4.0f };
const vector float g_VectorFive      = {  5.0f,  5.0f,  5.0f,  5.0f };
const vector float g_VectorEight     = {  8.0f,  8.0f,  8.0f,  8.0f };
const vector float g_VectorSixteen   = { 16.0f, 16.0f, 16.0f, 16.0f };
const vector float g_VectorThirtyTwo = { 32.0f, 32.0f, 32.0f, 32.0f };

const vector float g_VectorFloat_00_01_02_03 = {  0.0f,  1.0f,  2.0f,  3.0f };
const vector float g_VectorFloat_04_05_06_07 = {  4.0f,  5.0f,  6.0f,  7.0f };
const vector float g_VectorFloat_08_09_0A_0B = {  8.0f,  9.0f, 10.0f, 11.0f };
const vector float g_VectorFloat_0C_0D_0E_0F = { 12.0f, 13.0f, 14.0f, 15.0f };
const vector float g_VectorFloat_10_11_12_13 = { 16.0f, 17.0f, 18.0f, 19.0f };
const vector float g_VectorFloat_14_15_16_17 = { 20.0f, 21.0f, 22.0f, 23.0f };
const vector float g_VectorFloat_18_19_1A_1B = { 24.0f, 25.0f, 26.0f, 27.0f };
const vector float g_VectorFloat_1C_1D_1E_1F = { 28.0f, 29.0f, 30.0f, 31.0f };

const vector float g_VectorEpsilon = { fEpsilon, fEpsilon, fEpsilon, fEpsilon };

const vector unsigned char g_VectorPermuteYZX =
{
    (char)((1<<2)+0), (char)((1<<2)+1), (char)((1<<2)+2), (char)((1<<2)+3),
    (char)((2<<2)+0), (char)((2<<2)+1), (char)((2<<2)+2), (char)((2<<2)+3),
    (char)((0<<2)+0), (char)((0<<2)+1), (char)((0<<2)+2), (char)((0<<2)+3),
    (char)((3<<2)+0), (char)((3<<2)+1), (char)((3<<2)+2), (char)((3<<2)+3)
};

const vector unsigned char g_VectorPermuteZXY =
{
    (char)((2<<2)+0), (char)((2<<2)+1), (char)((2<<2)+2), (char)((2<<2)+3),
    (char)((0<<2)+0), (char)((0<<2)+1), (char)((0<<2)+2), (char)((0<<2)+3),
    (char)((1<<2)+0), (char)((1<<2)+1), (char)((1<<2)+2), (char)((1<<2)+3),
    (char)((3<<2)+0), (char)((3<<2)+1), (char)((3<<2)+2), (char)((3<<2)+3)
};

#else

const F32 g_VectorUnitX[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
const F32 g_VectorUnitY[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
const F32 g_VectorUnitZ[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
const F32 g_VectorUnitW[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

const F32 g_VectorZero[4]     = {  0.0f,  0.0f,  0.0f,  0.0f };
const F32 g_VectorHalf[4]     = {  0.5f,  0.5f,  0.5f,  0.5f };
const F32 g_VectorOne[4]      = {  1.0f,  1.0f,  1.0f,  1.0f };
const F32 g_VectorMinusOne[4] = { -1.0f, -1.0f, -1.0f, -1.0f };

const F32 g_VectorTwo[4]       = {  2.0f,  2.0f,  2.0f,  2.0f };
const F32 g_VectorThree[4]     = {  3.0f,  3.0f,  3.0f,  3.0f };
const F32 g_VectorFour[4]      = {  4.0f,  4.0f,  4.0f,  4.0f };
const F32 g_VectorFive[4]      = {  5.0f,  5.0f,  5.0f,  5.0f };
const F32 g_VectorEight[4]     = {  8.0f,  8.0f,  8.0f,  8.0f };
const F32 g_VectorSixteen[4]   = { 16.0f, 16.0f, 16.0f, 16.0f };
const F32 g_VectorThirtyTwo[4] = { 32.0f, 32.0f, 32.0f, 32.0f };

const F32 g_VectorFloat_00_01_02_03[4] = {  0.0f,  1.0f,  2.0f,  3.0f };
const F32 g_VectorFloat_04_05_06_07[4] = {  4.0f,  5.0f,  6.0f,  7.0f };
const F32 g_VectorFloat_08_09_0A_0B[4] = {  8.0f,  9.0f, 10.0f, 11.0f };
const F32 g_VectorFloat_0C_0D_0E_0F[4] = { 12.0f, 13.0f, 14.0f, 15.0f };
const F32 g_VectorFloat_10_11_12_13[4] = { 16.0f, 17.0f, 18.0f, 19.0f };
const F32 g_VectorFloat_14_15_16_17[4] = { 20.0f, 21.0f, 22.0f, 23.0f };
const F32 g_VectorFloat_18_19_1A_1B[4] = { 24.0f, 25.0f, 26.0f, 27.0f };
const F32 g_VectorFloat_1C_1D_1E_1F[4] = { 28.0f, 29.0f, 30.0f, 31.0f };

const F32 g_VectorEpsilon[4] = { fEpsilon, fEpsilon, fEpsilon, fEpsilon };

const F32 g_VectorFltMin[4] = { FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN };
const F32 g_VectorFltMax[4] = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };

#endif


#if defined( _AUDIO_MATHS_SPU_ )

const vector float g_VectorAbsMask = (vector float)(vector unsigned int)
{
    0x7FFFFFFF, 
    0x7FFFFFFF, 
    0x7FFFFFFF, 
    0x7FFFFFFF
};

const vector unsigned int g_VectorSignMask = { 0x80000000, 0x80000000, 0x80000000, 0x80000000 };

const vector unsigned int g_VectorMaskX = { 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000 };
const vector unsigned int g_VectorMaskY = { 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000 };
const vector unsigned int g_VectorMaskZ = { 0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000 };
const vector unsigned int g_VectorMaskW = { 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF };

const vector unsigned int g_VectorMaskXYZ = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 };

const vector unsigned char g_VectorMergeXY =
{
    (char)((0<<2)+0), (char)((0<<2)+1), (char)((0<<2)+2), (char)((0<<2)+3),
    (char)((4<<2)+0), (char)((4<<2)+1), (char)((4<<2)+2), (char)((4<<2)+3),
    (char)((1<<2)+0), (char)((1<<2)+1), (char)((1<<2)+2), (char)((1<<2)+3),
    (char)((5<<2)+0), (char)((5<<2)+1), (char)((5<<2)+2), (char)((5<<2)+3)
};

const vector unsigned char g_VectorUnpackHi16 =
{
    (char)0, (char)0, (char)0, (char)1,
    (char)0, (char)0, (char)2, (char)3,
    (char)0, (char)0, (char)4, (char)5,
    (char)0, (char)0, (char)6, (char)7
};

const vector unsigned char g_VectorUnpackLo16 =
{
    (char)0, (char)0, (char)8,  (char)9,
    (char)0, (char)0, (char)10, (char)11,
    (char)0, (char)0, (char)12, (char)13,
    (char)0, (char)0, (char)14, (char)15
};

const vector unsigned char g_VectorLoadUnaligned = 
{
    (char)0x0, (char)0x1, (char)0x2, (char)0x3,
    (char)0x4, (char)0x5, (char)0x6, (char)0x7,
    (char)0x8, (char)0x9, (char)0xA, (char)0xB,
    (char)0xC, (char)0xD, (char)0xE, (char)0xF
};

#elif defined( _AUDIO_MATHS_PPU_ )

const vector bool int g_VectorSignMask = { 0x80000000, 0x80000000, 0x80000000, 0x80000000 };

const vector bool int g_VectorMaskX = { 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000 };
const vector bool int g_VectorMaskY = { 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000 };
const vector bool int g_VectorMaskZ = { 0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000 };
const vector bool int g_VectorMaskW = { 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF };

const vector bool int g_VectorMaskXYZ = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 };

#endif


//-----------------------------------------------------------------------------
//	Assignment
//-----------------------------------------------------------------------------
//const Matrix& Matrix::operator = ( const Matrix& m )
//{
//	AX  = m.AX;
//	AY  = m.AY;
//	AZ  = m.AZ;
//	Pos = m.Pos;
//	return *this;
//}

//-----------------------------------------------------------------------------
//	Assignment
//-----------------------------------------------------------------------------
//const Matrix& Matrix::operator = ( const Vector v )
//{
//	Pos = v;
//	return *this;
//}

//-----------------------------------------------------------------------------
//	Boolean equal
//-----------------------------------------------------------------------------
//const bool Matrix::operator == ( const Matrix& m ) const
//{
//    #if defined( _AUDIO_MATHS_SPU_ )
//    const vector unsigned int vec_eq( spu_and( spu_and( spu_cmpeq( AX,  m.AX  ),
//                                                        spu_cmpeq( AY,  m.AY  ) ),
//                                               spu_and( spu_cmpeq( AZ,  m.AZ  ),
//                                                        spu_cmpeq( Pos, m.Pos ) ) ) );
//    return (bool)( vector_extract_x( vec_eq ) &
//                   vector_extract_y( vec_eq ) &
//                   vector_extract_z( vec_eq ) & 1 );
//    #elif defined( _AUDIO_MATHS_PPU_ )
//	return (bool)( vector3_all_eq( AX,  m.AX  ) &&
//		           vector3_all_eq( AY,  m.AY  ) &&
//		           vector3_all_eq( AZ,  m.AZ  ) &&
//		           vector3_all_eq( Pos, m.Pos ) );
//    #elif defined( PLATFORM_XENON )
//    const __vector4 eq( __vand( __vand( __vcmpeqfp( CX.xyzw, m.CX.xyzw ), 
//                                        __vcmpeqfp( CY.xyzw, m.CY.xyzw ) ),
//                                        __vcmpeqfp( CZ.xyzw, m.CZ.xyzw ) ) );
//    U32 CR;
//    __vcmpequwR( eq, __vzero(), &CR );
//    return ( CR >> CR6_ALL_FALSE_BIT ) & 1;
//    #else
//	return ( AX  == m.AX ) &&
//		   ( AY  == m.AY ) &&
//		   ( AZ  == m.AZ ) &&
//		   ( Pos == m.Pos );
//    #endif
//}

//-----------------------------------------------------------------------------
//	Boolean not equal
//-----------------------------------------------------------------------------
//const bool Matrix::operator != ( const Matrix& m ) const
//{
//    #if defined( _AUDIO_MATHS_SPU_ )
//    const vector unsigned int vec_eq( spu_and( spu_and( spu_cmpeq( AX,  m.AX  ),
//                                                        spu_cmpeq( AY,  m.AY  ) ),
//                                               spu_and( spu_cmpeq( AZ,  m.AZ  ),
//                                                        spu_cmpeq( Pos, m.Pos ) ) ) );
//    const vector unsigned int vec_ne( vector_not( vec_eq ) );
//    return (bool)( ( vector_extract_x( vec_ne ) |
//                     vector_extract_y( vec_ne ) |
//                     vector_extract_z( vec_ne ) ) & 1 );
//    #elif defined( _AUDIO_MATHS_PPU_ )
//	return (bool)( vector3_any_ne( AX,  m.AX  ) ||
//		           vector3_any_ne( AY,  m.AY  ) ||
//		           vector3_any_ne( AZ,  m.AZ  ) ||
//		           vector3_any_ne( Pos, m.Pos ) );
//    #elif defined( PLATFORM_XENON )
//    const __vector4 eq( __vand( __vand( __vcmpeqfp( CX.xyzw, m.CX.xyzw ), 
//                                        __vcmpeqfp( CY.xyzw, m.CY.xyzw ) ),
//                                        __vcmpeqfp( CZ.xyzw, m.CZ.xyzw ) ) );
//    U32 CR;
//    __vcmpequwR( eq, __vzero(), &CR );
//    return ~( CR >> CR6_ALL_FALSE_BIT ) & 1;
//    #else
//	return ( AX  != m.AX ) ||
//		   ( AY  != m.AY ) ||
//		   ( AZ  != m.AZ ) ||
//		   ( Pos != m.Pos );
//    #endif
//}

//-----------------------------------------------------------------------------
//	Add
//-----------------------------------------------------------------------------
const Matrix Matrix::operator + ( const Vector v ) const
{
    #if defined( _AUDIO_MATHS_SPU_ )
    
    const vector float vx( vector_splat_x( v.xyz ) );
    const vector float vy( vector_splat_y( v.xyz ) );
    const vector float vz( vector_splat_z( v.xyz ) );
    const vector float pos( spu_madd( vx, AX,
                            spu_madd( vy, AY,
                            spu_madd( vz, AZ,
                                      Pos ) ) ) );
    return Matrix( pos, AX, AY, AZ );
    
    #elif defined( _AUDIO_MATHS_PPU_ )
    
    const vector float vx( vector_splat_x( v.xyz ) );
    const vector float vy( vector_splat_y( v.xyz ) );
    const vector float vz( vector_splat_z( v.xyz ) );
    const vector float pos( vx * AX + vy * AY + vz * AZ + Pos );
    return Matrix( pos, AX, AY, AZ );
    
    #else
	
	return Matrix( MakeVector3( v.x * AX.x + v.y * AY.x + v.z * AZ.x,
			                    v.x * AX.y + v.y * AY.y + v.z * AZ.y,
			                    v.x * AX.z + v.y * AY.z + v.z * AZ.z ) + Pos,
		           AX, AY, AZ );
    
    #endif
}

//-----------------------------------------------------------------------------
//	Plus equals 
//-----------------------------------------------------------------------------
const Matrix& Matrix::operator += ( const Vector v )
{
	return *this = *this + v;
}

//-----------------------------------------------------------------------------
//	Subtract
//-----------------------------------------------------------------------------
const Matrix Matrix::operator - ( const Vector v ) const
{
	return *this + (-v);
}

//-----------------------------------------------------------------------------
//	Minus equals 
//-----------------------------------------------------------------------------
const Matrix& Matrix::operator -= (const Vector v)
{
	return *this = *this - v;
}

//-----------------------------------------------------------------------------
//	Matrix inverse
//-----------------------------------------------------------------------------
const Matrix Matrix::operator ! () const	//inverse
{
    #if defined( _AUDIO_MATHS_SPU_ )
    
    const float m00( vector_extract_x( AX ) );
    const float m01( vector_extract_y( AX ) );
    const float m02( vector_extract_z( AX ) );
    const float m10( vector_extract_x( AY ) );
    const float m11( vector_extract_y( AY ) );
    const float m12( vector_extract_z( AY ) );
    const float m20( vector_extract_x( AZ ) );
    const float m21( vector_extract_y( AZ ) );
    const float m22( vector_extract_z( AZ ) );
    const Vector ax( MakeVector3( make_vector3( m00, m10, m20 ) ) );
    const Vector ay( MakeVector3( make_vector3( m01, m11, m21 ) ) );
    const Vector az( MakeVector3( make_vector3( m02, m12, m22 ) ) );
    const Vector pos( MakeVector3( g_VectorZero ) );

    const Matrix m( pos, ax, ay, az );

	return m - MakeVector3( Pos );
    
    #elif defined( _AUDIO_MATHS_PPU_ )
    
    const vector float zero( g_VectorZero );
    const vector float tmp0( vec_mergeh( AX, AZ   ) );
    const vector float tmp1( vec_mergeh( AY, zero ) );
    const vector float tmp2( vec_mergel( AX, AZ   ) );
    const vector float tmp3( vec_mergel( AY, zero ) );
    const Vector ax( MakeVector3( vec_mergeh( tmp0, tmp1 ) ) );
    const Vector ay( MakeVector3( vec_mergel( tmp0, tmp1 ) ) );
    const Vector az( MakeVector3( vec_mergeh( tmp2, tmp3 ) ) );
    const Vector pos( MakeVector3( g_VectorZero ) );
	const Matrix m( pos, ax, ay, az );
	return m - MakeVector3( Pos );
    
    #else
	
	const Matrix m( MakeVector3( 0.0f, 0.0f, 0.0f ),
		            MakeVector3( AX.x, AY.x, AZ.x ),
		            MakeVector3( AX.y, AY.y, AZ.y ),
		            MakeVector3( AX.z, AY.z, AZ.z ) );
	return m - Pos;
    
    #endif
}

//-----------------------------------------------------------------------------
//	Matrix multiplication
//-----------------------------------------------------------------------------
const Matrix Matrix::operator * ( const Matrix& m ) const
{
    #if defined( _AUDIO_MATHS_SPU_ )
    
    const Vector pos( MakeVector3( spu_madd( vector_splat_x( m.Pos ), AX,
                                   spu_madd( vector_splat_y( m.Pos ), AY,
                                   spu_madd( vector_splat_z( m.Pos ), AZ, Pos ) ) ) ) );
    
    const Vector ax( MakeVector3( spu_madd( vector_splat_x( m.AX ), AX,
                                  spu_madd( vector_splat_y( m.AX ), AY,
                                  spu_mul ( vector_splat_z( m.AX ), AZ ) ) ) ) );
    
    const Vector ay( MakeVector3( spu_madd( vector_splat_x( m.AY ), AX,
                                  spu_madd( vector_splat_y( m.AY ), AY,
                                  spu_mul ( vector_splat_z( m.AY ), AZ ) ) ) ) );
    
    const Vector az( MakeVector3( spu_madd( vector_splat_x( m.AZ ), AX,
                                  spu_madd( vector_splat_y( m.AZ ), AY,
                                  spu_mul ( vector_splat_z( m.AZ ), AZ ) ) ) ) );
    
    return Matrix( pos, ax, ay, az );
    
    #elif defined( _AUDIO_MATHS_PPU_ )
    
    const Vector pos( MakeVector3( vector_splat_x( m.Pos ) * AX +
                                   vector_splat_y( m.Pos ) * AY +
                                   vector_splat_z( m.Pos ) * AZ + Pos ) );
    
    const Vector ax( MakeVector3( vector_splat_x( m.AX ) * AX +
                                  vector_splat_y( m.AX ) * AY +
                                  vector_splat_z( m.AX ) * AZ ) );
    
    const Vector ay( MakeVector3( vector_splat_x( m.AY ) * AX +
                                  vector_splat_y( m.AY ) * AY +
                                  vector_splat_z( m.AY ) * AZ ) );
    
    const Vector az( MakeVector3( vector_splat_x( m.AZ ) * AX +
                                  vector_splat_y( m.AZ ) * AY +
                                  vector_splat_z( m.AZ ) * AZ ) );
    
    return Matrix( pos, ax, ay, az );
    
    #else
    
	return Matrix( MakeVector3( m.Pos.x * AX.x + m.Pos.y * AY.x + m.Pos.z * AZ.x + Pos.x,
			                    m.Pos.x * AX.y + m.Pos.y * AY.y + m.Pos.z * AZ.y + Pos.y,
			                    m.Pos.x * AX.z + m.Pos.y * AY.z + m.Pos.z * AZ.z + Pos.z ),
		           MakeVector3( m.AX.x * AX.x + m.AX.y * AY.x + m.AX.z * AZ.x,
			                    m.AX.x * AX.y + m.AX.y * AY.y + m.AX.z * AZ.y,
			                    m.AX.x * AX.z + m.AX.y * AY.z + m.AX.z * AZ.z ),
		           MakeVector3( m.AY.x * AX.x + m.AY.y * AY.x + m.AY.z * AZ.x,
			                    m.AY.x * AX.y + m.AY.y * AY.y + m.AY.z * AZ.y,
			                    m.AY.x * AX.z + m.AY.y * AY.z + m.AY.z * AZ.z ),
		           MakeVector3( m.AZ.x * AX.x + m.AZ.y * AY.x + m.AZ.z * AZ.x,
			                    m.AZ.x * AX.y + m.AZ.y * AY.y + m.AZ.z * AZ.y,
			                    m.AZ.x * AX.z + m.AZ.y * AY.z + m.AZ.z * AZ.z ) );
    
    #endif
}

//-----------------------------------------------------------------------------
//	Matrix multiplication
//-----------------------------------------------------------------------------
const Matrix Matrix::operator * ( const Vector v ) const
{
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
	return Matrix( Pos * v.xyz, AX, AY, AZ );
    #else
	return Matrix( Pos * v, AX, AY, AZ );
	#endif
}

//-----------------------------------------------------------------------------
//	Matrix multiplication
//-----------------------------------------------------------------------------
const Matrix Matrix::operator * ( const float& scale ) const
{
    #if defined( _AUDIO_MATHS_SPU_ )
	return Matrix( Pos * spu_splats( scale ), AX, AY, AZ );
    #elif defined( _AUDIO_MATHS_PPU_ )
	return Matrix( Pos * make_scalar( scale ), AX, AY, AZ );
	#else
	return Matrix( Pos * _Scalar( scale ), AX, AY, AZ );
	#endif
}

//-----------------------------------------------------------------------------
//	Matrix multiplication equals
//-----------------------------------------------------------------------------
const Matrix& Matrix::operator *= ( const Matrix& m )
{
	return *this = *this * m;
}

//-----------------------------------------------------------------------------
//	Matrix multiplication equals
//-----------------------------------------------------------------------------
const Matrix& Matrix::operator *= ( const Vector v )
{
	return *this = *this * v;
}

//-----------------------------------------------------------------------------
//	Matrix multiplication equals
//-----------------------------------------------------------------------------
const Matrix& Matrix::operator *= ( const float& Scale )
{
	return *this = *this * Scale;
}

//-----------------------------------------------------------------------------
//	Get rotation matrix
//-----------------------------------------------------------------------------
const Matrix Matrix::RotateX( const float dAngle ) const
{
	float dSin = Maths::Sin( dAngle );
	float dCos = Maths::Cos( dAngle );

	return Matrix(
			MakeVector3( 0.0f, 0.0f,  0.0f ),
			MakeVector3( 1.0f, 0.0f,  0.0f ),
			MakeVector3( 0.0f, dCos, -dSin ),
			MakeVector3( 0.0f, dSin,  dCos ) );
}

//-----------------------------------------------------------------------------
//	Get rotation matrix
//-----------------------------------------------------------------------------
const Matrix Matrix::RotateY( const float dAngle ) const
{
	float dSin = Maths::Sin( dAngle );
	float dCos = Maths::Cos( dAngle );

	return Matrix(
			MakeVector3( 0.0f, 0.0f,  0.0f ),
			MakeVector3( dCos, 0.0f, -dSin ),
			MakeVector3( 0.0f, 1.0f,  0.0f ),
			MakeVector3( dSin, 0.0f,  dCos ) );
}

//-----------------------------------------------------------------------------
//	Get rotation matrix
//-----------------------------------------------------------------------------
const Matrix Matrix::RotateZ( const float dAngle ) const
{
	float dSin = Maths::Sin( dAngle );
	float dCos = Maths::Cos( dAngle );

	return Matrix(
			MakeVector3( 0.0f,  0.0f, 0.0f ),
			MakeVector3( dCos, -dSin, 0.0f ),
			MakeVector3( dSin,  dCos, 0.0f ),
			MakeVector3( 0.0f,  0.0f, 1.0f ) );
}

//-----------------------------------------------------------------------------
//	Identity
//-----------------------------------------------------------------------------
const Matrix Matrix::Identity() const
{
	return Matrix();
}

//-----------------------------------------------------------------------------
//	Transform
//-----------------------------------------------------------------------------
//const Vector Matrix::Transform( const Vector v ) const
//{
////	return Vector(v % AX, v % AY, v % AZ) + Pos;
//
//    Vector3 result;
//    #if defined( _AUDIO_MATHS_SPU_ )
//    result.xyz = spu_madd( vector_splat_x( v.xyz ), AX,
//                 spu_madd( vector_splat_y( v.xyz ), AY,
//                 spu_madd( vector_splat_z( v.xyz ), AZ, Pos ) ) );
//    #elif defined( _AUDIO_MATHS_PPU_ )
//    result.xyz = vec_madd( vector_splat_x( v.xyz ), AX,
//                 vec_madd( vector_splat_y( v.xyz ), AY,
//                 vec_madd( vector_splat_z( v.xyz ), AZ, Pos ) ) );
//    #elif defined( PLATFORM_XENON )
//    const __vector4 v_xyz1( __vrlimi( v.xyz, g_VectorOne, MASK_INSERT_W, 0 ) );
//    const __vector4 x( __vmsum4fp( CX.xyzw, v_xyz1 ) );
//    const __vector4 y( __vmsum4fp( CY.xyzw, v_xyz1 ) );
//    const __vector4 z( __vmsum4fp( CZ.xyzw, v_xyz1 ) );
//    result.xyz = __vmrghw( __vmrghw( x, z ), __vmrghw( y, __vzero() ) );
//	#else
//	result.x = v.x * AX.x + v.y * AY.x + v.z * AZ.x + Pos.x;
//	result.y = v.x * AX.y + v.y * AY.y + v.z * AZ.y + Pos.y;
//	result.z = v.x * AX.z + v.y * AY.z + v.z * AZ.z + Pos.z;
//	#endif
//	return result;
//}

//-----------------------------------------------------------------------------
//	Transform
//-----------------------------------------------------------------------------
const Vector Matrix::Rotate( const Vector v ) const
{
//	return Vector(v % AX, v % AY, v % AZ) + Pos;

    #if defined( _AUDIO_MATHS_SPU_ )
    return Vector( MakeVector3( spu_madd( vector_splat_x( v.xyz ), AX,
                                spu_madd( vector_splat_y( v.xyz ), AY,
                                spu_mul ( vector_splat_z( v.xyz ), AZ ) ) ) ) );
    #elif defined( _AUDIO_MATHS_PPU_ )
    return Vector( MakeVector3( vec_madd( vector_splat_x( v.xyz ), AX,
                                vec_madd( vector_splat_y( v.xyz ), AY,
                                vec_madd( vector_splat_z( v.xyz ), AZ, g_VectorZero ) ) ) ) );
	#else
	return MakeVector3( v.x * AX.x + v.y * AY.x + v.z * AZ.x,
			            v.x * AX.y + v.y * AY.y + v.z * AZ.y,
			            v.x * AX.z + v.y * AY.z + v.z * AZ.z );
	#endif
}

//-----------------------------------------------------------------------------
//	Billboard
//-----------------------------------------------------------------------------
const Matrix Matrix::Billboard() const
{
    #if defined( _AUDIO_MATHS_PPU_ ) || defined( _AUDIO_MATHS_SPU_ )
	return Matrix( MakeVector3( Pos ) );
    #else
	return Matrix( MakeVector3( Pos.x, Pos.y, Pos.z ) );
	#endif
}







void GenericDspLowPass1stOrder_x16(
    Vector4* __restrict pBuffer,
    const Scalar        vA,
    const Scalar        vB1,
    Vector4&            vPrevIn,
    Vector4&            vPrevOut,
    const U32           uSampleCount )
{
    //#if defined( COMPILER_VC )
    //__assume( uSampleCount > 0 );
    //#endif
    
    using namespace Audio::Maths;
    
	/// 1st order low pass filter
	
	//F32 fIn = 0.f;
	//F32 fOut = 0.f;
	//F32 fPrevIn = m_state.fPrevIn;
	//F32 fPrevOut = m_state.fPrevOut;
	//U32 uCount;
	//for ( uCount = 0; uCount < uSamplesRequired; uCount++ )
	//{
	//    fIn  = afShiftBuffer[uCount];
	//    fOut = m_state.m_fA * (fIn + fPrevIn) + m_state.m_fB * fPrevOut;
	//    afShiftBuffer[uCount] = fOut; 
	//    fPrevIn = fIn;
	//    fPrevOut = fOut;
	//}
	//m_state.fPrevIn = fIn;
	//m_state.fPrevOut = fOut;
    
    U32 uSampleCount_mod16 = uSampleCount & 0xF;
    //_ReadWriteBarrier();
    
    const Scalar vB2 = vB1 * vB1;
    const Scalar vB3 = vB1 * vB2;
    const Scalar vB4 = vB2 * vB2;
    
    const Scalar vA_x_vB1 = vA * vB1;
    const Scalar vA_x_vB2 = vA * vB2;
    const Scalar vA_x_vB3 = vA * vB3;
    
    //const Vector4 v4_zero( MakeVector4( g_VectorZero ) );
    
    Vector4 vIn_00_01_02_03; // = v4_zero;
    Vector4 vIn_01_02_03_04; // = v4_zero;
    Vector4 vIn_02_03_04_05; // = v4_zero;
    Vector4 vIn_03_04_05_06; // = v4_zero;
    Vector4 vIn_04_05_06_07; // = v4_zero;
    Vector4 vIn_05_06_07_08; // = v4_zero;
    Vector4 vIn_06_07_08_09; // = v4_zero;
    Vector4 vIn_07_08_09_0A; // = v4_zero;
    Vector4 vIn_08_09_0A_0B; // = v4_zero;
    Vector4 vIn_09_0A_0B_0C; // = v4_zero;
    Vector4 vIn_0A_0B_0C_0D; // = v4_zero;
    Vector4 vIn_0B_0C_0D_0E; // = v4_zero;
    Vector4 vIn_0C_0D_0E_0F; // = v4_zero;
    Vector4 vIn_0D_0E_0F_10; // = v4_zero;
    Vector4 vIn_0E_0F_10_11; // = v4_zero;
    Vector4 vIn_0F_10_11_12; // = v4_zero;
    Vector4 vIn_10_11_12_13 = vPrevIn;
    
    Vector4 vOut_00_01_02_03; // = v4_zero;
    Vector4 vOut_04_05_06_07; // = v4_zero;
    Vector4 vOut_08_09_0A_0B; // = v4_zero;
    Vector4 vOut_0C_0D_0E_0F; // = v4_zero;
    Vector4 vOut_10_11_12_13 = vPrevOut;
    
    #if defined( _AUDIO_MATHS_FAST_VECTOR4_DOT_PRODUCT_ )
    const Vector4 v_AB3_AB2_AB1_A = MakeVector4( vA_x_vB3, vA_x_vB2, vA_x_vB1, vA );
    #endif
    
    #define GENERIC_DSP_LOW_PASS_1ST_ORDER_USE_DO_WHILE_LOOP
    
    #if !defined( GENERIC_DSP_LOW_PASS_1ST_ORDER_USE_DO_WHILE_LOOP )
    for ( U32 uSampleIndex=0, uQuadSampleIndex=0; uSampleIndex < uSampleCount; uSampleIndex+=16, uQuadSampleIndex+=4 )
    #else
    U32 uSampleIndex=0;
    U32 uQuadSampleIndex=0;
    do
    #endif
    {
         vIn_00_01_02_03 =  vIn_10_11_12_13;
        vOut_00_01_02_03 = vOut_10_11_12_13;
        
        vIn_04_05_06_07 = pBuffer[ uQuadSampleIndex + 0 ];
        vIn_08_09_0A_0B = pBuffer[ uQuadSampleIndex + 1 ];
        vIn_0C_0D_0E_0F = pBuffer[ uQuadSampleIndex + 2 ];
        vIn_10_11_12_13 = pBuffer[ uQuadSampleIndex + 3 ];
        
        vIn_01_02_03_04 = RotateLeft<1>( vIn_00_01_02_03, vIn_04_05_06_07 );
        vIn_02_03_04_05 = RotateLeft<2>( vIn_00_01_02_03, vIn_04_05_06_07 );
        vIn_03_04_05_06 = RotateLeft<3>( vIn_00_01_02_03, vIn_04_05_06_07 );
        
        vIn_05_06_07_08 = RotateLeft<1>( vIn_04_05_06_07, vIn_08_09_0A_0B );
        vIn_06_07_08_09 = RotateLeft<2>( vIn_04_05_06_07, vIn_08_09_0A_0B );
        vIn_07_08_09_0A = RotateLeft<3>( vIn_04_05_06_07, vIn_08_09_0A_0B );
        
        vIn_09_0A_0B_0C = RotateLeft<1>( vIn_08_09_0A_0B, vIn_0C_0D_0E_0F );
        vIn_0A_0B_0C_0D = RotateLeft<2>( vIn_08_09_0A_0B, vIn_0C_0D_0E_0F );
        vIn_0B_0C_0D_0E = RotateLeft<3>( vIn_08_09_0A_0B, vIn_0C_0D_0E_0F );
        
        vIn_0D_0E_0F_10 = RotateLeft<1>( vIn_0C_0D_0E_0F, vIn_10_11_12_13 );
        vIn_0E_0F_10_11 = RotateLeft<2>( vIn_0C_0D_0E_0F, vIn_10_11_12_13 );
        vIn_0F_10_11_12 = RotateLeft<3>( vIn_0C_0D_0E_0F, vIn_10_11_12_13 );
        
        const Vector4 vSum_03_04_05_06 = vIn_03_04_05_06 + vIn_04_05_06_07;
        const Vector4 vSum_02_03_04_05 = vIn_02_03_04_05 + vIn_03_04_05_06;
        const Vector4 vSum_01_02_03_04 = vIn_01_02_03_04 + vIn_02_03_04_05;
        const Vector4 vSum_00_01_02_03 = vIn_00_01_02_03 + vIn_01_02_03_04;
        
        const Vector4 vSum_07_08_09_0A = vIn_07_08_09_0A + vIn_08_09_0A_0B;
        const Vector4 vSum_06_07_08_09 = vIn_06_07_08_09 + vIn_07_08_09_0A;
        const Vector4 vSum_05_06_07_08 = vIn_05_06_07_08 + vIn_06_07_08_09;
        const Vector4 vSum_04_05_06_07 = vIn_04_05_06_07 + vIn_05_06_07_08;
        
        const Vector4 vSum_0B_0C_0D_0E = vIn_0B_0C_0D_0E + vIn_0C_0D_0E_0F;
        const Vector4 vSum_0A_0B_0C_0D = vIn_0A_0B_0C_0D + vIn_0B_0C_0D_0E;
        const Vector4 vSum_09_0A_0B_0C = vIn_09_0A_0B_0C + vIn_0A_0B_0C_0D;
        const Vector4 vSum_08_09_0A_0B = vIn_08_09_0A_0B + vIn_09_0A_0B_0C;
        
        const Vector4 vSum_0F_10_11_12 = vIn_0F_10_11_12 + vIn_10_11_12_13;
        const Vector4 vSum_0E_0F_10_11 = vIn_0E_0F_10_11 + vIn_0F_10_11_12;
        const Vector4 vSum_0D_0E_0F_10 = vIn_0D_0E_0F_10 + vIn_0E_0F_10_11;
        const Vector4 vSum_0C_0D_0E_0F = vIn_0C_0D_0E_0F + vIn_0D_0E_0F_10;
        
        #if defined( _AUDIO_MATHS_FAST_VECTOR4_DOT_PRODUCT_ )
        
        vOut_04_05_06_07 = MakeVector4( v_AB3_AB2_AB1_A % vSum_00_01_02_03,
                                        v_AB3_AB2_AB1_A % vSum_01_02_03_04,
                                        v_AB3_AB2_AB1_A % vSum_02_03_04_05, 
                                        v_AB3_AB2_AB1_A % vSum_03_04_05_06 ) +
                           vB4 * vOut_00_01_02_03;
        
        vOut_08_09_0A_0B = MakeVector4( v_AB3_AB2_AB1_A % vSum_04_05_06_07,
                                        v_AB3_AB2_AB1_A % vSum_05_06_07_08,
                                        v_AB3_AB2_AB1_A % vSum_06_07_08_09,
                                        v_AB3_AB2_AB1_A % vSum_07_08_09_0A ) +
                           vB4 * vOut_04_05_06_07;
        
        vOut_0C_0D_0E_0F = MakeVector4( v_AB3_AB2_AB1_A % vSum_08_09_0A_0B,
                                        v_AB3_AB2_AB1_A % vSum_09_0A_0B_0C,
                                        v_AB3_AB2_AB1_A % vSum_0A_0B_0C_0D,
                                        v_AB3_AB2_AB1_A % vSum_0B_0C_0D_0E ) +
                           vB4 * vOut_08_09_0A_0B;
        
        vOut_10_11_12_13 = MakeVector4( v_AB3_AB2_AB1_A % vSum_0C_0D_0E_0F,
                                        v_AB3_AB2_AB1_A % vSum_0D_0E_0F_10,
                                        v_AB3_AB2_AB1_A % vSum_0E_0F_10_11,
                                        v_AB3_AB2_AB1_A % vSum_0F_10_11_12 ) +
                           vB4 * vOut_0C_0D_0E_0F;
        
        #else
        
        vOut_04_05_06_07 = ( ( vA       * vSum_03_04_05_06 +
                               vA_x_vB1 * vSum_02_03_04_05 ) +
                             ( vA_x_vB2 * vSum_01_02_03_04 +
                               vA_x_vB3 * vSum_00_01_02_03 ) ) +
                                    vB4 * vOut_00_01_02_03;
        
        vOut_08_09_0A_0B = ( ( vA       * vSum_07_08_09_0A +
                               vA_x_vB1 * vSum_06_07_08_09 ) +
                             ( vA_x_vB2 * vSum_05_06_07_08 +
                               vA_x_vB3 * vSum_04_05_06_07 ) ) +
                                    vB4 * vOut_04_05_06_07;
        
        vOut_0C_0D_0E_0F = ( ( vA       * vSum_0B_0C_0D_0E +
                               vA_x_vB1 * vSum_0A_0B_0C_0D ) +
                             ( vA_x_vB2 * vSum_09_0A_0B_0C +
                               vA_x_vB3 * vSum_08_09_0A_0B ) ) +
                                    vB4 * vOut_08_09_0A_0B;
        
        vOut_10_11_12_13 = ( ( vA       * vSum_0F_10_11_12 +
                               vA_x_vB1 * vSum_0E_0F_10_11 ) +
                             ( vA_x_vB2 * vSum_0D_0E_0F_10 +
                               vA_x_vB3 * vSum_0C_0D_0E_0F ) ) +
                                    vB4 * vOut_0C_0D_0E_0F;
        
        #endif
        
        pBuffer[ uQuadSampleIndex + 0 ] = vOut_04_05_06_07;
        pBuffer[ uQuadSampleIndex + 1 ] = vOut_08_09_0A_0B;
        pBuffer[ uQuadSampleIndex + 2 ] = vOut_0C_0D_0E_0F;
        pBuffer[ uQuadSampleIndex + 3 ] = vOut_10_11_12_13;
        
        #if defined( GENERIC_DSP_LOW_PASS_1ST_ORDER_USE_DO_WHILE_LOOP )
        uSampleIndex += 16;
        uQuadSampleIndex += 4;
        #endif
    }
    #if defined( GENERIC_DSP_LOW_PASS_1ST_ORDER_USE_DO_WHILE_LOOP )
    while ( uSampleIndex < uSampleCount );
    #endif
    
    const Vector4_U32 vSampleCount = _Vector4_U32( uSampleCount_mod16 );
    
    const Vector4_U32 v_1( _Vector4_U32<0x1>() );
    const Vector4_U32 v_2( _Vector4_U32<0x2>() );
    const Vector4_U32 v_3( _Vector4_U32<0x3>() );
    const Vector4_U32 v_4( _Vector4_U32<0x4>() );
    const Vector4_U32 v_5( _Vector4_U32<0x5>() );
    const Vector4_U32 v_6( _Vector4_U32<0x6>() );
    const Vector4_U32 v_7( _Vector4_U32<0x7>() );
    const Vector4_U32 v_8( _Vector4_U32<0x8>() );
    const Vector4_U32 v_9( _Vector4_U32<0x9>() );
    const Vector4_U32 v_A( _Vector4_U32<0xA>() );
    const Vector4_U32 v_B( _Vector4_U32<0xB>() );
    const Vector4_U32 v_C( _Vector4_U32<0xC>() );
    const Vector4_U32 v_D( _Vector4_U32<0xD>() );
    const Vector4_U32 v_E( _Vector4_U32<0xE>() );
    const Vector4_U32 v_F( _Vector4_U32<0xF>() );
    
    const Vector4 vOut_01_02_03_04 = RotateLeft<1>( vOut_00_01_02_03, vOut_04_05_06_07 );
    const Vector4 vOut_02_03_04_05 = RotateLeft<2>( vOut_00_01_02_03, vOut_04_05_06_07 );
    const Vector4 vOut_03_04_05_06 = RotateLeft<3>( vOut_00_01_02_03, vOut_04_05_06_07 );
                                                                 
    const Vector4 vOut_05_06_07_08 = RotateLeft<1>( vOut_04_05_06_07, vOut_08_09_0A_0B );
    const Vector4 vOut_06_07_08_09 = RotateLeft<2>( vOut_04_05_06_07, vOut_08_09_0A_0B );
    const Vector4 vOut_07_08_09_0A = RotateLeft<3>( vOut_04_05_06_07, vOut_08_09_0A_0B );
                                                                 
    const Vector4 vOut_09_0A_0B_0C = RotateLeft<1>( vOut_08_09_0A_0B, vOut_0C_0D_0E_0F );
    const Vector4 vOut_0A_0B_0C_0D = RotateLeft<2>( vOut_08_09_0A_0B, vOut_0C_0D_0E_0F );
    const Vector4 vOut_0B_0C_0D_0E = RotateLeft<3>( vOut_08_09_0A_0B, vOut_0C_0D_0E_0F );
                                                                 
    const Vector4 vOut_0D_0E_0F_10 = RotateLeft<1>( vOut_0C_0D_0E_0F, vOut_10_11_12_13 );
    const Vector4 vOut_0E_0F_10_11 = RotateLeft<2>( vOut_0C_0D_0E_0F, vOut_10_11_12_13 );
    const Vector4 vOut_0F_10_11_12 = RotateLeft<3>( vOut_0C_0D_0E_0F, vOut_10_11_12_13 );
    
    Vector4 vNewPrevIn  =  vIn_10_11_12_13;
    Vector4 vNewPrevOut = vOut_10_11_12_13;
    vNewPrevIn  = Select_EQ( vSampleCount, v_1,  vIn_01_02_03_04, vNewPrevIn );
    vNewPrevOut = Select_EQ( vSampleCount, v_1, vOut_01_02_03_04, vNewPrevOut );
    vNewPrevIn  = Select_EQ( vSampleCount, v_2,  vIn_02_03_04_05, vNewPrevIn );
    vNewPrevOut = Select_EQ( vSampleCount, v_2, vOut_02_03_04_05, vNewPrevOut );
    vNewPrevIn  = Select_EQ( vSampleCount, v_3,  vIn_03_04_05_06, vNewPrevIn );
    vNewPrevOut = Select_EQ( vSampleCount, v_3, vOut_03_04_05_06, vNewPrevOut );
    vNewPrevIn  = Select_EQ( vSampleCount, v_4,  vIn_04_05_06_07, vNewPrevIn );
    vNewPrevOut = Select_EQ( vSampleCount, v_4, vOut_04_05_06_07, vNewPrevOut );
    vNewPrevIn  = Select_EQ( vSampleCount, v_5,  vIn_05_06_07_08, vNewPrevIn );
    vNewPrevOut = Select_EQ( vSampleCount, v_5, vOut_05_06_07_08, vNewPrevOut );
    vNewPrevIn  = Select_EQ( vSampleCount, v_6,  vIn_06_07_08_09, vNewPrevIn );
    vNewPrevOut = Select_EQ( vSampleCount, v_6, vOut_06_07_08_09, vNewPrevOut );
    vNewPrevIn  = Select_EQ( vSampleCount, v_7,  vIn_07_08_09_0A, vNewPrevIn );
    vNewPrevOut = Select_EQ( vSampleCount, v_7, vOut_07_08_09_0A, vNewPrevOut );
    vNewPrevIn  = Select_EQ( vSampleCount, v_8,  vIn_08_09_0A_0B, vNewPrevIn );
    vNewPrevOut = Select_EQ( vSampleCount, v_8, vOut_08_09_0A_0B, vNewPrevOut );
    vNewPrevIn  = Select_EQ( vSampleCount, v_9,  vIn_09_0A_0B_0C, vNewPrevIn );
    vNewPrevOut = Select_EQ( vSampleCount, v_9, vOut_09_0A_0B_0C, vNewPrevOut );
    vNewPrevIn  = Select_EQ( vSampleCount, v_A,  vIn_0A_0B_0C_0D, vNewPrevIn );
    vNewPrevOut = Select_EQ( vSampleCount, v_A, vOut_0A_0B_0C_0D, vNewPrevOut );
    vNewPrevIn  = Select_EQ( vSampleCount, v_B,  vIn_0B_0C_0D_0E, vNewPrevIn );
    vNewPrevOut = Select_EQ( vSampleCount, v_B, vOut_0B_0C_0D_0E, vNewPrevOut );
    vNewPrevIn  = Select_EQ( vSampleCount, v_C,  vIn_0C_0D_0E_0F, vNewPrevIn );
    vNewPrevOut = Select_EQ( vSampleCount, v_C, vOut_0C_0D_0E_0F, vNewPrevOut );
    vNewPrevIn  = Select_EQ( vSampleCount, v_D,  vIn_0D_0E_0F_10, vNewPrevIn );
    vNewPrevOut = Select_EQ( vSampleCount, v_D, vOut_0D_0E_0F_10, vNewPrevOut );
    vNewPrevIn  = Select_EQ( vSampleCount, v_E,  vIn_0E_0F_10_11, vNewPrevIn );
    vNewPrevOut = Select_EQ( vSampleCount, v_E, vOut_0E_0F_10_11, vNewPrevOut );
    vNewPrevIn  = Select_EQ( vSampleCount, v_F,  vIn_0F_10_11_12, vNewPrevIn );
    vNewPrevOut = Select_EQ( vSampleCount, v_F, vOut_0F_10_11_12, vNewPrevOut );
    
    vPrevIn  = vNewPrevIn;
    vPrevOut = vNewPrevOut;
    
    //Vector4 vPrevIn0 = vIn_0C_0D_0E_0F;
    //Vector4 vPrevIn1 = vIn_10_11_12_13;
    //
    //Vector4 vPrevOut0 = vOut_0C_0D_0E_0F;
    //Vector4 vPrevOut1 = vOut_10_11_12_13;
    //
    //switch ( uSampleCount & 0xC )
    //{
    //    case 0 :
    //    vPrevIn0 = vIn_00_01_02_03;
    //    vPrevIn1 = vIn_04_05_06_07;
    //    vPrevOut0 = vOut_00_01_02_03;
    //    vPrevOut1 = vOut_04_05_06_07;
    //    break;
    //    case 4 :
    //    vPrevIn0 = vIn_04_05_06_07;
    //    vPrevIn1 = vIn_08_09_0A_0B;
    //    vPrevOut0 = vOut_04_05_06_07;
    //    vPrevOut1 = vOut_08_09_0A_0B;
    //    break;
    //    case 8 :
    //    vPrevIn0 = vIn_08_09_0A_0B;
    //    vPrevIn1 = vIn_0C_0D_0E_0F;
    //    vPrevOut0 = vOut_08_09_0A_0B;
    //    vPrevOut1 = vOut_0C_0D_0E_0F;
    //    break;
    //    //case 12 :
    //    //vPrevIn0 = vIn_0C_0D_0E_0F;
    //    //vPrevIn1 = vIn_10_11_12_13;
    //    //vPrevOut0 = vOut_0C_0D_0E_0F;
    //    //vPrevOut1 = vOut_10_11_12_13;
    //    //break;
    //};
    //
    //// TODO: RotateRight not implemented yet...
    ////vPrevIn  = RotateRight(  vPrevIn1,  vPrevIn0, -uSamplesRequired % 4 );
    ////vPrevOut = RotateRight( vPrevOut1, vPrevOut0, -uSamplesRequired % 4 );
    //
    //// Slower, but it'll work for now...
    //
    //switch ( uSampleCount & 0x3 )
    //{
    //    case 1 :
    //    vPrevIn  = RotateLeft<1>( vPrevIn0, vPrevIn1 );
    //    vPrevOut = RotateLeft<1>( vPrevOut0, vPrevOut1 );
    //    break;
    //    
    //    case 2 :
    //    vPrevIn  = RotateLeft<2>( vPrevIn0, vPrevIn1 );
    //    vPrevOut = RotateLeft<2>( vPrevOut0, vPrevOut1 );
    //    break;
    //    
    //    case 3 :
    //    vPrevIn  = RotateLeft<3>( vPrevIn0, vPrevIn1 );
    //    vPrevOut = RotateLeft<3>( vPrevOut0, vPrevOut1 );
    //    break;
    //    
    //    default :
    //    vPrevIn  = vPrevIn1;
    //    vPrevOut = vPrevOut1;
    //    break;
    //};
}



}; // namespace Maths
}; // namespace Audio

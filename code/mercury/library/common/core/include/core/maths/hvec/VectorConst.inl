#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_VECTOR_CONST_INL
#define SOFT_MATHS_HVEC_VECTOR_CONST_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_VECTOR_CONST_INL
#define MATHS_HVEC_VECTOR_CONST_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{


#if defined( _SPU_ ) || defined( _PPU_ )

template < int I > inline const vector float Const< vector float >::Int()
{
    #if defined( _SPU_ )
    if ( I == 0 ) return (vector float)spu_splats( (S32)0 );
    else          return spu_splats( ScalarConstInt<I>::m_Value );
    #elif defined( _PPU_ )
    if ( I == 0 ) return (vector float)vec_splat_s32( 0 );
    //else          return vec_splats( ScalarConstInt<I>::m_Value );
    else          return vec_splat( vec_lvlx( 0x0, &ScalarConstInt<I>::m_Value ), 0 );
    #endif
    //return VectorConstInt< I >::m_Value;
}

template < int N, int D > inline const vector float Const< vector float >::Frac()
{
    //return VectorConstFraction< N, D >::m_Value;
    #if defined( _SPU_ )
    return spu_splats( ScalarConstFraction<N,D>::m_Value );
    #elif defined( _PPU_ )
    //return vec_splat( ScalarConstFraction<N,D>::m_Value );
    return vec_splat( vec_lvlx( 0x0, &ScalarConstFraction<N,D>::m_Value ), 0 );
    #endif
}

inline const vector float Const< vector float >::Zero()
{
    #if defined( _SPU_ )
    return spu_splats( 0.0f );
    #else
    //return (vector float)( 0.0f, 0.0f, 0.0f, 0.0f );
    return (vector float)vec_splat_s32( 0 );
    #endif
}

inline const vector float Const< vector float >::Half()
{
    #if defined( _SPU_ )
    //return spu_splats( 0.5f );
    return spu_splats( ScalarConstFraction<1,2>::m_Value );
    #else
    //return (vector float)( 0.5f, 0.5f, 0.5f, 0.5f );
    //return vec_splats( ScalarConstFraction<1,2>::m_Value );
    return vec_splat( vec_lvlx( 0x0, &ScalarConstFraction<1,2>::m_Value ), 0 );
    #endif
}

inline const vector float Const< vector float >::Quarter()
{
    #if defined( _SPU_ )
    //return spu_splats( 0.25f );
    return spu_splats( ScalarConstFraction<1,4>::m_Value );
    #else
    //return (vector float)( 0.25f, 0.25f, 0.25f, 0.25f );
    //return vec_splats( ScalarConstFraction<1,4>::m_Value );
    return vec_splat( vec_lvlx( 0x0, &ScalarConstFraction<1,4>::m_Value ), 0 );
    #endif
}

inline const vector float Const< vector float >::One()
{
    #if defined( _SPU_ )
    //return spu_splats( 1.0f );
    return spu_splats( ScalarConstInt<1>::m_Value );
    #else
    //return (vector float)( 1.0f, 1.0f, 1.0f, 1.0f );
    //return vec_splats( ScalarConstInt<1>::m_Value );
    return vec_splat( vec_lvlx( 0x0, &ScalarConstInt<1>::m_Value ), 0 );
    #endif
}

inline const vector float Const< vector float >::Two()
{
    #if defined( _SPU_ )
    //return spu_splats( 2.0f );
    return spu_splats( ScalarConstInt<2>::m_Value );
    #else
    //return (vector float)( 2.0f, 2.0f, 2.0f, 2.0f );
    //return vec_splats( ScalarConstInt<2>::m_Value );
    return vec_splat( vec_lvlx( 0x0, &ScalarConstInt<2>::m_Value ), 0 );
    #endif
}

inline const vector float Const< vector float >::Three()
{
    #if defined( _SPU_ )
    //return spu_splats( 3.0f );
    return spu_splats( ScalarConstInt<3>::m_Value );
    #else
    //return (vector float)( 3.0f, 3.0f, 3.0f, 3.0f );
    //return vec_splats( ScalarConstInt<3>::m_Value );
    return vec_splat( vec_lvlx( 0x0, &ScalarConstInt<3>::m_Value ), 0 );
    #endif
}

inline const vector float Const< vector float >::Four()
{
    #if defined( _SPU_ )
    //return spu_splats( 4.0f );
    return spu_splats( ScalarConstInt<4>::m_Value );
    #else
    //return (vector float)( 4.0f, 4.0f, 4.0f, 4.0f );
    //return vec_splats( ScalarConstInt<4>::m_Value );
    return vec_splat( vec_lvlx( 0x0, &ScalarConstInt<4>::m_Value ), 0 );
    #endif
}

inline const vector float Const< vector float >::MinusOne()
{
    #if defined( _SPU_ )
    //return spu_splats( -1.0f );
    return spu_splats( ScalarConstInt<-1>::m_Value );
    #else
    //return (vector float)( -1.0f, -1.0f, -1.0f, -1.0f );
    //return vec_splats( ScalarConstInt<-1>::m_Value );
    return vec_splat( vec_lvlx( 0x0, &ScalarConstInt<-1>::m_Value ), 0 );
    #endif
}

inline const vector float Const< vector float >::MinusTwo()
{
    #if defined( _SPU_ )
    //return spu_splats( -2.0f );
    return spu_splats( ScalarConstInt<-2>::m_Value );
    #else
    //return (vector float)( -2.0f, -2.0f, -2.0f, -2.0f );
    //return vec_splats( ScalarConstInt<-2>::m_Value );
    return vec_splat( vec_lvlx( 0x0, &ScalarConstInt<-2>::m_Value ), 0 );
    #endif
}

inline const vector float Const< vector float >::MinusThree()
{
    #if defined( _SPU_ )
    //return spu_splats( -3.0f );
    return spu_splats( ScalarConstInt<-3>::m_Value );
    #else
    //return (vector float)( -3.0f, -3.0f, -3.0f, -3.0f );
    //return vec_splats( ScalarConstInt<-3>::m_Value );
    return vec_splat( vec_lvlx( 0x0, &ScalarConstInt<-3>::m_Value ), 0 );
    #endif
}

inline const vector float Const< vector float >::MinusFour()
{
    #if defined( _SPU_ )
    //return spu_splats( -4.0f );
    return spu_splats( ScalarConstInt<-4>::m_Value );
    #else
    //return (vector float)( -4.0f, -4.0f, -4.0f, -4.0f );
    //return vec_splats( ScalarConstInt<-4>::m_Value );
    return vec_splat( vec_lvlx( 0x0, &ScalarConstInt<-4>::m_Value ), 0 );
    #endif
}

inline const vector float Const< vector float >::UnitX()
{
    return m_UnitX;
}

inline const vector float Const< vector float >::UnitY()
{
    return m_UnitY;
}

inline const vector float Const< vector float >::UnitZ()
{
    return m_UnitZ;
}

inline const vector float Const< vector float >::UnitW()
{
    return m_UnitW;
}

inline const vector float Const< vector float >::NegativeUnitX()
{
    return -m_UnitX;
}

inline const vector float Const< vector float >::NegativeUnitY()
{
    return -m_UnitY;
}

inline const vector float Const< vector float >::NegativeUnitZ()
{
    return -m_UnitZ;
}

inline const vector float Const< vector float >::NegativeUnitW()
{
    return -m_UnitW;
}

inline const vector float Const< vector float >::Epsilon()
{
    #if defined( _SPU_ )
    return spu_splats( FLT_EPSILON );
    #else
		#if defined( vec_splats )
		return vec_splats( FLT_EPSILON );
		#else
		return (const vector float)( FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON );
		#endif
    #endif
}

inline const vector float Const< vector float >::OneMinusEpsilon()
{
    #if defined( _SPU_ )
    return spu_splats( 1.0f - FLT_EPSILON );
    #else
		#if defined( vec_splats )
		return vec_splats( 1.0f - FLT_EPSILON );
		#else
	    return (const vector float)( 1.0f - FLT_EPSILON, 1.0f - FLT_EPSILON, 1.0f - FLT_EPSILON, 1.0f - FLT_EPSILON );
		#endif	
	#endif
}

inline const vector float Const< vector float >::Pi()
{
    #if defined( _SPU_ )
    return spu_splats( 3.1415926535897932384626433832795f );
    #else
		#if defined( vec_splats )
		return vec_splats( 3.1415926535897932384626433832795f );
		#else
		return (const vector float)( 3.1415926535897932384626433832795f,
		                             3.1415926535897932384626433832795f,
		                             3.1415926535897932384626433832795f,
		                             3.1415926535897932384626433832795f );
		#endif
    #endif
}

inline const vector float Const< vector float >::PiDivTwo()
{
    #if defined( _SPU_ )
    return spu_splats( 1.5707963267948966192313216916398f );
    #else
		#if defined( vec_splats )
		return vec_splats( 1.5707963267948966192313216916398f );
		#else
		return (const vector float)( 1.5707963267948966192313216916398f,
		                             1.5707963267948966192313216916398f,
		                             1.5707963267948966192313216916398f,
		                             1.5707963267948966192313216916398f );
		#endif
    #endif
}

inline const vector float Const< vector float >::TwoPi()
{
    #if defined( _SPU_ )
    return spu_splats( 6.283185307179586476925286766559f );
    #else
		#if defined( vec_splats )
		return vec_splats( 6.283185307179586476925286766559f );
		#else
		return (const vector float)( 6.283185307179586476925286766559f,
		                             6.283185307179586476925286766559f,
		                             6.283185307179586476925286766559f,
		                             6.283185307179586476925286766559f );
		#endif
    #endif
}

inline const vector float Const< vector float >::RcpPi()
{
    #if defined( _SPU_ )
    return spu_splats( 0.31830988618379067153776752674503f );
    #else
		#if defined( vec_splats )
		return vec_splats( 0.31830988618379067153776752674503f );
		#else
		return (const vector float)( 0.31830988618379067153776752674503f,
		                             0.31830988618379067153776752674503f,
		                             0.31830988618379067153776752674503f,
		                             0.31830988618379067153776752674503f );
		#endif
    #endif
}

inline const vector float Const< vector float >::RcpTwoPi()
{
    #if defined( _SPU_ )
    return spu_splats( 0.15915494309189533576888376337251f );
    #else
		#if defined( vec_splats )
		return vec_splats( 0.15915494309189533576888376337251f );
		#else
		return (const vector float)( 0.15915494309189533576888376337251f,
		                             0.15915494309189533576888376337251f,
		                             0.15915494309189533576888376337251f,
		                             0.15915494309189533576888376337251f );
		#endif
    #endif
}

inline const vector float Const< vector float >::Log2()
{
    #if defined( _SPU_ )
    return spu_splats( 0.69314718055994530941723212145818f );
    #else
		#if defined( vec_splats )
		return vec_splats( 0.69314718055994530941723212145818f );
		#else
		return (const vector float)( 0.69314718055994530941723212145818f,
		                             0.69314718055994530941723212145818f,
		                             0.69314718055994530941723212145818f,
		                             0.69314718055994530941723212145818f );
		#endif
    #endif
}

inline const vector float Const< vector float >::Log10()
{
    #if defined( _SPU_ )
    return spu_splats( 2.3025850929940456840179914546844f );
    #else
		#if defined( vec_splats )
		return vec_splats( 2.3025850929940456840179914546844f );
		#else
		return (const vector float)( 2.3025850929940456840179914546844f,
		                             2.3025850929940456840179914546844f,
		                             2.3025850929940456840179914546844f,
		                             2.3025850929940456840179914546844f );
		#endif
    #endif
}

inline const vector float Const< vector float >::FltMin()
{
    #if defined( _SPU_ )
    return spu_splats( FLT_MIN );
    #else
		#if defined( vec_splats )
		return vec_splats( FLT_MIN );
		#else
	    return (const vector float)( FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN );
		#endif
    #endif
}

inline const vector float Const< vector float >::FltMax()
{
    #if defined( _SPU_ )
    return spu_splats( FLT_MAX );
    #else
		#if defined( vec_splats )
		return vec_splats( FLT_MAX );
		#else
	    return (const vector float)( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX );
		#endif
    #endif
}

inline const vector float Const< vector float >::NegFltMin()
{
    #if defined( _SPU_ )
    return spu_splats( -FLT_MIN );
    #else
		#if defined( vec_splats )
		return vec_splats( -FLT_MIN );
		#else
	    return (const vector float)( -FLT_MIN, -FLT_MIN, -FLT_MIN, -FLT_MIN );
		#endif
    #endif
}

inline const vector float Const< vector float >::NegFltMax()
{
    #if defined( _SPU_ )
    return spu_splats( -FLT_MAX );
    #else
		#if defined( vec_splats )
		return vec_splats( -FLT_MAX );
		#else
	    return (vector float)( -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX );
		#endif
    #endif
}

inline const vector float Const< vector float >::AbsMask()
{
    return (vector float)Const< vector unsigned int >::AbsMask();
}

inline const vector float Const< vector float >::SignMask()
{
    return (vector float)Const< vector unsigned int >::SignMask();
}

inline const vector float Const< vector float >::Mask( const bool x, const bool y, const bool z, const bool w )
{
    return (vector float)Const< vector unsigned int >::Mask( x, y, z, w );
}

template < bool x, bool y, bool z, bool w >
inline const vector float Const< vector float >::Mask()
{
    return (vector float)Const< vector unsigned int >::Mask<x,y,z,w>();
}



inline const vector signed int Const< vector signed int >::Zero()
{
    #if defined( _SPU_ )
    return spu_splats( (signed int)0 );
    #else
    //return (vector signed int)( 0, 0, 0, 0 );
    return vec_splat_s32( 0 );
    #endif
}

inline const vector unsigned int Const< vector unsigned int >::Zero()
{
    #if defined( _SPU_ )
    return spu_splats( (unsigned int)0 );
    #else
    //return (vector unsigned int)( 0, 0, 0, 0 );
    return vec_splat_u32( 0 );
    #endif
}

template < unsigned int I >
inline const vector unsigned int Const< vector unsigned int >::Int()
{
    #if defined( _SPU_ )
    return spu_splats( I );
    #else
    return vec_splats( ConstU32<I>::m_Value );
    #endif
}

inline const vector unsigned int Const< vector unsigned int >::AbsMask()
{
    return m_AbsMask;
}

inline const vector unsigned int Const< vector unsigned int >::SignMask()
{
    return m_SignMask;
}

inline const vector unsigned int Const< vector unsigned int >::Mask( const int mask )
{
    #if defined( _SPU_ )
    return spu_maskw( mask );
    #else
    return m_Mask[ mask & 0xF ];
    #endif
}

inline const vector unsigned int Const< vector unsigned int >::Mask( const bool x, const bool y, const bool z, const bool w )
{
    #if defined( _SPU_ )
    return spu_maskw( (int)x * 8 + (int)y * 4 + (int)z * 2 + (int)w );
    #else
    //const int index = ((x&1)<<3) + ((y&1)<<2) + ((z&1)<<1) + (w&1);
    //return m_Mask[index];
    return m_Mask[ (int)x * 8 + (int)y * 4 + (int)z * 2 + (int)w ];
    #endif
}

template < bool x, bool y, bool z, bool w >
inline const vector unsigned int Const< vector unsigned int >::Mask()
{
    #if defined( _SPU_ )
    //return spu_maskw( (int)x * 8 + (int)y * 4 + (int)z * 2 + (int)w );
    return spu_mask_word< (int)x * 8 + (int)y * 4 + (int)z * 2 + (int)w >();
    #else
    //const int index = ((x&1)<<3) + ((y&1)<<2) + ((z&1)<<1) + (w&1);
    //return m_Mask[index];
    return m_Mask[ (int)x * 8 + (int)y * 4 + (int)z * 2 + (int)w ];
    #endif
}


inline const vector unsigned short Const< vector unsigned short >::Zero()
{
    #if defined( _SPU_ )
    return spu_splats( (unsigned short)0 );
    #else
    return vec_splat_u16( 0 );
    #endif
}

template < unsigned short I >
inline const vector unsigned short Const< vector unsigned short >::Int()
{
    #if defined( _SPU_ )
    return spu_splats( I );
    #else
    if ( ( I >= -16 ) && ( I <= 15 ) )
        return vec_splat_u16( I );
    else
        //return vec_splats( ConstU16<I>::m_Value );
        return vec_splat( vec_lvlx( 0x0, &ConstU16<I>::m_Value ), 0 );
    #endif
}


#if defined( _SPU_ )

inline const vector unsigned int Const< vector unsigned int >::True()
{
    //return spu_splats( (unsigned int)-1 );
    //return m_Bool[1];
    //return spu_maskw( 0xF );
    return (vector unsigned int)spu_maskb( 0xFFFF );
}

inline const vector unsigned int Const< vector unsigned int >::False()
{
    //return spu_splats( (unsigned int)0 );
    //return m_Bool[0];
    //return spu_maskw( 0x0 );
    return (vector unsigned int)spu_maskb( 0x0000 );
}

inline const vector unsigned int Const< vector unsigned int >::Bool( const bool x )
{
    //return m_Bool[ (int)x ];
    //return spu_maskw( -(int)x ); // false --> 0, true --> -1
    return (vector unsigned int)spu_maskb( -(int)x ); // false --> 0, true --> -1
}

inline const vector unsigned int Const< vector unsigned int >::Bool( const bool x, const bool y, const bool z, const bool w )
{
    //return m_Mask[ (int)x * 8 + (int)y * 4 + (int)z * 2 + (int)w ];
    return spu_maskw( (int)x * 8 + (int)y * 4 + (int)z * 2 + (int)w );
}

template < bool x >
inline const vector unsigned int Const< vector unsigned int >::Bool()
{
    //return m_Bool[ (int)x ];
    //return spu_maskw( x ? 0xF : 0x0 );
    return (vector unsigned int)spu_maskb( x ? 0xFFFF : 0x0000 );
}

template < bool x, bool y, bool z, bool w >
inline const vector unsigned int Const< vector unsigned int >::Bool()
{
    //return m_Mask[ (int)x * 8 + (int)y * 4 + (int)z * 2 + (int)w ];
    //return spu_maskw( (int)x * 8 + (int)y * 4 + (int)z * 2 + (int)w );
    //return spu_mask_word< (int)x * 8 + (int)y * 4 + (int)z * 2 + (int)w >();
    return (vector unsigned int)spu_maskb(
        ( (unsigned short)x << 15 ) | ( (unsigned short)x << 14 ) | ( (unsigned short)x << 13 ) | ( (unsigned short)x << 12 ) |
        ( (unsigned short)y << 11 ) | ( (unsigned short)y << 10 ) | ( (unsigned short)y <<  9 ) | ( (unsigned short)y <<  8 ) |
        ( (unsigned short)z <<  7 ) | ( (unsigned short)z <<  6 ) | ( (unsigned short)z <<  5 ) | ( (unsigned short)z <<  4 ) |
        ( (unsigned short)w <<  3 ) | ( (unsigned short)w <<  2 ) | ( (unsigned short)w <<  1 ) | ( (unsigned short)w <<  0 ) );
}

template < bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7 >
inline const vector unsigned short Const< vector unsigned short >::Bool()
{
    //return *(vector unsigned short*)&ConstVec8Bool<b0,b1,b2,b3,b4,b5,b6,b7>::m_Data;
    return (vector unsigned short)spu_maskb(
        ( (unsigned short)b0 << 15 ) | ( (unsigned short)b0 << 14 ) |
        ( (unsigned short)b1 << 13 ) | ( (unsigned short)b1 << 12 ) |
        ( (unsigned short)b2 << 11 ) | ( (unsigned short)b2 << 10 ) |
        ( (unsigned short)b3 <<  9 ) | ( (unsigned short)b3 <<  8 ) |
        ( (unsigned short)b4 <<  7 ) | ( (unsigned short)b4 <<  6 ) |
        ( (unsigned short)b5 <<  5 ) | ( (unsigned short)b5 <<  4 ) |
        ( (unsigned short)b6 <<  3 ) | ( (unsigned short)b6 <<  2 ) |
        ( (unsigned short)b7 <<  1 ) | ( (unsigned short)b7 <<  0 ) );
}

template < bool b0, bool b1, bool b2,  bool b3,  bool b4,  bool b5,  bool b6,  bool b7,
           bool b8, bool b9, bool b10, bool b11, bool b12, bool b13, bool b14, bool b15 >
inline const vector unsigned char Const< vector unsigned char >::Bool()
{
    //return *(vector unsigned char*)&ConstVec16Bool<b0,b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15>::m_Data;
    return spu_maskb(
        ( (unsigned short)b0  << 15 ) |
        ( (unsigned short)b1  << 14 ) |
        ( (unsigned short)b2  << 13 ) |
        ( (unsigned short)b3  << 12 ) |
        ( (unsigned short)b4  << 11 ) |
        ( (unsigned short)b5  << 10 ) |
        ( (unsigned short)b6  <<  9 ) |
        ( (unsigned short)b7  <<  8 ) |
        ( (unsigned short)b8  <<  7 ) |
        ( (unsigned short)b9  <<  6 ) |
        ( (unsigned short)b10 <<  5 ) |
        ( (unsigned short)b11 <<  4 ) |
        ( (unsigned short)b12 <<  3 ) |
        ( (unsigned short)b13 <<  2 ) |
        ( (unsigned short)b14 <<  1 ) |
        ( (unsigned short)b15 <<  0 ) );
}

#elif defined( _PPU_ )

inline const vector bool int Const< vector bool int >::True()
{
    return (vector bool int)vec_splat_s32( -1 );
}

inline const vector bool int Const< vector bool int >::False()
{
    return (vector bool int)vec_splat_s32( 0 );
}

inline const vector bool int Const< vector bool int >::Bool( const bool x )
{
    return m_Bool[ (int)x ];
}

inline const vector bool int Const< vector bool int >::Bool( const bool x, bool y, bool z, bool w )
{
    return (vector bool int)Const< vector unsigned int >::m_Mask[ (int)x * 8 + (int)y * 4 + (int)z * 2 + (int)w ];
}

template < bool x >
inline const vector bool int Const< vector bool int >::Bool()
{
    return m_Bool[ (int)x ];
}

template < bool x, bool y, bool z, bool w >
inline const vector bool int Const< vector bool int >::Bool()
{
    return (vector bool int)Const< vector unsigned int >::m_Mask[ (int)x * 8 + (int)y * 4 + (int)z * 2 + (int)w ];
}

template < bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7 >
inline const vector bool short Const< vector bool short >::Bool()
{
    return *(vector bool short*)&ConstVec8Bool<b0,b1,b2,b3,b4,b5,b6,b7>::m_Data;
}

template < bool b0, bool b1, bool b2,  bool b3,  bool b4,  bool b5,  bool b6,  bool b7,
           bool b8, bool b9, bool b10, bool b11, bool b12, bool b13, bool b14, bool b15 >
inline const vector bool char Const< vector bool char >::Bool()
{
    return *(vector bool char*)&ConstVec16Bool<b0,b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15>::m_Data;
}

#endif


inline const vector unsigned char Const< vector unsigned char >::Zero()
{
    #if defined( _SPU_ )
    return spu_splats( (unsigned char)0 );
    #else
    //return vec_splats( (unsigned char)0 );
    return vec_splat_u8( 0 );
    #endif
}

inline const vector signed char Const< vector signed char >::Zero()
{
    #if defined( _SPU_ )
    return spu_splats( (signed char)0 );
    #else
    //return vec_splats( (signed char)0 );
    return vec_splat_s8( 0 );
    #endif
}

inline const vector unsigned char Const< vector unsigned char >::Pattern_UnpackS16_XY()
{
    return m_UnpackPatternS16_XY;
}


#endif // #if defined( _SPU_ ) || defined( _PPU_ )




#if defined( _VMX128_ )

template < int I >
NOALIAS __forceinline const __vector4 Const< __vector4 >::Int()
{
    //return VectorConstInt< I >::m_Value;
    if ( I == 0 )                           return __vzero();
    //else if ( ( I >= -16 ) && ( I <= 15 ) ) return __vcfsx( __vspltisw( Maths::Min(Maths::Max(I,-16),15) ), 0 );
    else if ( ( I >= -16 ) && ( I <= 15 ) ) return __vcfsx( __vspltisw( ( (I>=-16) && (I<=15) ) ? I : 0 ), 0 );
    else                                    return __vspltw( __lvlx( &ScalarConstInt<I>::m_Value, 0x0 ), VX );
}

template < int N, int D >
NOALIAS __forceinline const __vector4 Const< __vector4 >::Frac()
{
    //return VectorConstFraction< N, D >::m_Value;
    return __vspltw( __lvlx( &ScalarConstFraction<Num,Denom>::m_Value, 0x0 ), VX );
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::Zero()
{
    return __vzero();
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::Half()
{
    return m_Half;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::Quarter()
{
    return m_Quarter;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::One()
{
    return m_One;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::Two()
{
    return m_Two;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::Three()
{
    return m_Three;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::Four()
{
    return m_Four;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::MinusOne()
{
    return m_MinusOne;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::MinusTwo()
{
    return m_MinusTwo;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::MinusThree()
{
    return m_MinusThree;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::MinusFour()
{
    return m_MinusFour;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::UnitX()
{
    return m_UnitX;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::UnitY()
{
    return m_UnitY;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::UnitZ()
{
    return m_UnitZ;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::UnitW()
{
    return m_UnitW;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::NegativeUnitX()
{
    return __vxor( m_UnitX, m_SignMask );
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::NegativeUnitY()
{
    return __vxor( m_UnitY, m_SignMask );
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::NegativeUnitZ()
{
    return __vxor( m_UnitZ, m_SignMask );
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::NegativeUnitW()
{
    return __vxor( m_UnitW, m_SignMask );
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::Epsilon()
{
    return m_Epsilon;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::OneMinusEpsilon()
{
    return m_OneMinusEpsilon;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::Pi()
{
    return m_Pi;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::PiDivTwo()
{
    return m_PiDivTwo;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::TwoPi()
{
    return m_TwoPi;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::RcpPi()
{
    return m_RcpPi;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::RcpTwoPi()
{
    return m_RcpTwoPi;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::Log2()
{
    return m_Log2;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::Log10()
{
    return m_Log10;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::FltMin()
{
    return m_FltMin;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::FltMax()
{
    return m_FltMax;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::NegFltMin()
{
    return m_NegFltMin;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::NegFltMax()
{
    return m_NegFltMax;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::AbsMask()
{
    return m_AbsMask;
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::SignMask()
{
    return m_SignMask;
}

template < bool x, bool y, bool z, bool w >
NOALIAS __forceinline const __vector4 Const< __vector4 >::Mask()
{
    return m_Mask[ (int)x * 8 + (int)y * 4 + (int)z * 2 + (int)w ];
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::Bool( const bool x )
{
    return m_Bool[ (int)x ];
}

NOALIAS __forceinline const __vector4 Const< __vector4 >::Bool( const bool x, bool y, bool z, bool w )
{
    return m_Mask[ (int)x * 8 + (int)y * 4 + (int)z * 2 + (int)w ];
}

template < bool x >
NOALIAS __forceinline const __vector4 Const< __vector4 >::Bool()
{
    //return m_Bool[x];
    return x ? __vspltisw( -1 ) : __vspltisw( 0 );
}

template < bool x, bool y, bool z, bool w >
NOALIAS __forceinline const __vector4 Const< __vector4 >::Bool()
{
    //return m_Mask[ (int)x * 8 + (int)y * 4 + (int)z * 2 + (int)w ];
    return *(__vector4*)&ConstVec4Bool<x,y,z,w>::m_Data;
}

template < bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7 >
NOALIAS __forceinline const __vector4 Const< __vector4 >::Bool()
{
    return *(__vector4*)&ConstVec8Bool<b0,b1,b2,b3,b4,b5,b6,b7>::m_Data;
}

template < bool b0, bool b1, bool b2,  bool b3,  bool b4,  bool b5,  bool b6,  bool b7,
           bool b8, bool b9, bool b10, bool b11, bool b12, bool b13, bool b14, bool b15 >
NOALIAS __forceinline const __vector4 Const< __vector4 >::Bool()
{
    return *(__vector4*)&ConstVec16Bool<b0,b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15>::m_Data;
}

#endif // #if defined( _VMX128_ )




#if defined( _SSE_ )

template < int I >
NOALIAS __forceinline const __m128 Const< __m128 >::Int()
{
    return VectorConstInt< I >::m_Value;
}

template < int N, int D >
NOALIAS __forceinline const __m128 Const< __m128 >::Frac()
{
    return VectorConstFraction< N, D >::m_Value;
}

NOALIAS __forceinline const __m128 Const< __m128 >::Zero()
{
    return _mm_setzero_ps();
}

NOALIAS __forceinline const __m128 Const< __m128 >::Half()
{
    return m_Half;
}

NOALIAS __forceinline const __m128 Const< __m128 >::Quarter()
{
    return m_Quarter;
}

NOALIAS __forceinline const __m128 Const< __m128 >::One()
{
    return m_One;
}

NOALIAS __forceinline const __m128 Const< __m128 >::Two()
{
    return m_Two;
}

NOALIAS __forceinline const __m128 Const< __m128 >::Three()
{
    return m_Three;
}

NOALIAS __forceinline const __m128 Const< __m128 >::Four()
{
    return m_Four;
}

NOALIAS __forceinline const __m128 Const< __m128 >::MinusOne()
{
    return m_MinusOne;
}

NOALIAS __forceinline const __m128 Const< __m128 >::MinusTwo()
{
    return m_MinusTwo;
}

NOALIAS __forceinline const __m128 Const< __m128 >::MinusThree()
{
    return m_MinusThree;
}

NOALIAS __forceinline const __m128 Const< __m128 >::MinusFour()
{
    return m_MinusFour;
}

NOALIAS __forceinline const __m128 Const< __m128 >::UnitX()
{
    return m_UnitX;
}

NOALIAS __forceinline const __m128 Const< __m128 >::UnitY()
{
    return m_UnitY;
}

NOALIAS __forceinline const __m128 Const< __m128 >::UnitZ()
{
    return m_UnitZ;
}

NOALIAS __forceinline const __m128 Const< __m128 >::UnitW()
{
    return m_UnitW;
}

NOALIAS __forceinline const __m128 Const< __m128 >::NegativeUnitX()
{
    return _mm_xor_ps( m_UnitX, m_SignMask );
}

NOALIAS __forceinline const __m128 Const< __m128 >::NegativeUnitY()
{
    return _mm_xor_ps( m_UnitY, m_SignMask );
}

NOALIAS __forceinline const __m128 Const< __m128 >::NegativeUnitZ()
{
    return _mm_xor_ps( m_UnitZ, m_SignMask );
}

NOALIAS __forceinline const __m128 Const< __m128 >::NegativeUnitW()
{
    return _mm_xor_ps( m_UnitW, m_SignMask );
}

NOALIAS __forceinline const __m128 Const< __m128 >::Epsilon()
{
    return m_Epsilon;
}

NOALIAS __forceinline const __m128 Const< __m128 >::OneMinusEpsilon()
{
    return m_OneMinusEpsilon;
}

NOALIAS __forceinline const __m128 Const< __m128 >::Pi()
{
    return m_Pi;
}

NOALIAS __forceinline const __m128 Const< __m128 >::PiDivTwo()
{
    return m_PiDivTwo;
}

NOALIAS __forceinline const __m128 Const< __m128 >::TwoPi()
{
    return m_TwoPi;
}

NOALIAS __forceinline const __m128 Const< __m128 >::RcpPi()
{
    return m_RcpPi;
}

NOALIAS __forceinline const __m128 Const< __m128 >::RcpTwoPi()
{
    return m_RcpTwoPi;
}

NOALIAS __forceinline const __m128 Const< __m128 >::Log2()
{
    return m_Log2;
}

NOALIAS __forceinline const __m128 Const< __m128 >::Log10()
{
    return m_Log10;
}

NOALIAS __forceinline const __m128 Const< __m128 >::FltMin()
{
    return m_FltMin;
}

NOALIAS __forceinline const __m128 Const< __m128 >::FltMax()
{
    return m_FltMax;
}

NOALIAS __forceinline const __m128 Const< __m128 >::NegFltMin()
{
    return m_NegFltMin;
}

NOALIAS __forceinline const __m128 Const< __m128 >::NegFltMax()
{
    return m_NegFltMax;
}

NOALIAS __forceinline const __m128 Const< __m128 >::AbsMask()
{
    return m_AbsMask;
}

NOALIAS __forceinline const __m128 Const< __m128 >::SignMask()
{
    return m_SignMask;
}

template < bool x, bool y, bool z, bool w >
NOALIAS __forceinline const __m128 Const< __m128 >::Mask()
{
    return m_Mask[ (int)x * 8 + (int)y * 4 + (int)z * 2 + (int)w ];
}

NOALIAS __forceinline const __m128 Const< __m128 >::Bool( const bool x )
{
    return m_Bool[x];
}

NOALIAS __forceinline const __m128 Const< __m128 >::Bool( const bool x, bool y, bool z, bool w )
{
    return m_Mask[ (int)x * 8 + (int)y * 4 + (int)z * 2 + (int)w ];
}

template < bool x >
NOALIAS __forceinline const __m128 Const< __m128 >::Bool()
{
    return m_Bool[x];
}

template < bool x, bool y, bool z, bool w >
NOALIAS __forceinline const __m128 Const< __m128 >::Bool()
{
    return m_Mask[ (int)x * 8 + (int)y * 4 + (int)z * 2 + (int)w ];
}


NOALIAS __forceinline const __m128i Const< __m128i >::Zero()
{
    return _mm_setzero_si128();
}

template < bool b0 >
NOALIAS __forceinline const __m128i Const< __m128i >::Bool()
{
    return *(__m128i*)&ConstVec4Bool<b0,b0,b0,b0>::m_Data;
}

template < bool b0, bool b1, bool b2, bool b3 >
NOALIAS __forceinline const __m128i Const< __m128i >::Bool()
{
    return *(__m128i*)&ConstVec4Bool<b0,b1,b2,b3>::m_Data;
}

template < bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7 >
NOALIAS __forceinline const __m128i Const< __m128i >::Bool()
{
    return *(__m128i*)&ConstVec8Bool<b0,b1,b2,b3,b4,b5,b6,b7>::m_Data;
}

template < bool b0, bool b1, bool b2,  bool b3,  bool b4,  bool b5,  bool b6,  bool b7,
           bool b8, bool b9, bool b10, bool b11, bool b12, bool b13, bool b14, bool b15 >
NOALIAS __forceinline const __m128i Const< __m128i >::Bool()
{
    return *(__m128i*)&ConstVec16Bool<b0,b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15>::m_Data;
}

NOALIAS __forceinline const __m128i Const< __m128i >::SignMaskF32()
{
    return m_SignMaskF32;
}

NOALIAS __forceinline const __m128i Const< __m128i >::ExponentMaskF32()
{
    return m_ExponentMaskF32;
}

NOALIAS __forceinline const __m128i Const< __m128i >::MantissaMaskF32()
{
    return m_MantissaMaskF32;
}

#endif // #if defined( _SSE_ )




}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )




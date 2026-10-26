#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CONST_INL
#define SOFT_MATHS_HVEC_NEW_CONST_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CONST_INL
#define MATHS_HVEC_NEW_CONST_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Constants
/// @{

/// VBool constant true
NOALIAS __forceinline const VBool Const< VBool >::True()
{
    //return MakeVBool< true >();
    VBool result;
    #if defined( _SPU_ )
    result.m_V = Const< vector unsigned int >::Bool< true >();
    #elif defined( _PPU_ )
    result.m_V = Const< vector bool int >::Bool< true >();
    #elif defined( _VMX128_ )
    result.m_V = Const< __vector4 >::Bool< true >();
    #elif defined( _SSE_ )
    result.m_V = Const< __m128 >::Bool< true >();
    #else
    result.m_V = true;
    #endif
    return result;
}

/// VBool constant false
NOALIAS __forceinline const VBool Const< VBool >::False()
{
    //return MakeVBool< false >();
    VBool result;
    #if defined( _SPU_ )
    result.m_V = Const< vector unsigned int >::Bool< false >();
    #elif defined( _PPU_ )
    result.m_V = Const< vector bool int >::Bool< false >();
    #elif defined( _VMX128_ )
    result.m_V = Const< __vector4 >::Bool< false >();
    #elif defined( _SSE_ )
    result.m_V = Const< __m128 >::Bool< false >();
    #else
    result.m_V = false;
    #endif
    return result;
}


/// Vec4Bool constant true
NOALIAS __forceinline const Vec4Bool Const< Vec4Bool >::True()
{
    //return MakeVec4Bool< true, true, true, true >();
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = Const< vector unsigned int >::Bool< true, true, true, true >();
    #elif defined( _PPU_ )
    result.m_V = Const< vector bool int >::Bool< true, true, true, true >();
    #elif defined( _VMX128_ )
    result.m_V = Const< __vector4 >::Bool< true, true, true, true >();
    #elif defined( _SSE_ )
    result.m_V = Const< __m128 >::Bool< true, true, true, true >();
    #else
    result.m_V[VX] = true;
    result.m_V[VY] = true;
    result.m_V[VZ] = true;
    result.m_V[VW] = true;
    #endif
    return result;
}

/// Vec4Bool constant false
NOALIAS __forceinline const Vec4Bool Const< Vec4Bool >::False()
{
    //return MakeVec4Bool< false, false, false, false >();
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = Const< vector unsigned int >::Bool< false, false, false, false >();
    #elif defined( _PPU_ )
    result.m_V = Const< vector bool int >::Bool< false, false, false, false >();
    #elif defined( _VMX128_ )
    result.m_V = Const< __vector4 >::Bool< false, false, false, false >();
    #elif defined( _SSE_ )
    result.m_V = Const< __m128 >::Bool< false, false, false, false >();
    #else
    result.m_V[VX] = false;
    result.m_V[VY] = false;
    result.m_V[VZ] = false;
    result.m_V[VW] = false;
    #endif
    return result;
}


/// VF32 constant integer template
template < int I > NOALIAS __forceinline const VF32 Const< VF32 >::Int()
{
    VF32 result;
    #if defined( _SPU_ )
    if ( I == 0 ) result.m_V = Const< vector float >::Zero();
    else          result.m_V = spu_splats( ScalarConstInt<I>::m_Value );
    #elif defined( _PPU_ )
    if ( I == 0 ) result.m_V = Const< vector float >::Zero();
    //else          result.m_V = vec_splats( ScalarConstInt<I>::m_Value );
    else          result.m_V = vec_splat( vec_lvlx( 0x0, &ScalarConstInt<I>::m_Value ), 0 );
    #elif defined( _VMX128_ )
    if ( I == 0 )                           result.m_V = __vzero();
    //else if ( ( I >= -16 ) && ( I <= 15 ) ) result.m_V = __vcfsx( __vspltisw( Maths::Min(Maths::Max(I,-16),15) ), 0 );
    else if ( ( I >= -16 ) && ( I <= 15 ) ) result.m_V = __vcfsx( __vspltisw( ( (I>=-16) && (I<=15) ) ? I : 0 ), 0 );
    else                                    result.m_V = __vspltw( __lvlx( &ScalarConstInt<I>::m_Value, 0x0 ), VX );
    #elif defined( _SSE_ )
    if ( I == 0 ) result.m_V = _mm_setzero_ps();
    else          result.m_V = _mm_set1_ps( ScalarConstInt<I>::m_Value );
    #else
    result.m_V = (F32)I;
    #endif
    return result;
}

/// VF32 constant integer fraction template
template < int Num, int Denom > NOALIAS __forceinline const VF32 Const< VF32 >::Frac()
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_splats( ScalarConstFraction<Num,Denom>::m_Value );
    #elif defined( _PPU_ )
    //result.m_V = vec_splats( ScalarConstFraction<Num,Denom>::m_Value );
    result.m_V = vec_splat( vec_lvlx( 0x0, &ScalarConstFraction<Num,Denom>::m_Value ), 0 );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( __lvlx( &ScalarConstFraction<Num,Denom>::m_Value, 0x0 ), VX );
    #elif defined( _SSE_ )
    result.m_V = _mm_set1_ps( ScalarConstFraction<Num,Denom>::m_Value );
    #else
    result.m_V = (F32)Num / (F32)Denom;
    #endif
    return result;
}

// Common VF32 integer constants
NOALIAS __forceinline const VF32 Const< VF32 >::Zero()      { return Const< VF32 >::Int<0>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::One()       { return Const< VF32 >::Int<1>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Two()       { return Const< VF32 >::Int<2>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Three()     { return Const< VF32 >::Int<3>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Four()      { return Const< VF32 >::Int<4>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Five()      { return Const< VF32 >::Int<5>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Six()       { return Const< VF32 >::Int<6>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Seven()     { return Const< VF32 >::Int<7>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Eight()     { return Const< VF32 >::Int<8>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Nine()      { return Const< VF32 >::Int<9>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Ten()       { return Const< VF32 >::Int<10>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Eleven()    { return Const< VF32 >::Int<11>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Twelve()    { return Const< VF32 >::Int<12>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Thirteen()  { return Const< VF32 >::Int<13>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Fourteen()  { return Const< VF32 >::Int<14>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Fifteen()   { return Const< VF32 >::Int<15>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Sixteen()   { return Const< VF32 >::Int<16>(); }

NOALIAS __forceinline const VF32 Const< VF32 >::MinusOne()   { return Const< VF32 >::Int<-1>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::MinusTwo()   { return Const< VF32 >::Int<-2>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::MinusThree() { return Const< VF32 >::Int<-3>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::MinusFour()  { return Const< VF32 >::Int<-4>(); }

NOALIAS __forceinline const VF32 Const< VF32 >::Half()      { return Const< VF32 >::Frac<1,2>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Third()     { return Const< VF32 >::Frac<1,3>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Quarter()   { return Const< VF32 >::Frac<1,4>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Fifth()     { return Const< VF32 >::Frac<1,5>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Sixth()     { return Const< VF32 >::Frac<1,6>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Seventh()   { return Const< VF32 >::Frac<1,7>(); }
NOALIAS __forceinline const VF32 Const< VF32 >::Eighth()    { return Const< VF32 >::Frac<1,8>(); }

/// Load VF32 from an F32
NOALIAS __forceinline const VF32 LoadVF32( F32 const * const __restrict pSrc )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_splats( *pSrc );
    #elif defined( _PPU_ )
    result.m_V = vec_splat( vec_lvlx( 0x0, pSrc ), 0 );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( __lvlx( pSrc, 0x0 ), VX );
    #elif defined( _SSE_ )
    result.m_V = _mm_set1_ps( *pSrc );
    #else
    result.m_V = *pSrc;
    #endif
    return result;
}

NOALIAS __forceinline const VF32 Const< VF32 >::Min()               { return LoadVF32( &m_Min.m_Data ); }
NOALIAS __forceinline const VF32 Const< VF32 >::Max()               { return LoadVF32( &m_Max.m_Data ); }
NOALIAS __forceinline const VF32 Const< VF32 >::Epsilon()           { return LoadVF32( &m_Epsilon.m_Data ); }
NOALIAS __forceinline const VF32 Const< VF32 >::EpsilonSquared()    { return LoadVF32( &m_EpsilonSquared.m_Data ); }
NOALIAS __forceinline const VF32 Const< VF32 >::Epsilon2()          { return LoadVF32( &m_Epsilon2.m_Data ); }
NOALIAS __forceinline const VF32 Const< VF32 >::Epsilon3()          { return LoadVF32( &m_Epsilon3.m_Data ); }
NOALIAS __forceinline const VF32 Const< VF32 >::Pi()                { return LoadVF32( &m_Pi.m_Data ); }
NOALIAS __forceinline const VF32 Const< VF32 >::TwoPi()             { return LoadVF32( &m_TwoPi.m_Data ); }
NOALIAS __forceinline const VF32 Const< VF32 >::RcpPi()             { return LoadVF32( &m_RcpPi.m_Data ); }
NOALIAS __forceinline const VF32 Const< VF32 >::Rcp2Pi()            { return LoadVF32( &m_Rcp2Pi.m_Data ); }
NOALIAS __forceinline const VF32 Const< VF32 >::PiDiv2()            { return LoadVF32( &m_PiDiv2.m_Data ); }
NOALIAS __forceinline const VF32 Const< VF32 >::Sqrt2()             { return LoadVF32( &m_Sqrt2.m_Data ); }
NOALIAS __forceinline const VF32 Const< VF32 >::Sqrt2div2()         { return LoadVF32( &m_Sqrt2div2.m_Data ); }
NOALIAS __forceinline const VF32 Const< VF32 >::Sqrt3div2()         { return LoadVF32( &m_Sqrt3div2.m_Data ); }
NOALIAS __forceinline const VF32 Const< VF32 >::e()                 { return LoadVF32( &m_e.m_Data ); }
NOALIAS __forceinline const VF32 Const< VF32 >::PointNineNine()		{ return LoadVF32( &m_PointNineNine.m_Data );         }


/// Vec4F32 constant integer template
template < int I > NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Int()
{
    Vec4F32 result;
    #if defined( _SPU_ )
    if ( I == 0 ) result.m_V = Const< vector float >::Zero();
    else          result.m_V = spu_splats( ScalarConstInt<I>::m_Value );
    #elif defined( _PPU_ )
    if ( I == 0 ) result.m_V = Const< vector float >::Zero();
    //else          result.m_V = vec_splats( ScalarConstInt<I>::m_Value );
    else          result.m_V = vec_splat( vec_lvlx( 0x0, &ScalarConstInt<I>::m_Value ), 0 );
    #elif defined( _VMX128_ )
    if ( I == 0 )                           result.m_V = __vzero();
    //else if ( ( I >= -16 ) && ( I <= 15 ) ) result.m_V = __vcfsx( __vspltisw( Maths::Min(Maths::Max(I,-16),15) ), 0 );
    else if ( ( I >= -16 ) && ( I <= 15 ) ) result.m_V = __vcfsx( __vspltisw( ( (I>=-16) && (I<=15) ) ? I : 0 ), 0 );
    else                                    result.m_V = __vspltw( __lvlx( &ScalarConstInt<I>::m_Value, 0x0 ), VX );
    #elif defined( _SSE_ )
    if ( I == 0 ) result.m_V = _mm_setzero_ps();
    else          result.m_V = _mm_set1_ps( ScalarConstInt<I>::m_Value );
    #else
    result.m_V[VX] =
    result.m_V[VY] =
    result.m_V[VZ] =
    result.m_V[VW] = (F32)I;
    #endif
    return result;
}

/// Vec4F32 constant integer template
template < int X, int Y, int Z, int W > NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::VecInt()
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = *(vector float*)&VectorConstInt<X,Y,Z,W>::m_Data;
    #elif defined( _PPU_ )
    result.m_V = *(vector float*)&VectorConstInt<X,Y,Z,W>::m_Data;
    #elif defined( _VMX128_ )
    result.m_V = *(__vector4*)&VectorConstInt<X,Y,Z,W>::m_Data;
    #elif defined( _SSE_ )
    result.m_V = *(__m128*)&VectorConstInt<X,Y,Z,W>::m_Data;
    #else
    result.m_V[VX] = (F32)X;
    result.m_V[VY] = (F32)Y;
    result.m_V[VZ] = (F32)Z;
    result.m_V[VW] = (F32)W;
    #endif
    return result;
}

/// Vec4F32 constant integer fraction template
template < int Num, int Denom > NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Frac()
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_splats( ScalarConstFraction<Num,Denom>::m_Value );
    #elif defined( _PPU_ )
    //result.m_V = vec_splats( ScalarConstFraction<Num,Denom>::m_Value );
    result.m_V = vec_splat( vec_lvlx( 0x0, &ScalarConstFraction<Num,Denom>::m_Value ), 0 );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( __lvlx( &ScalarConstFraction<Num,Denom>::m_Value, 0x0 ), VX );
    #elif defined( _SSE_ )
    result.m_V = _mm_set1_ps( ScalarConstFraction<Num,Denom>::m_Value );
    #else
    result.m_V[VX] =
    result.m_V[VY] =
    result.m_V[VZ] =
    result.m_V[VW] = (F32)Num / (F32)Denom;
    #endif
    return result;
}

/// Common Vec4F32 integer constants
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Zero()      { return Const< Vec4F32 >::Int<0>(); }  ///< Constant Vec4F32(0,0,0,0)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::One()       { return Const< Vec4F32 >::Int<1>(); }  ///< Constant Vec4F32(1,1,1,1)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Two()       { return Const< Vec4F32 >::Int<2>(); }  ///< Constant Vec4F32(2,2,2,2)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Three()     { return Const< Vec4F32 >::Int<3>(); }  ///< Constant Vec4F32(3,3,3,3)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Four()      { return Const< Vec4F32 >::Int<4>(); }  ///< Constant Vec4F32(4,4,4,4)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Five()      { return Const< Vec4F32 >::Int<5>(); }  ///< Constant Vec4F32(5,5,5,5)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Six()       { return Const< Vec4F32 >::Int<6>(); }  ///< Constant Vec4F32(6,6,6,6)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Seven()     { return Const< Vec4F32 >::Int<7>(); }  ///< Constant Vec4F32(7,7,7,7)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Eight()     { return Const< Vec4F32 >::Int<8>(); }  ///< Constant Vec4F32(8,8,8,8)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Nine()      { return Const< Vec4F32 >::Int<9>(); }  ///< Constant Vec4F32(9,9,9,9)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Ten()       { return Const< Vec4F32 >::Int<10>(); } ///< Constant Vec4F32(10,10,10,10)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Eleven()    { return Const< Vec4F32 >::Int<11>(); } ///< Constant Vec4F32(11,11,11,11)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Twelve()    { return Const< Vec4F32 >::Int<12>(); } ///< Constant Vec4F32(12,12,12,12)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Thirteen()  { return Const< Vec4F32 >::Int<13>(); } ///< Constant Vec4F32(13,13,13,13)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Fourteen()  { return Const< Vec4F32 >::Int<14>(); } ///< Constant Vec4F32(14,14,14,14)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Fifteen()   { return Const< Vec4F32 >::Int<15>(); } ///< Constant Vec4F32(15,15,15,15)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Sixteen()   { return Const< Vec4F32 >::Int<16>(); } ///< Constant Vec4F32(16,16,16,16)

NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::MinusOne()   { return Const< Vec4F32 >::Int<-1>(); } ///< Constant Vec4F32(-1,-1,-1,-1)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::MinusTwo()   { return Const< Vec4F32 >::Int<-2>(); } ///< Constant Vec4F32(-2,-2,-2,-2)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::MinusThree() { return Const< Vec4F32 >::Int<-3>(); } ///< Constant Vec4F32(-3,-3,-3,-3)
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::MinusFour()  { return Const< Vec4F32 >::Int<-4>(); } ///< Constant Vec4F32(-4,-4,-4,-4)

NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::UnitX()
{
    Vec4F32 result;
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = Const< vector float >::UnitX();
    #elif defined( _VMX128_ )
    result.m_V = Const< __vector4 >::UnitX();
    #elif defined( _SSE_ )
    result.m_V = Const< __m128 >::UnitX();
    #else
    result.m_V[0] = 1.0f;
    result.m_V[1] = 0.0f;
    result.m_V[2] = 0.0f;
    result.m_V[3] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::UnitY()
{
    Vec4F32 result;
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = Const< vector float >::UnitY();
    #elif defined( _VMX128_ )
    result.m_V = Const< __vector4 >::UnitY();
    #elif defined( _SSE_ )
    result.m_V = Const< __m128 >::UnitY();
    #else
    result.m_V[0] = 0.0f;
    result.m_V[1] = 1.0f;
    result.m_V[2] = 0.0f;
    result.m_V[3] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::UnitZ()
{
    Vec4F32 result;
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = Const< vector float >::UnitZ();
    #elif defined( _VMX128_ )
    result.m_V = Const< __vector4 >::UnitZ();
    #elif defined( _SSE_ )
    result.m_V = Const< __m128 >::UnitZ();
    #else
    result.m_V[0] = 0.0f;
    result.m_V[1] = 0.0f;
    result.m_V[2] = 1.0f;
    result.m_V[3] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::UnitW()
{
    Vec4F32 result;
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = Const< vector float >::UnitW();
    #elif defined( _VMX128_ )
    result.m_V = Const< __vector4 >::UnitW();
    #elif defined( _SSE_ )
    result.m_V = Const< __m128 >::UnitW();
    #else
    result.m_V[0] = 0.0f;
    result.m_V[1] = 0.0f;
    result.m_V[2] = 0.0f;
    result.m_V[3] = 1.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::NegativeUnitX()
{
    Vec4F32 result;
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = Const< vector float >::NegativeUnitX();
    #elif defined( _VMX128_ )
    result.m_V = Const< __vector4 >::NegativeUnitX();
    #elif defined( _SSE_ )
    result.m_V = Const< __m128 >::NegativeUnitX();
    #else
    result.m_V[0] = -1.0f;
    result.m_V[1] = 0.0f;
    result.m_V[2] = 0.0f;
    result.m_V[3] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::NegativeUnitY()
{
    Vec4F32 result;
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = Const< vector float >::NegativeUnitY();
    #elif defined( _VMX128_ )
    result.m_V = Const< __vector4 >::NegativeUnitY();
    #elif defined( _SSE_ )
    result.m_V = Const< __m128 >::NegativeUnitY();
    #else
    result.m_V[0] = 0.0f;
    result.m_V[1] = -1.0f;
    result.m_V[2] = 0.0f;
    result.m_V[3] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::NegativeUnitZ()
{
    Vec4F32 result;
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = Const< vector float >::NegativeUnitZ();
    #elif defined( _VMX128_ )
    result.m_V = Const< __vector4 >::NegativeUnitZ();
    #elif defined( _SSE_ )
    result.m_V = Const< __m128 >::NegativeUnitZ();
    #else
    result.m_V[0] = 0.0f;
    result.m_V[1] = 0.0f;
    result.m_V[2] = -1.0f;
    result.m_V[3] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::NegativeUnitW()
{
    Vec4F32 result;
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = Const< vector float >::NegativeUnitW();
    #elif defined( _VMX128_ )
    result.m_V = Const< __vector4 >::NegativeUnitW();
    #elif defined( _SSE_ )
    result.m_V = Const< __m128 >::NegativeUnitW();
    #else
    result.m_V[0] = 0.0f;
    result.m_V[1] = 0.0f;
    result.m_V[2] = 0.0f;
    result.m_V[3] = -1.0f;
    #endif
    return result;
}

// Common Vec4F32 integer fraction constants

NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Half()      { return Const< Vec4F32 >::Frac<1,2>(); } ///< Constant Vec4F32( 1/2, 1/2, 1/2, 1/2 )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Third()     { return Const< Vec4F32 >::Frac<1,3>(); } ///< Constant Vec4F32( 1/3, 1/3, 1/3, 1/3 )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Quarter()   { return Const< Vec4F32 >::Frac<1,4>(); } ///< Constant Vec4F32( 1/4, 1/4, 1/4, 1/4 )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Fifth()     { return Const< Vec4F32 >::Frac<1,5>(); } ///< Constant Vec4F32( 1/5, 1/5, 1/5, 1/5 )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Sixth()     { return Const< Vec4F32 >::Frac<1,6>(); } ///< Constant Vec4F32( 1/6, 1/6, 1/6, 1/6 )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Seventh()   { return Const< Vec4F32 >::Frac<1,7>(); } ///< Constant Vec4F32( 1/7, 1/7, 1/7, 1/7 )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Eighth()    { return Const< Vec4F32 >::Frac<1,8>(); } ///< Constant Vec4F32( 1/8, 1/8, 1/8, 1/8 )

// Vec4F32 constants

#if defined( _SIMD_ )

/// Load Vec4F32 from an F32 array
NOALIAS __forceinline const Vec4F32 LoadVec4F32( F32 const * const __restrict pSrc )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_load_unaligned( pSrc );
    #elif defined( _PPU_ )
    result.m_V = vec_load_unaligned( pSrc );
    #elif defined( _VMX128_ )
    result.m_V = vmx_load_unaligned( pSrc );
    #elif defined( _SSE_ )
    result.m_V = sse_load_unaligned( pSrc );
    #else
    result.m_V[VX] = pSrc[VX];
    result.m_V[VY] = pSrc[VY];
    result.m_V[VZ] = pSrc[VZ];
    result.m_V[VW] = pSrc[VW];
    #endif
    return result;
}

NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Min()       { return LoadVec4F32( &Const< VF32 >::m_Min.m_Data );       } ///< Constant Vec4F32( FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Max()       { return LoadVec4F32( &Const< VF32 >::m_Max.m_Data );       } ///< Constant Vec4F32( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Epsilon()   { return LoadVec4F32( &Const< VF32 >::m_Epsilon.m_Data );   } ///< Constant Vec4F32( FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Epsilon2()  { return LoadVec4F32( &Const< VF32 >::m_Epsilon2.m_Data );  } ///< Constant Vec4F32( 0.01, 0.01, 0.01, 0.01 )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Epsilon3()  { return LoadVec4F32( &Const< VF32 >::m_Epsilon3.m_Data );  } ///< Constant Vec4F32( 1e-12, 1e-12, 1e-12, 1e-12 )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Pi()        { return LoadVec4F32( &Const< VF32 >::m_Pi.m_Data );        } ///< Constant Vec4F32( PI, PI, PI, PI )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::TwoPi()     { return LoadVec4F32( &Const< VF32 >::m_TwoPi.m_Data );     } ///< Constant Vec4F32( PI*2, PI*2, PI*2, PI*2 )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::RcpPi()     { return LoadVec4F32( &Const< VF32 >::m_RcpPi.m_Data );     } ///< Constant Vec4F32( 1/PI, 1/PI, 1/PI, 1/PI )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Rcp2Pi()    { return LoadVec4F32( &Const< VF32 >::m_Rcp2Pi.m_Data );    } ///< Constant Vec4F32( 1/(PI*2), 1/(PI*2), 1/(PI*2), 1/(PI*2) )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::PiDiv2()    { return LoadVec4F32( &Const< VF32 >::m_PiDiv2.m_Data );    } ///< Constant Vec4F32( PI/2, PI/2, PI/2, PI/2 )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Sqrt2()     { return LoadVec4F32( &Const< VF32 >::m_Sqrt2.m_Data );     } ///< Constant Vec4F32( sqrt(2), sqrt(2), sqrt(2), sqrt(2) )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Sqrt2div2() { return LoadVec4F32( &Const< VF32 >::m_Sqrt2div2.m_Data ); } ///< Constant Vec4F32( sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, sqrt(2)/2 )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::e()         { return LoadVec4F32( &Const< VF32 >::m_e.m_Data );         } ///< Constant Vec4F32( e, e, e, e ) : base for natural logarithm e ~= 2.718282

#else

NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Min()       { Vec4F32 result; result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = Const< VF32 >::m_Min.m_Data;       return result; } ///< Constant Vec4F32( FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Max()       { Vec4F32 result; result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = Const< VF32 >::m_Max.m_Data;       return result; } ///< Constant Vec4F32( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Epsilon()   { Vec4F32 result; result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = Const< VF32 >::m_Epsilon.m_Data;   return result; } ///< Constant Vec4F32( FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Epsilon2()  { Vec4F32 result; result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = Const< VF32 >::m_Epsilon2.m_Data;  return result; } ///< Constant Vec4F32( 0.01, 0.01, 0.01, 0.01 )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Epsilon3()  { Vec4F32 result; result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = Const< VF32 >::m_Epsilon3.m_Data;  return result; } ///< Constant Vec4F32( 1e-12, 1e-12, 1e-12, 1e-12 )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Pi()        { Vec4F32 result; result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = Const< VF32 >::m_Pi.m_Data;        return result; } ///< Constant Vec4F32( PI, PI, PI, PI )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::TwoPi()     { Vec4F32 result; result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = Const< VF32 >::m_TwoPi.m_Data;     return result; } ///< Constant Vec4F32( PI*2, PI*2, PI*2, PI*2 )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::RcpPi()     { Vec4F32 result; result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = Const< VF32 >::m_RcpPi.m_Data;     return result; } ///< Constant Vec4F32( 1/PI, 1/PI, 1/PI, 1/PI )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Rcp2Pi()    { Vec4F32 result; result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = Const< VF32 >::m_Rcp2Pi.m_Data;    return result; } ///< Constant Vec4F32( 1/(PI*2), 1/(PI*2), 1/(PI*2), 1/(PI*2) )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::PiDiv2()    { Vec4F32 result; result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = Const< VF32 >::m_PiDiv2.m_Data;    return result; } ///< Constant Vec4F32( PI/2, PI/2, PI/2, PI/2 )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Sqrt2()     { Vec4F32 result; result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = Const< VF32 >::m_Sqrt2.m_Data;     return result; } ///< Constant Vec4F32( sqrt(2), sqrt(2), sqrt(2), sqrt(2) )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::Sqrt2div2() { Vec4F32 result; result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = Const< VF32 >::m_Sqrt2div2.m_Data; return result; } ///< Constant Vec4F32( sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, sqrt(2)/2 )
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::e()         { Vec4F32 result; result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = Const< VF32 >::m_e.m_Data;         return result; } ///< Constant Vec4F32( e, e, e, e ) : base for natural logarithm e ~= 2.718282

#endif

template < bool X, bool Y, bool Z, bool W >
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::XorNegate()
{
    return m_XorNegate[ (int)X * 8 + (int)Y * 4 + (int)Z * 2 + (int)W ];
}

NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::XorNegateX() { return m_XorNegateX; }
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::XorNegateY() { return m_XorNegateY; }
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::XorNegateZ() { return m_XorNegateZ; }
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::XorNegateW() { return m_XorNegateW; }

NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::XorNegateXY() { return m_XorNegateXY; }
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::XorNegateXZ() { return m_XorNegateXZ; }
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::XorNegateXW() { return m_XorNegateXW; }
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::XorNegateYZ() { return m_XorNegateYZ; }
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::XorNegateYW() { return m_XorNegateYW; }
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::XorNegateZW() { return m_XorNegateZW; }

NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::XorNegateXYZ() { return m_XorNegateXYZ; }
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::XorNegateXYW() { return m_XorNegateXYW; }
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::XorNegateXZW() { return m_XorNegateXZW; }
NOALIAS __forceinline const Vec4F32 Const< Vec4F32 >::XorNegateYZW() { return m_XorNegateYZW; }


/// Vec8F32 constant integer template
template < int I > NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::Int()
{
    Vec8F32 result;
    result.m_V0 = result.m_V1 = Const<Vec4F32>::Int<I>();
    return result;
}

/// Vec8F32 constant integer fraction template
template < int Num, int Denom > NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::Frac()
{
    Vec8F32 result;
    result.m_V0 = result.m_V1 = Const<Vec4F32>::Frac<Num,Denom>();
    return result;
}

// Common Vec8F32 integer constants

NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::Zero()            { return Const< Vec8F32 >::Int<0>(); }      ///< Constant Vec8F32(0,0,0,0,0,0,0,0)
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::One()             { return Const< Vec8F32 >::Int<1>(); }      ///< Constant Vec8F32(1,1,1,1,1,1,1,1)
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::Two()             { return Const< Vec8F32 >::Int<2>(); }      ///< Constant Vec8F32(2,2,2,2,2,2,2,2)
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::Three()           { return Const< Vec8F32 >::Int<3>(); }      ///< Constant Vec8F32(3,3,3,3,3,3,3,3)
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::Four()            { return Const< Vec8F32 >::Int<4>(); }      ///< Constant Vec8F32(4,4,4,4,4,4,4,4)
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::MinusOne()        { return Const< Vec8F32 >::Int<-1>(); }     ///< Constant Vec8F32(-1,-1,-1,-1,-1,-1,-1,-1)
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::MinusTwo()        { return Const< Vec8F32 >::Int<-2>(); }     ///< Constant Vec8F32(-2,-2,-2,-2,-2,-2,-2,-2)
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::MinusThree()      { return Const< Vec8F32 >::Int<-3>(); }     ///< Constant Vec8F32(-3,-3,-3,-3,-3,-3,-3,-3)
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::MinusFour()       { return Const< Vec8F32 >::Int<-4>(); }     ///< Constant Vec8F32(-4,-4,-4,-4,-4,-4,-4,-4)

NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::UnitElement0()    { Vec8F32 result; result.m_V0 = Const<Vec4F32>::UnitX(); result.m_V1 = Const<Vec4F32>::Zero();  return result; }  ///< Constant Vec8F32(1,0,0,0,0,0,0,0)
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::UnitElement1()    { Vec8F32 result; result.m_V0 = Const<Vec4F32>::UnitY(); result.m_V1 = Const<Vec4F32>::Zero();  return result; }  ///< Constant Vec8F32(0,1,0,0,0,0,0,0)
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::UnitElement2()    { Vec8F32 result; result.m_V0 = Const<Vec4F32>::UnitZ(); result.m_V1 = Const<Vec4F32>::Zero();  return result; }  ///< Constant Vec8F32(0,0,1,0,0,0,0,0)
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::UnitElement3()    { Vec8F32 result; result.m_V0 = Const<Vec4F32>::UnitW(); result.m_V1 = Const<Vec4F32>::Zero();  return result; }  ///< Constant Vec8F32(0,0,0,1,0,0,0,0)
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::UnitElement4()    { Vec8F32 result; result.m_V0 = Const<Vec4F32>::Zero();  result.m_V1 = Const<Vec4F32>::UnitX(); return result; }  ///< Constant Vec8F32(0,0,0,0,1,0,0,0)
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::UnitElement5()    { Vec8F32 result; result.m_V0 = Const<Vec4F32>::Zero();  result.m_V1 = Const<Vec4F32>::UnitY(); return result; }  ///< Constant Vec8F32(0,0,0,0,0,1,0,0)
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::UnitElement6()    { Vec8F32 result; result.m_V0 = Const<Vec4F32>::Zero();  result.m_V1 = Const<Vec4F32>::UnitZ(); return result; }  ///< Constant Vec8F32(0,0,0,0,0,0,1,0)
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::UnitElement7()    { Vec8F32 result; result.m_V0 = Const<Vec4F32>::Zero();  result.m_V1 = Const<Vec4F32>::UnitW(); return result; }  ///< Constant Vec8F32(0,0,0,0,0,0,0,1)

NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::Min()             { Vec8F32 result; result.m_V0 = result.m_V1 = Const<Vec4F32>::Min();       return result; } ///< Constant Vec8F32( FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN, ... )
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::Max()             { Vec8F32 result; result.m_V0 = result.m_V1 = Const<Vec4F32>::Max();       return result; } ///< Constant Vec8F32( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, ... )
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::Epsilon()         { Vec8F32 result; result.m_V0 = result.m_V1 = Const<Vec4F32>::Epsilon();   return result; } ///< Constant Vec8F32( FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, ... )
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::Epsilon2()        { Vec8F32 result; result.m_V0 = result.m_V1 = Const<Vec4F32>::Epsilon2();  return result; } ///< Constant Vec8F32( 0.01, 0.01, 0.01, 0.01, ... )
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::Epsilon3()        { Vec8F32 result; result.m_V0 = result.m_V1 = Const<Vec4F32>::Epsilon3();  return result; } ///< Constant Vec8F32( 1e-12, 1e-12, 1e-12, 1e-12, ... )
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::Pi()              { Vec8F32 result; result.m_V0 = result.m_V1 = Const<Vec4F32>::Pi();        return result; } ///< Constant Vec8F32( PI, PI, PI, PI, ... )
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::TwoPi()           { Vec8F32 result; result.m_V0 = result.m_V1 = Const<Vec4F32>::TwoPi();     return result; } ///< Constant Vec8F32( PI*2, PI*2, PI*2, PI*2, ... )
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::RcpPi()           { Vec8F32 result; result.m_V0 = result.m_V1 = Const<Vec4F32>::RcpPi();     return result; } ///< Constant Vec8F32( 1/PI, 1/PI, 1/PI, 1/PI, ... )
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::Rcp2Pi()          { Vec8F32 result; result.m_V0 = result.m_V1 = Const<Vec4F32>::Rcp2Pi();    return result; } ///< Constant Vec8F32( 1/(PI*2), 1/(PI*2), 1/(PI*2), 1/(PI*2), ... )
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::PiDiv2()          { Vec8F32 result; result.m_V0 = result.m_V1 = Const<Vec4F32>::PiDiv2();    return result; } ///< Constant Vec8F32( PI/2, PI/2, PI/2, PI/2, ... )
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::Sqrt2()           { Vec8F32 result; result.m_V0 = result.m_V1 = Const<Vec4F32>::Sqrt2();     return result; } ///< Constant Vec8F32( sqrt(2), sqrt(2), sqrt(2), sqrt(2), ... )
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::Sqrt2div2()       { Vec8F32 result; result.m_V0 = result.m_V1 = Const<Vec4F32>::Sqrt2div2(); return result; } ///< Constant Vec8F32( sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, ... )
NOALIAS __forceinline const Vec8F32 Const< Vec8F32 >::e()               { Vec8F32 result; result.m_V0 = result.m_V1 = Const<Vec4F32>::e();         return result; } ///< Constant Vec8F32( e, e, e, e, ... ) : base for natural logarithm e ~= 2.718282


/// Vec12F32 constant integer template
template < int I > NOALIAS __forceinline const Vec12F32 Const< Vec12F32 >::Int()
{
    Vec12F32 result;
    result.m_V0 = result.m_V1 = result.m_V2 = Const<Vec4F32>::Int<I>();
    return result;
}

/// Vec12F32 constant integer fraction template
template < int Num, int Denom > NOALIAS __forceinline const Vec12F32 Const< Vec12F32 >::Frac()
{
    Vec12F32 result;
    result.m_V0 = result.m_V1 = result.m_V2 = Const<Vec4F32>::Frac<Num,Denom>();
    return result;
}

/// Common Vec12F32 integer constants
NOALIAS __forceinline const Vec12F32 Const< Vec12F32 >::Zero()
{
    return Const< Vec12F32 >::Int<0>();
}


/// Constant Vector2(I,I)
template < int I >
NOALIAS __forceinline const Vector2 Const< Vector2 >::Int()
{
    Vector2 result;
    result.m_V = Const<Vec4F32>::Int<I>();
    return result;
}

/// Constant Vector2( Num/Denom, Num/Denom )
template < int Num, int Denom >
NOALIAS __forceinline const Vector2 Const< Vector2 >::Frac()
{
    Vector2 result;
    result.m_V = Const<Vec4F32>::Frac<Num,Denom>();
    return result;
}

NOALIAS __forceinline const Vector2 Const< Vector2 >::Zero() ///< Constant Vector2(0,0)
{
    //return MakeVector2( Const<Vec4F32>::Zero() );
    Vector2 result;
    result.m_V = Const<Vec4F32>::Zero();
    return result;
}

NOALIAS __forceinline const Vector2 Const< Vector2 >::One() ///< Constant Vector2(1,1)
{
    //return MakeVector2( Const<Vec4F32>::One() );
    Vector2 result;
    result.m_V = Const<Vec4F32>::One();
    return result;
}

NOALIAS __forceinline const Vector2 Const< Vector2 >::Two() ///< Constant Vector2(2,2)
{
    //return MakeVector2( Const<Vec4F32>::Two() );
    Vector2 result;
    result.m_V = Const<Vec4F32>::Two();
    return result;
}

NOALIAS __forceinline const Vector2 Const< Vector2 >::Three() ///< Constant Vector2(3,3)
{
    //return MakeVector2( Const<Vec4F32>::Three() );
    Vector2 result;
    result.m_V = Const<Vec4F32>::Three();
    return result;
}

NOALIAS __forceinline const Vector2 Const< Vector2 >::Four() ///< Constant Vector2(4,4)
{
    //return MakeVector2( Const<Vec4F32>::Four() );
    Vector2 result;
    result.m_V = Const<Vec4F32>::Four();
    return result;
}

NOALIAS __forceinline const Vector2 Const< Vector2 >::MinusOne() ///< Constant Vector2(-1,-1)
{
    //return MakeVector2( Const<Vec4F32>::MinusOne() );
    Vector2 result;
    result.m_V = Const<Vec4F32>::MinusOne();
    return result;
}

NOALIAS __forceinline const Vector2 Const< Vector2 >::MinusTwo() ///< Constant Vector2(-2,-2)
{
    //return MakeVector2( Const<Vec4F32>::MinusTwo() );
    Vector2 result;
    result.m_V = Const<Vec4F32>::MinusTwo();
    return result;
}

NOALIAS __forceinline const Vector2 Const< Vector2 >::MinusThree() ///< Constant Vector2(-3,-3)
{
    //return MakeVector2( Const<Vec4F32>::MinusThree() );
    Vector2 result;
    result.m_V = Const<Vec4F32>::MinusThree();
    return result;
}

NOALIAS __forceinline const Vector2 Const< Vector2 >::MinusFour() ///< Constant Vector2(-4,-4)
{
    //return MakeVector2( Const<Vec4F32>::MinusFour() );
    Vector2 result;
    result.m_V = Const<Vec4F32>::MinusFour();
    return result;
}

NOALIAS __forceinline const Vector2 Const< Vector2 >::UnitX() ///< Constant Vector2(1,0)
{
    //return MakeVector2( Const<Vec4F32>::UnitX() );
    Vector2 result;
    result.m_V = Const<Vec4F32>::UnitX();
    return result;
}

NOALIAS __forceinline const Vector2 Const< Vector2 >::UnitY() ///< Constant Vector2(0,1)
{
    //return MakeVector2( Const<Vec4F32>::UnitY() );
    Vector2 result;
    result.m_V = Const<Vec4F32>::UnitY();
    return result;
}


/// Constant Vector3(I,I,I)
template < int I >
NOALIAS __forceinline const Vector3 Const< Vector3 >::Int()
{
    Vector3 result;
    result.m_V = Const<Vec4F32>::Int<I>();
    return result;
}

/// Constant Vector3( Num/Denom, Num/Denom, Num/Denom )
template < int Num, int Denom >
NOALIAS __forceinline const Vector3 Const< Vector3 >::Frac()
{
    Vector3 result;
    result.m_V = Const<Vec4F32>::Frac<Num,Denom>();
    return result;
}

NOALIAS __forceinline const Vector3 Const< Vector3 >::Zero()        { Vector3 result; result.m_V = Const<Vec4F32>::Zero();       return result; } ///< Constant Vector3(0,0,0)
NOALIAS __forceinline const Vector3 Const< Vector3 >::One()         { Vector3 result; result.m_V = Const<Vec4F32>::One();        return result; } ///< Constant Vector3(1,1,1)
NOALIAS __forceinline const Vector3 Const< Vector3 >::Two()         { Vector3 result; result.m_V = Const<Vec4F32>::Two();        return result; } ///< Constant Vector3(2,2,2)
NOALIAS __forceinline const Vector3 Const< Vector3 >::Three()       { Vector3 result; result.m_V = Const<Vec4F32>::Three();      return result; } ///< Constant Vector3(3,3,3)
NOALIAS __forceinline const Vector3 Const< Vector3 >::Four()        { Vector3 result; result.m_V = Const<Vec4F32>::Four();       return result; } ///< Constant Vector3(4,4,4)
NOALIAS __forceinline const Vector3 Const< Vector3 >::MinusOne()    { Vector3 result; result.m_V = Const<Vec4F32>::MinusOne();   return result; } ///< Constant Vector3(-1,-1,-1)
NOALIAS __forceinline const Vector3 Const< Vector3 >::MinusTwo()    { Vector3 result; result.m_V = Const<Vec4F32>::MinusTwo();   return result; } ///< Constant Vector3(-2,-2,-2)
NOALIAS __forceinline const Vector3 Const< Vector3 >::MinusThree()  { Vector3 result; result.m_V = Const<Vec4F32>::MinusThree(); return result; } ///< Constant Vector3(-3,-3,-3)
NOALIAS __forceinline const Vector3 Const< Vector3 >::MinusFour()   { Vector3 result; result.m_V = Const<Vec4F32>::MinusFour();  return result; } ///< Constant Vector3(-4,-4,-4)

NOALIAS __forceinline const Vector3 Const< Vector3 >::UnitX()       { Vector3 result; result.m_V = Const<Vec4F32>::UnitX(); return result; } ///< Constant Vector3(1,0,0)
NOALIAS __forceinline const Vector3 Const< Vector3 >::UnitY()       { Vector3 result; result.m_V = Const<Vec4F32>::UnitY(); return result; } ///< Constant Vector3(0,1,0)
NOALIAS __forceinline const Vector3 Const< Vector3 >::UnitZ()       { Vector3 result; result.m_V = Const<Vec4F32>::UnitZ(); return result; } ///< Constant Vector3(0,0,1)

NOALIAS __forceinline const Vector3 Const< Vector3 >::Min()         { Vector3 result; result.m_V = Const<Vec4F32>::Min();       return result; } ///< Constant Vector3( FLT_MIN, FLT_MIN, FLT_MIN )
NOALIAS __forceinline const Vector3 Const< Vector3 >::Max()         { Vector3 result; result.m_V = Const<Vec4F32>::Max();       return result; } ///< Constant Vector3( FLT_MAX, FLT_MAX, FLT_MAX )
NOALIAS __forceinline const Vector3 Const< Vector3 >::Epsilon()     { Vector3 result; result.m_V = Const<Vec4F32>::Epsilon();   return result; } ///< Constant Vector3( FLT_EPSILON, FLT_EPSILON, FLT_EPSILON )
NOALIAS __forceinline const Vector3 Const< Vector3 >::Epsilon2()    { Vector3 result; result.m_V = Const<Vec4F32>::Epsilon2();  return result; } ///< Constant Vector3( 0.01, 0.01, 0.01 )
NOALIAS __forceinline const Vector3 Const< Vector3 >::Epsilon3()    { Vector3 result; result.m_V = Const<Vec4F32>::Epsilon3();  return result; } ///< Constant Vector3( 1e-12, 1e-12, 1e-12 )
NOALIAS __forceinline const Vector3 Const< Vector3 >::Pi()          { Vector3 result; result.m_V = Const<Vec4F32>::Pi();        return result; } ///< Constant Vector3( PI, PI, PI )
NOALIAS __forceinline const Vector3 Const< Vector3 >::TwoPi()       { Vector3 result; result.m_V = Const<Vec4F32>::TwoPi();     return result; } ///< Constant Vector3( PI*2, PI*2, PI*2 )
NOALIAS __forceinline const Vector3 Const< Vector3 >::RcpPi()       { Vector3 result; result.m_V = Const<Vec4F32>::RcpPi();     return result; } ///< Constant Vector3( 1/PI, 1/PI, 1/PI )
NOALIAS __forceinline const Vector3 Const< Vector3 >::Rcp2Pi()      { Vector3 result; result.m_V = Const<Vec4F32>::Rcp2Pi();    return result; } ///< Constant Vector3( 1/(PI*2), 1/(PI*2), 1/(PI*2) )
NOALIAS __forceinline const Vector3 Const< Vector3 >::PiDiv2()      { Vector3 result; result.m_V = Const<Vec4F32>::PiDiv2();    return result; } ///< Constant Vector3( PI/2, PI/2, PI/2 )
NOALIAS __forceinline const Vector3 Const< Vector3 >::Sqrt2()       { Vector3 result; result.m_V = Const<Vec4F32>::Sqrt2();     return result; } ///< Constant Vector3( sqrt(2), sqrt(2), sqrt(2) )
NOALIAS __forceinline const Vector3 Const< Vector3 >::Sqrt2div2()   { Vector3 result; result.m_V = Const<Vec4F32>::Sqrt2div2(); return result; } ///< Constant Vector3( sqrt(2)/2, sqrt(2)/2, sqrt(2)/2 )
NOALIAS __forceinline const Vector3 Const< Vector3 >::e()           { Vector3 result; result.m_V = Const<Vec4F32>::e();         return result; } ///< Constant Vector3( e, e, e ) : base for natural logarithm e ~= 2.718282


/// Constant Vector4(I,I,I,I)
template < int I >
NOALIAS __forceinline const Vector4 Const< Vector4 >::Int()
{
    Vector4 result;
    result.m_V = Const<Vec4F32>::Int<I>();
    return result;
}

/// Constant Vector4( Num/Denom, Num/Denom, Num/Denom, Num/Denom )
template < int Num, int Denom >
NOALIAS __forceinline const Vector4 Const< Vector4 >::Frac()
{
    Vector4 result;
    result.m_V = Const<Vec4F32>::Frac<Num,Denom>();
    return result;
}

NOALIAS __forceinline const Vector4 Const< Vector4 >::Zero()        { Vector4 result; result.m_V = Const<Vec4F32>::Zero(); return result; }       ///< Constant Vector4(0,0,0,0)
NOALIAS __forceinline const Vector4 Const< Vector4 >::One()         { Vector4 result; result.m_V = Const<Vec4F32>::One(); return result; }        ///< Constant Vector4(1,1,1,1)
NOALIAS __forceinline const Vector4 Const< Vector4 >::Two()         { Vector4 result; result.m_V = Const<Vec4F32>::Two(); return result; }        ///< Constant Vector4(2,2,2,2)
NOALIAS __forceinline const Vector4 Const< Vector4 >::Three()       { Vector4 result; result.m_V = Const<Vec4F32>::Three(); return result; }      ///< Constant Vector4(3,3,3,3)
NOALIAS __forceinline const Vector4 Const< Vector4 >::Four()        { Vector4 result; result.m_V = Const<Vec4F32>::Four(); return result; }       ///< Constant Vector4(4,4,4,4)
NOALIAS __forceinline const Vector4 Const< Vector4 >::MinusOne()    { Vector4 result; result.m_V = Const<Vec4F32>::MinusOne(); return result; }   ///< Constant Vector4(-1,-1,-1,-1)
NOALIAS __forceinline const Vector4 Const< Vector4 >::MinusTwo()    { Vector4 result; result.m_V = Const<Vec4F32>::MinusTwo(); return result; }   ///< Constant Vector4(-2,-2,-2,-2)
NOALIAS __forceinline const Vector4 Const< Vector4 >::MinusThree()  { Vector4 result; result.m_V = Const<Vec4F32>::MinusThree(); return result; } ///< Constant Vector4(-3,-3,-3,-3)
NOALIAS __forceinline const Vector4 Const< Vector4 >::MinusFour()   { Vector4 result; result.m_V = Const<Vec4F32>::MinusFour(); return result; }  ///< Constant Vector4(-4,-4,-4,-4)

NOALIAS __forceinline const Vector4 Const< Vector4 >::UnitX()       { Vector4 result; result.m_V = Const<Vec4F32>::UnitX(); return result; }      ///< Constant Vector4(1,0,0,0)
NOALIAS __forceinline const Vector4 Const< Vector4 >::UnitY()       { Vector4 result; result.m_V = Const<Vec4F32>::UnitY(); return result; }      ///< Constant Vector4(0,1,0,0)
NOALIAS __forceinline const Vector4 Const< Vector4 >::UnitZ()       { Vector4 result; result.m_V = Const<Vec4F32>::UnitZ(); return result; }      ///< Constant Vector4(0,0,1,0)
NOALIAS __forceinline const Vector4 Const< Vector4 >::UnitW()       { Vector4 result; result.m_V = Const<Vec4F32>::UnitW(); return result; }      ///< Constant Vector4(0,0,0,1)

NOALIAS __forceinline const Vector4 Const< Vector4 >::Min() ///< Constant Vector4( FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN )
{
    //return MakeVector4( Const<Vec4F32>::Min() );
    Vector4 result; 
    result.m_V = Const<Vec4F32>::Min();
    return result;
}

NOALIAS __forceinline const Vector4 Const< Vector4 >::Max() ///< Constant Vector4( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX )
{
    //return MakeVector4( Const<Vec4F32>::Max() );
    Vector4 result; 
    result.m_V = Const<Vec4F32>::Max();
    return result;
}

NOALIAS __forceinline const Vector4 Const< Vector4 >::Epsilon() ///< Constant Vector4( FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON )
{
    //return MakeVector4( Const<Vec4F32>::Epsilon() );
    Vector4 result; 
    result.m_V = Const<Vec4F32>::Epsilon();
    return result;
}

NOALIAS __forceinline const Vector4 Const< Vector4 >::Epsilon2() ///< Constant Vector4( 0.01, 0.01, 0.01, 0.01 )
{
    //return MakeVector4( Const<Vec4F32>::Epsilon2() );
    Vector4 result; 
    result.m_V = Const<Vec4F32>::Epsilon2();
    return result;
}

NOALIAS __forceinline const Vector4 Const< Vector4 >::Epsilon3() ///< Constant Vector4( 1e-12, 1e-12, 1e-12, 1e-12 )
{
    //return MakeVector4( Const<Vec4F32>::Epsilon3() );
    Vector4 result; 
    result.m_V = Const<Vec4F32>::Epsilon3();
    return result;
}

NOALIAS __forceinline const Vector4 Const< Vector4 >::Pi() ///< Constant Vector4( PI, PI, PI, PI )
{
    //return MakeVector4( Const<Vec4F32>::Pi() );
    Vector4 result; 
    result.m_V = Const<Vec4F32>::Pi();
    return result;
}

NOALIAS __forceinline const Vector4 Const< Vector4 >::TwoPi() ///< Constant Vector4( PI*2, PI*2, PI*2, PI*2 )
{
    //return MakeVector4( Const<Vec4F32>::TwoPi() );
    Vector4 result; 
    result.m_V = Const<Vec4F32>::TwoPi();
    return result;
}

NOALIAS __forceinline const Vector4 Const< Vector4 >::RcpPi() ///< Constant Vector4( 1/PI, 1/PI, 1/PI, 1/PI )
{
    //return MakeVector4( Const<Vec4F32>::RcpPi() );
    Vector4 result; 
    result.m_V = Const<Vec4F32>::RcpPi();
    return result;
}

NOALIAS __forceinline const Vector4 Const< Vector4 >::Rcp2Pi() ///< Constant Vector4( 1/(PI*2), 1/(PI*2), 1/(PI*2), 1/(PI*2) )
{
    //return MakeVector4( Const<Vec4F32>::Rcp2Pi() );
    Vector4 result; 
    result.m_V = Const<Vec4F32>::Rcp2Pi();
    return result;
}

NOALIAS __forceinline const Vector4 Const< Vector4 >::PiDiv2() ///< Constant Vector4( PI/2, PI/2, PI/2, PI/2 )
{
    //return MakeVector4( Const<Vec4F32>::PiDiv2() );
    Vector4 result; 
    result.m_V = Const<Vec4F32>::PiDiv2();
    return result;
}

NOALIAS __forceinline const Vector4 Const< Vector4 >::Sqrt2() ///< Constant Vector4( sqrt(2), sqrt(2), sqrt(2), sqrt(2) )
{
    //return MakeVector4( Const<Vec4F32>::Sqrt2() );
    Vector4 result; 
    result.m_V = Const<Vec4F32>::Sqrt2();
    return result;
}

NOALIAS __forceinline const Vector4 Const< Vector4 >::Sqrt2div2() ///< Constant Vector4( sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, sqrt(2)/2 )
{
    //return MakeVector4( Const<Vec4F32>::Sqrt2div2() );
    Vector4 result; 
    result.m_V = Const<Vec4F32>::Sqrt2div2();
    return result;
}

NOALIAS __forceinline const Vector4 Const< Vector4 >::e() ///< Constant Vector4( e, e, e, e ) : base for natural logarithm e ~= 2.718282
{
    //return MakeVector4( Const<Vec4F32>::e() );
    Vector4 result; 
    result.m_V = Const<Vec4F32>::e();
    return result;
}


/// Constant Vector5(I,I,I,I,...)
template < int I >
NOALIAS __forceinline const Vector5 Const< Vector5 >::Int()
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Int<I>();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = (F32)I;
    #endif
    return result;
}

/// Constant Vector5( Num/Denom, Num/Denom, Num/Denom, Num/Denom, ... )
template < int Num, int Denom >
NOALIAS __forceinline const Vector5 Const< Vector5 >::Frac()
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Frac<Num,Denom>();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = (F32)Num / (F32)Denom;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::Zero() ///< Constant Vector5(0,0,0,0,0)
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Zero();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::One() ///< Constant Vector5(1,1,1,1,1)
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::One();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = 1.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::Two() ///< Constant Vector5(2,2,2,2,2)
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Two();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = 2.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::Three() ///< Constant Vector5(3,3,3,3,3)
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Three();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = 3.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::Four() ///< Constant Vector5(4,4,4,4,4)
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Four();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = 4.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::MinusOne() ///< Constant Vector5(-1,-1,-1,-1,-1)
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::MinusOne();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = -1.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::MinusTwo() ///< Constant Vector5(-2,-2,-2,-2,-2)
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::MinusTwo();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = -2.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::MinusThree() ///< Constant Vector5(-3,-3,-3,-3,-3)
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::MinusThree();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = -3.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::MinusFour() ///< Constant Vector5(-4,-4,-4,-4,-4)
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::MinusFour();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = -4.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::UnitElement0() ///< Constant Vector5(1,0,0,0,0)
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::UnitElement0();
    #else
    result.m_V[0] = 1.0f;
    result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::UnitElement1() ///< Constant Vector5(0,1,0,0,0)
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::UnitElement1();
    #else
    result.m_V[1] = 1.0f;
    result.m_V[0] = result.m_V[2] = result.m_V[3] = result.m_V[4] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::UnitElement2() ///< Constant Vector5(0,0,1,0,0)
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::UnitElement2();
    #else
    result.m_V[2] = 1.0f;
    result.m_V[0] = result.m_V[1] = result.m_V[3] = result.m_V[4] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::UnitElement3() ///< Constant Vector5(0,0,0,1,0)
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::UnitElement3();
    #else
    result.m_V[3] = 1.0f;
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[4] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::UnitElement4() ///< Constant Vector5(0,0,0,0,1)
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::UnitElement4();
    #else
    result.m_V[4] = 1.0f;
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = 0.0f;
    #endif
    return result;
}


NOALIAS __forceinline const Vector5 Const< Vector5 >::Min() ///< Constant Vector5( FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN, ... )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Min();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = Const< VF32 >::m_Min;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::Max() ///< Constant Vector5( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, ... )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Max();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = Const< VF32 >::m_Max;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::Epsilon() ///< Constant Vector5( FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, ... )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Epsilon();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = Const< VF32 >::m_Epsilon;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::Epsilon2() ///< Constant Vector5( 0.01, 0.01, 0.01, 0.01, ... )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Epsilon2();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = Const< VF32 >::m_Epsilon2;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::Epsilon3() ///< Constant Vector5( 1e-12, 1e-12, 1e-12, 1e-12, ... )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Epsilon3();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = Const< VF32 >::m_Epsilon3;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::Pi() ///< Constant Vector5( PI, PI, PI, PI, ... )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Pi();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = Const< VF32 >::m_Pi;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::TwoPi() ///< Constant Vector5( PI*2, PI*2, PI*2, PI*2, ... )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::TwoPi();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = Const< VF32 >::m_TwoPi;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::RcpPi() ///< Constant Vector5( 1/PI, 1/PI, 1/PI, 1/PI, ... )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::RcpPi();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = Const< VF32 >::m_RcpPi;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::Rcp2Pi() ///< Constant Vector5( 1/(PI*2), 1/(PI*2), 1/(PI*2), 1/(PI*2), ... )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Rcp2Pi();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = Const< VF32 >::m_Rcp2Pi;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::PiDiv2() ///< Constant Vector5( PI/2, PI/2, PI/2, PI/2, ... )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::PiDiv2();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = Const< VF32 >::m_PiDiv2;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::Sqrt2() ///< Constant Vector5( sqrt(2), sqrt(2), sqrt(2), sqrt(2), ... )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Sqrt2();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = Const< VF32 >::m_Sqrt2;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::Sqrt2div2() ///< Constant Vector5( sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, ... )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Sqrt2div2();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = Const< VF32 >::m_Sqrt2div2;
    #endif
    return result;
}

NOALIAS __forceinline const Vector5 Const< Vector5 >::e() ///< Constant Vector5( e, e, e, e, ... ) : base for natural logarithm e ~= 2.718282
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::e();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = Const< VF32 >::m_e;
    #endif
    return result;
}


/// Constant Vector6(I,I,I,I,...)
template < int I >
NOALIAS __forceinline const Vector6 Const< Vector6 >::Int()
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Int<I>();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = (F32)I;
    #endif
    return result;
}

/// Constant Vector6( Num/Denom, Num/Denom, Num/Denom, Num/Denom, ... )
template < int Num, int Denom >
NOALIAS __forceinline const Vector6 Const< Vector6 >::Frac()
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Frac<Num,Denom>();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = (F32)Num / (F32)Denom;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::Zero() ///< Constant Vector6(0,0,0,0,...)
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Zero();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::One() ///< Constant Vector6(1,1,1,1,...)
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::One();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = 1.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::Two() ///< Constant Vector6(2,2,2,2,...)
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Two();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = 2.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::Three() ///< Constant Vector6(3,3,3,3,...)
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Three();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = 3.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::Four() ///< Constant Vector6(4,4,4,4,...)
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Four();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = 4.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::MinusOne() ///< Constant Vector6(-1,-1,-1,-1,...)
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::MinusOne();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = -1.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::MinusTwo() ///< Constant Vector6(-2,-2,-2,-2,...)
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::MinusTwo();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = -2.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::MinusThree() ///< Constant Vector6(-3,-3,-3,-3,...)
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::MinusThree();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = -3.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::MinusFour() ///< Constant Vector6(-4,-4,-4,-4,...)
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::MinusFour();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = -4.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::UnitElement0() ///< Constant Vector6(1,0,0,0,0,0)
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::UnitElement0();
    #else
    result.m_V[0] = 1.0f;
    result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::UnitElement1() ///< Constant Vector6(0,1,0,0,0,0)
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::UnitElement1();
    #else
    result.m_V[1] = 1.0f;
    result.m_V[0] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::UnitElement2() ///< Constant Vector6(0,0,1,0,0,0)
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::UnitElement2();
    #else
    result.m_V[2] = 1.0f;
    result.m_V[0] = result.m_V[1] = result.m_V[3] = result.m_V[4] = result.m_V[5] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::UnitElement3() ///< Constant Vector6(0,0,0,1,0,0)
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::UnitElement3();
    #else
    result.m_V[3] = 1.0f;
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[4] = result.m_V[5] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::UnitElement4() ///< Constant Vector6(0,0,0,0,1,0)
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::UnitElement4();
    #else
    result.m_V[4] = 1.0f;
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[5] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::UnitElement5() ///< Constant Vector6(0,0,0,0,0,1)
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::UnitElement5();
    #else
    result.m_V[5] = 1.0f;
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::Min() ///< Constant Vector6( FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN, ... )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Min();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = Const< VF32 >::m_Min;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::Max() ///< Constant Vector6( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, ... )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Max();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = Const< VF32 >::m_Max;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::Epsilon() ///< Constant Vector6( FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, ... )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Epsilon();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = Const< VF32 >::m_Epsilon;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::Epsilon2() ///< Constant Vector6( 0.01, 0.01, 0.01, 0.01, ... )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Epsilon2();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = Const< VF32 >::m_Epsilon2;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::Epsilon3() ///< Constant Vector6( 1e-12, 1e-12, 1e-12, 1e-12, ... )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Epsilon3();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = Const< VF32 >::m_Epsilon3;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::Pi() ///< Constant Vector6( PI, PI, PI, PI, ... )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Pi();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = Const< VF32 >::m_Pi;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::TwoPi() ///< Constant Vector6( PI*2, PI*2, PI*2, PI*2, ... )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::TwoPi();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = Const< VF32 >::m_TwoPi;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::RcpPi() ///< Constant Vector6( 1/PI, 1/PI, 1/PI, 1/PI, ... )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::RcpPi();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = Const< VF32 >::m_RcpPi;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::Rcp2Pi() ///< Constant Vector6( 1/(PI*2), 1/(PI*2), 1/(PI*2), 1/(PI*2), ... )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Rcp2Pi();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = Const< VF32 >::m_Rcp2Pi;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::PiDiv2() ///< Constant Vector6( PI/2, PI/2, PI/2, PI/2, ... )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::PiDiv2();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = Const< VF32 >::m_PiDiv2;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::Sqrt2() ///< Constant Vector6( sqrt(2), sqrt(2), sqrt(2), sqrt(2), ... )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Sqrt2();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = Const< VF32 >::m_Sqrt2;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::Sqrt2div2() ///< Constant Vector6( sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, ... )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Sqrt2div2();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = Const< VF32 >::m_Sqrt2div2;
    #endif
    return result;
}

NOALIAS __forceinline const Vector6 Const< Vector6 >::e() ///< Constant Vector6( e, e, e, e, ... ) : base for natural logarithm e ~= 2.718282
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::e();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = Const< VF32 >::m_e;
    #endif
    return result;
}


/// Constant Vector7(I,I,I,I,...)
template < int I >
NOALIAS __forceinline const Vector7 Const< Vector7 >::Int()
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Int<I>();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = (F32)I;
    #endif
    return result;
}

/// Constant Vector7( Num/Denom, Num/Denom, Num/Denom, Num/Denom, ... )
template < int Num, int Denom >
NOALIAS __forceinline const Vector7 Const< Vector7 >::Frac()
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Frac<Num,Denom>();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = (F32)Num / (F32)Denom;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::Zero() ///< Constant Vector7(0,0,0,0,...)
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Zero();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::One() ///< Constant Vector7(1,1,1,1,...)
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::One();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = 1.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::Two() ///< Constant Vector7(2,2,2,2,...)
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Two();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = 2.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::Three() ///< Constant Vector7(3,3,3,3,...)
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Three();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = 3.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::Four() ///< Constant Vector7(4,4,4,4,...)
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Four();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = 4.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::MinusOne() ///< Constant Vector7(-1,-1,-1,-1,...)
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::MinusOne();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = -1.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::MinusTwo() ///< Constant Vector7(-2,-2,-2,-2,...)
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::MinusTwo();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = -2.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::MinusThree() ///< Constant Vector7(-3,-3,-3,-3,...)
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::MinusThree();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = -3.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::MinusFour() ///< Constant Vector7(-4,-4,-4,-4,...)
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::MinusFour();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = -4.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::UnitElement0() ///< Constant Vector7(1,0,0,0,0,0,0)
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::UnitElement0();
    #else
    result.m_V[0] = 1.0f;
    result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::UnitElement1() ///< Constant Vector7(0,1,0,0,0,0,0)
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::UnitElement1();
    #else
    result.m_V[1] = 1.0f;
    result.m_V[0] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::UnitElement2() ///< Constant Vector7(0,0,1,0,0,0,0)
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::UnitElement2();
    #else
    result.m_V[2] = 1.0f;
    result.m_V[0] = result.m_V[1] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::UnitElement3() ///< Constant Vector7(0,0,0,1,0,0,0)
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::UnitElement3();
    #else
    result.m_V[3] = 1.0f;
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[4] = result.m_V[5] = result.m_V[6] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::UnitElement4() ///< Constant Vector7(0,0,0,0,1,0,0)
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::UnitElement4();
    #else
    result.m_V[4] = 1.0f;
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[5] = result.m_V[6] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::UnitElement5() ///< Constant Vector7(0,0,0,0,0,1,0)
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::UnitElement5();
    #else
    result.m_V[5] = 1.0f;
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[6] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::UnitElement6() ///< Constant Vector7(0,0,0,0,0,0,1)
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::UnitElement6();
    #else
    result.m_V[6] = 1.0f;
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = 0.0f;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::Min() ///< Constant Vector7( FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN, ... )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Min();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = Const< VF32 >::m_Min;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::Max() ///< Constant Vector7( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, ... )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Max();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = Const< VF32 >::m_Max;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::Epsilon() ///< Constant Vector7( FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, ... )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Epsilon();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = Const< VF32 >::m_Epsilon;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::Epsilon2() ///< Constant Vector7( 0.01, 0.01, 0.01, 0.01, ... )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Epsilon2();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = Const< VF32 >::m_Epsilon2;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::Epsilon3() ///< Constant Vector7( 1e-12, 1e-12, 1e-12, 1e-12, ... )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Epsilon3();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = Const< VF32 >::m_Epsilon3;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::Pi() ///< Constant Vector7( PI, PI, PI, PI, ... )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Pi();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = Const< VF32 >::m_Pi;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::TwoPi() ///< Constant Vector7( PI*2, PI*2, PI*2, PI*2, ... )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::TwoPi();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = Const< VF32 >::m_TwoPi;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::RcpPi() ///< Constant Vector7( 1/PI, 1/PI, 1/PI, 1/PI, ... )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::RcpPi();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = Const< VF32 >::m_RcpPi;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::Rcp2Pi() ///< Constant Vector7( 1/(PI*2), 1/(PI*2), 1/(PI*2), 1/(PI*2), ... )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Rcp2Pi();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = Const< VF32 >::m_Rcp2Pi;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::PiDiv2() ///< Constant Vector7( PI/2, PI/2, PI/2, PI/2, ... )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::PiDiv2();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = Const< VF32 >::m_PiDiv2;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::Sqrt2() ///< Constant Vector7( sqrt(2), sqrt(2), sqrt(2), sqrt(2), ... )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Sqrt2();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = Const< VF32 >::m_Sqrt2;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::Sqrt2div2() ///< Constant Vector7( sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, ... )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::Sqrt2div2();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = Const< VF32 >::m_Sqrt2div2;
    #endif
    return result;
}

NOALIAS __forceinline const Vector7 Const< Vector7 >::e() ///< Constant Vector7( e, e, e, e, ... ) : base for natural logarithm e ~= 2.718282
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Const<Vec8F32>::e();
    #else
    result.m_V[0] = result.m_V[1] = result.m_V[2] = result.m_V[3] = result.m_V[4] = result.m_V[5] = result.m_V[6] = Const< VF32 >::m_e;
    #endif
    return result;
}


/// Vector8 constant integer template
template < int I > NOALIAS __forceinline const Vector8 Const< Vector8 >::Int()
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::Int<I>();
    return result;
}

/// Vector8 constant integer fraction template
template < int Num, int Denom > NOALIAS __forceinline const Vector8 Const< Vector8 >::Frac()
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::Frac<Num,Denom>();
    return result;
}

/// Constant Vector8(0,0,0,0,...)
NOALIAS __forceinline const Vector8 Const< Vector8 >::Zero()
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::Zero();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::One() ///< Constant Vector8(1,1,1,1,...)
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::One();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::Two() ///< Constant Vector8(2,2,2,2,...)
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::Two();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::Three() ///< Constant Vector8(3,3,3,3,...)
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::Three();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::Four() ///< Constant Vector8(4,4,4,4,...)
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::Four();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::MinusOne() ///< Constant Vector8(-1,-1,-1,-1,...)
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::MinusOne();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::MinusTwo() ///< Constant Vector8(-2,-2,-2,-2,...)
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::MinusTwo();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::MinusThree() ///< Constant Vector8(-3,-3,-3,-3,...)
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::MinusThree();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::MinusFour() ///< Constant Vector8(-4,-4,-4,-4,...)
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::MinusFour();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::UnitElement0() ///< Constant Vector8(1,0,0,0,0,0,0,0)
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::UnitElement0();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::UnitElement1() ///< Constant Vector8(0,1,0,0,0,0,0,0)
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::UnitElement1();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::UnitElement2() ///< Constant Vector8(0,0,1,0,0,0,0,0)
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::UnitElement2();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::UnitElement3() ///< Constant Vector8(0,0,0,1,0,0,0,0)
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::UnitElement3();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::UnitElement4() ///< Constant Vector8(0,0,0,0,1,0,0,0)
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::UnitElement4();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::UnitElement5() ///< Constant Vector8(0,0,0,0,0,1,0,0)
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::UnitElement5();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::UnitElement6() ///< Constant Vector8(0,0,0,0,0,0,1,0)
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::UnitElement6();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::UnitElement7() ///< Constant Vector8(0,0,0,0,0,0,0,1)
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::UnitElement6();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::Min() ///< Constant Vector8( FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN, ... )
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::Min();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::Max() ///< Constant Vector8( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, ... )
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::Max();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::Epsilon() ///< Constant Vector8( FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, ... )
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::Epsilon();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::Epsilon2() ///< Constant Vector8( 0.01, 0.01, 0.01, 0.01, ... )
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::Epsilon2();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::Epsilon3() ///< Constant Vector8( 1e-12, 1e-12, 1e-12, 1e-12, ... )
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::Epsilon3();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::Pi() ///< Constant Vector8( PI, PI, PI, PI, ... )
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::Pi();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::TwoPi() ///< Constant Vector8( PI*2, PI*2, PI*2, PI*2, ... )
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::TwoPi();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::RcpPi() ///< Constant Vector8( 1/PI, 1/PI, 1/PI, 1/PI, ... )
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::RcpPi();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::Rcp2Pi() ///< Constant Vector8( 1/(PI*2), 1/(PI*2), 1/(PI*2), 1/(PI*2), ... )
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::Rcp2Pi();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::PiDiv2() ///< Constant Vector8( PI/2, PI/2, PI/2, PI/2, ... )
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::PiDiv2();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::Sqrt2() ///< Constant Vector8( sqrt(2), sqrt(2), sqrt(2), sqrt(2), ... )
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::Sqrt2();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::Sqrt2div2() ///< Constant Vector8( sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, ... )
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::Sqrt2div2();
    return result;
}

NOALIAS __forceinline const Vector8 Const< Vector8 >::e() ///< Constant Vector8( e, e, e, e, ... ) : base for natural logarithm e ~= 2.718282
{
    Vector8 result;
    result.m_V = Const<Vec8F32>::e();
    return result;
}


NOALIAS __forceinline const Point3 Const< Point3 >::Zero() ///< Constant Point3(0,0,0)
{
    //return MakePoint3( Const<Vec4F32>::Zero() );
    Point3 result;
    result.m_V = Const<Vec4F32>::Zero();
    return result;
}

NOALIAS __forceinline const Point3 Const< Point3 >::UnitX() ///< Constant Point3(1,0,0)
{
    //return MakePoint3( Const<Vec4F32>::UnitX() );
    Point3 result;
    result.m_V = Const<Vec4F32>::UnitX();
    return result;
}

NOALIAS __forceinline const Point3 Const< Point3 >::UnitY() ///< Constant Point3(0,1,0)
{
    //return MakePoint3( Const<Vec4F32>::UnitY() );
    Point3 result;
    result.m_V = Const<Vec4F32>::UnitY();
    return result;
}

NOALIAS __forceinline const Point3 Const< Point3 >::UnitZ() ///< Constant Point3(0,0,1)
{
    //return MakePoint3( Const<Vec4F32>::UnitZ() );
    Point3 result;
    result.m_V = Const<Vec4F32>::UnitZ();
    return result;
}


NOALIAS __forceinline const HPoint3 Const< HPoint3 >::Zero() ///< Constant HPoint3(0,0,0,1)
{
    //return MakeHPoint3( Const<Vector3>::Zero(), Const<VF32>::One() );
    HPoint3 result;
    result.m_V = Const<Vec4F32>::VecInt<0,0,0,1>();
    return result;
}

NOALIAS __forceinline const HPoint3 Const< HPoint3 >::UnitX() ///< Constant HPoint3(1,0,0,1)
{
    //return MakeHPoint3( Const<Vector3>::UnitX(), Const<VF32>::One() );
    HPoint3 result;
    result.m_V = Const<Vec4F32>::VecInt<1,0,0,1>();
    return result;
}

NOALIAS __forceinline const HPoint3 Const< HPoint3 >::UnitY() ///< Constant HPoint3(0,1,0,1)
{
    //return MakeHPoint3( Const<Vector3>::UnitY(), Const<VF32>::One() );
    HPoint3 result;
    result.m_V = Const<Vec4F32>::VecInt<0,1,0,1>();
    return result;
}

NOALIAS __forceinline const HPoint3 Const< HPoint3 >::UnitZ() ///< Constant HPoint3(0,0,1,1)
{
    //return MakeHPoint3( Const<Vector3>::UnitZ(), Const<VF32>::One() );
    HPoint3 result;
    result.m_V = Const<Vec4F32>::VecInt<0,0,1,1>();
    return result;
}


NOALIAS __forceinline const Normal3 Const< Normal3 >::Zero() ///< Constant Normal3(0,0,0)
{
    //return MakeNormal3( Const<Vec4F32>::Zero() );
    Normal3 result;
    result.m_V = Const<Vec4F32>::Zero();
    return result;
}

NOALIAS __forceinline const Normal3 Const< Normal3 >::AxisX() ///< Constant Normal3(1,0,0)
{
    //return MakeNormal3( Const<Vec4F32>::UnitX() );
    Normal3 result;
    result.m_V = Const<Vec4F32>::UnitX();
    return result;
}

NOALIAS __forceinline const Normal3 Const< Normal3 >::AxisY() ///< Constant Normal3(0,1,0)
{
    //return MakeNormal3( Const<Vec4F32>::UnitY() );
    Normal3 result;
    result.m_V = Const<Vec4F32>::UnitY();
    return result;
}

NOALIAS __forceinline const Normal3 Const< Normal3 >::AxisZ() ///< Constant Normal3(0,0,1)
{
    //return MakeNormal3( Const<Vec4F32>::UnitZ() );
    Normal3 result;
    result.m_V = Const<Vec4F32>::UnitZ();
    return result;
}

NOALIAS __forceinline const Normal3 Const< Normal3 >::NegativeAxisX() ///< Constant Normal3(-1,0,0)
{
    //return MakeNormal3( -Const<Vec4F32>::UnitX() );
    Normal3 result;
    result.m_V = Const<Vec4F32>::NegativeUnitX();
    return result;
}

NOALIAS __forceinline const Normal3 Const< Normal3 >::NegativeAxisY() ///< Constant Normal3(0,-1,0)
{
    //return MakeNormal3( -Const<Vec4F32>::UnitY() );
    Normal3 result;
    result.m_V = Const<Vec4F32>::NegativeUnitY();
    return result;
}

NOALIAS __forceinline const Normal3 Const< Normal3 >::NegativeAxisZ() ///< Constant Normal3(0,0,-1)
{
    //return MakeNormal3( -Const<Vec4F32>::UnitZ() );
    Normal3 result;
    result.m_V = Const<Vec4F32>::NegativeUnitZ();
    return result;
}


/// Quaternion constant zero
NOALIAS __forceinline const Quaternion Const< Quaternion >::Zero()
{
    Quaternion result;
    result.m_Q = Const< Vec4F32 >::Zero();
    return result;
}

/// Quaternion constant identity
NOALIAS __forceinline const Quaternion Const< Quaternion >::Identity()
{
    Quaternion result;
    result.m_Q = Const< Vec4F32 >::UnitW();
    return result;
}


/// QuatTrans constant Zero
NOALIAS __forceinline const QuatTrans Const< QuatTrans >::Zero()
{
    QuatTrans result;
    result.m_Q = Const<Quaternion>::Zero();
    result.m_T = Const<Vector3>::Zero();
    return result;
}

/// QuatTrans constant Identity
NOALIAS __forceinline const QuatTrans Const< QuatTrans >::Identity()
{
    QuatTrans result;
    result.m_Q = Const<Quaternion>::Identity();
    result.m_T = Const<Vector3>::Zero();
    return result;
}


/// DualQuaternion constant Zero
NOALIAS __forceinline const DualQuaternion Const< DualQuaternion >::Zero()
{
    DualQuaternion result;
    result.m_V = Const<Vec8F32>::Zero();
    return result;
}

/// DualQuaternion constant Identity
NOALIAS __forceinline const DualQuaternion Const< DualQuaternion >::Identity()
{
    DualQuaternion result;
    result.m_V.m_V0 = Const<Vec4F32>::UnitW();
    result.m_V.m_V1 = Const<Vec4F32>::Zero();
    return result;
}




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


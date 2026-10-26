#ifndef MATHS_HVEC_VECTORINT_H
#define MATHS_HVEC_VECTORINT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

template < U32 I > class Const_U32 { public: static const U32 m_Value; };
template < U32 I > const U32 Const_U32<I>::m_Value = I;

template < S32 I > class Const_S32 { public: static const S32 m_Value; };
template < S32 I > const S32 Const_S32<I>::m_Value = I;

class PASSINREG Scalar_U32 // U32 splatted into all 4 elements
{
public:
    
    #if defined( _PPU_ ) || defined( _SPU_ )
    vector unsigned int m_V;
    #elif defined( _VMX128_ )
    __vector4 m_V;
    #elif defined( _SSE_ )
    __m128i m_V;
    #else
    U32 m_V;
    #endif
    
} __attribute__(( aligned( 16 ) ));

class PASSINREG Scalar_S32 // S32 splatted into all 4 elements
{
public:
    
    #if defined( _PPU_ ) || defined( _SPU_ )
    vector signed int m_V;
    #elif defined( _VMX128_ )
    __vector4 m_V;
    #elif defined( _SSE_ )
    __m128i m_V;
    #else
    U32 m_V;
    #endif
    
} __attribute__(( aligned( 16 ) ));

class PASSINREG Vector3_U32 //Vector of 3 U32s
{
public:
#if defined( _PPU_ ) || defined( _SPU_ )
    vector signed int m_V;
    #elif defined( _VMX128_ )
    __vector4 m_V;
    #elif defined( _SSE_ )
    __m128i m_V;
    #else
    U32 m_V[3];
    #endif
    
} __attribute__(( aligned( 16 ) ));

//template <> NOALIAS __forceinline const Scalar_U32 Load( U32 const * const  __restrict ptr ) __attribute__(( always_inline ));
//template <> NOALIAS __forceinline const Scalar_S32 Load( S32 const * const  __restrict ptr ) __attribute__(( always_inline ));

//template <> NOALIAS __forceinline void Store( const Scalar_U32 PCREF scalar, U32* const __restrict ptr ) __attribute__(( always_inline ));
//template <> NOALIAS __forceinline void Store( const Scalar_S32 PCREF scalar, S32* const __restrict ptr ) __attribute__(( always_inline ));

NOALIAS __forceinline const Scalar_U32 Make_Scalar_U32( const U32& i ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar_S32 Make_Scalar_S32( const S32& i ) __attribute__(( always_inline ));

template < U32 I > NOALIAS __forceinline const Scalar_U32 Make_Scalar_U32() __attribute__(( always_inline ));
template < S32 I > NOALIAS __forceinline const Scalar_S32 Make_Scalar_S32() __attribute__(( always_inline ));

NOALIAS __forceinline const Scalar_U32 operator ++ ( Scalar_U32 & scalar ) __attribute__(( always_inline )); // ++Prefix
NOALIAS __forceinline const Scalar_S32 operator ++ ( Scalar_S32 & scalar ) __attribute__(( always_inline )); // ++Prefix

NOALIAS __forceinline const Scalar_U32 operator ++ ( Scalar_U32 & scalar, int ) __attribute__(( always_inline )); // Postfix++
NOALIAS __forceinline const Scalar_S32 operator ++ ( Scalar_S32 & scalar, int ) __attribute__(( always_inline )); // Postfix++

NOALIAS __forceinline const Scalar_U32 operator -- ( Scalar_U32 & scalar ) __attribute__(( always_inline )); // --Prefix
NOALIAS __forceinline const Scalar_S32 operator -- ( Scalar_S32 & scalar ) __attribute__(( always_inline )); // --Prefix

NOALIAS __forceinline const Scalar_U32 operator -- ( Scalar_U32 & scalar, int ) __attribute__(( always_inline )); // Postfix--
NOALIAS __forceinline const Scalar_S32 operator -- ( Scalar_S32 & scalar, int ) __attribute__(( always_inline )); // Postfix--

NOALIAS __forceinline const Scalar_U32 operator += ( Scalar_U32 & src0, const Scalar_U32 PCREF src1 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar_S32 operator += ( Scalar_S32 & src0, const Scalar_S32 PCREF src1 ) __attribute__(( always_inline ));
                                                                                                     
NOALIAS __forceinline const Scalar_U32 operator -= ( Scalar_U32 & src0, const Scalar_U32 PCREF src1 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar_S32 operator -= ( Scalar_S32 & src0, const Scalar_S32 PCREF src1 ) __attribute__(( always_inline ));

NOALIAS __forceinline const Scalar_U32 operator + ( const Scalar_U32 PCREF src0, const Scalar_U32 PCREF src1 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar_S32 operator + ( const Scalar_S32 PCREF src0, const Scalar_S32 PCREF src1 ) __attribute__(( always_inline ));

NOALIAS __forceinline const Scalar_U32 operator - ( const Scalar_U32 PCREF src0, const Scalar_U32 PCREF src1 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar_S32 operator - ( const Scalar_S32 PCREF src0, const Scalar_S32 PCREF src1 ) __attribute__(( always_inline ));

NOALIAS __forceinline const Scalar_U32 Select( const Scalar_Bool PCREF condition, const Scalar_U32 PCREF resultIfTrue, const Scalar_U32 PCREF resultIfFalse ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar_S32 Select( const Scalar_Bool PCREF condition, const Scalar_S32 PCREF resultIfTrue, const Scalar_S32 PCREF resultIfFalse ) __attribute__(( always_inline ));

//Vector3_U32 functions

NOALIAS __forceinline const Vector3_U32 operator ++ (Vector3_U32 &vector) __attribute__((always_inline)); //Prefix++
NOALIAS __forceinline const Vector3_U32 operator ++ (Vector3_U32 &vector, int) __attribute__((always_inline)); //Postfix++

NOALIAS __forceinline const Vector3_U32 operator -- (Vector3_U32 &vector) __attribute__((always_inline)); //Prefix--
NOALIAS __forceinline const Vector3_U32 operator -- (Vector3_U32 &vector, int) __attribute__((always_inline)); //Postfix--

NOALIAS __forceinline const Vector3_U32 operator += (Vector3_U32& vector, Vector3_U32 &vector2) __attribute__((always_inline));
NOALIAS __forceinline const Vector3_U32 operator -= (Vector3_U32& vector, Vector3_U32 &vector2) __attribute__((always_inline));

NOALIAS __forceinline const Vector3_U32 operator + (const Vector3_U32 PCREF vector, const Vector3_U32 PCREF vector2) __attribute__((always_inline));
NOALIAS __forceinline const Vector3_U32 operator - (const Vector3_U32 PCREF vector, const Vector3_U32 PCREF sub) __attribute__((always_inline));

NOALIAS __forceinline const Vector3_U32 Select( const Vector3_Bool PCREF condition, const Vector3_U32 PCREF ifTrue, const Vector3_U32 PCREF ifFalse) __attribute__((always_inline));

NOALIAS __forceinline const Scalar_U32 X(const Vector3_U32 PCREF vector) __attribute__((always_inline));
NOALIAS __forceinline const Scalar_U32 Y(const Vector3_U32 PCREF vector) __attribute__((always_inline));
NOALIAS __forceinline const Scalar_U32 Z(const Vector3_U32 PCREF vector) __attribute__((always_inline));

//template <> const Maths::Vector3_U32 Load( U32_3 const * const __restrict pSrc );
//template <> void Store( const Vector3_U32 PCREF src, U32_3 * const __restrict pDst );






}; // namespace Maths

#endif // #ifndef MATHS_HVEC_VECTORINT_H

